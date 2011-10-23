/*
===========================================================================

Wolfenstein: Enemy Territory GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Wolfenstein: Enemy Territory GPL Source Code (Wolf ET Source Code).  

Wolf ET Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Wolf ET Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Wolf ET Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Wolf: ET Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Wolf ET Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

/*
** WIN_GLIMP.C
**
** This file contains ALL Win32 specific stuff having to do with the
** OpenGL refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** GLimp_EndFrame
** GLimp_Init
** GLimp_LogComment
** GLimp_Shutdown
**
** Note that the GLW_xxx functions are Windows specific GL-subsystem
** related functions that are relevant ONLY to win_glimp.c
*/
#include <assert.h>
#include <GL/glew.h>

#include "../rendererGL3/tr_local.h"
#include "../qcommon/qcommon.h"
#include "resource.h"
#include "win_local.h"
#include <GL/wglew.h>
#include "glw_win.h"



extern void     WG_CheckHardwareGamma(void);
extern void     WG_RestoreGamma(void);

static char	   *WinGetLastErrorLocal()
{
	static char buf[MAXPRINTMSG];
	
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			buf, 
			MAXPRINTMSG, NULL);
	
	return buf;
}

typedef enum
{
	RSERR_OK,

	RSERR_INVALID_FULLSCREEN,
	RSERR_INVALID_MODE,

	RSERR_UNKNOWN
} rserr_t;

#define TRY_PFD_SUCCESS     0
#define TRY_PFD_FAIL_SOFT   1
#define TRY_PFD_FAIL_HARD   2

static void     GLW_InitExtensions(void);
static rserr_t  GLW_SetMode(int mode, int colorbits, qboolean cdsFullscreen);

static WinVars_t* g_wvPtr = NULL;



//
// variable declarations
//
glwstate_t      glw_state;

cvar_t         *r_allowSoftwareGL;	// don't abort out if the pixelformat claims software
cvar_t         *r_maskMinidriver;	// allow a different dll name to be treated as if it were opengl32.dll

int             gl_NormalFontBase = 0;
static qboolean fontbase_init = qfalse;



/*
** GLW_StartDriverAndSetMode
*/
static qboolean GLW_StartDriverAndSetMode(int mode, int colorbits, qboolean cdsFullscreen)
{
	rserr_t         err;

	err = GLW_SetMode(r_mode->integer, colorbits, cdsFullscreen);

	switch (err)
	{
		case RSERR_INVALID_FULLSCREEN:
			ri.Printf(PRINT_ALL, "...WARNING: fullscreen unavailable in this mode\n");
			return qfalse;
		case RSERR_INVALID_MODE:
			ri.Printf(PRINT_ALL, "...WARNING: could not set the given mode (%d)\n", mode);
			return qfalse;
		default:
			break;
	}
	return qtrue;
}

/*
** ChoosePFD
**
** Helper function that replaces ChoosePixelFormat.
*/
#define MAX_PFDS 256

static int GLW_ChoosePFD(HDC hDC, PIXELFORMATDESCRIPTOR * pPFD)
{
	PIXELFORMATDESCRIPTOR pfds[MAX_PFDS + 1];
	int             maxPFD = 0;
	int             i;
	int             bestMatch = 0;

	ri.Printf(PRINT_ALL, "...GLW_ChoosePFD( %d, %d, %d )\n", (int)pPFD->cColorBits, (int)pPFD->cDepthBits,
			  (int)pPFD->cStencilBits);

	// count number of PFDs
	maxPFD = DescribePixelFormat(hDC, 1, sizeof(PIXELFORMATDESCRIPTOR), &pfds[0]);
	if(maxPFD > MAX_PFDS)
	{
		ri.Printf(PRINT_WARNING, "...numPFDs > MAX_PFDS (%d > %d)\n", maxPFD, MAX_PFDS);
		maxPFD = MAX_PFDS;
	}

	ri.Printf(PRINT_ALL, "...%d PFDs found\n", maxPFD - 1);

	// grab information
	for(i = 1; i <= maxPFD; i++)
	{
		DescribePixelFormat(hDC, i, sizeof(PIXELFORMATDESCRIPTOR), &pfds[i]);
	}

	// look for a best match
	for(i = 1; i <= maxPFD; i++)
	{
		// make sure this has hardware acceleration
		if((pfds[i].dwFlags & PFD_GENERIC_FORMAT) != 0)
		{
			if(!r_allowSoftwareGL->integer)
			{
				if(r_verbose->integer)
				{
					ri.Printf(PRINT_ALL, "...PFD %d rejected, software acceleration\n", i);
				}
				continue;
			}
		}

		// verify pixel type
		if(pfds[i].iPixelType != PFD_TYPE_RGBA)
		{
			if(r_verbose->integer)
			{
				ri.Printf(PRINT_ALL, "...PFD %d rejected, not RGBA\n", i);
			}
			continue;
		}

		// verify proper flags
		if(((pfds[i].dwFlags & pPFD->dwFlags) & pPFD->dwFlags) != pPFD->dwFlags)
		{
			if(r_verbose->integer)
			{
				ri.Printf(PRINT_ALL, "...PFD %d rejected, improper flags (%x instead of %x)\n", i, pfds[i].dwFlags,
						  pPFD->dwFlags);
			}
			continue;
		}

		// verify enough bits
		if(pfds[i].cDepthBits < 15)
		{
			continue;
		}
		if((pfds[i].cStencilBits < 4) && (pPFD->cStencilBits > 0))
		{
			continue;
		}

		//
		// selection criteria (in order of priority):
		//
		//  PFD_STEREO
		//  colorBits
		//  depthBits
		//  stencilBits
		//
		if(bestMatch)
		{
			// check stereo
			if((pfds[i].dwFlags & PFD_STEREO) && (!(pfds[bestMatch].dwFlags & PFD_STEREO)) && (pPFD->dwFlags & PFD_STEREO))
			{
				bestMatch = i;
				continue;
			}

			if(!(pfds[i].dwFlags & PFD_STEREO) && (pfds[bestMatch].dwFlags & PFD_STEREO) && (pPFD->dwFlags & PFD_STEREO))
			{
				bestMatch = i;
				continue;
			}

			// check color
			if(pfds[bestMatch].cColorBits != pPFD->cColorBits)
			{
				// prefer perfect match
				if(pfds[i].cColorBits == pPFD->cColorBits)
				{
					bestMatch = i;
					continue;
				}
				// otherwise if this PFD has more bits than our best, use it
				else if(pfds[i].cColorBits > pfds[bestMatch].cColorBits)
				{
					bestMatch = i;
					continue;
				}
			}

			// check depth
			if(pfds[bestMatch].cDepthBits != pPFD->cDepthBits)
			{
				// prefer perfect match
				if(pfds[i].cDepthBits == pPFD->cDepthBits)
				{
					bestMatch = i;
					continue;
				}
				// otherwise if this PFD has more bits than our best, use it
				else if(pfds[i].cDepthBits > pfds[bestMatch].cDepthBits)
				{
					bestMatch = i;
					continue;
				}
			}

			// check stencil
			if(pfds[bestMatch].cStencilBits != pPFD->cStencilBits)
			{
				// prefer perfect match
				if(pfds[i].cStencilBits == pPFD->cStencilBits)
				{
					bestMatch = i;
					continue;
				}
				// otherwise if this PFD has more bits than our best, use it
				else if((pfds[i].cStencilBits > pfds[bestMatch].cStencilBits) && (pPFD->cStencilBits > 0))
				{
					bestMatch = i;
					continue;
				}
			}
		}
		else
		{
			bestMatch = i;
		}
	}

	if(!bestMatch)
	{
		return 0;
	}

	if((pfds[bestMatch].dwFlags & PFD_GENERIC_FORMAT) != 0)
	{
		if(!r_allowSoftwareGL->integer)
		{
			ri.Printf(PRINT_ALL, "...no hardware acceleration found\n");
			return 0;
		}
		else
		{
			ri.Printf(PRINT_ALL, "...using software emulation\n");
		}
	}
	else if(pfds[bestMatch].dwFlags & PFD_GENERIC_ACCELERATED)
	{
		ri.Printf(PRINT_ALL, "...MCD acceleration found\n");
	}
	else
	{
		ri.Printf(PRINT_ALL, "...hardware acceleration found\n");
	}

	*pPFD = pfds[bestMatch];

	return bestMatch;
}

