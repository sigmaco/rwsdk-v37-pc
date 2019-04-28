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
 * physics.c
 *
 * Copyright (C) 2000 Criterion Software Ltd
 *
 * Author: RenderWare Team
 *                                                                         
 * Purpose: Example to demonstrate how user-plugins can be used to extend
 *          RenderWare objects.
 *                         
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"

#include "physics.h"

#define rwID_EXAMPLE (0xff)

#define PHYSICSGLOBAL(var) \
    (RWPLUGINOFFSET(PhysicsGlobals, RwEngineInstance, GlobalOffset)->var)

#define PHYSICSLOCAL(clump, var) \
    (RWPLUGINOFFSET(PhysicsLocals, clump, LocalOffset)->var)

#define PHYSICSLOCALCONST(clump, var) \
    (RWPLUGINOFFSETCONST(PhysicsLocals, clump, LocalOffset)->var)

#define CAPTION (RWSTRING("RenderWare Graphics Plugin Construction Example"))

typedef struct physicsGlobals PhysicsGlobals;
struct physicsGlobals
{
        RwChar *caption;
        RwReal gravity;
        RwReal minSpeed;
};

typedef struct physicsLocals PhysicsLocals;
struct physicsLocals
{
        RwReal speed;
        RwReal bounciness;
        RwBool active;
};

static RwInt32 GlobalOffset = -1;
static RwInt32 LocalOffset = -1;



/*
 *****************************************************************************
 */
static void *
PhysicsGlobalDataConstructor(void *clump,
                             RwInt32 offset __RWUNUSED__,
                             RwInt32 size __RWUNUSED__)
{
    /*
     * Allocate memory for the caption string, plus the NULL
     * termination character (2 bytes for unicode)...
     */
    PHYSICSGLOBAL(caption) = (RwChar *)RwMalloc(sizeof(CAPTION)+2,
                                                rwID_NAOBJECT);

    if( !PHYSICSGLOBAL(caption) )
    {
        return NULL;
    }

    /*
     * Initialize the global data...
     */
    rwstrcpy(PHYSICSGLOBAL(caption), CAPTION);
    PHYSICSGLOBAL(gravity) = -100.0f;
    PHYSICSGLOBAL(minSpeed) = 10.0f;

    return clump;
}


static void *
PhysicsGlobalDataDestructor(void *clump,
                            RwInt32 offset __RWUNUSED__,
                            RwInt32 size __RWUNUSED__)
{
    /*
     * Free the memory allocated for the caption string...
     */
    if( PHYSICSGLOBAL(caption) )
    {
        RwFree(PHYSICSGLOBAL(caption));

        PHYSICSGLOBAL(caption) = NULL;
    }

    return clump;
}


/*
 *****************************************************************************
 */
static void *
PhysicsDataConstructor(void *clump,
                       RwInt32 offset __RWUNUSED__,
                       RwInt32 size __RWUNUSED__)
{
    if( LocalOffset > 0 )
    {
        PHYSICSLOCAL(clump, speed) = 0.0f;
        PHYSICSLOCAL(clump, bounciness) = 0.0f;
        PHYSICSLOCAL(clump, active) = TRUE;
    }

    return clump;
}


static void *
PhysicsDataDestructor(void *clump,
                      RwInt32 offset __RWUNUSED__,
                      RwInt32 size __RWUNUSED__)
{
    return clump;
}


static void *
PhysicsDataCopier(void *dstClump, const void *srcClump,
                  RwInt32 offset __RWUNUSED__,
                  RwInt32 size __RWUNUSED__)
{
    PHYSICSLOCAL(dstClump, speed) = PHYSICSLOCALCONST(srcClump, speed);

    PHYSICSLOCAL(dstClump, bounciness) = PHYSICSLOCALCONST(srcClump, bounciness);

    PHYSICSLOCAL(dstClump, active) = PHYSICSLOCALCONST(srcClump, active);

    return dstClump;
}


/*
 *****************************************************************************
 */
static RwStream *
PhysicsDataReadStream(RwStream *stream,
                      RwInt32 length __RWUNUSED__,
                      void *clump,
                      RwInt32 offset __RWUNUSED__,
                      RwInt32 size __RWUNUSED__)
{
    PhysicsLocals binaryData;

    /*
     * Read in the extension data...
     */
    if( RwStreamRead(stream, &binaryData, sizeof(binaryData)) != sizeof(binaryData))
    {
        return NULL;
    }

    /*
     * Convert it back to whatever endian machine we are running on...
     */
    (void)RwMemNative(&binaryData, sizeof(binaryData));

    /*
     * Convert the reals back to whatever format this machine likes...
     */
    (void)RwMemFloat32ToReal(&binaryData.speed, sizeof(binaryData.speed));
    (void)RwMemFloat32ToReal(&binaryData.bounciness, sizeof(binaryData.bounciness));

    /*
     * Copy the data into the clump...
     */
    RpClumpPhysicsSetSpeed((RpClump *)clump, binaryData.speed);
    RpClumpPhysicsSetBounciness((RpClump *)clump, binaryData.bounciness);
    RpClumpPhysicsSetActive((RpClump *)clump, binaryData.active);

    return stream;
}


