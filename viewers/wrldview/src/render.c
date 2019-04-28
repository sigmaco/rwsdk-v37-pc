
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
 * render.c
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjadj & Matt Reynolds.
 * Reviewed by: John Irwin (with substantial edits).
 *
 * Purpose: RenderWare3 BSP viewer.
 *
 ****************************************************************************/

#include "rwcore.h"
#include "rpworld.h"
#include "rppvs.h"
#include "rprandom.h"

#include "render.h"
#include "main.h"
#include "pvsgen.h"
#include "world.h"

typedef struct
{
    RwV3d *vertices;
    RwBool lengths;
} TriStripData;

static RwIm3DVertex *Im3DVertices = (RwIm3DVertex *)NULL;
static RwIm3DVertex *TripStripIm3DVertices = (RwIm3DVertex *)NULL;
static RwBool WorldSectorVisible = TRUE;

static RpWorldSectorCallBackRender PVSWorldSectorRenderCallback;

static RwRGBA Red     = {255,   0,   0, 255};
static RwRGBA Cyan    = {  0, 255, 255, 255};
static RwRGBA Green   = {  0, 255,   0, 255};
static RwRGBA Magenta = {255,   0, 255, 255};
static RwRGBA Blue    = {  0,   0, 255, 255};
static RwRGBA Yellow  = {255, 255,   0, 255};
static RwRGBA White   = {255, 255, 255, 255};

RwBool TrianglesOn = FALSE;
RwBool WireFrameOn = FALSE;
RwBool NormalsOn = FALSE;
RwBool WorldSectorsOn = FALSE;
RwBool SingleSectorOn = FALSE;
RwInt32 TriStripLength = 1;


/*
 *****************************************************************************
 */
void
FreeTriStripVertices(void)
{
    RwFree(TripStripIm3DVertices);

    return;
}


/*
 *****************************************************************************
 */
static RpMesh *
MeshGetNumberOfMeshVertices(RpMesh *mesh, 
                            RpMeshHeader * meshHeader __RWUNUSED__,
    void *data)
{
    static RwUInt32 numVerts = 0;

    if( mesh->numIndices > numVerts )
    {
        *(RwUInt32 *)data = mesh->numIndices;

        numVerts = mesh->numIndices;
    }

    return mesh;
}

static RpWorldSector *
SectorGetNumberOfMeshVertices(RpWorldSector *sector, void *data)
{
    RpWorldSectorForAllMeshes(sector, MeshGetNumberOfMeshVertices, data);

    return sector;
}