/*
** void GLW_CreatePFD
**
** Helper function zeros out then fills in a PFD
*/
static void GLW_CreatePFD(PIXELFORMATDESCRIPTOR * pPFD, int colorbits, int depthbits, int stencilbits, qboolean stereo)
{
	PIXELFORMATDESCRIPTOR src = {
		sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
		1,						// version number
		PFD_DRAW_TO_WINDOW |	// support window
			PFD_SUPPORT_OPENGL |	// support OpenGL
			PFD_DOUBLEBUFFER,	// double buffered
		PFD_TYPE_RGBA,			// RGBA type
		24,						// 24-bit color depth
		0, 0, 0, 0, 0, 0,		// color bits ignored
		0,						// no alpha buffer
		0,						// shift bit ignored
		0,						// no accumulation buffer
		0, 0, 0, 0,				// accum bits ignored
		24,						// 24-bit z-buffer
		8,						// 8-bit stencil buffer
		0,						// no auxiliary buffer
		PFD_MAIN_PLANE,			// main layer
		0,						// reserved
		0, 0, 0					// layer masks ignored
	};

	src.cColorBits = colorbits;
	src.cDepthBits = depthbits;
	src.cStencilBits = stencilbits;

	if(stereo)
	{
		ri.Printf(PRINT_ALL, "...attempting to use stereo\n");
		src.dwFlags |= PFD_STEREO;
		glConfig.stereoEnabled = qtrue;
	}
	else
	{
		glConfig.stereoEnabled = qfalse;
	}

	*pPFD = src;
}

/*
** GLW_MakeContext
*/
static int GLW_MakeContext(PIXELFORMATDESCRIPTOR * pPFD)
{
	int             pixelformat;

	//
	// don't putz around with pixelformat if it's already set (e.g. this is a soft
	// reset of the graphics system)
	//
	if(!glw_state.pixelFormatSet)
	{
		//
		// choose, set, and describe our desired pixel format.  If we're
		// using a minidriver then we need to bypass the GDI functions,
		// otherwise use the GDI functions.
		//
		if((pixelformat = GLW_ChoosePFD(glw_state.hDC, pPFD)) == 0)
		{
			ri.Printf(PRINT_ALL, "...GLW_ChoosePFD failed\n");
			return TRY_PFD_FAIL_SOFT;
		}
		ri.Printf(PRINT_ALL, "...PIXELFORMAT %d selected\n", pixelformat);

		DescribePixelFormat(glw_state.hDC, pixelformat, sizeof(*pPFD), pPFD);

		if(SetPixelFormat(glw_state.hDC, pixelformat, pPFD) == FALSE)
		{
			ri.Printf(PRINT_ALL, "...SetPixelFormat failed\n", glw_state.hDC);
			return TRY_PFD_FAIL_SOFT;
		}

		glw_state.pixelFormatSet = qtrue;
	}

	// startup the OpenGL subsystem by creating a context and making it current
	if(!glw_state.hGLRC)
	{
		ri.Printf(PRINT_ALL, "...creating GL context: ");
		if((glw_state.hGLRC = wglCreateContext(glw_state.hDC)) == 0)
		{
			ri.Printf(PRINT_ALL, "failed\n");

			return TRY_PFD_FAIL_HARD;
		}
		ri.Printf(PRINT_ALL, "succeeded\n");

		ri.Printf(PRINT_ALL, "...making context current: ");
		if(!wglMakeCurrent(glw_state.hDC, glw_state.hGLRC))
		{
			wglDeleteContext(glw_state.hGLRC);
			glw_state.hGLRC = NULL;
			ri.Printf(PRINT_ALL, "failed\n");
			return TRY_PFD_FAIL_HARD;
		}
		ri.Printf(PRINT_ALL, "succeeded\n");
	}

	return TRY_PFD_SUCCESS;
}


/*
** GLW_InitDriver
**
** - get a DC if one doesn't exist
** - create an HGLRC if one doesn't exist
*/
static qboolean GLW_InitDriver(int colorbits)
{
	int             tpfd;
	int             depthbits, stencilbits;
	static PIXELFORMATDESCRIPTOR pfd;	// save between frames since 'tr' gets cleared

	ri.Printf(PRINT_ALL, "Initializing OpenGL driver\n");

	// get a DC for our window if we don't already have one allocated
	if(glw_state.hDC == NULL)
	{
		ri.Printf(PRINT_ALL, "...getting DC: ");

		if((glw_state.hDC = GetDC(g_wvPtr->hWnd)) == NULL)
		{
			ri.Printf(PRINT_ALL, "failed\n");
			return qfalse;
		}
		ri.Printf(PRINT_ALL, "succeeded\n");
	}

	if(colorbits == 0)
	{
		colorbits = glw_state.desktopBitsPixel;
	}

	// implicitly assume Z-buffer depth == desktop color depth
	if(r_depthbits->integer == 0)
	{
		if(colorbits > 16)
		{
			depthbits = 24;
		}
		else
		{
			depthbits = 16;
		}
	}
	else
	{
		depthbits = r_depthbits->integer;
	}

	// do not allow stencil if Z-buffer depth likely won't contain it
	if(r_stencilbits->integer == 0)
	{
		stencilbits = 8;
	}
	else
	{
		stencilbits = r_stencilbits->integer;
	}
	if(depthbits < 24)
	{
		stencilbits = 0;
	}

	//
	// make two attempts to set the PIXELFORMAT
	//

	// first attempt: r_colorbits, depthbits, and r_stencilbits
	if(!glw_state.pixelFormatSet)
	{
		GLW_CreatePFD(&pfd, colorbits, depthbits, stencilbits, r_stereo->integer);
		if((tpfd = GLW_MakeContext(&pfd)) != TRY_PFD_SUCCESS)
		{
			if(tpfd == TRY_PFD_FAIL_HARD)
			{
				ri.Printf(PRINT_WARNING, "...failed hard\n");
				return qfalse;
			}

			// punt if we've already tried the desktop bit depth and no stencil bits
			if((r_colorbits->integer == glw_state.desktopBitsPixel) && (stencilbits == 0))
			{
				ReleaseDC(g_wvPtr->hWnd, glw_state.hDC);
				glw_state.hDC = NULL;

				ri.Printf(PRINT_ALL, "...failed to find an appropriate PIXELFORMAT\n");

				return qfalse;
			}

			// second attempt: desktop's color bits and no stencil
			if(colorbits > glw_state.desktopBitsPixel)
			{
				colorbits = glw_state.desktopBitsPixel;
			}
			GLW_CreatePFD(&pfd, colorbits, depthbits, 0, r_stereo->integer);
			if(GLW_MakeContext(&pfd) != TRY_PFD_SUCCESS)
			{
				if(glw_state.hDC)
				{
					ReleaseDC(g_wvPtr->hWnd, glw_state.hDC);
					glw_state.hDC = NULL;
				}

				ri.Printf(PRINT_ALL, "...failed to find an appropriate PIXELFORMAT\n");

				return qfalse;
			}
		}

		/*
		 ** report if stereo is desired but unavailable
		 */
		if(!(pfd.dwFlags & PFD_STEREO) && (r_stereo->integer != 0))
		{
			ri.Printf(PRINT_ALL, "...failed to select stereo pixel format\n");
			glConfig.stereoEnabled = qfalse;
		}
	}

	/*
	 ** store PFD specifics
	 */
	glConfig.colorBits = (int)pfd.cColorBits;
	glConfig.depthBits = (int)pfd.cDepthBits;
	glConfig.stencilBits = (int)pfd.cStencilBits;

	return qtrue;
}

