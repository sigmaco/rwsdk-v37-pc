/****************************************************************************
 *
 * This file is a product of Criterion Software Ltd.
 *
 * This file is provided as is with no warranties of any kind and is
 * provided without any obligation on Criterion Software Ltd.
 * or Canon Inc. to assist in its use or modification.
 *
 * Criterion Software Ltd. and Canon Inc. will not, under any
 * circumstances, be liable for any lost revenue or other damages
 * arising from the use of this file.
 *
 * Copyright (c) 2000 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *                                                                         
 * winvideo.c
 *
 * Copyright (C) 2000 Criterion Technologies.
 *
 * Original author: Martin Slater.
 *                                                                         
 * Purpose: Plays splash screen from an AVI file.
 *                         
 ****************************************************************************/
#include <windows.h>
#include <windowsx.h>
#include <time.h>
#include <vfw.h>

/*
      Math routines for 32 bit signed and unsiged numbers used by the
	  audio player. */
LONG MulDiv32( LONG a, LONG b, LONG c )
{
    return (LONG)( Int32x32To64(a,b) / c );
}


DWORD MulDivRD( DWORD a, DWORD b, DWORD c )
{
    return (DWORD)( UInt32x32To64(a,b) / c );
}

DWORD MulDivRN( DWORD a, DWORD b, DWORD c )
{
    return (DWORD)( (UInt32x32To64(a,b)+c/2) / c );
}

DWORD MulDivRU( DWORD a, DWORD b, DWORD c )
{
    return (DWORD)( (UInt32x32To64(a,b)+c-1) / c );
}

/* Some code references these by other names. */
#define muldiv32    MulDivRN
#define muldivrd32  MulDivRD
#define muldivru32  MulDivRU


/*	AUDIO PLAYING SUPPORT */
static	HWAVEOUT	shWaveOut = 0;	/* Current MCI device ID */
static	LONG		slBegin;
static	LONG		slCurrent;
static	LONG		slEnd;
static	BOOL		sfLooping;
static	BOOL		sfPlaying = FALSE;

#define MAX_AUDIO_BUFFERS	16
#define MIN_AUDIO_BUFFERS	2
#define AUDIO_BUFFER_SIZE	32768 

static	UINT		swBuffers;			 // total # buffers
static	UINT		swBuffersOut;	    // buffers device has
static	UINT		swNextBuffer;	    // next buffer to fill
static	LPWAVEHDR	salpAudioBuf[MAX_AUDIO_BUFFERS];

static	PAVISTREAM	spavi;				 // stream we're playing
static	LONG		slSampleSize;	    // size of an audio sample

static	LONG		sdwBytesPerSec;
static	LONG		sdwSamplesPerSec;

/* Function prototypes */
int PlaySplashScreen(HWND hwnd, HDC hdc, char *filename, RECT *r);
void aviaudioCloseDevice(void);
BOOL aviaudioOpenDevice(HWND hwnd, PAVISTREAM pavi);
BOOL aviaudioiFillBuffers(void);
BOOL aviaudioPlay(HWND hwnd, PAVISTREAM pavi, LONG lStart, LONG lEnd, BOOL fWait);
void aviaudioStop(void);
void aviaudioMessage(HWND hwnd, unsigned msg, WPARAM wParam, LPARAM lParam);

