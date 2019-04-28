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
 * winvideo.c
 *
 * Copyright (C) 2000 Criterion Technologies.
 *
 * Original author: Martin Slater.
 *                                                                         
 * Purpose: Plays splash screen from an AVI file.
 *                         
 ****************************************************************************/
#ifndef __SPLASHSCREEN__H_
#define __SPLASHSCREEN__H_

int PlaySplashScreen(HWND hwnd, HDC hdc, char *filename, RECT *r);
void aviaudioMessage(HWND hwnd, unsigned msg, WPARAM wParam, LPARAM lParam);


#endif // __SPLASHSCREEN__H_