/*
** GLW_CreateWindow
**
** Responsible for creating the Win32 window and initializing the OpenGL driver.
*/
#define WINDOW_STYLE    ( WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_VISIBLE )
static qboolean GLW_CreateWindow(int width, int height, int colorbits, qboolean cdsFullscreen)
{
	RECT            r;
	cvar_t         *vid_xpos, *vid_ypos;
	int             stylebits;
	int             x, y, w, h;
	int             exstyle;

	// register the window class if necessary
	if(!g_wvPtr->classRegistered)
	{
		WNDCLASS        wc;

		memset(&wc, 0, sizeof(wc));

		wc.style = 0;
		wc.lpfnWndProc = (WNDPROC) glw_state.wndproc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = g_wvPtr->hInstance;
		wc.hIcon = LoadIcon(g_wvPtr->hInstance, MAKEINTRESOURCE(IDI_ICON1));
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (void *)COLOR_GRAYTEXT;
		wc.lpszMenuName = 0;
		wc.lpszClassName = CLIENT_WINDOW_TITLE;

		if(!RegisterClass(&wc))
		{
			char			*error;

			error = WinGetLastErrorLocal();
			ri.Error(ERR_VID_FATAL, "GLW_CreateWindow: could not register window class: '%s'", error);
		}
		g_wvPtr->classRegistered = qtrue;
		ri.Printf(PRINT_ALL, "...registered window class\n");
	}

	// create the HWND if one does not already exist
	if(!g_wvPtr->hWnd)
	{
		// compute width and height
		r.left = 0;
		r.top = 0;
		r.right = width;
		r.bottom = height;

		if(cdsFullscreen)
		{
			exstyle = WS_EX_TOPMOST;
			stylebits = WS_POPUP | WS_VISIBLE | WS_SYSMENU;
		}
		else
		{
			exstyle = 0;
			stylebits = WINDOW_STYLE | WS_SYSMENU;
			AdjustWindowRect(&r, stylebits, FALSE);
		}

		w = r.right - r.left;
		h = r.bottom - r.top;

		if(cdsFullscreen)
		{
			x = 0;
			y = 0;
		}
		else
		{
			vid_xpos = ri.Cvar_Get("vid_xpos", "", 0);
			vid_ypos = ri.Cvar_Get("vid_ypos", "", 0);
			x = vid_xpos->integer;
			y = vid_ypos->integer;

			// adjust window coordinates if necessary
			// so that the window is completely on screen
			if(x < 0)
			{
				x = 0;
			}
			if(y < 0)
			{
				y = 0;
			}

			if(w < glw_state.desktopWidth && h < glw_state.desktopHeight)
			{
				if(x + w > glw_state.desktopWidth)
				{
					x = (glw_state.desktopWidth - w);
				}
				if(y + h > glw_state.desktopHeight)
				{
					y = (glw_state.desktopHeight - h);
				}
			}
		}

		g_wvPtr->hWnd = CreateWindowEx(exstyle, CLIENT_WINDOW_TITLE,
								   CLIENT_WINDOW_TITLE, stylebits, x, y, w, h, NULL, NULL, g_wvPtr->hInstance, NULL);

		if(!g_wvPtr->hWnd)
		{
			char* error = WinGetLastErrorLocal();
			ri.Error(ERR_VID_FATAL, "GLW_CreateWindow(): could not create window: '%s'", error);
		}

		ShowWindow(g_wvPtr->hWnd, SW_SHOW);
		UpdateWindow(g_wvPtr->hWnd);
		ri.Printf(PRINT_ALL, "...created window@%d,%d (%dx%d)\n", x, y, w, h);
	}
	else
	{
		ri.Printf(PRINT_ALL, "...window already present, CreateWindowEx skipped\n");
	}

	if(!GLW_InitDriver(colorbits))
	{
		ShowWindow(g_wvPtr->hWnd, SW_HIDE);
		DestroyWindow(g_wvPtr->hWnd);
		g_wvPtr->hWnd = NULL;

		return qfalse;
	}

	SetForegroundWindow(g_wvPtr->hWnd);
	SetFocus(g_wvPtr->hWnd);

	return qtrue;
}

static void PrintCDSError(int value)
{
	switch (value)
	{
		case DISP_CHANGE_RESTART:
			ri.Printf(PRINT_ALL, "restart required\n");
			break;
		case DISP_CHANGE_BADPARAM:
			ri.Printf(PRINT_ALL, "bad param\n");
			break;
		case DISP_CHANGE_BADFLAGS:
			ri.Printf(PRINT_ALL, "bad flags\n");
			break;
		case DISP_CHANGE_FAILED:
			ri.Printf(PRINT_ALL, "DISP_CHANGE_FAILED\n");
			break;
		case DISP_CHANGE_BADMODE:
			ri.Printf(PRINT_ALL, "bad mode\n");
			break;
		case DISP_CHANGE_NOTUPDATED:
			ri.Printf(PRINT_ALL, "not updated\n");
			break;
		default:
			ri.Printf(PRINT_ALL, "unknown error %d\n", value);
			break;
	}
}

/*
** GLW_SetMode
*/
static rserr_t GLW_SetMode(int mode, int colorbits, qboolean cdsFullscreen)
{
	HDC             hDC;
	const char     *win_fs[] = { "W", "FS" };
	int             cdsRet;
	DEVMODE         dm;

	// print out informational messages
	ri.Printf(PRINT_ALL, "...setting mode %d:", mode);
	if(!R_GetModeInfo(&glConfig.vidWidth, &glConfig.vidHeight, &glConfig.windowAspect, mode))
	{
		ri.Printf(PRINT_ALL, " invalid mode\n");
		return RSERR_INVALID_MODE;
	}
	ri.Printf(PRINT_ALL, " %d %d %s\n", glConfig.vidWidth, glConfig.vidHeight, win_fs[cdsFullscreen]);

	// check our desktop attributes
	hDC = GetDC(GetDesktopWindow());
	glw_state.desktopBitsPixel = GetDeviceCaps(hDC, BITSPIXEL);
	glw_state.desktopWidth = GetDeviceCaps(hDC, HORZRES);
	glw_state.desktopHeight = GetDeviceCaps(hDC, VERTRES);
	ReleaseDC(GetDesktopWindow(), hDC);

	// verify desktop bit depth
	if(glConfig.driverType != GLDRV_VOODOO)
	{
		if(glw_state.desktopBitsPixel < 15 || glw_state.desktopBitsPixel == 24)
		{
			if(colorbits == 0 || (!cdsFullscreen && colorbits >= 15))
			{
				if(MessageBox(NULL,
							  "It is highly unlikely that a correct\n"
							  "windowed display can be initialized with\n"
							  "the current desktop display depth.  Select\n"
							  "'OK' to try anyway.  Press 'Cancel' if you to quit.\n"
							  , "Low Desktop Color Depth", MB_OKCANCEL | MB_ICONEXCLAMATION) != IDOK)
				{
					return RSERR_INVALID_MODE;
				}
			}
		}
	}

	// do a CDS if needed
	if(cdsFullscreen)
	{
		memset(&dm, 0, sizeof(dm));

		dm.dmSize = sizeof(dm);

		dm.dmPelsWidth = glConfig.vidWidth;
		dm.dmPelsHeight = glConfig.vidHeight;
		dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

		if(r_displayRefresh->integer != 0)
		{
			dm.dmDisplayFrequency = r_displayRefresh->integer;
			dm.dmFields |= DM_DISPLAYFREQUENCY;
		}
		else
		{
			DEVMODE         dmode;

			if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmode))
			{
				dm.dmDisplayFrequency = dmode.dmDisplayFrequency;
				dm.dmFields |= DM_DISPLAYFREQUENCY;
			}
		}

		// try to change color depth if possible
		if(colorbits != 0)
		{
			if(glw_state.allowdisplaydepthchange)
			{
				dm.dmBitsPerPel = colorbits;
				dm.dmFields |= DM_BITSPERPEL;
				ri.Printf(PRINT_ALL, "...using colorsbits of %d\n", colorbits);
			}
			else
			{
				ri.Printf(PRINT_ALL, "WARNING:...changing depth not supported on Win95 < pre-OSR 2.x\n");
			}
		}
		else
		{
			ri.Printf(PRINT_ALL, "...using desktop display depth of %d\n", glw_state.desktopBitsPixel);
		}

		// if we're already in fullscreen then just create the window
		if(glw_state.cdsFullscreen)
		{
			ri.Printf(PRINT_ALL, "...already fullscreen, avoiding redundant CDS\n");

			if(!GLW_CreateWindow(glConfig.vidWidth, glConfig.vidHeight, colorbits, qtrue))
			{
				ri.Printf(PRINT_ALL, "...restoring display settings\n");
				ChangeDisplaySettings(0, 0);
				return RSERR_INVALID_MODE;
			}
		}

		// need to call CDS
		else
		{
			ri.Printf(PRINT_ALL, "...calling CDS: ");

			// try setting the exact mode requested, because some drivers don't report
			// the low res modes in EnumDisplaySettings, but still work
			if((cdsRet = ChangeDisplaySettings(&dm, CDS_FULLSCREEN)) == DISP_CHANGE_SUCCESSFUL)
			{
				ri.Printf(PRINT_ALL, "ok\n");

				if(!GLW_CreateWindow(glConfig.vidWidth, glConfig.vidHeight, colorbits, qtrue))
				{
					ri.Printf(PRINT_ALL, "...restoring display settings\n");
					ChangeDisplaySettings(0, 0);
					return RSERR_INVALID_MODE;
				}

				glw_state.cdsFullscreen = qtrue;
			}
			else
			{
				// the exact mode failed, so scan EnumDisplaySettings for the next largest mode
				DEVMODE         devmode;
				int             modeNum;

				ri.Printf(PRINT_ALL, "failed, ");

				PrintCDSError(cdsRet);

				ri.Printf(PRINT_ALL, "...trying next higher resolution:");

				// we could do a better matching job here...
				for(modeNum = 0;; modeNum++)
				{
					if(!EnumDisplaySettings(NULL, modeNum, &devmode))
					{
						modeNum = -1;
						break;
					}
					if(devmode.dmPelsWidth >= glConfig.vidWidth
					   && devmode.dmPelsHeight >= glConfig.vidHeight && devmode.dmBitsPerPel >= 15)
					{
						break;
					}
				}

				if(modeNum != -1 && (cdsRet = ChangeDisplaySettings(&devmode, CDS_FULLSCREEN)) == DISP_CHANGE_SUCCESSFUL)
				{
					ri.Printf(PRINT_ALL, " ok\n");
					if(!GLW_CreateWindow(glConfig.vidWidth, glConfig.vidHeight, colorbits, qtrue))
					{
						ri.Printf(PRINT_ALL, "...restoring display settings\n");
						ChangeDisplaySettings(0, 0);
						return RSERR_INVALID_MODE;
					}

					glw_state.cdsFullscreen = qtrue;
				}
				else
				{
					ri.Printf(PRINT_ALL, " failed, ");

					PrintCDSError(cdsRet);

					ri.Printf(PRINT_ALL, "...restoring display settings\n");
					ChangeDisplaySettings(0, 0);

					glw_state.cdsFullscreen = qfalse;
					glConfig.isFullscreen = qfalse;
					if(!GLW_CreateWindow(glConfig.vidWidth, glConfig.vidHeight, colorbits, qfalse))
					{
						return RSERR_INVALID_MODE;
					}
					return RSERR_INVALID_FULLSCREEN;
				}
			}
		}
	}
	else
	{
		if(glw_state.cdsFullscreen)
		{
			ChangeDisplaySettings(0, 0);
		}

		glw_state.cdsFullscreen = qfalse;
		if(!GLW_CreateWindow(glConfig.vidWidth, glConfig.vidHeight, colorbits, qfalse))
		{
			return RSERR_INVALID_MODE;
		}
	}

	// success, now check display frequency, although this won't be valid on Voodoo(2)
	memset(&dm, 0, sizeof(dm));
	dm.dmSize = sizeof(dm);
	if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
	{
		glConfig.displayFrequency = dm.dmDisplayFrequency;
	}

	// NOTE: this is overridden later on standalone 3Dfx drivers
	glConfig.isFullscreen = cdsFullscreen;

	return RSERR_OK;
}

