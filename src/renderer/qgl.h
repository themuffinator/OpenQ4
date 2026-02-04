// Copyright (C) 2004 Id Software, Inc.
//
/*
** QGL.H
*/

#ifndef __QGL_H__
#define __QGL_H__

#include "../external/glew/glew.h"

// RAVEN BEGIN
// dluetscher: added some wgl calls to the XENON version, as well as the Windows version
typedef struct tagPIXELFORMATDESCRIPTOR PIXELFORMATDESCRIPTOR, *PPIXELFORMATDESCRIPTOR, FAR *LPPIXELFORMATDESCRIPTOR;
// RAVEN END

extern  int   ( WINAPI * qwglChoosePixelFormat )(HDC, CONST PIXELFORMATDESCRIPTOR * RESTRICT );
extern  int   ( WINAPI * qwglDescribePixelFormat) (HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);
extern  int   ( WINAPI * qwglGetPixelFormat)(HDC);
extern  BOOL  ( WINAPI * qwglSetPixelFormat)(HDC, int, CONST PIXELFORMATDESCRIPTOR * RESTRICT );
extern  BOOL  ( WINAPI * qwglSwapBuffers)(HDC);

// RAVEN BEGIN
// dluetscher: added block of functions from WGL ARB pbuffer and render texture extensions
typedef struct HPBUFFERARB__ *HPBUFFERARB;
extern BOOL  ( WINAPI * qwglBindTexImageARB)(HPBUFFERARB, int);
extern BOOL	 ( WINAPI * qwglChoosePixelFormatARB)(HDC hdc, const int * RESTRICT piAttribIList, const FLOAT * RESTRICT pfAttribFList, UINT nMaxFormats, int * RESTRICT piFormats, UINT * RESTRICT nNumFormats);
extern HPBUFFERARB ( WINAPI * qwglCreatePbufferARB)(HDC hDC, int iPixelFormat, int iWidth, int iHeight, const int * RESTRICT piAttribList);
extern BOOL  ( WINAPI * qwglDestroyPbufferARB)(HPBUFFERARB);
extern HDC	 ( WINAPI * qwglGetPbufferDCARB)(HPBUFFERARB);
extern int	 ( WINAPI * qwglReleasePbufferDCARB)(HPBUFFERARB, HDC);
extern BOOL  ( WINAPI * qwglReleaseTexImageARB)(HPBUFFERARB, int);
extern BOOL  ( WINAPI * qwglSetPbufferAttribARB)(HPBUFFERARB, const int * RESTRICT );
// RAVEN END

extern BOOL  ( WINAPI * qwglCopyContext)(HGLRC, HGLRC, UINT);
extern HGLRC ( WINAPI * qwglCreateContext)(HDC);
extern HGLRC ( WINAPI * qwglCreateLayerContext)(HDC, int);
extern BOOL  ( WINAPI * qwglDeleteContext)(HGLRC);
extern HGLRC ( WINAPI * qwglGetCurrentContext)(VOID);
extern HDC   ( WINAPI * qwglGetCurrentDC)(VOID);
extern PROC  ( WINAPI * qwglGetProcAddress)(LPCSTR);
extern BOOL  ( WINAPI * qwglMakeCurrent)(HDC, HGLRC);
extern BOOL  ( WINAPI * qwglShareLists)(HGLRC, HGLRC);

extern BOOL  ( WINAPI * qwglUseFontBitmaps)(HDC, DWORD, DWORD, DWORD);

extern BOOL  ( WINAPI * qwglUseFontOutlines)(HDC, DWORD, DWORD, DWORD, FLOAT,
                                           FLOAT, int, LPGLYPHMETRICSFLOAT);

extern BOOL ( WINAPI * qwglDescribeLayerPlane)(HDC, int, int, UINT,
                                            LPLAYERPLANEDESCRIPTOR);
extern int  ( WINAPI * qwglSetLayerPaletteEntries)(HDC, int, int, int,
                                                CONST COLORREF * RESTRICT );
extern int  ( WINAPI * qwglGetLayerPaletteEntries)(HDC, int, int, int,
                                                COLORREF * RESTRICT );
extern BOOL ( WINAPI * qwglRealizeLayerPalette)(HDC, int, BOOL);
extern BOOL ( WINAPI * qwglSwapLayerBuffers)(HDC, UINT);

#endif