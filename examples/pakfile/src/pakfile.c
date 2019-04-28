
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
 ****************************************************************************/

/****************************************************************************
 *
 * pakfile.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: To illustrate the use of PAK files for the storing and 
 *          retrieval of RenderWare data.
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "rwcore.h"
#include "skeleton.h"
#include "pakfile.h"

#if (defined (_XBOX))
#define FILE_TYPE   RtXboxFile
#elif (defined (WIN32) && !defined(XBOX))
#define FILE_TYPE   RtWinFile
#elif (defined (SKY))
#define FILE_TYPE   RtSkyHostFile
#elif (defined(DOLPHIN))
#define FILE_TYPE   RtGcnDVDFile
#endif

/* 
 * A PAK file table of contents entry...
 */
typedef struct PakTOCEntry PakTOCEntry;
struct PakTOCEntry
{
    RwChar name[56];
    RwInt32 offset;
    RwInt32 fileSize;
};

/* 
 * An open PAK file...
 * This is opened by the default file system
 */
typedef struct PakFile PakFile;
struct PakFile
{
    /* 
     * General...
     */
    RwChar dirName[260];
    RtFile *file;
    RtInt64 pakFileSize;
    RtInt64 pakFilePosition;
    RwInt32 numOpenFiles;
    
    /* 
     * Table of contents...
     */
    PakTOCEntry *tocEntry;
    RwInt32 tocSize;
};


/* 
 * An open file within a PAK...
 * A PakFile is an RtFile opened by the Pak File System
 */
typedef struct PakOpenFile PakOpenFile;
struct PakOpenFile
{
    FILE_TYPE file;
    
    RwUInt32 id;
    PakFile  *pakFile;
    PakTOCEntry *tocEntry;
    RwInt32 bytesRead;
    RwBool eof;
};

typedef struct PakFileSystem PakFileSystem;
struct PakFileSystem
{
    RtFileSystem  dfs;
    
    PakOpenFile  *files;
};

#define PAKHEADERID                             \
(                                               \
        ('P'<< 0) |                             \
	('A'<< 8) |                             \
	('C'<<16) |                             \
	('K'<<24)                               \
)
    
static PakFile *PAKFile = (PakFile *)NULL;

static RtInt64
_PakFSeek( RtFile           *file,
           RwInt32           offset,
           RtFileSeekMethod  fPosition);
           
RtFile *
PakFSGetFileObject( RtFileSystem *fs, RwUInt32 index );
  
/*
 *****************************************************************************
 */
static void
SetInt64Value(RtInt64 *value, RwInt32 set)
{    
#if (defined(WIN32))
#if (defined(_MSC_VER))
    value->noSupportValue.low  = set;
#else
    value->noSupportValue.high = set;
#endif 
#else
    value->supportValue = (RwInt64)set;
#endif
}

/*
 *****************************************************************************
 */
static RwInt32
GetInt64Value(RtInt64 value)
{    
#if (defined(WIN32))
#if (defined(_MSC_VER))
    return value.noSupportValue.low;
#else
    return value.noSupportValue.high;
#endif 
#else
    return (RwInt32)value.supportValue;
#endif
}

/*
 *****************************************************************************
 */
