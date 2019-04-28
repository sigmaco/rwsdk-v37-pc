// rwstouch.cpp : Defines the entry point for the console application.
//


#include <stdio.h>

#include <rwcore.h>
#include <rpcollis.h>
#include <rpdmorph.h>
#include <rphanim.h>
#include <rtimport.h>
#include <rpmatfx.h>
#include <rpmipkl.h>
#include <rpmorph.h>
#include <rpltmap.h>
#include <rtquat.h>
#include <rpspline.h>
#include <rpskin.h>
#include <rtslerp.h>
#include <rpusrdat.h>
#include <rpworld.h>
#include <rtworld.h>
#include <rtvcat.h>
#include <rpanisot.h>
#include <rppatch.h>
#include <rtpitexd.h>
#include <rttoc.h>
#include <rppvs.h>
#include <rtbmp.h>
#include <rtpng.h>
#include <rtpitexd.h>
#include <rpprtstd.h>
#include <rpprtadv.h>
#include <rt2d.h>
#include <rt2danim.h>
#include <rptoon.h>
#include <rpuvanim.h>
#include <rpnormmap.h>
#include <rtcmpkey.h>
#include <rpuvanim.h>
#include <rpadc.h>

#include <string>
#include <vector>
using namespace std;

#include "shared.h"

RwTextureCallBackRead DefaultTextureReadCallBack;
RpWorld *World=NULL;
RwCamera *Camera=NULL;

// RWStream
//     Wrapper for RwStreams, simplifies exception handling / destruction
//
class RWStream
{
public:
    RWStream(RwStream *stream_) : stream(stream_), data(0)
    {
        if (!stream)
        {
            throw exception("Could not open stream");
        }
    };
    RWStream(RwStream *stream_, void *data_) : stream(stream_), data(data_) {};
    ~RWStream() { RwStreamClose(stream, data); }
    RwStream *get() { return stream; }
    const RwStream *get() const { return stream; }
private:
    // Disallow copying
    RWStream(const RWStream& rhs);
    RWStream& operator=(const RWStream& rhs);

    RwStream *stream;
    void *data;
};



// TextureReadCallBack
//    Allows materials to retain textures
static RwTexture *
TextureReadCallBack(const RwChar *name, const RwChar *mask)
{
   RwTexture *tex = (*DefaultTextureReadCallBack)(name, mask);

   if (!tex)
   {
      RwUInt32 rasterFlags = rwRASTERTYPETEXTURE | rwRASTERFORMAT8888;

      if (RwTextureGetMipmapping())
      {
          rasterFlags |= rwRASTERFORMATMIPMAP;
      }

      if (RwTextureGetAutoMipmapping())
      {
          rasterFlags |= rwRASTERFORMATAUTOMIPMAP;
      }

      RwRaster *raster
         = RwRasterCreate(
            32,32,32,
            rasterFlags
         );
      tex = RwTextureCreate(raster);
      RwTextureSetName(tex, name);

      // If there's a mask, we need to set it too
      if (mask)
      {
          RwTextureSetMaskName(tex, mask);
      }

      // Add to the texture dictionary (create if necessary)
      RwTexDictionary *_TexDictionary = RwTexDictionaryGetCurrent();


      if (_TexDictionary == 0)
      {
          _TexDictionary = RwTexDictionaryCreate ();

          RwTexDictionarySetCurrent (_TexDictionary);
      }


      RwTexDictionaryAddTexture(_TexDictionary,  tex);
   }

   return tex;
}

static void
InterceptTextureRead()
{
   DefaultTextureReadCallBack = RwTextureGetReadCallBack();
   RwTextureSetReadCallBack(TextureReadCallBack);
}

static void
ResetTextureRead()
{
   RwTextureSetReadCallBack(DefaultTextureReadCallBack);
}

static RpWorld *
CreateWorld(void)
{
    RpWorld *world;
    RwBBox bb;

    bb.inf.x = bb.inf.y = bb.inf.z = -100.0f;
    bb.sup.x = bb.sup.y = bb.sup.z = 100.0f;

    world = RpWorldCreate(&bb);

    return world;
}

/*
 *****************************************************************************
 */
static RwCamera *
CreateCamera(RpWorld *world)
{
    RwCamera *camera = RwCameraCreate();

    RwCameraSetFrame(camera, RwFrameCreate());

    RwCameraSetRaster(camera,
            RwRasterCreate(64, 64, 0, rwRASTERTYPECAMERA));

    if( camera )
    {
        RpWorldAddCamera(world, camera);

        RwCameraSetNearClipPlane(camera, 0.1f);
        RwCameraSetFarClipPlane(camera, 30.0f);

        return camera;
    }

    return NULL;
}

