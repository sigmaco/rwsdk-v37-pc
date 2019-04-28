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
 * Copyright (c) 2001 Criterion Software Ltd.
 * All Rights Reserved.
 *
 */

/****************************************************************************
 *
 * scene.c
 *
 * Copyright (C) 2001 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *
 * Purpose: Example of a particle system, using a ptank for the rendering, 
 *          using a skinned character as an emitter.
 *
*****************************************************************************/
#include "rwcore.h"
#include "rpworld.h"
#include "rpskin.h"
#include "rphanim.h"
#include "rpcollis.h"
#include "rtcharse.h"

#include "skeleton.h"
#include "menu.h"

#include "main.h"

#include "scene.h"

#include "prtsyst.h"

extern RwCamera *Camera;

/*
 *  World objects
 */
static RpWorld     *World = NULL;
static RwChar       WorldTexturePath[] = RWSTRING("models/textures/");
static RwChar       WorldPath[] = RWSTRING("models/world.bsp");

static RpClump     *Clump = NULL;
static RwChar       ClumpPath[] = RWSTRING("models/clump.dff");
static const RwV3d  ClumpStartPos = { 30.0f, 46.0f, 0.0f };
static RwSphere     ClumpBoundingSphere;
static RwSphere     ClumpWorldBoundingSphere;

static RtAnimAnimation *Anim = NULL;
static RwChar       AnimPath[] = RWSTRING("models/clump.anm");
static RpHAnimHierarchy *Hierarchy = NULL;
static RwBool       AnimEnabled = TRUE;
static RwReal       AnimSpeedFactor = 0.3f; /* 1.0f is normal speed */

#define ANIM_CLUMP_SPEED  100.0f    


static prtSystObj  *PrtSystem;

/*
 *  Lights
 */
static RpLight     *DirectionalLight = NULL;
static RpLight     *AmbientLight = NULL;

#define ALIGHT_VALUE        0.2f /* Ambient light level */
#define DLIGHT_VALUE        0.8f /* Directional light intensity */
#define DLIGHT_ELEVATION    45.0f
#define DLIGHT_AZIMUTH      60.0f

/*
 *  Misc
 */
static const RwV3d  Xaxis = { 1.0f, 0.0f, 0.0f };
static const RwV3d  Yaxis = { 0.0f, 1.0f, 0.0f };
static const RwV3d  Zaxis = { 0.0f, 0.0f, 1.0f };

/*
 *****************************************************************************
 */
static RpWorld *
WorldLoad(RwChar * filename)
{
    RpWorld            *world = NULL;
    RwChar             *path;

    /*
     *  Get path.
     */
    path = RsPathnameCreate(filename);
    if (path)
    {
        RwStream           *stream;

        /*
         *  Open stream to file.
         */
        stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
        RsPathnameDestroy(path);
        if (stream)
        {
            /*
             *  Find world data chunk in the stream.
             */
            if (RwStreamFindChunk(stream, rwID_WORLD, NULL, NULL))
            {
                world = RpWorldStreamRead(stream);
            }

            RwStreamClose(stream, NULL);
        }
    }

    return (world);
}


/*
 *****************************************************************************
 */
static RpClump *
ClumpLoad(RwChar * filename)
{
    RwChar             *path;
    RpClump            *clump = NULL;

    /*
     *  Get path.
     */
    path = RsPathnameCreate(filename);
    if (path)
    {
        RwStream           *stream;

        /* 
         *  Open stream to file.
         */
        stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, path);
        RsPathnameDestroy(path);
        if (stream)
        {
            /*
             *  Find a clump chunk in the stream.
             */
            if (RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL))
            {
                clump = RpClumpStreamRead(stream);
            }

            RwStreamClose(stream, NULL);
        }
    }

    return clump;
}


/*
 ******************************************************************************
 */
static RwFrame *
ClumpGetHierarchyRecurse(RwFrame * frame, void *ptr)
{
    RpHAnimHierarchy  **hierarchy = (RpHAnimHierarchy **) ptr;

    *hierarchy = RpHAnimFrameGetHierarchy(frame);
    if (!*hierarchy)
    {
        /*
         *  Look in child frames.
         */
        RwFrameForAllChildren(frame, ClumpGetHierarchyRecurse,
                              (void *) hierarchy);

        if (!*hierarchy)
        {
            return frame;      /* Still haven't found it */
        }
    }

    return NULL;
}