/*
	This function will play an AVI file containing one video and one
	audio stream. The video is rendered to the passed HDC and fill 
	the region specified by r.

	Returns 0 on failure, 1 on success.
*/
int
PlaySplashScreen(HWND hwnd, HDC hdc, char *filename, RECT *r)
{
    PAVISTREAM vid_stream = NULL, snd_stream = NULL;
    PGETFRAME get_frame = NULL;
    AVISTREAMINFO stream_info, snd_stream_info;
    unsigned int swidth = r->right - r->left;
    unsigned int sheight = r->bottom - r->top;

    /* Init avi library */
    AVIFileInit();

    if(0 != AVIStreamOpenFromFile(&vid_stream, filename, streamtypeVIDEO, 0, OF_READ, NULL)) 
    {
        OutputDebugString("Failed to open video steam\n");
        return 0;
    }

    /* Get the stream info */
    if(0 != AVIStreamInfo(vid_stream, &stream_info, sizeof(AVISTREAMINFO)))
        return 0;

    get_frame = AVIStreamGetFrameOpen(vid_stream, NULL);
    if(NULL == get_frame) 
    {
        OutputDebugString("Failed to open get frame\n");
        return 0;
    }

    /* Load sound stream if available */
    if(0 != AVIStreamOpenFromFile(&snd_stream, filename, streamtypeAUDIO, 0, OF_READ, NULL))
    {
        OutputDebugString("Failed to load sound stream\n");
        return 0;
    }

    /* Get the sound stream info */
    if(0 != AVIStreamInfo(snd_stream, &snd_stream_info, sizeof(AVISTREAMINFO)))
        return 0;


    /* Kick off playing sound channel if available */
    if(NULL != snd_stream)
    {
        aviaudioPlay(hwnd, snd_stream,  AVIStreamStart(snd_stream), AVIStreamEnd(snd_stream), FALSE);
    }


    /* Play movie */
    { 
        HDRAWDIB dibdc;
        float start_time;
        float time_scale = ((float)stream_info.dwRate) / stream_info.dwScale;

        /* Save the start time of the AVI file */
        start_time = ((float)clock()) / CLOCKS_PER_SEC;

        dibdc = DrawDibOpen();

        while(1) 
        {
            BITMAPINFO* pbmi;

            /* Use the clock to find which frame we should be drawing */
            float cur_time		= ((float)clock()) / CLOCKS_PER_SEC;
            float elapsed_time  = cur_time - start_time;
            DWORD cur_frame		= (DWORD)( elapsed_time * time_scale);

            /* If we exceed the AVI length, quit loop */
            if(cur_frame >= stream_info.dwLength)
                break;

            /* Get the current frame of the video */
            if(NULL == (pbmi = (BITMAPINFO*)AVIStreamGetFrame(get_frame, cur_frame)))
                return 0;

            /* Render frame stretching to fill entire screen */
            DrawDibDraw(dibdc, hdc,	0, 0, swidth, sheight, &pbmi->bmiHeader, NULL,
			0, 0, -1, -1, DDF_BACKGROUNDPAL);
        }

        DrawDibClose(dibdc);
    }

    /* Shut down audio player */
    aviaudioStop();
	 
    /* cleanup */
    if(NULL != snd_stream)
        AVIStreamRelease(snd_stream);

    if(NULL != vid_stream)
        AVIStreamRelease(vid_stream);

    return 1;
}

/*
	aviaudioCloseDevice -- close the open audio device, if any.    
*/
void aviaudioCloseDevice(void)
{
    UINT	w;

    if(shWaveOut) 
    {
        while(swBuffers > 0) 
        {
            --swBuffers;
            waveOutUnprepareHeader(shWaveOut, salpAudioBuf[swBuffers], sizeof(WAVEHDR));
            GlobalFreePtr((LPBYTE) salpAudioBuf[swBuffers]);
        }
		
        w = waveOutClose(shWaveOut);

        shWaveOut = NULL;	
    }
}

/*
	aviaudioOpenDevice -- get ready to play waveform data.
*/
BOOL aviaudioOpenDevice(HWND hwnd, PAVISTREAM pavi)
{
    UINT		w;
    LPVOID		lpFormat;
    LONG		cbFormat;
    AVISTREAMINFO	strhdr;

    if(!pavi)		// no wave data to play
        return FALSE;

    if(shWaveOut)	// already something playing
        return TRUE;

    spavi = pavi;

    AVIStreamInfo(pavi, &strhdr, sizeof(strhdr));

    slSampleSize = (LONG) strhdr.dwSampleSize;
    if(slSampleSize <= 0 || slSampleSize > AUDIO_BUFFER_SIZE)
        return FALSE;

    AVIStreamFormatSize(pavi, 0, &cbFormat);

    lpFormat = GlobalAllocPtr(GHND, cbFormat);
    if(!lpFormat)
        return FALSE;

    AVIStreamReadFormat(pavi, 0, lpFormat, &cbFormat);

    sdwSamplesPerSec = ((LPWAVEFORMAT) lpFormat)->nSamplesPerSec;
    sdwBytesPerSec = ((LPWAVEFORMAT) lpFormat)->nAvgBytesPerSec;

    w = waveOutOpen(&shWaveOut, WAVE_MAPPER, lpFormat, (DWORD) (UINT) hwnd, 0L, CALLBACK_WINDOW);

    
    /* Maybe we failed because someone is playing sound already.
       Shut any sound off, and try once more before giving up. */
    if(w) 
    {
        sndPlaySound(NULL, 0);
        w = waveOutOpen(&shWaveOut, WAVE_MAPPER, lpFormat,(DWORD) (UINT) hwnd, 0L, CALLBACK_WINDOW);
    }

    if(w != 0) 
        return FALSE;
    

    for (swBuffers = 0; swBuffers < MAX_AUDIO_BUFFERS; swBuffers++) {
        if(!(salpAudioBuf[swBuffers] = (LPWAVEHDR)GlobalAllocPtr(GMEM_MOVEABLE | GMEM_SHARE,(DWORD)(sizeof(WAVEHDR) + AUDIO_BUFFER_SIZE))))
            break;
		
        salpAudioBuf[swBuffers]->dwFlags = WHDR_DONE;
        salpAudioBuf[swBuffers]->lpData = (LPSTR)((LPBYTE) salpAudioBuf[swBuffers] + sizeof(WAVEHDR));
        salpAudioBuf[swBuffers]->dwBufferLength = AUDIO_BUFFER_SIZE;
		
        if(!waveOutPrepareHeader(shWaveOut, salpAudioBuf[swBuffers], sizeof(WAVEHDR)))
            continue;
		
        GlobalFreePtr((LPBYTE) salpAudioBuf[swBuffers]);
        break;
    }

    if(swBuffers < MIN_AUDIO_BUFFERS) 
    {
        aviaudioCloseDevice();
        return FALSE;
    }

    swBuffersOut = 0;
    swNextBuffer = 0;

    sfPlaying = FALSE;

    return TRUE;
}