// read stubs
static RtDict *
UVAnimDictStreamRead(RwStream *stream)
{
	return RtDictSchemaStreamReadDict(RpUVAnimGetDictSchema(), stream);
}


static void
Touch(ToolAPI *api, char *filename)
{
    string outFileName = string(string(filename) + ".out");
    {
        RWStream input(RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename));

        try
        {
            RWStream output(
                RwStreamOpen(
                    rwSTREAMFILENAME,
                    rwSTREAMWRITE,
                    outFileName.c_str()
                )
            );
    
            // Do the conversion
            RwChunkHeaderInfo header;
            if (!RwStreamReadChunkHeaderInfo(input.get(), &header))
                throw exception("Could not read chunk header\n");
    
#define RWUPDATE(x)                                 \
    {                                               \
        x *my##x = x##StreamRead(input.get());      \
        if (!my##x)                                 \
            throw exception("Could not read "#x);   \
        if (!x##StreamWrite(my##x, output.get()))   \
        {                                           \
            x##Destroy(my##x);                      \
            throw exception("Could not write "#x);  \
        }                                           \
        x##Destroy(my##x);                          \
    }
#define RWUPDATE2(obj, reader, writer, destroyer)     \
    {                                                 \
        obj *my##obj = reader(input.get());           \
        if (!my##obj)                                 \
            throw exception("Could not read "#obj);   \
        if (!writer(my##obj, output.get()))           \
        {                                             \
            destroyer(my##obj);                       \
            throw exception("Could not write "#obj);  \
        }                                             \
        destroyer(my##obj);                           \
    }
            do {
                switch (header.type)
                {
                    case rwID_CLUMP:
                        RWUPDATE(RpClump);
                        break;
                    case rwID_ATOMIC:
                        RWUPDATE(RpAtomic);
                        break;
                    case rwID_WORLD:
                        RWUPDATE(RpWorld);
                        break;
                    case rwID_HANIMANIMATION:
                        RWUPDATE(RpHAnimAnimation);
                        break;
                    case rwID_DMORPHANIMATION:
                        RWUPDATE(RpDMorphAnimation);
                        break;
                    case rwID_TEXDICTIONARY:
                          // Note: rw_TEXDICTIONARY is handled, but RW does not
                          // guarantee compatibility between versions.
                        RWUPDATE(RwTexDictionary);
                        break;
        #if (defined(NULL_DRVMODEL_H) || defined(XBOX_DRVMODEL_H) || defined (GCN_DRVMODEL_H))
                    case rwID_MTEFFECTDICT:
                        RWUPDATE(RpMTEffectDict);
                        break;
        #endif  /* (defined(XBOX_DRVMODEL_H) || defined (GCN_DRVMODEL_H)) */
                    case rwID_PITEXDICTIONARY:
                        RWUPDATE2(RwTexDictionary, RtPITexDictionaryStreamRead,
                                  RtPITexDictionaryStreamWrite, RwTexDictionaryDestroy );
                        break;
                    case rwID_TOC:
                        ToolAPIReportWarning(api, "TOC discarded.\n");
                        RwStreamSkip(input.get(), header.length);
                        break;
					case rwID_UVANIMDICT:
						RWUPDATE2(RtDict, UVAnimDictStreamRead,
							      RtDictStreamWrite, RtDictDestroy);
						break;
					case rwID_UVANIMPLUGIN:
						RWUPDATE(RpUVAnim);
						break;
                    default:
                        /* Pass through */
                        {
                            RwChar buffer[80];
                            sprintf(buffer, "Chunk id 0x%x (length %d) not handled; passing through unchanged", header.type, header.length);
                            ToolAPIReportWarning(api, buffer);
    
                            if (!RwStreamWriteChunkHeader(output.get(), header.type, header.length))
                            {
                                throw exception("Could not write chunk header.");
                            }
                            vector<RwUInt8> readbuffer(header.length);
                            if (!(header.length==RwStreamRead(input.get(), &readbuffer[0], header.length)))
                            {
                                throw exception("Could not read chunk body.");
                            }
                            if (!RwStreamWrite(output.get(), &readbuffer[0], header.length))
                            {
                                throw exception("Could not write chunk body.");
                            }
    
                        }
                }
            } while(RwStreamReadChunkHeaderInfo(input.get(), &header));
        }
        catch (...)
        {
            remove(outFileName.c_str());
            throw;
        }
    }

#ifdef _WIN32
     // If we're a WIN32 build, rename the original file and new files to
     // <original.old> and <original> respectively.
    remove(string(string(filename)+".old").c_str());
    rename(filename, string(string(filename)+".old").c_str());
    rename(outFileName.c_str(), filename);
#endif
}

static RwInt32
ProcessFileWrapper(ToolAPI *api, RwChar *filename)
{
    try
    {
        Touch(api, filename);
    }
    catch (const exception &ex)
    {
        string error = string("Error, could not process file ")
                        + filename
                        + " - " + ex.what();
        ToolAPIReportError(api, error.c_str());
        return 1;
    }
    return 0;
}

extern "C" {

static RwInt32
ProcessFile(ToolAPI *api, RwChar *filename)
{
    return ProcessFileWrapper(api, filename);
}

RwInt32
AttachPlugins(ToolAPI *api)
{
    const RwUInt32 succeeded = 0;
    const RwUInt32 failed = 1;
    
    if( !RpWorldPluginAttach() )
    {
        return failed;
    }

    if( !RpPVSPluginAttach() )
    {
        return failed;
    }

    if( !RpCollisionPluginAttach() )
    {
        return failed;
    }

    if (!RpSkinPluginAttach())
    {
        return failed;
    }

    if( !RpHAnimPluginAttach() )
    {
        return failed;
    }

    if (!RpMorphPluginAttach())
    {
        return failed;
    }

    if (!RpDMorphPluginAttach())
    {
        return failed;
    }

    if (!RpPatchPluginAttach())
    {
        return failed;
    }

    if( !RpMatFXPluginAttach() )
    {
        return failed;
    }

    if (!RpLtMapPluginAttach())
    {
        return failed;
    }

    if( !RpAnisotPluginAttach() )
    {
        return failed;
    }

    if( !RpSplinePluginAttach() )
    {
        return failed;
    }

    if( !RpUserDataPluginAttach() )
    {
        return failed;
    }

    if ( !RtAnimInitialize() )
    {
        return failed;
    }

    if( !RpPTankPluginAttach() )
    {
        return failed;
    }

    if ( !RpPrtStdPluginAttach() )
    {
        return failed;
    }

    if ( !RpPrtAdvPluginAttach() )
    {
        return failed;
    }

    if ( !RpToonPluginAttach() )
    {
        return failed;
    }
#ifndef SKY2_DRVMODEL_H
    if ( !RpMipmapKLPluginAttach() )
    {
        return failed;
    }
#endif

    if ( !RpUVAnimPluginAttach() )
    {
        return failed;
    }

    if ( !RpNormMapPluginAttach() )
    {
        return failed;
    }

    if ( !RpADCPluginAttach() )
    {
        return failed;
    }

    return succeeded;
}

RwInt32 Startup(ToolAPI *api, RwChar *currentPath)
{
    /* The engine is set up...*/
    /* RegisterImageLoaders(); */
    RtCompressedKeyFrameRegister();

    World = CreateWorld();

    Camera = CreateCamera(World);

    Rt2dOpen(Camera);

    Rt2dAnimOpen();


    /* Retain textures even if they can't be loaded */
    InterceptTextureRead();

    return 0;
}

RwInt32 Shutdown(ToolAPI *api)
{
    RwRaster *ras = RwCameraGetRaster(Camera);
    RwCameraSetRaster(Camera, NULL);
    if (ras != NULL)
        RwRasterDestroy(ras);

    RpWorldRemoveCamera(World, Camera);
    RwCameraDestroy(Camera);

    RpWorldDestroy(World);

    Rt2dClose();
    Rt2dAnimClose();

    ResetTextureRead();

    return 0;
}

/*
 *
 */

int main(int argc, char* argv[])
{
    ToolAPI api;
    RwInt32 err;

    ToolAPIInitAPI(&api, "rwstouch",
                   "Update RenderWare files to current version. Supported "
                   "chunk IDs: rwID_CLUMP,  rwID_ATOMIC,  rwID_WORLD, "
                   "rwID_HANIMANIMATION,  rwID_DMORPHANIMATION,  "
                   "rwID_MTEFFECTDICT, rwID_PITEXDICTIONARY",
                   NULL, 0);

    api.userProcessFileFn = ProcessFile;
    api.userStartupFn = Startup;
    api.userShutdownFn = Shutdown;
    api.userPluginAttachFn = AttachPlugins;

    err = ToolAPIExecute(&api, argc, argv);

    ToolAPIShutdownAPI(&api);

    return (err);
}

}