static RpHAnimHierarchy *
ClumpGetHierarchy(RpClump * clump)
{
    RwFrame            *frame = RpClumpGetFrame(clump);
    RpHAnimHierarchy   *hierarchy;

    /*
     *  Find the point in a clump frame hierarchy connected to RpHAnimHierarchy
     */

    hierarchy = RpHAnimFrameGetHierarchy(frame);
    if (!hierarchy)
    {
        RwFrameForAllChildren(frame, ClumpGetHierarchyRecurse,
                              (void *) &hierarchy);
    }

    return hierarchy;
}


/*
 ******************************************************************************
 */
static RpAtomic *
GetFirstAtomic(RpAtomic * atomic, void *data)
{
    RpAtomic          **firstAtomic = (RpAtomic **) data;

    *firstAtomic = atomic;
    return NULL;
}


/*
 ******************************************************************************
 */
static RwBool
SetupClump(void)
{
    RpAtomic           *atomic;
    RwChar             *path;

    /*
     *  Load clump, and get the atomic (we assume there's only one)
     */
    Clump = ClumpLoad(ClumpPath);
    if (!Clump)
    {
        return FALSE;
    }

    RpClumpForAllAtomics(Clump, GetFirstAtomic, (void *) &atomic);

    /*
     *  Position the clump and initialize bounding sphere data
     */
    RwFrameTranslate(RpClumpGetFrame(Clump), &ClumpStartPos,
                     rwCOMBINEREPLACE);

    ClumpBoundingSphere = *RpAtomicGetBoundingSphere(atomic);

    ClumpWorldBoundingSphere.radius = ClumpBoundingSphere.radius;

    RwV3dTransformPoint(&ClumpWorldBoundingSphere.center,
                        &ClumpBoundingSphere.center,
                        RwFrameGetMatrix(RpClumpGetFrame(Clump)));

    /*
     *  Setup hierarchy animation.
     */
    path = RsPathnameCreate(AnimPath);
    Anim = RtAnimAnimationRead(path);
    RsPathnameDestroy(path);
    if (!Anim)
    {
        return FALSE;
    }

    Hierarchy = ClumpGetHierarchy(Clump);
    RpHAnimHierarchySetCurrentAnim(Hierarchy, Anim);
    RpHAnimHierarchyUpdateMatrices(Hierarchy);

    /* 
     *  Set hierarchy for the atomic's skin.
     */
    RpSkinAtomicSetHAnimHierarchy(atomic, Hierarchy);
    RpHAnimHierarchyAttach(Hierarchy);
    RpHAnimHierarchySetFlags(Hierarchy, RpHAnimHierarchyGetFlags(Hierarchy) |
                        rpHANIMHIERARCHYUPDATEMODELLINGMATRICES |
                        rpHANIMHIERARCHYUPDATELTMS |
                        rpHANIMHIERARCHYLOCALSPACEMATRICES |
                        0 );

    return TRUE;
}


/*
 ******************************************************************************
 */
static void
DestroyClump(void)
{
    if (Anim)
    {
        RtAnimAnimationDestroy(Anim);
        Anim = NULL;
    }

    if (Clump)
    {
        RpClumpDestroy(Clump);
        Clump = NULL;
    }

    return;
}


/*
 ******************************************************************************
 */
static void
UpdateClump(RwReal deltaTime)
{
    if (AnimEnabled)
    {
        RwReal deltaT;
        RwFrame            *frame = RpClumpGetFrame(Clump);
        RwMatrix           *matrix = RwFrameGetMatrix(frame);
        RwV3d              *pos = RwMatrixGetPos(matrix);
        const RwBBox       *bbox = RpWorldGetBBox(World);
        RwV3d               tr;

        /*
         *  Adjust the time step.
         */
        deltaT = deltaTime * AnimSpeedFactor;

        /* 
         *  Update animation on hierarchy.
         */
        if (deltaT < 0.0f)
        {
            RpHAnimHierarchySubAnimTime(Hierarchy, -deltaT);
        }
        else
        {
            /* check if we've been playing backwards */
            if (!Hierarchy->currentAnim->pNextFrame)
            {
                /* we have so reset the animation for forward playback */
                RwReal              targetTime;
                RtAnimAnimation     *currentAnim;
                currentAnim = RpHAnimHierarchyGetCurrentAnim(Hierarchy);

                targetTime = Hierarchy->currentAnim->currentTime + deltaT;
                RpHAnimHierarchySetCurrentAnim(Hierarchy,currentAnim);
                RpHAnimHierarchySetCurrentAnimTime(Hierarchy,
                                                   targetTime);
            }
            else
            {
                RpHAnimHierarchyAddAnimTime(Hierarchy, deltaT);
            }
        }
        RpHAnimHierarchyUpdateMatrices(Hierarchy);

        /*
         *  Translate the clump along z axis, but wrap around when we
         *  reach the edge of the world.
         */
        tr.x = tr.y = 0.0f;

        if (pos->z > bbox->sup.z)
        {
            tr.z = bbox->inf.z - bbox->sup.z;
        }
        else if (pos->z < bbox->inf.z)
        {
            tr.z = bbox->sup.z - bbox->inf.z;
        }
        else
        {
            tr.z = deltaT * ANIM_CLUMP_SPEED;
        }

        RwFrameTranslate(frame, &tr, rwCOMBINEPOSTCONCAT);

        /*
         *  Update the clump world bounding sphere
         */
        RwV3dTransformPoint(&ClumpWorldBoundingSphere.center,
                            &ClumpBoundingSphere.center, matrix);



    }

    return;
}


