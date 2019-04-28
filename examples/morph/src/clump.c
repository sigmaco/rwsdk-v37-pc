
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
 * clump.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *                                                                         
 * Purpose: Example to demonstrate the morph plugin.
 *
 *****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#include "skeleton.h"

#include "morph.h"



/*
 *****************************************************************************
 */
RpClump *
CreateClump(RpWorld *world)
{
    RwStream *stream;
    RwChar *path;

    path = RsPathnameCreate(RWSTRING("models/textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    RwImageSetGamma(1.7f);

    path = RsPathnameCreate(RWSTRING("models/world.dff"));
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);   
           
    if( stream )
    {
        RpClump *clump = NULL;

        if( RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL) )
        {
            clump = RpClumpStreamRead(stream);
        }

        RwStreamClose(stream, NULL);
           
        if( clump )
        {
            RwFrame *clumpFrame;
            RwV3d pos = {0.0f, 0.0f, 4.0f};
            RwV3d xAxis = {1.0f, 0.0f, 0.0f};
            RwV3d yAxis = {0.0f, 1.0f, 0.0f};
        
            clumpFrame = RpClumpGetFrame(clump);

            RwFrameRotate(clumpFrame, &xAxis, 45.0f, rwCOMBINEREPLACE);        
            RwFrameRotate(clumpFrame, &yAxis, 45.0f, rwCOMBINEPOSTCONCAT);        

            RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);

            ClumpSetupInterpolators(clump);

            RpWorldAddClump(world, clump);

            return clump;
        }
    }

    return NULL;
}


/*
 *****************************************************************************
 */
void
ClumpRotate(RpClump *clump, RwCamera *camera, RwReal xAngle, RwReal yAngle)
{
    RwFrame *clumpFrame = NULL;
    RwMatrix *cameraMatrix = NULL;
    RwV3d right, up, pos;

    clumpFrame = RpClumpGetFrame(clump);
     
    /*
     * Rotate clump about it's origin...
     */           
    cameraMatrix = RwFrameGetMatrix(RwCameraGetFrame(camera));
    right = *RwMatrixGetRight(cameraMatrix);
    up = *RwMatrixGetUp(cameraMatrix);

    pos = *RwMatrixGetPos(RwFrameGetMatrix(clumpFrame));

    /*
     * Translate back to the origin...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);

    /*
     * Do the rotation...
     */
    RwFrameRotate(clumpFrame, &up, xAngle, rwCOMBINEPOSTCONCAT);
    RwFrameRotate(clumpFrame, &right, yAngle, rwCOMBINEPOSTCONCAT);

    /*
     * And translate back...
     */
    RwV3dScale(&pos, &pos, -1.0f);
    RwFrameTranslate(clumpFrame, &pos, rwCOMBINEPOSTCONCAT);

    return;
}

/*
 *****************************************************************************
 */