static RwStream *
PhysicsDataWriteStream(RwStream *stream,
                       RwInt32 length __RWUNUSED__,
                       const void *clump,
                       RwInt32 offset __RWUNUSED__,
                       RwInt32 size __RWUNUSED__)
{
    PhysicsLocals binaryData;

    /*
     * Capture the data in a nice convenient structure...
     */
    binaryData.speed = PHYSICSLOCALCONST(clump, speed);
    binaryData.bounciness = PHYSICSLOCALCONST(clump, bounciness);
    binaryData.active = PHYSICSLOCALCONST(clump, active);

    /*
     * Convert RwReals to form suitable for binary data storage...
     */
    (void)RwMemRealToFloat32(&binaryData.speed, sizeof(binaryData.speed));
    (void)RwMemRealToFloat32(&binaryData.bounciness, sizeof(binaryData.bounciness));

    /*
     * Then make sure the whole block is little endian for storage...
     */
    (void)RwMemLittleEndian(&binaryData, sizeof(binaryData));

    /*
     * Now we can write it to the stream...
     */
    if( !RwStreamWrite(stream, &binaryData, sizeof(binaryData)) )
    {
        return NULL;
    }

    return stream;
}


static RwInt32
PhysicsDataGetStreamSize(const void *stream __RWUNUSED__,
                         RwInt32 offset __RWUNUSED__,
                         RwInt32 size __RWUNUSED__)
{
    return sizeof(PhysicsLocals);
}


/*
 *****************************************************************************
 */
RwBool
RpClumpPhysicsPluginAttach(void)
{
    RwInt32 offset;

    /*
     * Register global space...
     */
    GlobalOffset = RwEngineRegisterPlugin(sizeof(PhysicsGlobals),
                                          MAKECHUNKID(rwVENDORID_CRITERIONTK, rwID_EXAMPLE),
                                          PhysicsGlobalDataConstructor,
                                          PhysicsGlobalDataDestructor);

    if( GlobalOffset < 0 )
    {
        return FALSE;
    }

    /*
     * Register clump extension space...
     */
    LocalOffset = RpClumpRegisterPlugin(sizeof(PhysicsLocals),
                                        MAKECHUNKID(rwVENDORID_CRITERIONTK, rwID_EXAMPLE),
                                        PhysicsDataConstructor,
                                        PhysicsDataDestructor,
                                        PhysicsDataCopier);

    if( LocalOffset < 0 )
    {
        return FALSE;
    }

    /*
     * Register binary stream functionality for extension data...
     */
    offset = -1;
    offset = RpClumpRegisterPluginStream(
                                         MAKECHUNKID(rwVENDORID_CRITERIONTK, rwID_EXAMPLE),
                                         PhysicsDataReadStream,
                                         PhysicsDataWriteStream,
                                         PhysicsDataGetStreamSize);

    if( offset != LocalOffset )
    {
        return FALSE;
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
RwReal
RpClumpPhysicsGetGravity(void)
{
    return PHYSICSGLOBAL(gravity);
}


RwReal
RpClumpPhysicsGetMinSpeed(void)
{
    return PHYSICSGLOBAL(minSpeed);
}


RwChar *
RpClumpPhysicsGetCaption(void)
{
    return PHYSICSGLOBAL(caption);
}


/*
 *****************************************************************************
 */
RwBool
RpClumpPhysicsSetSpeed(RpClump *clump, const RwReal speed)
{
    if( LocalOffset > 0 )
    {
        PHYSICSLOCAL(clump, speed) = speed;

        return TRUE;
    }

    /*
     * Usually means the plugin has not been attached...
     */
    return FALSE;
}


/*
 *****************************************************************************
 */
RwBool
RpClumpPhysicsIncSpeed(RpClump *clump, const RwReal speed)
{
    if( LocalOffset > 0 )
    {
        PHYSICSLOCAL(clump, speed) += speed;

        return TRUE;
    }

    /*
     * Usually means the plugin has not been attached...
     */
    return FALSE;
}


/*
 *****************************************************************************
 */
RwReal
RpClumpPhysicsGetSpeed(RpClump *clump)
{
    if( LocalOffset > 0 )
    {
        return PHYSICSLOCAL(clump, speed);
    }

    /*
     * Usually means the plugin has not been attached...
     */
    return 0.0f;
}


/*
 *****************************************************************************
 */
RwReal
RpClumpPhysicsGetBounciness(RpClump *clump)
{
    if( LocalOffset > 0 )
    {
        return PHYSICSLOCAL(clump, bounciness);
    }

    /*
     * Usually means the plugin has not been attached...
     */
    return 0.0f;
}


/*
 *****************************************************************************
 */
RwBool
RpClumpPhysicsSetBounciness(RpClump *clump, const RwReal bounce)
{
    if( LocalOffset > 0 )
    {
        PHYSICSLOCAL(clump, bounciness) = bounce;

        return TRUE;
    }

    /*
     * Usually means the plugin has not been attached...
     */
    return FALSE;
}


/*
 *****************************************************************************
 */
RwBool
RpClumpPhysicsGetActive(RpClump *clump)
{
    if( LocalOffset > 0 )
    {
        return PHYSICSLOCAL(clump, active);
    }

    /*
     * Usually means the plugin has not been attached...
     */
    return FALSE;
}


/*
 *****************************************************************************
 */
RwBool
RpClumpPhysicsSetActive(RpClump *clump, const RwBool active)
{
    if( LocalOffset > 0 )
    {
        PHYSICSLOCAL(clump, active) = active;

        return TRUE;
    }

    /*
     * Usually means the plugin has not been attached...
     */
    return FALSE;
}

/*
*****************************************************************************
*/