/*
	Return the time in milliseconds corresponding to the currently playing
	audio sample, or -1 if no audio is playing.
	WARNING: Some sound cards are pretty inaccurate!
*/
LONG aviaudioTime(void)
{
    MMTIME	mmtime;

    if(!sfPlaying)
        return -1;

    mmtime.wType = TIME_SAMPLES;

    waveOutGetPosition(shWaveOut, &mmtime, sizeof(mmtime));

    if(mmtime.wType == TIME_SAMPLES)
        return AVIStreamSampleToTime(spavi, slBegin) + muldiv32(mmtime.u.sample, 1000, sdwSamplesPerSec);
    else if (mmtime.wType == TIME_BYTES)
        return AVIStreamSampleToTime(spavi, slBegin) + muldiv32(mmtime.u.cb, 1000, sdwBytesPerSec);
    else
        return -1;
}


/*
	 Fill up any empty audio buffers and ship them out to the device.
*/
BOOL aviaudioiFillBuffers(void)
{
    LONG		lRead;
    UINT		w;
    LONG		lSamplesToPlay;

    /* We're not playing, so do nothing. */
    if (!sfPlaying)
        return TRUE;

    while (swBuffersOut < swBuffers) {
        if(slCurrent >= slEnd) 
        {
            if (sfLooping) 
            {
				/* Looping, so go to the beginning. */
                slCurrent = slBegin;
            } else
                break;
        }

        /* Figure out how much data should go in this buffer */
        lSamplesToPlay = slEnd - slCurrent;
        if(lSamplesToPlay > AUDIO_BUFFER_SIZE / slSampleSize)
            lSamplesToPlay = AUDIO_BUFFER_SIZE / slSampleSize;


        AVIStreamRead(spavi, slCurrent, lSamplesToPlay, salpAudioBuf[swNextBuffer]->lpData,
                      AUDIO_BUFFER_SIZE,  &(LONG)salpAudioBuf[swNextBuffer]->dwBufferLength, &lRead);
		
        if(lRead != lSamplesToPlay) 
        {
            return FALSE;
        }
        slCurrent += lRead;
		
        w = waveOutWrite(shWaveOut, salpAudioBuf[swNextBuffer],sizeof(WAVEHDR));
		
        if(w != 0) 
        {
            return FALSE;
        }
		
        ++swBuffersOut;
        ++swNextBuffer;
        if(swNextBuffer >= swBuffers)
            swNextBuffer = 0;
    }

    if(swBuffersOut == 0 && slCurrent >= slEnd)
        aviaudioStop();

    /* We've filled all of the buffers we can or want to. */
    return TRUE;
}

/*
	aviaudioPlay -- Play audio, starting at a given frame		
								|
*/
BOOL aviaudioPlay(HWND hwnd, PAVISTREAM pavi, LONG lStart, LONG lEnd, BOOL fWait)
{
    if (lStart < 0)
        lStart = AVIStreamStart(pavi);

    if (lEnd < 0)
        lEnd = AVIStreamEnd(pavi);

    if(lStart >= lEnd)
        return FALSE;

    if(!aviaudioOpenDevice(hwnd, pavi))
        return FALSE;

    if(!sfPlaying) 
    {
        /*	We're beginning play, so pause until we've filled the buffers
                for a seamless start */
        waveOutPause(shWaveOut);

        slBegin = lStart;
        slCurrent = lStart;
        slEnd = lEnd;
        sfPlaying = TRUE;
    }
    else 
    {
        slEnd = lEnd;
    }

    aviaudioiFillBuffers();
    
    /* Now unpause the audio and away it goes! */
    waveOutRestart(shWaveOut);

    /* Caller wants us not to return until play is finished */
    if(fWait) 
    {
        while(swBuffersOut > 0)
            Yield();
    }

    return TRUE;
}

/*
	aviaudioMessage -- handle wave messages received by		
	window controlling audio playback.  When audio buffers are	
	done, this routine calls aviaudioiFillBuffers to fill them	
	up again.							|
*/
void aviaudioMessage(HWND hwnd, unsigned msg, WPARAM wParam, LPARAM lParam)
{
    if(msg == MM_WOM_DONE) 
    {
        --swBuffersOut;
        aviaudioiFillBuffers();
    }
}


/*
	aviaudioStop -- stop playing, close the device.		
*/
void aviaudioStop(void)
{
    UINT	w;

    if(shWaveOut != 0) 
    {
        w = waveOutReset(shWaveOut);

        sfPlaying = FALSE;
	
        aviaudioCloseDevice();
    }
}
