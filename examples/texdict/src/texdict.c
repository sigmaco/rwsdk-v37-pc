
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
 * texdict.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *                                                                         
 * Purpose: Display the difference in texture loading times by using a
 *          texture dictionary.
 *                         
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#include "skeleton.h"
#include "menu.h"

#include "texdict.h"

RwReal TimeWithoutTexDict = 0;
RwReal TimeWithTexDict = 0;

RpLight *AmbientLight = NULL;
RpWorld *World = NULL;
RwCamera *Camera = NULL;

RwBool LoadingWorld = FALSE;



/*
 *****************************************************************************
 */
void
CameraSetPosition(RwCamera *camera, RpWorld *world)
{
    RwFrame *frame;
    RwV3d pos, right, at;
    const RwBBox *bbox;

    frame = RwCameraGetFrame(camera);

    /* 
     * Rotate the camera so it looks straight down...
     */
    right = *RwMatrixGetRight(RwFrameGetMatrix(frame));
    RwFrameRotate(frame, &right, 90.0f, rwCOMBINEREPLACE);

    /* 
     * Move the camera to the center of the world...
     */
    bbox = RpWorldGetBBox(world);
    RwV3dSub(&pos, &(bbox->sup), &(bbox->inf));
    RwV3dScale(&pos, &pos, 0.5f);
    RwV3dAdd(&pos, &pos, &(bbox->inf));
    RwFrameTranslate(frame, &pos, rwCOMBINEPOSTCONCAT);

    /* 
     * And back it up a little...
     */
    at = *RwMatrixGetAt(RwFrameGetMatrix(frame));
    RwV3dScale(&at, &at, -100.0f);
    RwFrameTranslate(frame, &at, rwCOMBINEPOSTCONCAT);
    
    return;
}


/*
 *****************************************************************************
 */
static RpWorld *
LoadWorld(void)
{
    RwStream *stream;
    RwChar *path;
    RpWorld *world;
    RwUInt32 time;

    /*
     * Check that the current dictionary is empty... 
     */
    RwTexDictionaryDestroy(RwTexDictionaryGetCurrent());
    RwTexDictionarySetCurrent(RwTexDictionaryCreate());
    
    /* 
     * Start the timer...
     */
    time = RsTimer();

    /* 
     * Attempt to load in the BSP file...
     */
    path = RsPathnameCreate(RWSTRING ("models/dungeon/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    path = RsPathnameCreate(RWSTRING ("models/dungeon.bsp"));
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);

    world = NULL;

    if( stream )
    {
        if( RwStreamFindChunk(stream, rwID_WORLD, NULL, NULL) )
        {
            world = RpWorldStreamRead(stream);
        }

        RwStreamClose(stream, NULL);
    }

    /* 
     * Stop the timer and calculate the load time...
     */
    time = RsTimer() - time;

    TimeWithoutTexDict = (RwReal)time / 1000.0f;

    /* 
     * Save the texture dictionary...
     */
    path = RsPathnameCreate(RWSTRING("models/dungeon/dungeon.txd"));
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, path);
    RsPathnameDestroy(path);

    if( stream )
    {
        RwTexDictionaryStreamWrite(RwTexDictionaryGetCurrent(), stream);
        
        RwStreamClose(stream, NULL);
    }

    /*
     * Remove the non-dictionary loaded world...
     */
    RpWorldDestroy(world);

    
    /* 
     * This time we load the world after loading the texture dictionary.
     * Start the timer...
     */
    time = RsTimer();

    /* 
     * Attempt to load in the texture dictionary file...
     */
    path = RsPathnameCreate(RWSTRING("models/dungeon/dungeon.txd"));
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);

    if( stream )
    {
        RwTexDictionary *texDict = NULL;

        if( RwStreamFindChunk(stream, rwID_TEXDICTIONARY, NULL, NULL) )
        {
            texDict = RwTexDictionaryStreamRead(stream);
        }

        RwStreamClose(stream, NULL);

        /* 
         * Make it the current dictionary...
         */
        if( texDict )
        {
            RwTexDictionaryDestroy(RwTexDictionaryGetCurrent());

            RwTexDictionarySetCurrent(texDict);
        }
    }

    /* 
     * Attempt to load in the BSP file...
     */
    path = RsPathnameCreate(RWSTRING ("models/dungeon.bsp"));
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);

    world = NULL;

    if( stream )
    {
        if( RwStreamFindChunk(stream, rwID_WORLD, NULL, NULL) )
        {
            world = RpWorldStreamRead(stream);
        }

        RwStreamClose(stream, NULL);
    }

    /* 
     * Stop the timer and calculate the load time...
     */
    time = RsTimer() - time;

    TimeWithTexDict = (RwReal)time / 1000.0f;

    return world;
}


/*
 *****************************************************************************
 */
void
ReloadWorld(void)
{
    /*
     * Firstly we remove the ambient light and camera from
     * the present world...
     */
    RpWorldRemoveLight(World, AmbientLight);
    RpWorldRemoveCamera(World, Camera);

    /* 
     * Now destroy the present world...
     */
    RpWorldDestroy(World);

    /*
     * Load in a new world and add the old ambient light
     * and camera from the previous world...
     */
    World = LoadWorld();
    if( World == NULL )
    {
        RsErrorMessage(RWSTRING("Cannot create world."));

        return;
    }

    CameraSetPosition(Camera, World);

    /*
     * Add the ambient light and camera to the new world...
     */
    RpWorldAddLight(World, AmbientLight);
    RpWorldAddCamera(World, Camera);

    return;
}

/*
 *****************************************************************************
 */