RwBool 
ResizeTriStripVertexArray(RpWorld *world)
{
    static RwUInt32 numIm3DVerts = 0;
    RwUInt32 numVerts = 0;

    /*
     * When resizing the tristrip immediate mode array we only need to
     * allocate space to hold the largest mesh, not all the meshes...
     */

    RpWorldForAllWorldSectors(world, SectorGetNumberOfMeshVertices,
        (void *)&numVerts);

    if( numVerts > numIm3DVerts )
    {
        RwIm3DVertex *vertices = (RwIm3DVertex *)NULL;

        vertices = (RwIm3DVertex *)RwRealloc(TripStripIm3DVertices, numVerts *
            sizeof(RwIm3DVertex), rwID_NAOBJECT);

        if( vertices )
        {
            TripStripIm3DVertices = vertices;

            numIm3DVerts = numVerts;

            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
void
FreeIm3DVertices(void)
{
    if( Im3DVertices )
    {
        RwFree(Im3DVertices);
    }

    return;
}


/*
 *****************************************************************************
 */
static RpWorldSector *
WorldSectorCompareSize(RpWorldSector *worldSector, void *data)
{
    RwInt32 *currentNumVerts = (RwInt32 *)data;
    RwInt32 temp;

    /*
     * For the triangle mesh...
     */
    temp = 6 * RpWorldSectorGetNumTriangles(worldSector);

    if( temp > *currentNumVerts )
    {
        *currentNumVerts = temp;
    }

    /*
     * For the vertex normals...
     */
    temp = 2 * RpWorldSectorGetNumVertices(worldSector);

    if( temp > *currentNumVerts )
    {
        *currentNumVerts = temp;
    }

    return worldSector;
}


RwBool 
ResizeIm3DVertexArray(RpWorld *world)
{
    RwInt32 numVerts;

    static RwInt32 numIm3DVerts = 0;

    /*
     * At least, we need 8 vertices for each world-sector bounding-box...
     */
    numVerts = 8;

    RpWorldForAllWorldSectors(world, 
        WorldSectorCompareSize, (void *)&numVerts);

    /*
     * Resize only if we need more memory...
     */
    if( numVerts > numIm3DVerts )
    {
        RwIm3DVertex *vertices = (RwIm3DVertex *)NULL;

        vertices = (RwIm3DVertex *)RwRealloc(Im3DVertices, 
            numVerts * sizeof(RwIm3DVertex), rwID_NAOBJECT);

        if( vertices )
        {
            Im3DVertices = vertices;

            numIm3DVerts = numVerts;

            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    return TRUE;
}


/*
 *****************************************************************************
 */
static void
RenderWorldSectorBoundingBox(RpWorldSector *worldSector)
{
    const RwBBox *bbox;
    RwIm3DVertex *vertex;
    RwRGBA color;
    RwInt32 i;

    static RwImVertexIndex indices[24] =
    {
        0, 1, 1, 3, 3, 2, 2, 0, 4, 5, 5, 7,
        7, 6, 6, 4, 0, 4, 1, 5, 2, 6, 3, 7
    };

    /*
     * If the world has PVS data, visible world-sectors are 
     * rendered in yellow, culled world-sectors in blue...
     */
    color = WorldSectorVisible ? Yellow : Blue;

    bbox = RpWorldSectorGetBBox(worldSector);

    vertex = Im3DVertices;

    for(i=0; i<8; i++)
    {
        RwIm3DVertexSetPos(vertex, 
            i & 1 ? bbox->sup.x : bbox->inf.x,
            i & 2 ? bbox->sup.y : bbox->inf.y,
            i & 4 ? bbox->sup.z : bbox->inf.z);

        RwIm3DVertexSetRGBA(vertex, 
            color.red, color.green, color.blue, color.alpha);

        vertex++;
    }

#ifdef SKY
    /*
     * Temp fix to stop im3d culling on PS2...
     */
    RpSkySelectTrueTLClipper(FALSE);

#endif /* SKY */

    if( RwIm3DTransform(Im3DVertices, 8, (RwMatrix *)NULL, rwIM3D_ALLOPAQUE) )
    {
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST, indices, 24);

        RwIm3DEnd();
    }

#ifdef SKY
    
    RpSkySelectTrueTLClipper(TRUE);

#endif /* SKY */

    return;
}


/*
 *****************************************************************************
 */
static void
RenderWorldSectorVertexNormals(RpWorldSector *worldSector)
{
    RwV3d *worldVertices;
    RpVertexNormal *worldNormals;
    RwIm3DVertex *vertex;
    RwInt32 numVertices, i;
    RwRGBA color;

    numVertices = RpWorldSectorGetNumVertices(worldSector);
    if( numVertices == 0 )
    {
        return;
    }

    worldVertices = worldSector->vertices;
    worldNormals = worldSector->normals;
    vertex = Im3DVertices;
    
    /*
     * If the world has PVS data, visible normals are 
     * rendered in green, culled normals in magenta...
     */
    color = WorldSectorVisible ? Green : Magenta;

    for(i=0; i<numVertices; i++)
    {
        RwV3d normal;

        RPV3DFROMVERTEXNORMAL(normal, *worldNormals);

        RwIm3DVertexSetPos(vertex, 
            worldVertices->x, worldVertices->y, worldVertices->z);

        RwIm3DVertexSetRGBA(vertex, 
            color.red, color.green, color.blue, color.alpha);

        vertex++;

        RwIm3DVertexSetPos(vertex, 
            worldVertices->x + NormalsScaleFactor * normal.x, 
            worldVertices->y + NormalsScaleFactor * normal.y,
            worldVertices->z + NormalsScaleFactor * normal.z);

        RwIm3DVertexSetRGBA(vertex, 
            color.red, color.green, color.blue, color.alpha);

        vertex++;

        worldVertices++;
        worldNormals++;
    }

    if( RwIm3DTransform(Im3DVertices, 
            numVertices*2, (RwMatrix *)NULL, rwIM3D_ALLOPAQUE) )
    {
        RwIm3DRenderPrimitive(rwPRIMTYPELINELIST);

        RwIm3DEnd();
    }

    return;
}


/*
 *****************************************************************************
 */
static void
RenderWorldSectorWireMesh(RpWorldSector *worldSector)
{
    RwV3d *worldVertices;
    RpTriangle *worldTriangles;
    RwIm3DVertex *vertex;
    RwInt32 numPolygons, i;
    RwRGBA color;

    numPolygons = RpWorldSectorGetNumTriangles(worldSector);
    if( numPolygons == 0 )
    {
        return;
    }

    worldVertices = worldSector->vertices;
    worldTriangles = worldSector->triangles;
    vertex = Im3DVertices;

    /*
     * If the world has PVS data, visible triangles are 
     * rendered in cyan, culled triangles in red...
     */
    color = WorldSectorVisible ? Cyan : Red;

    for(i=0; i<numPolygons; i++)
    {
        RwV3d vert[3];

        vert[0] = worldVertices[worldTriangles->vertIndex[0]];
        vert[1] = worldVertices[worldTriangles->vertIndex[1]];
        vert[2] = worldVertices[worldTriangles->vertIndex[2]];

        RwIm3DVertexSetPos(vertex, vert[0].x, vert[0].y, vert[0].z);
        RwIm3DVertexSetRGBA(vertex, 
            color.red, color.green, color.blue, color.alpha);
        vertex++;

        RwIm3DVertexSetPos(vertex, vert[1].x, vert[1].y, vert[1].z);
        RwIm3DVertexSetRGBA(vertex, 
            color.red, color.green, color.blue, color.alpha);
        vertex++;

        RwIm3DVertexSetPos(vertex, vert[1].x, vert[1].y, vert[1].z);
        RwIm3DVertexSetRGBA(vertex, 
            color.red, color.green, color.blue, color.alpha);
        vertex++;

        RwIm3DVertexSetPos(vertex, vert[2].x, vert[2].y, vert[2].z);
        RwIm3DVertexSetRGBA(vertex, 
            color.red, color.green, color.blue, color.alpha);
        vertex++;

        RwIm3DVertexSetPos(vertex, vert[2].x, vert[2].y, vert[2].z);
        RwIm3DVertexSetRGBA(vertex, 
            color.red, color.green, color.blue, color.alpha);
        vertex++;

        RwIm3DVertexSetPos(vertex, vert[0].x, vert[0].y, vert[0].z);
        RwIm3DVertexSetRGBA(vertex, 
            color.red, color.green, color.blue, color.alpha);
        vertex++;

        worldTriangles++;
    }

    if( RwIm3DTransform(Im3DVertices, 
            numPolygons*6, (RwMatrix *)NULL, rwIM3D_ALLOPAQUE) )
    {
        RwIm3DRenderPrimitive(rwPRIMTYPELINELIST);

        RwIm3DEnd();
    }

    return;
}


/*
 *****************************************************************************
 */
static RpWorldSector *
WorldSectorRenderCallback(RpWorldSector *worldSector)
{
    if( SingleSectorOn && (worldSector != CurrentWorldSector) )
    {
        return worldSector;
    }

    /*
     * If we've got here then this world sector lies within the current
     * camera's view frustum...
     */
    NumCameraWorldSectors++;

    NumCameraTriangles += RpWorldSectorGetNumTriangles(worldSector);

    /*
     * Determine whether we can see this world sector...
     */
    if( PVSOn )
    {
        /*
         * Is the PVS culling this sector...
         */
        WorldSectorVisible = RpPVSWorldSectorVisible(worldSector);
    }
    else
    {
        /*
         * If we're not using the PVS, this sector is in the view frustum, 
         * and must be rendered whether we can see it or not...
         */
        WorldSectorVisible = TRUE;
    }

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEGOURAUD);

    if( WorldSectorVisible )
    {
        NumPVSWorldSectors++;

        NumPVSTriangles += RpWorldSectorGetNumTriangles(worldSector);

        if( TrianglesOn )
        {
            PVSWorldSectorRenderCallback(worldSector);
        }
    }

    /*
     * Draw the world sector bounding-box, triangle mesh and 
     * vertex normals if required...
     */
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)NULL);
    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEFLAT);

    if( WorldSectorsOn )
    {
        RenderWorldSectorBoundingBox(worldSector);
    }

    if( WireFrameOn )
    {
        RenderWorldSectorWireMesh(worldSector);
    }

    if( NormalsOn )
    {
        RenderWorldSectorVertexNormals(worldSector);
    }

    return worldSector;
}


