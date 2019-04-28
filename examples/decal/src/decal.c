#include "rwcore.h"
#include "rpworld.h"
#include "rpcollis.h"
#include "rpptank.h"

#include "skeleton.h"
#include "menu.h"

#include "decal.h"

/* Local types */

typedef struct {
    RwReal smallestDist;
    RwV3d  normal;
} CollisionInfo;

/* Local variables */
static RpWorld* DecalWorld = NULL;
static RpAtomic* DecalAtomic = NULL;
static RwMatrix* decalList = NULL;
static RwUInt32 nbDecals;
static RwUInt32 currentDecal;
static RwTexture* decalTex = NULL;

#define CAMERA_SEARCH_DOWN 40.0f
#define NOT_FOUND 65535.0f
#define CAMERA_HEIGHT 1.0f
#define MAX_DECAL 200
#define EPSILON 0.000001

static RpWorld *
LoadWorld(void)
{
    RwStream *stream;
    RwChar *path;
    RpWorld *world;
    RpWorld *worldResult;

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
     * Now we can build the collision data
     */
    worldResult = RpCollisionWorldBuildData( world, NULL );

    return world;
}


RwBool
DecalInit(void)
{
    RwChar      *path = RsPathnameCreate(RWSTRING("models/"));
    RwRGBA      decalColor;
    RwTexCoords decalUVs[2];
    RwFrame     *frame;

    DecalWorld = LoadWorld();
    if( NULL == DecalWorld )
    {
        return FALSE;
    }

    /* Init pTank */
    DecalAtomic = RpPTankAtomicCreate( MAX_DECAL,
                                       rpPTANKDFLAGMATRIX |
                                       rpPTANKDFLAGCNSVTX2TEXCOORDS,
                                       0);

    if( NULL == DecalAtomic )
    {
        return FALSE;
    }

    /* Create and add a frame to the PTank */
    frame = RwFrameCreate();

    RwMatrixSetIdentity( RwFrameGetMatrix( frame ) );

    RpAtomicSetFrame( DecalAtomic,frame );

    decalList = (RwMatrix*) RwMalloc( MAX_DECAL * sizeof( RwMatrix ),
                                      rwID_NAOBJECT );
    if( NULL == decalList )
    {
        return FALSE;
    }

    nbDecals = 0;
    currentDecal = 0;

    /* Load the texture */
    RwImageSetPath(path);
    RsPathnameDestroy(path);
    decalTex = RwTextureRead(RWSTRING("blood"), NULL );
    if (!decalTex)
    {
        return (FALSE);
    }

    /* Set the appropriate parameters for the texture */
    RwTextureSetFilterMode( decalTex, rwFILTERLINEAR );

    /* Set the constant color */
    decalColor.red = 255;
    decalColor.green = 32;
    decalColor.blue = 32;
    decalColor.alpha = 254;

    RpPTankAtomicSetConstantColor( DecalAtomic, &decalColor );

    /* Set the constant texture coordinates */
    decalUVs[0].u = 0.0f;
    decalUVs[0].v = 0.0f;
    decalUVs[1].u = 1.0f;
    decalUVs[1].v = 1.0f;

    RpPTankAtomicSetConstantVtx2TexCoords( DecalAtomic, decalUVs );

    /* Set the texture */
    RpPTankAtomicSetTexture( DecalAtomic , decalTex );
    RpPTankAtomicSetVertexAlpha( DecalAtomic, TRUE );

    /* Set the blend mode */
    RpPTankAtomicSetBlendModes( DecalAtomic, rwBLENDONE, rwBLENDONE );
    return TRUE;
}

void
DecalClose(void)
{
    if( NULL != decalTex )
    {
        RwTextureDestroy( decalTex );
    }

    if( NULL != decalList )
    {
        RwFree( decalList );
    }

    if( NULL != DecalWorld )
    {
        RpWorldDestroy( DecalWorld );
    }

    if( NULL != DecalAtomic )
    {
        RwFrame *frame = RpAtomicGetFrame(DecalAtomic);

        if (frame != NULL)
        {
            RpAtomicSetFrame(DecalAtomic, NULL);
            RwFrameDestroy(frame);
        }

        RpPTankAtomicDestroy( DecalAtomic );
    }
}

RpWorld*
GetCurrentWorld(void)
{
    return DecalWorld;
}

static RpCollisionTriangle *
IntersectionDecalCallBack(RpIntersection *intersection __RWUNUSED__,
                          RpWorldSector *sector __RWUNUSED__,
                          RpCollisionTriangle *collTriangle,
                          RwReal distance,
                          void* data)
{
    CollisionInfo *collInfo = (CollisionInfo *)data;

    if( distance < collInfo->smallestDist )
    {
        collInfo->smallestDist = distance;
        collInfo->normal = collTriangle->normal;
    }

    return collTriangle;
}