/*
 *****************************************************************************
 */
static RwBool
SetupLights(void)
{
    RwFrame            *frame;
    RwRGBAReal          col;

    /* 
     *  Create ambient light
     */
    AmbientLight = RpLightCreate(rpLIGHTAMBIENT);
    if (!AmbientLight)
    {
        return FALSE;
    }

    col.red = col.green = col.blue = ALIGHT_VALUE;
    col.alpha = 0.0f;
    RpLightSetColor(AmbientLight, &col);

    RpWorldAddLight(World, AmbientLight);

    /*
     *  Create directional light
     */
    DirectionalLight = RpLightCreate(rpLIGHTDIRECTIONAL);
    if (!DirectionalLight)
    {
        return FALSE;
    }

    col.red = col.green = col.blue = DLIGHT_VALUE;
    col.alpha = 0.0f;
    RpLightSetColor(DirectionalLight, &col);

    frame = RwFrameCreate();
    RwFrameRotate(frame, &Xaxis, DLIGHT_ELEVATION, rwCOMBINEREPLACE);
    RwFrameRotate(frame, &Yaxis, -DLIGHT_AZIMUTH, rwCOMBINEPOSTCONCAT);
    RpLightSetFrame(DirectionalLight, frame);

    RpWorldAddLight(World, DirectionalLight);

    return TRUE;
}


/*
 *****************************************************************************
 */
static void
DestroyLights(void)
{
    RwFrame            *frame;

    /*
     *  Destroy ambient light.
     */
    RpWorldRemoveLight(World, AmbientLight);
    RpLightDestroy(AmbientLight);
    AmbientLight = NULL;

    /*
     *  Destroy directional light.
     */
    RpWorldRemoveLight(World, DirectionalLight);
    frame = RpLightGetFrame(DirectionalLight);
    RpLightSetFrame(DirectionalLight, NULL);
    RwFrameDestroy(frame);
    RpLightDestroy(DirectionalLight);
    DirectionalLight = NULL;

    return;
}


/*
 ******************************************************************************
 */
static RwBool
SetupParticle(void)
{
    RpAtomic *atomic;
    RwTexture *texture;

#if (defined WIN32 || defined XBOX)
    /*
     * Turn mipmaping on for the particles, 
     * That will save on the fill rate
     */
    RwBool automipmap = RwTextureGetAutoMipmapping();
    RwBool mipmap = RwTextureGetMipmapping();
    RwTextureSetAutoMipmapping(TRUE);
    RwTextureSetMipmapping(TRUE);
#endif

    texture = RwTextureRead(RWSTRING("particle"),RWSTRING("particle"));

#if (defined WIN32 || defined XBOX)
    RwTextureSetFilterMode(texture,rwFILTERMIPLINEAR);
    RwTextureSetAutoMipmapping(automipmap);
    RwTextureSetMipmapping(mipmap);
#else
    RwTextureSetFilterMode(texture,rwFILTERLINEAR );
#endif


    /* Initialise the dragon's geomesh particle generator */
    RpClumpForAllAtomics(Clump, GetFirstAtomic, (void*)&atomic);
        
    if( atomic )
    {
        RwV3d gravity    = {0.f, 98.1f, 0.f};
        RwV3d wind       = {0.f, 0.f, 0.0f};
        RwRGBAReal color = {212.0f,112.0f,48.0f,255.0f};

        PrtSystem = PrtSystemCreate(5000 ,0.50f,1.0f/70.0f,atomic,texture);
        if( NULL == PrtSystem )
        {
            return FALSE;
        }

        PrtSystem->color = color;

        PrtSystem->gravity  = gravity; 
	    PrtSystem->wind     = wind;  
	    PrtSystem->friction = 0.25f;
            
        PrtSystem->speed   = 0.25f;
        PrtSystem->rndSpeed   = 0.1f; 
        PrtSystem->disp   = 0.0f;  

        PrtSystemSetParticleFadeRange(PrtSystem, 0.2f, 0.0f);

        RpAtomicSetFrame(PrtSystem->ptank,RpClumpGetFrame(Clump));

        PrtSystem->active = TRUE;

        return TRUE;
    }

    return FALSE;

}