/*
 *****************************************************************************
 */
static void
RenderWorldBoundingBox(RpWorld *world)
{
    const RwBBox *bbox;
    RwIm3DVertex *vertex;
    RwRGBA color;
    RwInt32 i;

    static RwImVertexIndex indices[24] =
    {
        0, 1, 1, 3, 3, 2, 2, 0, 4, 5, 5, 7,
        7, 6, 6, 4, 0, 4, 1, 5, 2, 6, 3, 7
    };

    color = White;

    bbox = RpWorldGetBBox(world);

    vertex = Im3DVertices;

    for(i=0; i<8; i++)
    {
        RwIm3DVertexSetPos(vertex, 
            i & 1 ? bbox->sup.x : bbox->inf.x,
            i & 2 ? bbox->sup.y : bbox->inf.y,
            i & 4 ? bbox->sup.z : bbox->inf.z);

        RwIm3DVertexSetRGBA(vertex, 
            color.red, color.green, color.blue, color.alpha);

        vertex++;
    }

    if( RwIm3DTransform(Im3DVertices, 8, (RwMatrix *)NULL, rwIM3D_ALLOPAQUE) )
    {
        RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST, indices, 24);

        RwIm3DEnd();
    }
    
    return;
}


/*
 *****************************************************************************
 */
