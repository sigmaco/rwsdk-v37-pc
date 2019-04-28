#ifndef DECAL_H
#define DECAL_H

#if defined(__cplusplus)
extern "C"
{
#endif /* defined(__cplusplus) */

extern RwBool
DecalInit(void);

extern void
DecalClose(void);

extern RpWorld*
GetCurrentWorld( void );

extern void
WorldRender(void);

extern RpWorldSector*
FindCameraInWorldSector( RwV3d *cameraPos );

extern RwV3d*
FindNewCameraPosition( RwV3d *cameraPos );

extern void
FindDecal( RwCamera* camera );

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* DECAL_H */