/*
** GLW_InitExtensions
*/
static void GLW_InitExtensions(void)
{
	ri.Printf(PRINT_ALL, "Initializing OpenGL extensions\n");

	// WGL_EXT_swap_control
	if(WGLEW_EXT_swap_control)
	{
		ri.Printf(PRINT_ALL, "...using WGL_EXT_swap_control\n");
		r_swapInterval->modified = qtrue;	// force a set next frame
	}
	else
	{
		ri.Printf(PRINT_ALL, "...WGL_EXT_swap_control not found\n");
	}

	GL_CheckErrors();

	// GL_ARB_multitexture
	if(glConfig.driverType != GLDRV_OPENGL3)
	{
		if(GLEW_ARB_multitexture)
		{
			glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &glConfig.maxActiveTextures);

			if(glConfig.maxActiveTextures > 1)
			{
				ri.Printf(PRINT_ALL, "...using GL_ARB_multitexture\n");
			}
			else
			{
				ri.Error(ERR_VID_FATAL, "...not using GL_ARB_multitexture, < 2 texture units\n");
			}
		}
		else
		{
			ri.Error(ERR_VID_FATAL, "...GL_ARB_multitexture not found\n");
		}
	}

	// GL_ARB_multisample
	if(GLEW_ARB_multisample)
	{
		ri.Printf(PRINT_ALL, "...using GL_ARB_multisample\n");
	}
	else
	{
		ri.Error(ERR_VID_FATAL, "...GL_ARB_multisample not found\n");
	}
	

	// GL_ARB_depth_texture
	if(GLEW_ARB_depth_texture)
	{
		ri.Printf(PRINT_ALL, "...using GL_ARB_depth_texture\n");
	}
	else
	{
		ri.Error(ERR_VID_FATAL, "...GL_ARB_depth_texture not found\n");
	}

	// GL_ARB_texture_cube_map
	if(GLEW_ARB_texture_cube_map)
	{
		glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB, &glConfig2.maxCubeMapTextureSize);
		ri.Printf(PRINT_ALL, "...using GL_ARB_texture_cube_map\n");
	}
	else
	{
		ri.Error(ERR_VID_FATAL, "...GL_ARB_texture_cube_map not found\n");
	}
	GL_CheckErrors();

	// GL_ARB_vertex_program
	if(GLEW_ARB_vertex_program)
	{
		ri.Printf(PRINT_ALL, "...using GL_ARB_vertex_program\n");
	}
	else
	{
		ri.Error(ERR_VID_FATAL, "...GL_ARB_vertex_program not found\n");
	}

	// GL_ARB_vertex_buffer_object
	if(GLEW_ARB_vertex_buffer_object)
	{
		ri.Printf(PRINT_ALL, "...using GL_ARB_vertex_buffer_object\n");
	}
	else
	{
		ri.Error(ERR_VID_FATAL, "...GL_ARB_vertex_buffer_object not found\n");
	}

	// GL_ARB_occlusion_query
	glConfig2.occlusionQueryAvailable = qfalse;
	glConfig2.occlusionQueryBits = 0;
	if(GLEW_ARB_occlusion_query)
	{
		if(r_ext_occlusion_query->value)
		{
			glConfig2.occlusionQueryAvailable = qtrue;
			glGetQueryivARB(GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS, &glConfig2.occlusionQueryBits);
			ri.Printf(PRINT_ALL, "...using GL_ARB_occlusion_query\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_ARB_occlusion_query\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_ARB_occlusion_query not found\n");
	}
	GL_CheckErrors();

	// GL_ARB_shader_objects
	if(GLEW_ARB_shader_objects)
	{
		ri.Printf(PRINT_ALL, "...using GL_ARB_shader_objects\n");
	}
	else
	{
		ri.Error(ERR_VID_FATAL, "...GL_ARB_shader_objects not found\n");
	}

	// GL_ARB_vertex_shader
	if(GLEW_ARB_vertex_shader)
	{
		int				reservedComponents;

		GL_CheckErrors();
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB, &glConfig2.maxVertexUniforms); GL_CheckErrors();
		//glGetIntegerv(GL_MAX_VARYING_FLOATS_ARB, &glConfig2.maxVaryingFloats); GL_CheckErrors();
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS_ARB, &glConfig2.maxVertexAttribs); GL_CheckErrors();

		reservedComponents = 16 * 10; // approximation how many uniforms we have besides the bone matrices

		if(glConfig.driverType == GLDRV_MESA)
		{
			// HACK
			// restrict to number of vertex uniforms to 512 because of:
			// xreal.x86_64: nv50_program.c:4181: nv50_program_validate_data: Assertion `p->param_nr <= 512' failed

			glConfig2.maxVertexUniforms = Q_bound(0, glConfig2.maxVertexUniforms, 512);
		}

		glConfig2.maxVertexSkinningBones = (int) Q_bound(0.0, (Q_max(glConfig2.maxVertexUniforms - reservedComponents, 0) / 16), MAX_BONES);
		glConfig2.vboVertexSkinningAvailable = r_vboVertexSkinning->integer && ((glConfig2.maxVertexSkinningBones >= 12) ? qtrue : qfalse);

		ri.Printf(PRINT_ALL, "...using GL_ARB_vertex_shader\n");
	}
	else
	{
		ri.Error(ERR_VID_FATAL, "...GL_ARB_vertex_shader not found\n");
	}
	GL_CheckErrors();

	// GL_ARB_fragment_shader
	if(GLEW_ARB_fragment_shader)
	{
		ri.Printf(PRINT_ALL, "...using GL_ARB_fragment_shader\n");
	}
	else
	{
		ri.Error(ERR_VID_FATAL, "...GL_ARB_fragment_shader not found\n");
	}

	// GL_ARB_shading_language_100
	if(GLEW_ARB_shading_language_100)
	{
		Q_strncpyz(glConfig2.shadingLanguageVersion, (char*)glGetString(GL_SHADING_LANGUAGE_VERSION_ARB),
				   sizeof(glConfig2.shadingLanguageVersion));
		ri.Printf(PRINT_ALL, "...using GL_ARB_shading_language_100\n");
	}
	else
	{
		ri.Printf(ERR_VID_FATAL, "...GL_ARB_shading_language_100 not found\n");
	}
	GL_CheckErrors();

	// GL_ARB_texture_non_power_of_two
	glConfig2.textureNPOTAvailable = qfalse;
	if(GLEW_ARB_texture_non_power_of_two)
	{
		if(r_ext_texture_non_power_of_two->integer)
		{
			glConfig2.textureNPOTAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_ARB_texture_non_power_of_two\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_ARB_texture_non_power_of_two\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_ARB_texture_non_power_of_two not found\n");
	}

	// GL_ARB_draw_buffers
	glConfig2.drawBuffersAvailable = qfalse;
	if(GLEW_ARB_draw_buffers)
	{
		glGetIntegerv(GL_MAX_DRAW_BUFFERS_ARB, &glConfig2.maxDrawBuffers);

		if(r_ext_draw_buffers->integer)
		{
			glConfig2.drawBuffersAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_ARB_draw_buffers\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_ARB_draw_buffers\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_ARB_draw_buffers not found\n");
	}

	// GL_ARB_half_float_pixel
	glConfig2.textureHalfFloatAvailable = qfalse;
	if(GLEW_ARB_half_float_pixel)
	{
		if(r_ext_half_float_pixel->integer)
		{
			glConfig2.textureHalfFloatAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_ARB_half_float_pixel\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_ARB_half_float_pixel\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_ARB_half_float_pixel not found\n");
	}

	// GL_ARB_texture_float
	glConfig2.textureFloatAvailable = qfalse;
	if(GLEW_ARB_texture_float)
	{
		if(r_ext_texture_float->integer)
		{
			glConfig2.textureFloatAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_ARB_texture_float\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_ARB_texture_float\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_ARB_texture_float not found\n");
	}

	// GL_ARB_texture_compression
	glConfig.textureCompression = TC_NONE;
	if(GLEW_ARB_texture_compression)
	{
		if(r_ext_compressed_textures->integer)
		{
			glConfig2.ARBTextureCompressionAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_ARB_texture_compression\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_ARB_texture_compression\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_ARB_texture_compression not found\n");
	}

	// GL_ARB_vertex_array_object
	glConfig2.vertexArrayObjectAvailable = qfalse;
	if(GLEW_ARB_vertex_array_object)
	{
		if(r_ext_vertex_array_object->integer)
		{
			glConfig2.vertexArrayObjectAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_ARB_vertex_array_object\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_ARB_vertex_array_object\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_ARB_vertex_array_object not found\n");
	}

	// GL_EXT_texture_compression_s3tc
	if(GLEW_EXT_texture_compression_s3tc)
	{
		if(r_ext_compressed_textures->integer)
		{
			glConfig.textureCompression = TC_S3TC;
			ri.Printf(PRINT_ALL, "...using GL_EXT_texture_compression_s3tc\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_EXT_texture_compression_s3tc\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_EXT_texture_compression_s3tc not found\n");
	}

	// GL_EXT_texture3D
	glConfig2.texture3DAvailable = qfalse;
	if(GLEW_EXT_texture3D)
	{
		//if(r_ext_texture3d->value)
		{
			glConfig2.texture3DAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_EXT_texture3D\n");
		}
		/*
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_EXT_texture3D\n");
		}
		*/
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_EXT_texture3D not found\n");
	}

	// GL_EXT_stencil_wrap
	glConfig2.stencilWrapAvailable = qfalse;
	if(GLEW_EXT_stencil_wrap)
	{
		if(r_ext_stencil_wrap->value)
		{
			glConfig2.stencilWrapAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_EXT_stencil_wrap\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_EXT_stencil_wrap\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_EXT_stencil_wrap not found\n");
	}

	// GL_EXT_texture_filter_anisotropic
	glConfig2.textureAnisotropyAvailable = qfalse;
	if(GLEW_EXT_texture_filter_anisotropic)
	{
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &glConfig2.maxTextureAnisotropy);

		if(r_ext_texture_filter_anisotropic->value)
		{
			glConfig2.textureAnisotropyAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_EXT_texture_filter_anisotropic\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_EXT_texture_filter_anisotropic\n");
		}
	}
	else
	{
		ri.Cvar_Set("r_ext_texture_filter_anisotropic", "0");
		ri.Printf(PRINT_ALL, "...GL_EXT_texture_filter_anisotropic not found\n");
	}
	GL_CheckErrors();

	// GL_EXT_stencil_two_side
	if(GLEW_EXT_stencil_two_side)
	{
		if(r_ext_stencil_two_side->value)
		{
			ri.Printf(PRINT_ALL, "...using GL_EXT_stencil_two_side\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_EXT_stencil_two_side\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_EXT_stencil_two_side not found\n");
	}

	// GL_EXT_depth_bounds_test
	if(GLEW_EXT_depth_bounds_test)
	{
		if(r_ext_depth_bounds_test->value)
		{
			ri.Printf(PRINT_ALL, "...using GL_EXT_depth_bounds_test\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_EXT_depth_bounds_test\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_EXT_depth_bounds_test not found\n");
	}

	// GL_EXT_framebuffer_object
	glConfig2.framebufferObjectAvailable = qfalse;
	if(GLEW_EXT_framebuffer_object)
	{
		glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE_EXT, &glConfig2.maxRenderbufferSize);
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &glConfig2.maxColorAttachments);

		if(r_ext_framebuffer_object->value)
		{
			glConfig2.framebufferObjectAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_EXT_framebuffer_object\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_EXT_framebuffer_object\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_EXT_framebuffer_object not found\n");
	}
	GL_CheckErrors();

	// GL_EXT_packed_depth_stencil
	glConfig2.framebufferPackedDepthStencilAvailable = qfalse;
	if(GLEW_EXT_packed_depth_stencil && glConfig.driverType != GLDRV_MESA)
	{
		if(r_ext_packed_depth_stencil->integer)
		{
			glConfig2.framebufferPackedDepthStencilAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_EXT_packed_depth_stencil\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_EXT_packed_depth_stencil\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_EXT_packed_depth_stencil not found\n");
	}

	// GL_EXT_framebuffer_blit
	glConfig2.framebufferBlitAvailable = qfalse;
	if(GLEW_EXT_framebuffer_blit)
	{
		if(r_ext_framebuffer_blit->integer)
		{
			glConfig2.framebufferBlitAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_EXT_framebuffer_blit\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_EXT_framebuffer_blit\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_EXT_framebuffer_blit not found\n");
	}

	// GL_EXTX_framebuffer_mixed_formats
	/*
	glConfig2.framebufferMixedFormatsAvailable = qfalse;
	if(GLEW_EXTX_framebuffer_mixed_formats)
	{
		if(r_extx_framebuffer_mixed_formats->integer)
		{
			glConfig2.framebufferMixedFormatsAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_EXTX_framebuffer_mixed_formats\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_EXTX_framebuffer_mixed_formats\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_EXTX_framebuffer_mixed_formats not found\n");
	}
	*/

	// GL_ATI_separate_stencil
	if(GLEW_ATI_separate_stencil)
	{
		if(r_ext_separate_stencil->value)
		{
			ri.Printf(PRINT_ALL, "...using GL_ATI_separate_stencil\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_ATI_separate_stencil\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_ATI_separate_stencil not found\n");
	}

	// GL_SGIS_generate_mipmap
	glConfig2.generateMipmapAvailable = qfalse;
	if(Q_stristr(glConfig.extensions_string, "GL_SGIS_generate_mipmap"))
	{
		if(r_ext_generate_mipmap->value)
		{
			glConfig2.generateMipmapAvailable = qtrue;
			ri.Printf(PRINT_ALL, "...using GL_SGIS_generate_mipmap\n");
		}
		else
		{
			ri.Printf(PRINT_ALL, "...ignoring GL_SGIS_generate_mipmap\n");
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_SGIS_generate_mipmap not found\n");
	}

	// GL_GREMEDY_string_marker
	if(GLEW_GREMEDY_string_marker)
	{
		ri.Printf(PRINT_ALL, "...using GL_GREMEDY_string_marker\n");
	}
	else
	{
		ri.Printf(PRINT_ALL, "...GL_GREMEDY_string_marker not found\n");
	}
}

/*
** GLW_CheckOSVersion
*/
static qboolean GLW_CheckOSVersion(void)
{
#define OSR2_BUILD_NUMBER 1111

	OSVERSIONINFO   vinfo;

	vinfo.dwOSVersionInfoSize = sizeof(vinfo);

	glw_state.allowdisplaydepthchange = qfalse;

	if(GetVersionEx(&vinfo))
	{
		if(vinfo.dwMajorVersion > 4)
		{
			glw_state.allowdisplaydepthchange = qtrue;
		}
		else if(vinfo.dwMajorVersion == 4)
		{
			if(vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
			{
				glw_state.allowdisplaydepthchange = qtrue;
			}
			else if(vinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
			{
				if(LOWORD(vinfo.dwBuildNumber) >= OSR2_BUILD_NUMBER)
				{
					glw_state.allowdisplaydepthchange = qtrue;
				}
			}
		}
	}
	else
	{
		ri.Printf(PRINT_ALL, "GLW_CheckOSVersion() - GetVersionEx failed\n");
		return qfalse;
	}

	return qtrue;
}


static void GLDebugCallback(unsigned int source, unsigned int type, unsigned int id,
							unsigned int severity, int length,
							const char* message, void* userParam)
{

	// DebugOutputToFile(source, type, id, severity, message);

	{

		/*
             char debSource[16], debType[20], debSev[5];

             if(source == GL_DEBUG_SOURCE_API_ARB)

                    strcpy(debSource, "OpenGL");

             else if(source == GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB)

                    strcpy(debSource, "Windows");

             else if(source == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB)

                    strcpy(debSource, "Shader Compiler");

             else if(source == GL_DEBUG_SOURCE_THIRD_PARTY_ARB)

                    strcpy(debSource, "Third Party");

             else if(source == GL_DEBUG_SOURCE_APPLICATION_ARB)

                    strcpy(debSource, "Application");

             else if(source == GL_DEBUG_SOURCE_OTHER_ARB)

                    strcpy(debSource, "Other");

 

             if(type == GL_DEBUG_TYPE_ERROR_ARB)

                    strcpy(debType, "Error");

             else if(type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB)

                    strcpy(debType, "Deprecated behavior");

             else if(type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB)

                    strcpy(debType, "Undefined behavior");

             else if(type == GL_DEBUG_TYPE_PORTABILITY_ARB)

                    strcpy(debType, "Portability");

             else if(type == GL_DEBUG_TYPE_PERFORMANCE_ARB)

                    strcpy(debType, "Performance");

             else if(type == GL_DEBUG_TYPE_OTHER_ARB)

                    strcpy(debType, "Other");

 

             if(severity == GL_DEBUG_SEVERITY_HIGH_ARB)

                    strcpy(debSev, "High");

             else if(severity == GL_DEBUG_SEVERITY_MEDIUM_ARB)

                    strcpy(debSev, "Medium");

             else if(severity == GL_DEBUG_SEVERITY_LOW_ARB)

                    strcpy(debSev, "Low");
		*/
 

			// ri.Error(ERR_FATAL, "OpenGL Error: Source:%s\tType:%s\tID:%d\tSeverity:%s\tMessage:%s\n", debSource,debType,id,debSev,message);
		 ri.Error(ERR_FATAL, "OpenGL Error: Message:%s\n", message);


             //fclose(f);

       }
}



static void GLW_InitOpenGL3xContext()
{
	int				retVal;
	const char     *success[] = { "failed", "success" };

	if(!r_glCoreProfile->integer)
		return;

	// try to initialize an OpenGL 3.x context
	if(WGLEW_ARB_create_context || wglewIsSupported("WGL_ARB_create_context"))
	{
		//if(!g_wvPtr->openGL3ContextCreated)
		{
			int				attribs[256];	// should be really enough
			int				numAttribs;

			/*
			int             attribs[] =
			{
				WGL_CONTEXT_MAJOR_VERSION_ARB, r_glMinMajorVersion->integer,
				WGL_CONTEXT_MINOR_VERSION_ARB, r_glMinMinorVersion->integer,
				WGL_CONTEXT_FLAGS_ARB,
				WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,// | WGL_CONTEXT_DEBUG_BIT_ARB,
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
				0
			};
			*/

			memset(attribs, 0, sizeof(attribs));
			numAttribs = 0;

			attribs[numAttribs++] = WGL_CONTEXT_MAJOR_VERSION_ARB;
			attribs[numAttribs++] = r_glMinMajorVersion->integer;

			attribs[numAttribs++] = WGL_CONTEXT_MINOR_VERSION_ARB;
			attribs[numAttribs++] = r_glMinMinorVersion->integer;


			if(WGLEW_ARB_create_context_profile)
			{
				attribs[numAttribs++] = WGL_CONTEXT_FLAGS_ARB;

#if 0
				if(GLXEW_ARB_debug_output)
				{
					attribs[numAttribs++] = WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB |  WGL_CONTEXT_DEBUG_BIT_ARB;
				}
				else
#endif
				{
					attribs[numAttribs++] = WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
				}

				attribs[numAttribs++] = WGL_CONTEXT_PROFILE_MASK_ARB;
				attribs[numAttribs++] = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
			}

			// set current context to NULL
			retVal = wglMakeCurrent(glw_state.hDC, NULL) != 0;
			ri.Printf(PRINT_ALL, "...wglMakeCurrent( %p, %p ): %s\n", glw_state.hDC, NULL, success[retVal]);

			// delete HGLRC
			if(glw_state.hGLRC)
			{
				retVal = wglDeleteContext(glw_state.hGLRC) != 0;
				ri.Printf(PRINT_ALL, "...deleting standard GL context: %s\n", success[retVal]);
				glw_state.hGLRC = NULL;
			}

			ri.Printf(PRINT_ALL, "...initializing OpenGL %i.%i context ", r_glMinMajorVersion->integer, r_glMinMinorVersion->integer);

			glw_state.hGLRC = wglCreateContextAttribsARB(glw_state.hDC, 0, attribs);
			//g_wvPtr->hGLRC = wglCreateContextAttribsARB(glw_state.hDC, g_wvPtr->hGLRC, attribs);

			if(wglMakeCurrent(glw_state.hDC, glw_state.hGLRC))
			{
				//g_wvPtr->openGL3ContextCreated = qtrue;

				ri.Printf(PRINT_ALL, " done\n");
				glConfig.driverType = GLDRV_OPENGL3;

				/*
				if(GLEW_ARB_debug_output)
				{
					glDebugMessageCallbackARB(GLDebugCallback, NULL);
				}
				*/
			}
			else
			{
				ri.Error(ERR_VID_FATAL, "Could not initialize OpenGL %i.%i context\n"
										"Make sure your graphics card supports OpenGL %i.%i or newer",
										r_glMinMajorVersion->integer, r_glMinMinorVersion->integer,
										r_glMinMajorVersion->integer, r_glMinMinorVersion->integer);
			}
		}
		/*
		else
		{
			// set current context to NULL
			retVal = wglMakeCurrent(NULL, NULL) != 0;
			ri.Printf(PRINT_ALL, "...wglMakeCurrent( NULL, NULL ): %s\n", success[retVal]);

			// delete HGLRC
			if(glw_state.hGLRC)
			{
				retVal = wglDeleteContext(glw_state.hGLRC) != 0;
				ri.Printf(PRINT_ALL, "...deleting standard GL context: %s\n", success[retVal]);
				glw_state.hGLRC = NULL;
			}

			if(!wglMakeCurrent(glw_state.hDC, g_wvPtr->hGLRC))
			{
				ri.Error(ERR_VID_FATAL, "GLW_StartOpenGL() - could not reactivate OpenGL %i.%i context", r_glMinMajorVersion->integer, r_glMinMinorVersion->integer);
			}
		}
		*/
	}
	else
	{
		ri.Error(ERR_VID_FATAL, "Could not initialize OpenGL %i.%i context: no WGL_ARB_create_context\n"
								"Make sure your graphics card supports OpenGL %i.%i or newer",
								r_glMinMajorVersion->integer, r_glMinMinorVersion->integer,
								r_glMinMajorVersion->integer, r_glMinMinorVersion->integer);
	}
}

static void GLW_StartOpenGL()
{
	GLenum			glewResult;

	glConfig.driverType = GLDRV_ICD;

	// create the window and set up the context
	if(!GLW_StartDriverAndSetMode(r_mode->integer, r_colorbits->integer, r_fullscreen->integer))
	{
		// if we're on a 24/32-bit desktop and we're going fullscreen on an ICD,
		// try it again but with a 16-bit desktop
		if(r_colorbits->integer != 16 || r_fullscreen->integer != qtrue || r_mode->integer != 3)
		{
			if(!GLW_StartDriverAndSetMode(3, 16, qtrue))
			{
				ri.Error(ERR_VID_FATAL, "GLW_StartOpenGL() - could not load OpenGL subsystem");
			}
		}
	}

	glewResult = glewInit();
	if(GLEW_OK != glewResult)
	{
		// glewInit failed, something is seriously wrong
		ri.Error(ERR_VID_FATAL, "GLW_StartOpenGL() - could not load OpenGL subsystem: %s", glewGetErrorString(glewResult));
	}
	else
	{
		ri.Printf(PRINT_ALL, "Using GLEW %s\n", glewGetString(GLEW_VERSION));
	}

	GLW_InitOpenGL3xContext();
}

/*
** GLimp_EndFrame
*/
void GLimp_EndFrame(void)
{
	// swapinterval stuff
	if(r_swapInterval->modified)
	{
		r_swapInterval->modified = qfalse;

		if(!glConfig.stereoEnabled)
		{						// why?
			if(WGLEW_EXT_swap_control)
			{
				wglSwapIntervalEXT(r_swapInterval->integer);
			}
		}
	}


	// don't flip if drawing to front buffer
	if(Q_stricmp(r_drawBuffer->string, "GL_FRONT") != 0)
	{
		if((glConfig.driverType > GLDRV_ICD) && WGLEW_EXT_swap_control)
		{
			if(!SwapBuffers(glw_state.hDC))
			{
				ri.Error(ERR_VID_FATAL, "GLimp_EndFrame() - SwapBuffers() failed!\n");
			}
		}
		else
		{
			SwapBuffers(glw_state.hDC);
		}
	}

	// check logging
	//QGL_EnableLogging(r_logFile->integer);
}


/*
** GLimp_Init
**
** This is the platform specific OpenGL initialization function.  It
** is responsible for loading OpenGL, initializing it, setting
** extensions, creating a window of the appropriate size, doing
** fullscreen manipulations, etc.  Its overall responsibility is
** to make sure that a functional OpenGL subsystem is operating
** when it returns to the ref.
*/
void GLimp_Init(void)
{
	cvar_t         *lastValidRenderer = ri.Cvar_Get("r_lastValidRenderer", "(uninitialized)", CVAR_ARCHIVE);
	cvar_t         *cv;

	ri.Printf(PRINT_ALL, "Initializing OpenGL subsystem\n");

	// check OS version to see if we can do fullscreen display changes
	if(!GLW_CheckOSVersion())
	{
		ri.Error(ERR_VID_FATAL, "GLimp_Init() - incorrect operating system\n");
	}

	// RB: added
	g_wvPtr = ri.Sys_GetSystemHandles();
	if(!g_wvPtr)
	{
		ri.Error(ERR_VID_FATAL, "GLimp_Init() - could not receive WinVars_t g_wv\n");
	}

	// save off hInstance and wndproc
	cv = ri.Cvar_Get("win_hinstance", "", 0);
	sscanf(cv->string, "%i", (int *)&g_wvPtr->hInstance);

	cv = ri.Cvar_Get("win_wndproc", "", 0);
	sscanf(cv->string, "%i", (int *)&glw_state.wndproc);

	r_allowSoftwareGL = ri.Cvar_Get("r_allowSoftwareGL", "0", CVAR_LATCH);
	r_maskMinidriver = ri.Cvar_Get("r_maskMinidriver", "0", CVAR_LATCH);

	// load appropriate DLL and initialize subsystem
	GLW_StartOpenGL();

	GL_CheckErrors();

	ri.Printf(PRINT_ALL, "GL_VENDOR: '%s'\n", glGetString(GL_VENDOR));
	ri.Printf(PRINT_ALL, "GL_RENDERER: '%s'\n", glGetString(GL_RENDERER));
	ri.Printf(PRINT_ALL, "GL_VERSION: '%s'\n", glGetString(GL_VERSION));

	// get our config strings
	Q_strncpyz(glConfig.vendor_string, glGetString(GL_VENDOR), sizeof(glConfig.vendor_string));
	Q_strncpyz(glConfig.renderer_string, glGetString(GL_RENDERER), sizeof(glConfig.renderer_string));
	Q_strncpyz(glConfig.version_string, glGetString(GL_VERSION), sizeof(glConfig.version_string));

#if 0
	Q_strncpyz(glConfig.extensions_string, glGetString(GL_EXTENSIONS), sizeof(glConfig.extensions_string));
	// TTimo - safe check
	if(strlen(glGetString(GL_EXTENSIONS)) >= sizeof(glConfig.extensions_string))
	{
		Com_Printf(S_COLOR_YELLOW "WARNNING: GL extensions string too long (%d), truncated to %d\n",
				   strlen(glGetString(GL_EXTENSIONS)), sizeof(glConfig.extensions_string));
	}
#else
	
#endif

	GL_CheckErrors();

	glConfig.hardwareType = GLHW_GENERIC;

	//
	// NOTE: if changing cvars, do it within this block.  This allows them
	// to be overridden when testing driver fixes, etc. but only sets
	// them to their default state when the hardware is first installed/run.
	//
//	if(Q_stricmp(lastValidRenderer->string, glConfig.renderer_string))
//	{
//		glConfig.hardwareType = GLHW_GENERIC;
//
//		ri.Cvar_Set("r_textureMode", "GL_LINEAR_MIPMAP_NEAREST");
//
////----(SA)  FIXME: RETURN TO DEFAULT  Another id build change for DK/DM
//			ri.Cvar_Set("r_picmip", "1");	//----(SA)    was "1" // JPW NERVE back to 1
////----(SA)
//		}
//	}

	//
	// this is where hardware specific workarounds that should be
	// detected/initialized every startup should go.
	//
	if(Q_stristr(glConfig.renderer_string, "geforce"))
	{
		if(glConfig.driverType == GLDRV_OPENGL3)
		{
			glConfig.hardwareType = GLHW_NV_DX10;
		}
		else if(Q_stristr(glConfig.renderer_string, "8400") ||
		   Q_stristr(glConfig.renderer_string, "8500") ||
		   Q_stristr(glConfig.renderer_string, "8600") ||
		   Q_stristr(glConfig.renderer_string, "8800") ||
		   Q_stristr(glConfig.renderer_string, "9500") ||
		   Q_stristr(glConfig.renderer_string, "9600") ||
		   Q_stristr(glConfig.renderer_string, "9800") ||
		   Q_stristr(glConfig.renderer_string, "gts 240") ||
		   Q_stristr(glConfig.renderer_string, "gts 250") ||
		   Q_stristr(glConfig.renderer_string, "gtx 260") ||
		   Q_stristr(glConfig.renderer_string, "gtx 275") ||
		   Q_stristr(glConfig.renderer_string, "gtx 280") ||
		   Q_stristr(glConfig.renderer_string, "gtx 285") ||
		   Q_stristr(glConfig.renderer_string, "gtx 295") ||
		   Q_stristr(glConfig.renderer_string, "gt 320") ||
		   Q_stristr(glConfig.renderer_string, "gt 330") ||
		   Q_stristr(glConfig.renderer_string, "gt 340") ||
		   Q_stristr(glConfig.renderer_string, "gt 415") ||
		   Q_stristr(glConfig.renderer_string, "gt 420") ||
		   Q_stristr(glConfig.renderer_string, "gt 425") ||
		   Q_stristr(glConfig.renderer_string, "gt 430") ||
		   Q_stristr(glConfig.renderer_string, "gt 435") ||
		   Q_stristr(glConfig.renderer_string, "gt 440") ||
		   Q_stristr(glConfig.renderer_string, "gt 520") ||
		   Q_stristr(glConfig.renderer_string, "gt 525") ||
		   Q_stristr(glConfig.renderer_string, "gt 540") ||
		   Q_stristr(glConfig.renderer_string, "gt 550") ||
		   Q_stristr(glConfig.renderer_string, "gt 555") ||
		   Q_stristr(glConfig.renderer_string, "gts 450") ||
		   Q_stristr(glConfig.renderer_string, "gtx 460") ||
		   Q_stristr(glConfig.renderer_string, "gtx 470") ||
		   Q_stristr(glConfig.renderer_string, "gtx 480") ||
		   Q_stristr(glConfig.renderer_string, "gtx 485") ||
		   Q_stristr(glConfig.renderer_string, "gtx 560") ||
		   Q_stristr(glConfig.renderer_string, "gtx 570") ||
		   Q_stristr(glConfig.renderer_string, "gtx 580") ||
		   Q_stristr(glConfig.renderer_string, "gtx 590"))
			glConfig.hardwareType = GLHW_NV_DX10;
	}
	else if(Q_stristr(glConfig.renderer_string, "quadro fx"))
	{
		if(Q_stristr(glConfig.renderer_string, "3600"))
			glConfig.hardwareType = GLHW_NV_DX10;
	}
	else if(Q_stristr(glConfig.renderer_string, "rv770"))
	{
		glConfig.hardwareType = GLHW_ATI_DX10;
	}
	else if(Q_stristr(glConfig.renderer_string, "radeon hd"))
	{
		glConfig.hardwareType = GLHW_ATI_DX10;
	}
	else if(Q_stristr(glConfig.renderer_string, "eah4850") || Q_stristr(glConfig.renderer_string, "eah4870"))
	{
		glConfig.hardwareType = GLHW_ATI_DX10;
	}
	else if(Q_stristr(glConfig.renderer_string, "radeon"))
	{
		glConfig.hardwareType = GLHW_ATI;
	}

	ri.Cvar_Set("r_lastValidRenderer", glConfig.renderer_string);

	GLW_InitExtensions();
	WG_CheckHardwareGamma();

	GLW_InitExtensions();
}

/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.
*/
void GLimp_Shutdown(void)
{
//  const char *strings[] = { "soft", "hard" };
	const char     *success[] = { "failed", "success" };
	int             retVal;

	// FIXME: Brian, we need better fallbacks from partially initialized failures
	/*
	if(!wglMakeCurrent)
	{
		return;
	}
	*/

	ri.Printf(PRINT_ALL, "Shutting down OpenGL subsystem\n");

	// restore gamma.  We do this first because 3Dfx's extension needs a valid OGL subsystem
	WG_RestoreGamma();

	// release DC
	if(glw_state.hDC)
	{
		// delete HGLRC
		if(glw_state.hGLRC)
		{
			// set current context to NULL
			retVal = wglMakeCurrent(glw_state.hDC, NULL) != 0;
			ri.Printf(PRINT_ALL, "...wglMakeCurrent( %p, %p ): %s\n", glw_state.hDC, NULL, success[retVal]);

			retVal = wglDeleteContext(glw_state.hGLRC) != 0;
			ri.Printf(PRINT_ALL, "...deleting GL context: %s\n", success[retVal]);
			glw_state.hGLRC = NULL;
		}

		retVal = ReleaseDC(g_wvPtr->hWnd, glw_state.hDC) != 0;
		ri.Printf(PRINT_ALL, "...releasing DC: %s\n", success[retVal]);
		glw_state.hDC = NULL;
	}

	// destroy window
#if 1
	if(g_wvPtr->hWnd)
	{
		ri.Printf(PRINT_ALL, "...destroying window\n");
		ShowWindow(g_wvPtr->hWnd, SW_HIDE);
		DestroyWindow(g_wvPtr->hWnd);
		g_wvPtr->hWnd = NULL;
		glw_state.pixelFormatSet = qfalse;
	}
#endif

	// close the r_logFile
	if(glw_state.log_fp)
	{
		fclose(glw_state.log_fp);
		glw_state.log_fp = 0;
	}

	// reset display settings
	if(glw_state.cdsFullscreen)
	{
		ri.Printf(PRINT_ALL, "...resetting display\n");
		ChangeDisplaySettings(0, 0);
		glw_state.cdsFullscreen = qfalse;
	}

	// shutdown QGL subsystem
	//QGL_Shutdown();

	memset(&glConfig, 0, sizeof(glConfig));
	memset(&glState, 0, sizeof(glState));

	g_wvPtr = NULL;
}

/*
** GLimp_LogComment
*/
void GLimp_LogComment(char *comment)
{
	static char		buf[4096];

	if(r_logFile->integer && GLEW_GREMEDY_string_marker)
	{
		// copy string and ensure it has a trailing '\0'
		Q_strncpyz(buf, comment, sizeof(buf));

		glStringMarkerGREMEDY(strlen(buf), buf);
	}
}


/*
===========================================================

SMP acceleration

===========================================================
*/

HANDLE          renderCommandsEvent;
HANDLE          renderCompletedEvent;
HANDLE          renderActiveEvent;

void            (*glimpRenderThread) (void);

void GLimp_RenderThreadWrapper(void)
{
	glimpRenderThread();

	// unbind the context before we die
	wglMakeCurrent(glw_state.hDC, NULL);
}

/*
=======================
GLimp_SpawnRenderThread
=======================
*/
HANDLE          renderThreadHandle;
int             renderThreadId;
qboolean GLimp_SpawnRenderThread(void (*function) (void))
{

	renderCommandsEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	renderCompletedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	renderActiveEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	glimpRenderThread = function;

	renderThreadHandle = CreateThread(NULL,	// LPSECURITY_ATTRIBUTES lpsa,
									  0,	// DWORD cbStack,
									  (LPTHREAD_START_ROUTINE) GLimp_RenderThreadWrapper,	// LPTHREAD_START_ROUTINE lpStartAddr,
									  0,	// LPVOID lpvThreadParm,
									  0,	//   DWORD fdwCreate,
									  &renderThreadId);

	if(!renderThreadHandle)
	{
		return qfalse;
	}

	return qtrue;
}

static void    *smpData;
static int      wglErrors;

void           *GLimp_RendererSleep(void)
{
	void           *data;

	if(!wglMakeCurrent(glw_state.hDC, NULL))
	{
		wglErrors++;
	}

	ResetEvent(renderActiveEvent);

	// after this, the front end can exit GLimp_FrontEndSleep
	SetEvent(renderCompletedEvent);

	WaitForSingleObject(renderCommandsEvent, INFINITE);

	if(!wglMakeCurrent(glw_state.hDC, glw_state.hGLRC))
	{
		wglErrors++;
	}

	ResetEvent(renderCompletedEvent);
	ResetEvent(renderCommandsEvent);

	data = smpData;

	// after this, the main thread can exit GLimp_WakeRenderer
	SetEvent(renderActiveEvent);

	return data;
}


void GLimp_FrontEndSleep(void)
{
	WaitForSingleObject(renderCompletedEvent, INFINITE);

	if(!wglMakeCurrent(glw_state.hDC, glw_state.hGLRC))
	{
		wglErrors++;
	}
}


void GLimp_WakeRenderer(void *data)
{
	smpData = data;

	if(!wglMakeCurrent(glw_state.hDC, NULL))
	{
		wglErrors++;
	}

	// after this, the renderer can continue through GLimp_RendererSleep
	SetEvent(renderCommandsEvent);

	WaitForSingleObject(renderActiveEvent, INFINITE);
}