static RwBool
_PakFileSkipSet(PakFile *pakFile, RwUInt32 offset)
{
    /*
     * Purpose:   Set absolute position in PAK file.
     * On entry:  PAK file; file offset from start.
     * On exit:   Success status.
     */
    RwUInt32 fSize;
    
    if( !pakFile || !pakFile->file )
    {
        return FALSE;
    }

    /* 
     * Ensure that our offset is within the file...
     */
    fSize = (RwUInt32)GetInt64Value(pakFile->pakFileSize);
    if( offset > fSize - 1 )
    {
        offset = fSize - 1;
    }

    /* 
     * Seek from start of file...
     */
    {
        RtInt64 seekVal;
        RwInt32 val;
       
        seekVal = _PakFSeek(pakFile->file, offset, RTFILE_POS_BEGIN);
        val = GetInt64Value(seekVal);
        
        if(val)
        {
            return FALSE;
        }
    }

    /* 
     * Store file position...
     */
    SetInt64Value(&pakFile->pakFilePosition, offset);

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwBool
_PakSortTOC(PakFile *pakFile)
{
   /*
    * Purpose:   Sort table of contents by name alphabetically.
    * On entry:  PAK file.
    * On exit:   Success status.
    */

    RwInt32 i, j;
    RwBool noSwaps;
    PakTOCEntry tempEntry;

    /* 
     * Sort TOC entries into alphabetical name order...
     */
    for(i = pakFile->tocSize - 1; i > 0; i--)
    {
        noSwaps = TRUE;

        for(j = 1; j <= i; j++)
        {
            /* 
             * Swap entries...
             */
            if( rwstrcmp(pakFile->tocEntry[j - 1].name, pakFile->tocEntry[j].name) > 0 )
            {
                tempEntry = pakFile->tocEntry[j - 1];
                pakFile->tocEntry[j - 1] = pakFile->tocEntry[j];
                pakFile->tocEntry[j] = tempEntry;

                noSwaps = FALSE;
            }
        }

        if( noSwaps )
        {
            return TRUE;
        }
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
static RwInt32
_PakFindFirstMatch(PakFile *pakFile, const RwChar *pathname)
{
    /*
     * Purpose:   Find first file in TOC that matches this path.
     * On entry:  PAK file; path name.
     * On exit:   TOC index, -1 if fails.
     */

    RwInt32 start, end, halfway, compare;

    RwChar *psTOCpath;


    if( !pakFile || !pakFile->file || !pakFile->tocEntry )
    {
        return -1;
    }

    /* 
     * Do binary chop to find first matching entry...
     */
    start = 0;
    end = pakFile->tocSize - 1;
    halfway = (start + end)>>1;

    psTOCpath = RsPathnameCreate( pakFile->tocEntry[halfway].name );

    compare = rwstrcmp(pathname, psTOCpath );

    RsPathnameDestroy( psTOCpath );

    while( (compare != 0) && (start != end) )
    {
        if( compare < 0 )
        {
            end = halfway;
        }
        else
        {
            if( start < (end - 1) )
            {
                start = halfway;
            }
            else
            {
                start = end;
            }
        }

        halfway = (start + end)>>1;

        psTOCpath = RsPathnameCreate( pakFile->tocEntry[halfway].name );

        compare = rwstrcmp(pathname, psTOCpath );

        RsPathnameDestroy( psTOCpath );
    }

    /* 
     * If we didnt find an exact match is our path a sub-dir of our match...?
     */
    if( compare != 0 )
    {
        psTOCpath = RsPathnameCreate( pakFile->tocEntry[halfway].name );

        if( strncmp(pathname, psTOCpath, rwstrlen(pathname)) )
        {
            RsPathnameDestroy( psTOCpath );

            return -1;
        }

        RsPathnameDestroy( psTOCpath );
    }

    return halfway;
}


/*
 *****************************************************************************
 */
static RwBool
_PakFExist(RtFileSystem *fs __RWUNUSED__, const RwChar *fileName)
{
    /*
     * Purpose:   File i/o override for PAK files: FEXIST.
     * On entry:  File name.
     * On exit:   Existence.
     */

    RwInt32 nameLen, dirLen, i;

    RwChar *pathname;

    RtFileSystem *defaultFS = RtFSManagerGetDefaultFileSystem();
    
    if(defaultFS)
    {
        RtFileSystemFileFunctionTable defaultFFT = defaultFS->fsFileFunc;

        /* 
         * Check if open PAK file might contain requested file...
         */
        nameLen = rwstrlen(fileName);
        dirLen  = rwstrlen(PAKFile->dirName);
    
        if( (nameLen < dirLen) || strncmp(fileName, PAKFile->dirName, dirLen) )
        {
            /* 
             * Doesn't match open PAK file, 
             * Pass down to the default file system...
             */
            return defaultFFT.fExists(defaultFS, fileName);
        }

        /* 
         * Look for the file in the PAK file...
         */
        pathname = RsPathnameCreate(fileName + dirLen);

        i = _PakFindFirstMatch(PAKFile, pathname);

        RsPathnameDestroy(pathname);

        if( i >= 0 )
        {
            return (TRUE);
        }

        /* 
         * File wasn't in PAK file so pass down to the default file system 
         * rather than fail...
         */
        return defaultFFT.fExists(defaultFS, fileName);
    }
    else
    {
        return (FALSE);
    }

}


/*
 *****************************************************************************
 */
static RtFileSystemError
_PakFOpen( RtFileSystem  *fs __RWUNUSED__,
           RtFile        *file,
           const RwChar  *filename,
           RwUInt32       flags)
{
    /*
     * Purpose:   File i/o override for PAK files: FOPEN.
     * On entry:  File name; access.
     * On exit:   File pointer.
     */

    RwInt32 nameLen, dirLen, i;

    RwChar *pathname;

    RtFileSystem *defaultFS = RtFSManagerGetDefaultFileSystem();
    
    if (defaultFS)
    {
        /* 
         * Get the default file system function table 
         */
        RtFileSystemFileFunctionTable defaultFFT = defaultFS->fsFileFunc;
        
        /* 
         * Check if open PAK file might contain requested file...
         */
        nameLen = rwstrlen(filename);
        dirLen  = rwstrlen(PAKFile->dirName);
    
        if( (nameLen < dirLen) || strncmp(filename, PAKFile->dirName, dirLen) )
        {
            /* 
             * Doesn't match open PAK file, 
             * Pass down to the default file system...
             */
            return defaultFFT.open(defaultFS, file, filename, flags);
        }

        /* 
         * Look for the file in the PAK file...
         */
        pathname = RsPathnameCreate(filename + dirLen);

        i = _PakFindFirstMatch(PAKFile, pathname);

        RsPathnameDestroy(pathname);

        if( i >= 0 )
        {
            PakOpenFile *openFile = (PakOpenFile *)file;

            if( openFile )
            {
                openFile->id = PAKHEADERID;
                openFile->pakFile = PAKFile;
                openFile->tocEntry = &PAKFile->tocEntry[i];
                openFile->bytesRead = 0;
                openFile->eof = FALSE;
            
                /* 
                 * Increment number of files open in this PAK file...
                 */
                PAKFile->numOpenFiles++;

                return RTFS_ERROR_NOERROR;
            }
        }

        /* 
         * File wasn't in PAK file so pass down to the default file system 
         * rather than fail...
         */
        return defaultFFT.open(defaultFS, file, filename, flags);
        
    }

    return (RTFILE_ERROR_FILESYSTEM);
    
}

/*
 *****************************************************************************
 */
static void
_PakFClose(RtFile *file)

{
    /*
     * Purpose:   File i/o override for PAK files: FCLOSE.
     * On entry:  File pointer.
     * On exit:   0 if successful, -1 otherwise.
     */

    PakOpenFile *openFile = (PakOpenFile *)file;

    RtFileSystem *defaultFS = RtFSManagerGetDefaultFileSystem();
    
    if(defaultFS)
    {
        RtFileSystemFileFunctionTable defaultFFT = defaultFS->fsFileFunc;

        /* 
         * If this aint our file, pass it thru...
         */
        if( openFile->id != PAKHEADERID )
        {
            defaultFFT.close(file);
        }

        /* 
         * Decrement number of files open in this PAK file...
         */
        openFile->pakFile->numOpenFiles--;
    }
}


/*
 *****************************************************************************
 */
static RwUInt32
_PakFRead( RtFile     *file,
           void       *pBuffer,
           RwUInt32    nBytes)
{
    /*
     * Purpose:   File i/o override for PAK files: FREAD.
     * On entry:  Dest address; element size; num. of elements; file pointer.
     * On exit:   Number of elements read.
     */

    PakOpenFile *openFile = (PakOpenFile *)file;
    RwUInt32 position, bytesLeft, bytesToRead;
    RwUInt32 bytes = 0;

    RtFileSystem *defaultFS = RtFSManagerGetDefaultFileSystem();
 
    if (defaultFS)
    {
        /* 
         * Get the default file system function table 
         */
        RtFileSystemFileFunctionTable defaultFFT = defaultFS->fsFileFunc;
        
        /* 
         * If this aint our file, pass it thru...
         * directly to the default file system
         */
        if( openFile->id != PAKHEADERID )
        {
            return defaultFFT.read(file, pBuffer, nBytes);
        }

        /* 
         * End-of-file?
         */
        if( openFile->eof )
        {
            return 0;
        }

        /* 
         * Position file...
         */
        position = openFile->tocEntry->offset + openFile->bytesRead;
        
        if ((RwUInt32)GetInt64Value(openFile->pakFile->pakFilePosition) != position)
        {
            _PakFileSkipSet(openFile->pakFile, position);
        }

        /* 
         * Cap bytes to read to end of file (allow to just overrun)...
         */
        bytesLeft = (openFile->tocEntry->fileSize - openFile->bytesRead);
        bytesToRead = nBytes;
        if( bytesToRead > bytesLeft + 1 )
        {
            bytesToRead = bytesLeft + 1;
        }

    #ifdef PAKMAXFREADBYTES

        /* 
         * Read from file in several limited size chunks...
         */
        bytes = 0;
        while( bytesToRead > PAKMAXFREADBYTES )
        {
            RwUInt32 bytesRead;

            bytesRead = defaultFFT.read(openFile->pakFile->file, pBuffer, PAKMAXFREADBYTES);
        
            bytes += bytesRead;
            bytesToRead -= bytesRead;
        
            (RwUInt8 *)pBuffer += bytesRead;
        }

        /* 
         * Read remaining bytes...
         */
        bytes += defaultFFT.read(openFile->pakFile->file, pBuffer, bytesToRead);

    #else /* PAKMAXFREADBYTES */

        /* 
         * Read from file in one chunk...
         */
        bytes = defaultFFT.read(openFile->pakFile->file, pBuffer, bytesToRead);

    #endif /* PAKMAXFREADBYTES */

        /* 
         * Increment file position...
         */
        {
            RwInt32 curPos = GetInt64Value(openFile->pakFile->pakFilePosition);
            SetInt64Value(&openFile->pakFile->pakFilePosition, curPos + bytes);
        }
        /* 
         * Increment bytes read...
         */
        openFile->bytesRead += bytes;

        /* 
         * If we have passed the bounds of the current chunk 
         * then report end-of-file...
         */
        if( openFile->bytesRead >= openFile->tocEntry->fileSize )
        {
            bytes -= (openFile->bytesRead - openFile->tocEntry->fileSize);
        
            openFile->eof = TRUE;
        }
    }
    
    return bytes;
}

/*
 *****************************************************************************
 */
#if 0
static RwChar *
_PakFGets(RwChar *buffer, int maxLen, void *file)
{
    /*
     * Purpose:   File i/o override for PAK files: FGETS.
     * On entry:  Receive buffer; max size of buffer; file pointer.
     * On exit:   Buffer.
     */

    PakOpenFile *openFile = (PakOpenFile *)file;
    RwUInt32 position, bytesLeft, bytes;

    /* 
     * If this aint our file, pass it thru...
     */
    if( openFile->id != PAKHEADERID )
    {
        return RwFgets(buffer, maxLen, file);
    }

    /* 
     * End-of-file?
     */
    if( openFile->eof )
    {
        return (RwChar *)NULL;
    }

    /* 
     * Position file...
     */
    position = openFile->tocEntry->offset + openFile->bytesRead;
    if ((RwUInt32)GetInt64Value(openFile->pakFile->pakFilePosition) != position)
    {
        _PakFileSkipSet(openFile->pakFile, position);
    }

    /* 
     * Cap max len to end of file (allow to just overrun)...
     */
    bytesLeft = (openFile->tocEntry->fileSize - openFile->bytesRead);
    if( maxLen < 0 )
    {
        maxLen = 0;
    }
    else if( maxLen > (int)(bytesLeft + 1) )
    {
        maxLen = (int)(bytesLeft + 1);
    }

    /* 
     * Do normal fgets then check that it didnt overrun chunk...
     */
    if( !RwFgets(buffer, maxLen, openFile->pakFile->file) )
    {
        openFile->eof = TRUE;

        return (RwChar *)NULL;
    }

    bytes = rwstrlen(buffer);

    /* 
     * Increment file position...
     */
    {
        RwInt32 curPos = GetInt64Value(openFile->pakFile->pakFilePosition);
        SetInt64Value(&openFile->pakFile->pakFilePosition, curPos + bytes);
    }

    /* 
     * Increment bytes read...
     */
    openFile->bytesRead += bytes;

    /* 
     * If we have passed the bounds of the current chunk 
     * then report end-of-file...
     */
    if( openFile->bytesRead >= openFile->tocEntry->fileSize )
    {
        bytes -= (openFile->bytesRead - openFile->tocEntry->fileSize);
        buffer[bytes] = '\0';

        openFile->eof = TRUE;
    }

    /* 
     * Replace CR+LF with just LF...
     */
    if( (buffer[bytes - 2] == 13) && (buffer[bytes - 1] == 10) )
    {
        buffer[bytes - 2] = 10;
        buffer[bytes - 1] = 0;
    }

    return buffer;
}
#endif

/*
 *****************************************************************************
 */
static RwBool
_PakFEof(RtFile *file)
{
    /*
     * Purpose:   File i/o override for PAK files: FEOF.
     * On entry:  File pointer.
     * On exit:   EoF or not.
     */

    PakOpenFile *openFile = (PakOpenFile *)file;
    RtFileSystem *defaultFS = RtFSManagerGetDefaultFileSystem();
    RtFileSystemFileFunctionTable defaultFFT = defaultFS->fsFileFunc;

    /* 
     * If this aint our file, pass it thru...
     */
    if( openFile->id != PAKHEADERID )
    {
        return defaultFFT.isEOF(file);
    }

    /* 
     * Did we get an end-of-file?...
     */
    if( openFile->eof )
    {
        return EOF;
    }
    else
    {
        return FALSE;
    }
}


/*
 *****************************************************************************
 */
static RtInt64
_PakFSeek( RtFile           *file,
           RwInt32           offset,
           RtFileSeekMethod  fPosition)
{
    RtInt64 ret;

    /*
     * Purpose:   File i/o override for PAK files: FSEEK.
     * On entry:  File pointer; offset; origin.
     * On exit:   0 = success.
     */
    PakOpenFile *openFile = (PakOpenFile *)file;

    RtFileSystem *defaultFS = RtFSManagerGetDefaultFileSystem();

    if(defaultFS)
    {
        RtFileSystemFileFunctionTable defaultFFT = defaultFS->fsFileFunc;
 
        /* 
         * If this aint our file, pass it thru...
         */
        if( openFile->id != PAKHEADERID )
        {
            ret = defaultFFT.setPosition(file, offset, fPosition);
            return ret;
        }

        /* 
         * What's the origin?...
         */
        if( fPosition == RTFILE_POS_BEGIN )
        {
            RwUInt32 pos = openFile->tocEntry->offset + offset;

            if (pos > (RwUInt32)GetInt64Value(openFile->pakFile->pakFileSize) - 1)
            {
                pos = (RwUInt32)GetInt64Value(openFile->pakFile->pakFileSize) - 1;
            }
            
            SetInt64Value(&openFile->pakFile->pakFilePosition, pos);
            
            openFile->bytesRead = pos;

            return defaultFFT.setPosition(openFile->pakFile->file, pos, RTFILE_POS_BEGIN);
        }
        else if( fPosition == RTFILE_POS_CURRENT )
        {
            RwUInt32 pos = offset;

            if (pos > (RwUInt32)GetInt64Value(openFile->pakFile->pakFileSize) -1 )
            {
                pos = (RwUInt32)GetInt64Value(openFile->pakFile->pakFileSize) - 1;
            }

            {
                RwInt32 curPos = GetInt64Value(openFile->pakFile->pakFilePosition);
                SetInt64Value(&openFile->pakFile->pakFilePosition, curPos + pos);
            }

            openFile->bytesRead += pos;

            return defaultFFT.setPosition(openFile->pakFile->file, pos, fPosition);
        }
        else if( fPosition == RTFILE_POS_END )
        {
            RwUInt32 pos = openFile->tocEntry->offset + 
                openFile->tocEntry->fileSize + offset;

            if (pos > (RwUInt32)GetInt64Value(openFile->pakFile->pakFileSize) -1 )
            {
                pos = (RwUInt32)GetInt64Value(openFile->pakFile->pakFileSize) - 1;
            }

            SetInt64Value(&openFile->pakFile->pakFilePosition, pos);

            openFile->bytesRead = pos;

            return defaultFFT.setPosition(openFile->pakFile->file, pos, RTFILE_POS_BEGIN);
        }
    }
    
    SetInt64Value(&ret, 1);
    return ret;
}


/*
 *****************************************************************************
 */
RwBool
PakFileClose(void)
{
    /*
     * Purpose:   Close a PAK file.
     * On entry:  PAK file.
     * On exit:   Success status.
     */

    if( !PAKFile )
    {
        return FALSE;
    }

    /* 
     * Files still open in this PAK file?...
     */
    if( PAKFile->numOpenFiles != 0 )
    {
        return FALSE;
    }

    /* 
     * Free table of contents cache...
     */
    if( PAKFile->tocEntry )
    {
        RwFree(PAKFile->tocEntry);
        PAKFile->tocEntry = (PakTOCEntry *)NULL;
        PAKFile->tocSize = 0;
    }

    /* 
     * Close file...
     */
    if( PAKFile->file )
    {
        RwFclose(PAKFile->file);
        
        PAKFile->file = NULL;
    }

    /* 
     * Free PAK file...
     */
    RwFree(PAKFile);
    PAKFile = (PakFile *)NULL;

    return TRUE;
}

/*
 *****************************************************************************
 */
RwBool
PakFileOpenExt(RwChar *filename, RwChar *directory)
{
    /*
     * Purpose:   Open a PAK file.
     * On entry:  File name; directory that this PAK file represents.
     * On exit:   PAK file.
     */
    RwChar *pos;
    RwInt32 id, tocOffset, tocLen, nameLen, i;
    RwChar separator;
    
    
    
    /* 
     * Check no PAK Files are already open...
     */
    if( PAKFile )
    {
        return FALSE;
    } 

    /* 
     * Create PAK file structure...
     */
    PAKFile = (PakFile *) RwMalloc(sizeof(PakFile), rwID_NAOBJECT);
    if( !PAKFile )
    {
        return FALSE;
    }

    /* 
     * Initialize...
     */
    if( directory )
    {
        /* 
         * Directory has been specified...
         */
        rwstrcpy(PAKFile->dirName, directory);
    }
    else
    {
        /* 
         * Default directory - take path name of PAK file...
         */
        rwstrcpy(PAKFile->dirName, filename);
        pos = rwstrrchr(PAKFile->dirName, '.');
        if( pos )
        {
            *pos = '\0';
        }

        rwstrncat(PAKFile->dirName, RWSTRING("\\"), 1);
    }

    PAKFile->file = NULL;
    SetInt64Value(&PAKFile->pakFileSize, 0);
    SetInt64Value(&PAKFile->pakFilePosition, 0);
    PAKFile->numOpenFiles = 0;
    PAKFile->tocEntry = (PakTOCEntry *)NULL;
    PAKFile->tocSize = 0;

    /* 
     * Open PAK file...
     */
    PAKFile->file = RwFopen(filename, RWSTRING("rb"));
    if( !PAKFile->file )
    {
        PakFileClose();

        return FALSE;
    }

    /* 
     * Set file size...
     * The pak file will be read using the default file system
     */    
#ifdef SKY
    RwFseek(PAKFile->file, 0, RTFILE_POS_END);
    SetInt64Value(&PAKFile->pakFileSize, RwInt32MAXVAL);
#else
    RwFseek(PAKFile->file, 0, RTFILE_POS_END);
    SetInt64Value(&PAKFile->pakFileSize, RwFtell(PAKFile->file));
#endif
    RwFseek(PAKFile->file, 0, RTFILE_POS_BEGIN);

    /* Is this a PAK file? */
    RwFread(&id, sizeof(id), 1, PAKFile->file);
    (void)RwMemNative32(&id, sizeof(id));
    if( id != PAKHEADERID )
    {
        PakFileClose();

        return FALSE;
    }
    
    /* 
     * Cache table of contents...
     */
    RwFread(&tocOffset, sizeof(tocOffset), 1, PAKFile->file);
    (void)RwMemNative32(&tocOffset, sizeof(tocOffset));
    RwFread(&tocLen, sizeof(tocLen), 1, PAKFile->file);
    (void)RwMemNative32(&tocLen, sizeof(tocLen));
    tocLen /= 64;

    PAKFile->tocEntry = (PakTOCEntry *) RwMalloc(sizeof(PakTOCEntry)*tocLen,
                                                 rwID_NAOBJECT);
    if( !PAKFile->tocEntry )
    {
        PakFileClose();

        return FALSE;
    } 

    /* 
     * Get platform path separator...
     */
    separator = RsPathGetSeparator();

    _PakFileSkipSet(PAKFile, tocOffset);

    for(PAKFile->tocSize = 0; PAKFile->tocSize < tocLen; PAKFile->tocSize++)
    {
        /* 
         * Get next file from PAK file...
         */
        RwFread(&PAKFile->tocEntry[PAKFile->tocSize],
                 sizeof(PakTOCEntry), 1, PAKFile->file);
        (void)RwMemNative32(&PAKFile->tocEntry[PAKFile->tocSize].offset, sizeof(RwInt32));
        (void)RwMemNative32(&PAKFile->tocEntry[PAKFile->tocSize].fileSize, sizeof(RwInt32));

        /* 
         * Convert any misdirected path separators...
         */
        if( separator == '/' )
        {
            nameLen = rwstrlen(PAKFile->tocEntry[PAKFile->tocSize].name);
            
            for(i = 0; i < nameLen; i++)
            {
                if( PAKFile->tocEntry[PAKFile->tocSize].name[i] == '\\' )
                {
                    PAKFile->tocEntry[PAKFile->tocSize].name[i] = '/';
                }
            }
        }
        else if( separator == '\\' )
        {
            nameLen = rwstrlen(PAKFile->tocEntry[PAKFile->tocSize].name);
            
            for(i = 0; i < nameLen; i++)
            {
                if( PAKFile->tocEntry[PAKFile->tocSize].name[i] == '/' )
                {
                    PAKFile->tocEntry[PAKFile->tocSize].name[i] = '\\';
                }
            }
        }
    }

    /* 
     * Sort TOC entries into alphabetical order...
     */
    _PakSortTOC(PAKFile);

    return TRUE;
}


/*
 *****************************************************************************
 */
RwBool
PakFileOpen(RwChar *filename)
{
    /*
     * Purpose:   Open a PAK file.
     * On entry:  File name.
     * On exit:   PAK file.
     */
    const RwBool result = PakFileOpenExt(filename, (RwChar*)NULL);

    return result;
}

/****************************************************************************
 * This function must be implemented in every file system
 */
RtFile *
PakFSGetFileObject( RtFileSystem *fs, RwUInt32 index )
{
    if (index < (RwUInt32)((RtFileSystem *)fs)->maxNbOpenFiles)
    {
        return ((RtFile *)&((PakFileSystem *)fs)->files[index]);
    }
    else
    {
        return (NULL);
    }
}

/****************************************************************************
 * Initialise the Pak File System
 */
RtFileSystem *
#if (defined(WIN32))
PakFSystemInit( RwInt32 maxNbOpenFiles,
                RwChar *deviceName,
                RwChar *fileSystemName )
#else
PakFSystemInit( RwInt32  maxNbOpenFiles,
                void    *buffer,
                RwInt32  maxReadSize,
                RwChar  *deviceName,
                RwChar  *fileSystemName )
#endif
{
    RwInt32        i;
    PakOpenFile   *files;
    PakFileSystem *pakFileSystem = NULL;
    RtFileSystem  *fs;

    if (deviceName)
    {
        /* First check that this file system hasn't already been registered */
        if (RtFSManagerGetFileSystemFromName(fileSystemName) != NULL)
        {
            return (FALSE);
        }

        pakFileSystem = (PakFileSystem *)RwMalloc(sizeof (PakFileSystem),
                                                 rwMEMHINTDUR_GLOBAL);
        
        if (pakFileSystem)
        {
            fs = (RtFileSystem *)pakFileSystem;

            /* Initialise the get object function for this file system */
            fs->fsGetObject    = PakFSGetFileObject;
            fs->fsClose        = RtFSManagerGetDefaultFileSystem()->fsClose;

            fs->fsFileFunc.open             = _PakFOpen;
            fs->fsFileFunc.close            = _PakFClose;
            fs->fsFileFunc.read             = _PakFRead;
            fs->fsFileFunc.isEOF            = _PakFEof;
            fs->fsFileFunc.setPosition      = _PakFSeek;
            fs->fsFileFunc.fExists          = _PakFExist;

            /* Now allocate memory for maximum number of open files */
            pakFileSystem->files = (PakOpenFile *)RwCalloc(maxNbOpenFiles,
                                    sizeof (PakOpenFile),
                                    rwMEMHINTDUR_GLOBAL);

            files = pakFileSystem->files;

            /* ... do other file system specific initialisation... */
            for (i = 0; i < maxNbOpenFiles; i++)
            {
                RtFile * f    = (RtFile *)&files[i];
                f->fileSystem = (RtFileSystem *)pakFileSystem;

#if (defined(SKY))
                ((FILE_TYPE *)f)->fBuffer.mBuffer    = &((RwChar *)buffer)[i * maxReadSize];
                ((FILE_TYPE *)f)->fBuffer.bufferSize = maxReadSize;
#elif (defined (DOLPHIN))
                ((FILE_TYPE *)f)->fBuffer.buffer       = &((RwChar *)buffer)[i * maxReadSize];
                ((FILE_TYPE *)f)->fBuffer.bufferSize   = maxReadSize;
                ((FILE_TYPE *)f)->fBuffer.valid        = FALSE;
                ((FILE_TYPE *)f)->fBuffer.bufferOffset = 0;
#endif
            }

            /* Init this file system */
            if (_rtFSInit( (RtFileSystem *)pakFileSystem,
                            maxNbOpenFiles,
                            fileSystemName, 
                            deviceName) !=
                            RTFS_ERROR_NOERROR)
            {
                RwFree(pakFileSystem->files);
                RwFree(pakFileSystem);
                return (NULL);
            }
        }
    }

    return ((RtFileSystem *)pakFileSystem);
}