void
WorldRender(RpWorld *world, RwCamera *camera)
{
    if( SingleSectorOn )
    {
        /* 
         * Draw the world's bounding-box for reference...
         */
        RenderWorldBoundingBox(world);
    }

    /*
     * Reset rendering statistics...
     */
    NumCameraWorldSectors = 0;
    NumCameraTriangles = 0;
    NumPVSWorldSectors = 0;
    NumPVSTriangles = 0;

    if( PVSOn )
    {
        RwFrame *frame = (RwFrame *)NULL;

        /*
         * Switch-on PVS culling...
         */
        RpPVSHook(world);

        /*
         * ...daisy-chain with the PVS's render callback so we can determine
         * which sectors are being culled...
         */
        PVSWorldSectorRenderCallback = RpWorldGetSectorRenderCallBack(world);
        RpWorldSetSectorRenderCallBack(world, WorldSectorRenderCallback);

        /*
         * ...tell the PVS culling system where the camera is...
         */
        frame = RwCameraGetFrame(camera);
        RpPVSSetViewPosition(world, RwMatrixGetPos(RwFrameGetLTM(frame)));

        /*
         * ...attempt to render the world...
         */
        RpWorldRender(world);

        /*
         * ...return to the PVS's original render callback...
         */
        RpWorldSetSectorRenderCallBack(world, PVSWorldSectorRenderCallback);

        /*
         * ...and switch-off PVS culling.
         * We need to hook/unhook every frame so that we can decide at
         * any time whether to use PVS culling (via PVSOn). Otherwise,
         * we can just hook it and leave it permanently enabled...
         */
        RpPVSUnhook(world);
    }
    else
    {
        /*
         * In this case the render callback has not been overridden
         * so the callback does the default thing. We still intercept for
         * counting purposes...
         */
        PVSWorldSectorRenderCallback = RpWorldGetSectorRenderCallBack(world);
        RpWorldSetSectorRenderCallBack(world, WorldSectorRenderCallback);

        RpWorldRender(world);

        /*
         * ...return to the PVS's original render callback...
         */
        RpWorldSetSectorRenderCallBack(world, PVSWorldSectorRenderCallback);
    }

    return;
}