static RwBool
FindDecalCollisionPoint(RwLine *line, RwV3d *collPoint, RwV3d *normal)
{
    RpIntersection Intersection;
    RpWorld* result;
    CollisionInfo collInfo;

    Intersection.t.line = (*line);
    Intersection.type = rpINTERSECTLINE;

    collInfo.smallestDist = NOT_FOUND;

    result = RpCollisionWorldForAllIntersections( DecalWorld,
                                                  &Intersection,
                                                  IntersectionDecalCallBack,
                                                  &collInfo );
    if( collInfo.smallestDist == NOT_FOUND )
    {
        return FALSE;
    }

    collPoint->x = line->start.x + collInfo.smallestDist * (line->end.x - line->start.x);
    collPoint->y = line->start.y + collInfo.smallestDist * (line->end.y - line->start.y);
    collPoint->z = line->start.z + collInfo.smallestDist * (line->end.z - line->start.z);

    *normal = collInfo.normal;

    return TRUE;
}

void
FindDecal(RwCamera *camera)
{
    RwV3d decalPos;
    RwV3d lineEnd;
    RwLine line;
    RwMatrix* LTM;
    RwFrame *cameraFrame;
    RwReal length;
    RwV3d   decalNormal;
    RwBool  triangleFound;

    cameraFrame = RwCameraGetFrame( camera );
    LTM = RwFrameGetLTM( cameraFrame );
    /* Scale the at vector to create a line from the point of view */
    RwV3dScale( &lineEnd, &LTM->at, CAMERA_SEARCH_DOWN );
    RwV3dAdd( &lineEnd, &lineEnd, &LTM->pos );
    line.start.x = LTM->pos.x;
    line.start.y = LTM->pos.y;
    line.start.z = LTM->pos.z;
    line.end.x = lineEnd.x;
    line.end.y = lineEnd.y;
    line.end.z = lineEnd.z;

    triangleFound = FindDecalCollisionPoint( &line, &decalPos, &decalNormal );

    if( triangleFound )
    {
        decalList[currentDecal].pos.x = decalPos.x;
        decalList[currentDecal].pos.y = decalPos.y;
        decalList[currentDecal].pos.z = decalPos.z;

        RwV3dIncrementScaled( &decalList[currentDecal].pos, &decalNormal, 0.01f);

        /* right */
        RwV3dCrossProduct(&decalList[currentDecal].right,
                          RwMatrixGetAt(RwFrameGetMatrix
                                        (RwCameraGetFrame
                                         (camera))), &decalNormal);

        /* up */
        RwV3dCrossProduct(&decalList[currentDecal].up, &decalList[currentDecal].right, &decalNormal);

        /* Get the right size for the decals */
        length = RwV3dLength(&decalList[currentDecal].right);
        if (length > 0.0f)
        {
            RwV3dScale(&decalList[currentDecal].right, &decalList[currentDecal].right, (1.0f / length));
        }

        length = RwV3dLength(&decalList[currentDecal].up);
        if (length > 0.0f)
        {
            RwV3dScale(&decalList[currentDecal].up, &decalList[currentDecal].up, (1.0f / length));
        }

        /* manage the number of decals and wrap if necessary */
        if( nbDecals < MAX_DECAL - 1)
        {
            nbDecals++;
        }

        currentDecal++;
        if( currentDecal == MAX_DECAL )
        {
            currentDecal = 0;
        }
    }
}

static void
RenderDecal(void)
{
    RpPTankLockStruct ptankLockStruct;
    RwUInt32 i;
    RwMatrix* decalMatrix;

    /*
     * Early out if no decals;
     */
    if( 0 == nbDecals) return;

    /*
     * We need to regenerate the particles per frame
     */
    RpPTankAtomicSetActiveParticlesCount( DecalAtomic, nbDecals );
    RpPTankAtomicLock( DecalAtomic, &ptankLockStruct, rpPTANKLFLAGMATRIX,
                       rpPTANKLOCKWRITE);
    decalMatrix = ( RwMatrix* ) ptankLockStruct.data;

    for( i =0; i < nbDecals ; i++ )
    {
        /* pos */
        decalMatrix->pos.x = decalList[i].pos.x;
        decalMatrix->pos.y = decalList[i].pos.y;
        decalMatrix->pos.z = decalList[i].pos.z;

        decalMatrix->right.x = decalList[i].right.x;
        decalMatrix->right.y = decalList[i].right.y;
        decalMatrix->right.z = decalList[i].right.z;

        decalMatrix->up.x = decalList[i].up.x;
        decalMatrix->up.y = decalList[i].up.y;
        decalMatrix->up.z = decalList[i].up.z;

        decalMatrix = (RwMatrix*)(((RwUInt32)decalMatrix) + ptankLockStruct.stride );
    }
    RpPTankAtomicUnlock( DecalAtomic );

    /* No culling, no writing to the ZBuffer */
    RwRenderStateSet( rwRENDERSTATECULLMODE, ( void* )rwCULLMODECULLNONE );
    RwRenderStateSet( rwRENDERSTATEZWRITEENABLE, (void *) FALSE );

    RpPTankAtomicSetBlendModes( DecalAtomic, rwBLENDONE, rwBLENDONE );

    /* render */
    RpAtomicRender( DecalAtomic );

    /* Reset old render states */
    RwRenderStateSet( rwRENDERSTATECULLMODE, ( void* )rwCULLMODECULLBACK );
    RwRenderStateSet( rwRENDERSTATEZWRITEENABLE, (void *) TRUE );
}

void
WorldRender(void)
{
    /* Enable back face culling for the world. */
    RwRenderStateSet( rwRENDERSTATECULLMODE, ( void* )rwCULLMODECULLBACK );
    RpWorldRender( DecalWorld );

    RenderDecal();
}


