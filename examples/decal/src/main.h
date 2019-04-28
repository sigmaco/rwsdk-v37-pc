#ifndef MAIN_H
#define MAIN_H

#define MAXCAMERASPEED 200.0f
#define MAXCAMERAROTSPEED 120.0f

#if defined(__cplusplus)
extern "C"
{
#endif /* defined(__cplusplus) */

extern RwReal   CameraPitchRate;
extern RwReal   CameraTurnRate;
extern RwReal   CameraSpeed;
extern RwReal   CameraStrafeSpeed;

extern void
AddDecal( void );

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */
    
#endif /* MAIN_H */
