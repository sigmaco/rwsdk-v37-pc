#include <rwcore.h>
#include <rpworld.h>

#include "skeleton.h"
#include "utils.h"

RpClump *
DffLoad(RwChar *filename)
{
    RwStream    *stream = NULL;
    RpClump     *clump = NULL;
    RwChar	    *pathName;

    /* Open stream */
    pathName = RsPathnameCreate(filename);
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, pathName);
    if (stream)
    {
        /* Find clump chunk */
        if (RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL))
        {
            /* Load clump */
            clump = RpClumpStreamRead(stream);
        }

        /* close the stream */
        RwStreamClose( stream, NULL );
    }


    RsPathnameDestroy(pathName);

    return (clump);
}

void
AtomicGetBBox(RpAtomic *atom, RwBBox *box)
{
    RpGeometry    *geom   = RpAtomicGetGeometry(atom);
    RwInt32        nVerts = RpGeometryGetNumVertices(geom);
    RpMorphTarget *morph  = RpGeometryGetMorphTarget(geom, 0);
    RwV3d         *verts  = RpMorphTargetGetVertices(morph);
    RwUInt16       i;

    box->inf = *verts;
    box->sup = *verts;

    for (verts++, i = 1; i < nVerts; i++, verts++)
    {
        if (verts->x < box->inf.x) box->inf.x = verts->x;
        if (verts->y < box->inf.y) box->inf.y = verts->y;
        if (verts->z < box->inf.z) box->inf.z = verts->z;
        if (verts->x > box->sup.x) box->sup.x = verts->x;
        if (verts->y > box->sup.y) box->sup.y = verts->y;
        if (verts->z > box->sup.z) box->sup.z = verts->z;
    }

    RwV3dScale(&box->sup, &box->sup, 1.09f);
    RwV3dScale(&box->inf, &box->inf, 1.09f);
}

void
DffSave(RpAtomic *atomic, char *filename)
{
    if (atomic)
    {
        RwStream *stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, filename);
        RpClump  *clump  = RpAtomicGetClump(atomic);

        if (stream && clump)
        {
            RpClumpStreamWrite(clump, stream);
            RsWarningMessage("Wrote model!");

            RwStreamClose(stream, NULL);
        }
        else
        {
            RsWarningMessage("Couldn't open stream!");
        }
    }
    else
    {
        RsWarningMessage("No atomic to export!");
    }
}


/* RwsLoadTOC function */

RtTOC *
RwsLoadTOC( RwChar *filename )
{
    RwChar      *pathName;

    RwStream    *stream;

    RtTOC       *rwsTOC;


    pathName = RsPathnameCreate( filename );
    if ( NULL == pathName )
    {
        return (void *)NULL;
    }

    rwsTOC = (RtTOC *)NULL;

    /* try to read the stream */
    stream = RwStreamOpen( rwSTREAMFILENAME,
                           rwSTREAMREAD,
                           pathName );
    if ( NULL != stream )
    {
        /* find and read the TOC */
        if ( FALSE != RwStreamFindChunk( stream,
                                         rwID_TOC,
                                         NULL,
                                         NULL ) )
        {
            rwsTOC = RtTOCStreamRead( stream );
        }

        RwStreamClose( stream, NULL );
    }

    RsPathnameDestroy( pathName );

    return rwsTOC;
}


/* RwsLoadWorldFromTOC function */

RpWorld *
RwsLoadWorldFromTOC( RwChar *filename,
                     RtTOC *toc,
                     AcceptTOCEntryCB acceptTOCEntry )
{
    RpWorld     *world;

    RwInt32     idx;


    world = (RpWorld *)NULL;
    for ( idx = 0; idx < RtTOCGetNumEntries( toc ); idx += 1 )
    {
        RtTOCEntry  *tocEntry;


        tocEntry = RtTOCGetEntry( toc, idx );

        /* we're only interested in worlds */
        if ( rwID_WORLD != tocEntry->id )
        {
            continue;
        }

        /* is this the world we want? */
        if ( FALSE != acceptTOCEntry( tocEntry ) )
        {
            RwChar              *pathName;

            RwStream            *stream;

            RwChunkHeaderInfo   chunkHdrInfo;


            pathName = RsPathnameCreate( filename );
            if ( NULL == pathName )
            {
                return (RpWorld *)NULL;
            }

            stream = RwStreamOpen( rwSTREAMFILENAME,
                                   rwSTREAMREAD,
                                   pathName );

            /* skip through the stream using the TOC entry's offset */
            RwStreamSkip( stream, tocEntry->offset );

            /* read the chunk as if we'd found it sequentially */
            if ( NULL != RwStreamReadChunkHeaderInfo( stream,
                                                      &chunkHdrInfo ) )
            {
                world = RpWorldStreamRead( stream );
            }

            RwStreamClose( stream, NULL );

            RsPathnameDestroy( pathName );

            break;
        }
    }

    return world;
}



/* RwsLoadWorld function */

RpWorld *
RwsLoadWorld(RwChar *filename)
{
    RpWorld  *world = NULL;
    RwStream *stream = NULL;
    RwChar	 *pathName;

    pathName = RsPathnameCreate(filename);

    /* try to read in the stream and treat as a world */
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, pathName);
    if (stream)
    {
        /* Find clump chunk */
        if (RwStreamFindChunk(stream, rwID_WORLD, NULL, NULL))
        {
            /* Load the world */
            world = RpWorldStreamRead(stream);
        }

        /* close the stream */
        RwStreamClose( stream, NULL );
    }

    RsPathnameDestroy(pathName);

    return world;
}


/* BspLoad function */

RpWorld *
BspLoad(RwChar *filename)
{
    RpWorld  *world = NULL;
    RwStream *stream = NULL;
    RwChar	 *pathName;

    pathName = RsPathnameCreate(filename);

    /* try to read in the stream and treat as a BSP */
    stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, pathName);
    if (stream)
    {
        /* Find clump chunk */
        if (RwStreamFindChunk(stream, rwID_WORLD, NULL, NULL))
        {
            /* Load the world */
            world = RpWorldStreamRead(stream);
        }

        /* close the stream */
        RwStreamClose( stream, NULL );
    }

    RsPathnameDestroy(pathName);

    return world;
}