/*
 ******************************************************************************
 */
RwBool
SceneMenuInit(void)
{
    static RwChar       animLabel[] = RWSTRING("Animation_A");
    static RwChar       animSpeedLabel[] = RWSTRING("Animation speed");
    static RwChar       prtStatusLabel[] = RWSTRING("Emit particles_E");

    MenuAddEntryBool(animLabel, &AnimEnabled, NULL);

    MenuAddEntryReal(animSpeedLabel, &AnimSpeedFactor, NULL, -2.0f,
                     5.0f, 0.1f);

    MenuAddSeparator();

    MenuAddEntryBool(prtStatusLabel, &PrtSystem->active, NULL);

    return TRUE;
}


/*
 ******************************************************************************
 */
RpWorld            *
SceneOpen(void)
{
    RwChar *path;

    /*
     *  Setup the world, clump, particle system, lights and shadow.
     */
    path = RsPathnameCreate(WorldTexturePath);
    RwImageSetPath(path);
    RsPathnameDestroy(path);
    World = WorldLoad(WorldPath);

    if (World &&
        SetupClump() &&
        SetupParticle() &&
        SetupLights() )
    {

        return World;
    }

    return NULL;
}


/*
 ******************************************************************************
 */
void
SceneClose(void)
{
    /*
     *  Destroy lights, particle system, clump, and world.
     */
    DestroyLights();
    
    if( PrtSystem )
    {
        RpAtomicSetFrame(PrtSystem->ptank, NULL);
        PrtSystemDestroy(PrtSystem);
    }

    DestroyClump();

    if (World)
    {
        RpWorldDestroy(World);
        World = NULL;
    }

    return;
}


/*
 ******************************************************************************
 */
RwBool
SceneUpdate(RwReal deltaTime)
{
    /*
     *  Update clump animation.
     */
    UpdateClump(deltaTime);
    
    /*
     *  Update the particle system.
     */
    PrtSystemUpdate(PrtSystem,deltaTime);

    return TRUE;
}


/*
 ******************************************************************************
 */
RwBool
SceneRender(void)
{
    /*
     *  Render the world sectors
     */
    RpWorldRender(World);

    /*
    *  Render the clump
    */
    RpClumpRender(Clump);

    /*
     *  Render the particle system
     */
    PrtSystemRender(PrtSystem);

    return TRUE;
}


/*
 ******************************************************************************
 */
void
SceneDisplayOnScreenInfo(void)
{
    RwChar              caption[256];

    if( TRUE == PrtSystem->active )
    {
        RsSprintf(caption, RWSTRING("System Status    :   On"));
        RsCharsetPrint(Charset, caption, 0,2, rsPRINTPOSTOPRIGHT);

        RsSprintf(caption, RWSTRING("Active Particles : %4d"), PrtSystem->actPrtNum);
        RsCharsetPrint(Charset, caption, 0,3, rsPRINTPOSTOPRIGHT);

        RsSprintf(caption, RWSTRING("Skined Vertex Per Frame : %4d"), PrtSystem->emitionRate);
        RsCharsetPrint(Charset, caption, 0,4, rsPRINTPOSTOPRIGHT);

    }
    else
    {
        RsSprintf(caption, RWSTRING("System Status    :  Off"));
        RsCharsetPrint(Charset, caption, 0,2, rsPRINTPOSTOPRIGHT);

        RsSprintf(caption, RWSTRING("Active Particles : %4d"), PrtSystem->actPrtNum);
        RsCharsetPrint(Charset, caption, 0,3, rsPRINTPOSTOPRIGHT);

        RsSprintf(caption, RWSTRING("Skined Vertex Per Frame : %4d"), 0);
        RsCharsetPrint(Charset, caption, 0,4, rsPRINTPOSTOPRIGHT);
    }
}
