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
 * view.h
 *
 * Copyright (C) 2001 Criterion Technologies.
 *
 * Original author: Alexandre Hadjdaj.
 *
 * Purpose: 2d Viewer base file.
 *
 ****************************************************************************/


#ifndef VIEW_H
#define VIEW_H

extern Rt2dObject *MainScene;

extern RwBool ObjectLoaded;

extern RwV2d TranslateXStep;
extern RwV2d TranslateYStep;
extern RwV2d RotateOrigin;

extern RwV2d Position;
extern RwInt32 WinHeight;
extern RwInt32 WinWidth;

extern RwBool CmdZoomIn;
extern RwBool CmdZoomOut;
extern RwBool ViewChanged;
extern RwBool justLoaded;

extern Rt2dMaestro *Maestro;
extern Rt2dObject *MaestroScene;
extern Rt2dObject *MaestroBBox;

#ifdef    __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern void
PreLoadObject(RwChar *filename);

extern RwBool
LoadObject(RwChar *filename);

extern RwBool
CreateViewer(RwCamera *camera);

extern void
DestroyViewer(void);

extern void
InputUpdateViewer(void);

extern void
UpdateViewer(RwReal deltaTime);

extern void
RenderViewer(RwCamera *camera);

#if (defined(SKY) || defined(_XBOX) || defined(DOLPHIN))
extern void InitializeMouseCursor(void);
extern void UpdateMouseCursor(void);
#endif  /* (defined(SKY) || defined(_XBOX) || defined(DOLPHIN)) */

#ifdef    __cplusplus
}
#endif /* __cplusplus */

#endif /* VIEW_H */
