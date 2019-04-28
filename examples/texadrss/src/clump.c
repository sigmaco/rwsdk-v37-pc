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
 * Purpose: RenderWare Graphics texture addressing example.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#include "skeleton.h"

#include "texadrss.h"


/*
 *****************************************************************************
 */
RpClump *
ClumpCreate(RpWorld *world)
{
    RpClump *clump;
    RwChar *path;
    RwStream *stream;

    path = RsPathnameCreate(RWSTRING("models/textures/"));
    RwImageSetPath(path);
    RsPathnameDestroy(path);

    path = RsPathnameCreate(RWSTRING("models/cube.dff"));
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
    RsPathnameDestroy(path);

#ifdef SKY
    RpSkyTextureSetDefaultMipmapK(-1.0f);
#endif

    clump = NULL;

    if( stream )
    {
        if( RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL) )
        {
            clump = RpClumpStreamRead(stream);

            if( clump )
            {
                RwV3d pos;

                pos.x = pos.y = 0.0f;
                pos.z = 4.0f;

                RwFrameTranslate(RpClumpGetFrame(clump), &pos, rwCOMBINEREPLACE);

                RpWorldAddClump(world, clump);
            }
        }

        RwStreamClose(stream, NULL);
    }

    return clump;
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
 ***************************************************************************
 */
void
ClumpTranslateZ(RpClump *clump, RwCamera *camera, RwReal zDelta)
{
    RwFrame *clumpFrame, *cameraFrame;
    RwV3d delta;

    clumpFrame = RpClumpGetFrame(clump);
    cameraFrame = RwCameraGetFrame(camera);

    RwV3dScale(&delta, RwMatrixGetAt(RwFrameGetMatrix(cameraFrame)), zDelta); 

    RwFrameTranslate(clumpFrame, &delta, rwCOMBINEPOSTCONCAT);

    return;
}

/*
 *****************************************************************************
 */