static RpMesh *
RenderTriStrip(RpMesh *mesh, 
               RpMeshHeader * meshHeader __RWUNUSED__, 
               void *data)
{
    RwV3d *vertices = ((TriStripData *)data)->vertices;
    RwBool lengths = ((TriStripData *)data)->lengths;
    RwImVertexIndex *currentIndex = mesh->indices;
    RwRGBA color;
    RwInt32 vertCounter  = 0;
    RwUInt32 i;
    RwInt32 j;

    color.red = color.green = color.blue = 0;
    color.alpha = 255;

    if( !lengths )
    {
        RpRandomSeed((RwUInt32)mesh);
    }

    /*
     * Process the index array, test for duplicate vertices...
     */
    for(i=0; i<mesh->numIndices; i++)
    {
        RwIm3DVertexSetPos(&TripStripIm3DVertices[i],
            vertices[*currentIndex].x, vertices[*currentIndex].y,
            vertices[*currentIndex].z);

        /*
         * Check indices for join-strip degenerates...
         */
        if( (i > 0) && ((*currentIndex) == (*(currentIndex-1))) )
        {
            /*
             * This is degenerate - start a new strip if we've got a
             * triangle...
             */
            if( vertCounter >= 3 )
            {
                if( lengths )
                {
                    if( (vertCounter-3)<TriStripLength )
                    {
                        color.red = (RwUInt8)(255 - (255 * (vertCounter - 3))
                            / TriStripLength);
                    }
                    else
                    {
                        color.red = 0;
                    }
                }
                else
                {
                    color.red = (RwUInt8)(RpRandom() % 200) + 50;
                    color.green = (RwUInt8)(RpRandom() % 200) + 50;
                    color.blue = (RwUInt8)(RpRandom() % 200) + 50;
                }

                /* Colour my verts */
                for(j=0; j<vertCounter; j++)
                {
                    RwIm3DVertexSetRGBA(&TripStripIm3DVertices[i - (j+1)],
                        color.red, color.green, color.blue, color.alpha);
                }

                /* Zero counter */
                vertCounter = 0;
            }
        }
        else
        {
            /* count vertices. Need 3 to make a triangle. */
            vertCounter++;
        }

        /* Point to the next source vertex */
        currentIndex++;
    }

    /* final geometry */
    currentIndex--;
    if(vertCounter >= 3)
    {
        if(lengths)
        {
            if((vertCounter-3)<TriStripLength)
            {
                color.red = 
                    (RwUInt8)(255 - (255*(vertCounter-3))/TriStripLength);
            }
            else
            {
                color.red = 0;
            }
        }
        else
        {
            color.red = (RwUInt8)(RpRandom() % 200) + 50;
            color.green = (RwUInt8)(RpRandom() % 200) + 50;
            color.blue = (RwUInt8)(RpRandom() % 200) + 50;
        }

        /* Colour my verts */
        for(j = 0; j < vertCounter; j++)
        {
            RwIm3DVertexSetRGBA(&TripStripIm3DVertices[i - (j+1)], color.red,
                color.green, color.blue, color.alpha);
        }
    }

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEFLAT);

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);

    if( RwIm3DTransform(TripStripIm3DVertices, mesh->numIndices, (RwMatrix *)NULL,
            rwIM3D_ALLOPAQUE) )
    {                         
        RwIm3DRenderPrimitive(rwPRIMTYPETRISTRIP); 
        RwIm3DEnd();
    }   

    return mesh;
}

RpWorldSector *
RenderWorldSectorTriStrip(RpWorldSector *sector, void *data)
{ 
    TriStripData info;
    
    info.lengths = (RwBool)data;
    info.vertices = sector->vertices;
    RpWorldSectorForAllMeshes(sector, RenderTriStrip, (void *)&info);

    return sector;
}

/*
 *****************************************************************************
 */
static RpMesh *
RenderMeshes(RpMesh *mesh, 
             RpMeshHeader * meshHeader __RWUNUSED__, 
             void *data)
{
    RwV3d *vertices = ((TriStripData *)data)->vertices;
    RwImVertexIndex *currentIndex = mesh->indices;
    RwRGBA color;
    RwUInt32 i;

    RpRandomSeed((RwUInt32)mesh);
    color.red = (RwUInt8)(RpRandom() % 200) + 50;
    color.green = (RwUInt8)(RpRandom() % 200) + 50;
    color.blue = (RwUInt8)(RpRandom() % 200) + 50;
    color.alpha = 255;

    /* Process the index array, test for duplicate vertices */
    for(i=0; i<mesh->numIndices; i++, currentIndex++)
    {
        RwIm3DVertexSetPos(&TripStripIm3DVertices[i],
            vertices[*currentIndex].x, vertices[*currentIndex].y,
            vertices[*currentIndex].z);

        RwIm3DVertexSetRGBA(&TripStripIm3DVertices[i], color.red, color.green,
            color.blue, color.alpha );
    }

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void *)rwSHADEMODEFLAT);

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void *)TRUE);
  
    if( RwIm3DTransform(TripStripIm3DVertices, mesh->numIndices, (RwMatrix *)NULL,
            rwIM3D_ALLOPAQUE) )
    {                         
        RwIm3DRenderPrimitive(rwPRIMTYPETRISTRIP);
        RwIm3DEnd();
    }   

    return mesh;
}

RpWorldSector *
RenderWorldSectorMesh(RpWorldSector *sector, void *data)
{
    TriStripData info;

    info.lengths = (RwBool)data;
    info.vertices = sector->vertices;
    RpWorldSectorForAllMeshes(sector, RenderMeshes, &info);

    return sector;
}

/*
 *****************************************************************************
 */
