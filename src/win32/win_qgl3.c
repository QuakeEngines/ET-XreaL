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
** QGL_WIN.C
**
** This file implements the operating system binding of GL to QGL function
** pointers.  When doing a port of Quake3 you must implement the following
** two functions:
**
** QGL_Init() - loads libraries, assigns function pointers, etc.
** QGL_Shutdown() - unloads libraries, NULLs function pointers
*/
#include <float.h>
#include <stddef.h>
#include "../rendererGL3/tr_local.h"
#include "glw_win.h"

void            QGL_EnableLogging(qboolean enable);

int             (WINAPI * qwglSwapIntervalEXT) (int interval);

BOOL(WINAPI * qwglGetDeviceGammaRamp3DFX) (HDC, LPVOID);
BOOL(WINAPI * qwglSetDeviceGammaRamp3DFX) (HDC, LPVOID);

int             (WINAPI * qwglChoosePixelFormat) (HDC, CONST PIXELFORMATDESCRIPTOR *);
int             (WINAPI * qwglDescribePixelFormat) (HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);
int             (WINAPI * qwglGetPixelFormat) (HDC);

BOOL(WINAPI * qwglSetPixelFormat) (HDC, int, CONST PIXELFORMATDESCRIPTOR *);
BOOL(WINAPI * qwglSwapBuffers) (HDC);

BOOL(WINAPI * qwglCopyContext) (HGLRC, HGLRC, UINT);
HGLRC(WINAPI * qwglCreateContext) (HDC);
HGLRC(WINAPI * qwglCreateLayerContext) (HDC, int);
BOOL(WINAPI * qwglDeleteContext) (HGLRC);
HGLRC(WINAPI * qwglGetCurrentContext) (VOID);
HDC(WINAPI * qwglGetCurrentDC) (VOID);
PROC(WINAPI * qwglGetProcAddress) (LPCSTR);
BOOL(WINAPI * qwglMakeCurrent) (HDC, HGLRC);
BOOL(WINAPI * qwglShareLists) (HGLRC, HGLRC);
BOOL(WINAPI * qwglUseFontBitmaps) (HDC, DWORD, DWORD, DWORD);

BOOL(WINAPI * qwglUseFontOutlines) (HDC, DWORD, DWORD, DWORD, FLOAT, FLOAT, int, LPGLYPHMETRICSFLOAT);

BOOL(WINAPI * qwglDescribeLayerPlane) (HDC, int, int, UINT, LPLAYERPLANEDESCRIPTOR);
int             (WINAPI * qwglSetLayerPaletteEntries) (HDC, int, int, int, CONST COLORREF *);
int             (WINAPI * qwglGetLayerPaletteEntries) (HDC, int, int, int, COLORREF *);

BOOL(WINAPI * qwglRealizeLayerPalette) (HDC, int, BOOL);
BOOL(WINAPI * qwglSwapLayerBuffers) (HDC, UINT);


// OpenGL 2.x core API
void            (APIENTRY * qglBindTexture) (GLenum target, GLuint texture);
void            (APIENTRY * qglBlendFunc) (GLenum sfactor, GLenum dfactor);
void            (APIENTRY * qglClear) (GLbitfield mask);
void            (APIENTRY * qglClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void            (APIENTRY * qglClearDepth) (GLclampd depth);
void            (APIENTRY * qglClearStencil) (GLint s);
void            (APIENTRY * qglColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void            (APIENTRY * qglCopyPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
void            (APIENTRY * qglCopyTexImage1D) (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y,
												GLsizei width, GLint border);
void            (APIENTRY * qglCopyTexImage2D) (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y,
												GLsizei width, GLsizei height, GLint border);
void            (APIENTRY * qglCopyTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
void            (APIENTRY * qglCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y,
												   GLsizei width, GLsizei height);
void            (APIENTRY * qglCullFace) (GLenum mode);
void            (APIENTRY * qglDeleteTextures) (GLsizei n, const GLuint * textures);
void            (APIENTRY * qglDepthFunc) (GLenum func);
void            (APIENTRY * qglDepthMask) (GLboolean flag);
void            (APIENTRY * qglDepthRange) (GLclampd zNear, GLclampd zFar);
void            (APIENTRY * qglDisable) (GLenum cap);
void            (APIENTRY * qglDrawArrays) (GLenum mode, GLint first, GLsizei count);
void            (APIENTRY * qglDrawBuffer) (GLenum mode);
void            (APIENTRY * qglDrawElements) (GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);
void            (APIENTRY * qglEnable) (GLenum cap);
void            (APIENTRY * qglFinish) (void);
void            (APIENTRY * qglFlush) (void);
void            (APIENTRY * qglFrontFace) (GLenum mode);
void            (APIENTRY * qglGenTextures) (GLsizei n, GLuint * textures);
void            (APIENTRY * qglGetBooleanv) (GLenum pname, GLboolean * params);

GLenum(APIENTRY * qglGetError) (void);
void            (APIENTRY * qglGetFloatv) (GLenum pname, GLfloat * params);
void            (APIENTRY * qglGetIntegerv) (GLenum pname, GLint * params);
const GLubyte  *(APIENTRY * qglGetString) (GLenum name);
void            (APIENTRY * qglGetTexParameterfv) (GLenum target, GLenum pname, GLfloat * params);
void            (APIENTRY * qglGetTexParameteriv) (GLenum target, GLenum pname, GLint * params);
void            (APIENTRY * qglHint) (GLenum target, GLenum mode);

GLboolean(APIENTRY * qglIsEnabled) (GLenum cap);
GLboolean(APIENTRY * qglIsTexture) (GLuint texture);
void            (APIENTRY * qglLineWidth) (GLfloat width);
void            (APIENTRY * qglPolygonMode) (GLenum face, GLenum mode);
void            (APIENTRY * qglPolygonOffset) (GLfloat factor, GLfloat units);
void            (APIENTRY * qglReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type,
											GLvoid * pixels);
void            (APIENTRY * qglScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
void            (APIENTRY * qglSelectBuffer) (GLsizei size, GLuint * buffer);
void            (APIENTRY * qglShadeModel) (GLenum mode);
void            (APIENTRY * qglStencilFunc) (GLenum func, GLint ref, GLuint mask);
void            (APIENTRY * qglStencilMask) (GLuint mask);
void            (APIENTRY * qglStencilOp) (GLenum fail, GLenum zfail, GLenum zpass);
void            (APIENTRY * qglTexImage1D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border,
											GLenum format, GLenum type, const GLvoid * pixels);
void            (APIENTRY * qglTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
											GLint border, GLenum format, GLenum type, const GLvoid * pixels);
void            (APIENTRY * qglTexParameterf) (GLenum target, GLenum pname, GLfloat param);
void            (APIENTRY * qglTexParameterfv) (GLenum target, GLenum pname, const GLfloat * params);
void            (APIENTRY * qglTexParameteri) (GLenum target, GLenum pname, GLint param);
void            (APIENTRY * qglTexParameteriv) (GLenum target, GLenum pname, const GLint * params);
void            (APIENTRY * qglTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format,
											   GLenum type, const GLvoid * pixels);
void            (APIENTRY * qglTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
											   GLsizei height, GLenum format, GLenum type, const GLvoid * pixels);
void            (APIENTRY * qglViewport) (GLint x, GLint y, GLsizei width, GLsizei height);

// GL_ARB_multitexture
void            (APIENTRY * qglActiveTextureARB) (GLenum texture);

// GL_ARB_texture_compression
void            (APIENTRY * qglCompressedTexImage3DARB) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
void            (APIENTRY * qglCompressedTexImage2DARB) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
void            (APIENTRY * qglCompressedTexImage1DARB) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
void            (APIENTRY * qglCompressedTexSubImage3DARB) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
void            (APIENTRY * qglCompressedTexSubImage2DARB) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
void            (APIENTRY * qglCompressedTexSubImage1DARB) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
void            (APIENTRY * qglGetCompressedTexImageARB) (GLenum target, GLint level, GLvoid *img);

// GL_ARB_vertex_program
void            (APIENTRY * qglVertexAttrib4fARB) (GLuint, GLfloat, GLfloat, GLfloat, GLfloat);
void            (APIENTRY * qglVertexAttrib4fvARB) (GLuint, const GLfloat *);
void            (APIENTRY * qglVertexAttribPointerARB) (GLuint index, GLint size, GLenum type, GLboolean normalized,
														GLsizei stride, const GLvoid * pointer);
void            (APIENTRY * qglEnableVertexAttribArrayARB) (GLuint index);
void            (APIENTRY * qglDisableVertexAttribArrayARB) (GLuint index);

// GL_ARB_vertex_buffer_object
void            (APIENTRY * qglBindBufferARB) (GLenum target, GLuint buffer);
void            (APIENTRY * qglDeleteBuffersARB) (GLsizei n, const GLuint * buffers);
void            (APIENTRY * qglGenBuffersARB) (GLsizei n, GLuint * buffers);

GLboolean		(APIENTRY * qglIsBufferARB) (GLuint buffer);
void            (APIENTRY * qglBufferDataARB) (GLenum target, GLsizeiptrARB size, const GLvoid * data, GLenum usage);
void            (APIENTRY * qglBufferSubDataARB) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid * data);
void            (APIENTRY * qglGetBufferSubDataARB) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid * data);

// GL_ARB_occlusion_query
void            (APIENTRY * qglGenQueriesARB) (GLsizei n, GLuint * ids);
void            (APIENTRY * qglDeleteQueriesARB) (GLsizei n, const GLuint * ids);

GLboolean(APIENTRY * qglIsQueryARB) (GLuint id);
void            (APIENTRY * qglBeginQueryARB) (GLenum target, GLuint id);
void            (APIENTRY * qglEndQueryARB) (GLenum target);
void            (APIENTRY * qglGetQueryivARB) (GLenum target, GLenum pname, GLint * params);
void            (APIENTRY * qglGetQueryObjectivARB) (GLuint id, GLenum pname, GLint * params);
void            (APIENTRY * qglGetQueryObjectuivARB) (GLuint id, GLenum pname, GLuint * params);

GLboolean(APIENTRY * qglUnmapBufferARB) (GLenum target);
void            (APIENTRY * qglGetBufferParameterivARB) (GLenum target, GLenum pname, GLint * params);
void            (APIENTRY * qglGetBufferPointervARB) (GLenum target, GLenum pname, GLvoid * *params);

// GL_ARB_shader_objects
void            (APIENTRY * qglDeleteObjectARB) (GLhandleARB obj);

GLhandleARB(APIENTRY * qglGetHandleARB) (GLenum pname);
void            (APIENTRY * qglDetachObjectARB) (GLhandleARB containerObj, GLhandleARB attachedObj);

GLhandleARB(APIENTRY * qglCreateShaderObjectARB) (GLenum shaderType);
void            (APIENTRY * qglShaderSourceARB) (GLhandleARB shaderObj, GLsizei count, const GLcharARB * *string,
												 const GLint * length);
void            (APIENTRY * qglCompileShaderARB) (GLhandleARB shaderObj);

GLhandleARB(APIENTRY * qglCreateProgramObjectARB) (void);
void            (APIENTRY * qglAttachObjectARB) (GLhandleARB containerObj, GLhandleARB obj);
void            (APIENTRY * qglLinkProgramARB) (GLhandleARB programObj);
void            (APIENTRY * qglUseProgramObjectARB) (GLhandleARB programObj);
void            (APIENTRY * qglValidateProgramARB) (GLhandleARB programObj);
void            (APIENTRY * qglUniform1fARB) (GLint location, GLfloat v0);
void            (APIENTRY * qglUniform2fARB) (GLint location, GLfloat v0, GLfloat v1);
void            (APIENTRY * qglUniform3fARB) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
void            (APIENTRY * qglUniform4fARB) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void            (APIENTRY * qglUniform1iARB) (GLint location, GLint v0);
void            (APIENTRY * qglUniform2iARB) (GLint location, GLint v0, GLint v1);
void            (APIENTRY * qglUniform3iARB) (GLint location, GLint v0, GLint v1, GLint v2);
void            (APIENTRY * qglUniform4iARB) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
void            (APIENTRY * qglUniform2fvARB) (GLint location, GLsizei count, const GLfloat * value);
void            (APIENTRY * qglUniform3fvARB) (GLint location, GLsizei count, const GLfloat * value);
void            (APIENTRY * qglUniform4fvARB) (GLint location, GLsizei count, const GLfloat * value);
void            (APIENTRY * qglUniform2ivARB) (GLint location, GLsizei count, const GLint * value);
void            (APIENTRY * qglUniform3ivARB) (GLint location, GLsizei count, const GLint * value);
void            (APIENTRY * qglUniform4ivARB) (GLint location, GLsizei count, const GLint * value);
void            (APIENTRY * qglUniformMatrix2fvARB) (GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
void            (APIENTRY * qglUniformMatrix3fvARB) (GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
void            (APIENTRY * qglUniformMatrix4fvARB) (GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
void            (APIENTRY * qglGetObjectParameterfvARB) (GLhandleARB obj, GLenum pname, GLfloat * params);
void            (APIENTRY * qglGetObjectParameterivARB) (GLhandleARB obj, GLenum pname, GLint * params);
void            (APIENTRY * qglGetInfoLogARB) (GLhandleARB obj, GLsizei maxLength, GLsizei * length, GLcharARB * infoLog);
void            (APIENTRY * qglGetAttachedObjectsARB) (GLhandleARB containerObj, GLsizei maxCount, GLsizei * count,
													   GLhandleARB * obj);
GLint(APIENTRY * qglGetUniformLocationARB) (GLhandleARB programObj, const GLcharARB * name);
void            (APIENTRY * qglGetActiveUniformARB) (GLhandleARB programObj, GLuint index, GLsizei maxIndex, GLsizei * length,
													 GLint * size, GLenum * type, GLcharARB * name);
void            (APIENTRY * qglGetUniformfvARB) (GLhandleARB programObj, GLint location, GLfloat * params);
void            (APIENTRY * qglGetUniformivARB) (GLhandleARB programObj, GLint location, GLint * params);
void            (APIENTRY * qglGetShaderSourceARB) (GLhandleARB obj, GLsizei maxLength, GLsizei * length, GLcharARB * source);

// GL_ARB_vertex_shader
void            (APIENTRY * qglBindAttribLocationARB) (GLhandleARB programObj, GLuint index, const GLcharARB * name);
void            (APIENTRY * qglGetActiveAttribARB) (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei * length,
													GLint * size, GLenum * type, GLcharARB * name);
GLint(APIENTRY * qglGetAttribLocationARB) (GLhandleARB programObj, const GLcharARB * name);

// GL_ARB_draw_buffers
void            (APIENTRY * qglDrawBuffersARB) (GLsizei n, const GLenum * bufs);

// GL_ARB_vertex_array_object
void			(APIENTRY * qglBindVertexArray) (GLuint array);
void			(APIENTRY * qglDeleteVertexArrays) (GLsizei n, const GLuint *arrays);
void			(APIENTRY * qglGenVertexArrays) (GLsizei n, GLuint *arrays);
GLboolean		(APIENTRY * qglIsVertexArray) (GLuint array);

// GL_EXT_texture3D
void			(APIENTRY * qglTexImage3DEXT) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void			(APIENTRY * qglTexSubImage3DEXT) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);

// GL_EXT_stencil_two_side
void            (APIENTRY * qglActiveStencilFaceEXT) (GLenum face);

// GL_ATI_separate_stencil
void            (APIENTRY * qglStencilFuncSeparateATI) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
void            (APIENTRY * qglStencilOpSeparateATI) (GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);

// GL_EXT_depth_bounds_test
void            (APIENTRY * qglDepthBoundsEXT) (GLclampd zmin, GLclampd zmax);

// GL_EXT_framebuffer_object
GLboolean(APIENTRY * qglIsRenderbufferEXT) (GLuint renderbuffer);
void            (APIENTRY * qglBindRenderbufferEXT) (GLenum target, GLuint renderbuffer);
void            (APIENTRY * qglDeleteRenderbuffersEXT) (GLsizei n, const GLuint * renderbuffers);
void            (APIENTRY * qglGenRenderbuffersEXT) (GLsizei n, GLuint * renderbuffers);
void            (APIENTRY * qglRenderbufferStorageEXT) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
void            (APIENTRY * qglGetRenderbufferParameterivEXT) (GLenum target, GLenum pname, GLint * params);

GLboolean(APIENTRY * qglIsFramebufferEXT) (GLuint framebuffer);
void            (APIENTRY * qglBindFramebufferEXT) (GLenum target, GLuint framebuffer);
void            (APIENTRY * qglDeleteFramebuffersEXT) (GLsizei n, const GLuint * framebuffers);
void            (APIENTRY * qglGenFramebuffersEXT) (GLsizei n, GLuint * framebuffers);

GLenum(APIENTRY * qglCheckFramebufferStatusEXT) (GLenum target);
void            (APIENTRY * qglFramebufferTexture1DEXT) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture,
														 GLint level);
void            (APIENTRY * qglFramebufferTexture2DEXT) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture,
														 GLint level);
void            (APIENTRY * qglFramebufferTexture3DEXT) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture,
														 GLint level, GLint zoffset);
void            (APIENTRY * qglFramebufferRenderbufferEXT) (GLenum target, GLenum attachment, GLenum renderbuffertarget,
															GLuint renderbuffer);
void            (APIENTRY * qglGetFramebufferAttachmentParameterivEXT) (GLenum target, GLenum attachment, GLenum pname,
																		GLint * params);
void            (APIENTRY * qglGenerateMipmapEXT) (GLenum target);

// GL_EXT_framebuffer_blit
void			(APIENTRY * qglBlitFramebufferEXT) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);


#if defined(WIN32)
// WGL_ARB_create_context
HGLRC(APIENTRY * qwglCreateContextAttribsARB) (HDC hdC, HGLRC hShareContext, const int *attribList);
#endif



static void     (APIENTRY * dllBindTexture) (GLenum target, GLuint texture);
static void     (APIENTRY * dllBlendFunc) (GLenum sfactor, GLenum dfactor);
static void     (APIENTRY * dllClear) (GLbitfield mask);
static void     (APIENTRY * dllClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
static void     (APIENTRY * dllClearDepth) (GLclampd depth);
static void     (APIENTRY * dllClearStencil) (GLint s);
static void     (APIENTRY * dllColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
static void     (APIENTRY * dllCopyPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
static void     (APIENTRY * dllCopyTexImage1D) (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y,
												GLsizei width, GLint border);
static void     (APIENTRY * dllCopyTexImage2D) (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y,
												GLsizei width, GLsizei height, GLint border);
static void     (APIENTRY * dllCopyTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
static void     (APIENTRY * dllCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y,
												   GLsizei width, GLsizei height);
static void     (APIENTRY * dllCullFace) (GLenum mode);
static void     (APIENTRY * dllDeleteTextures) (GLsizei n, const GLuint * textures);
static void     (APIENTRY * dllDepthFunc) (GLenum func);
static void     (APIENTRY * dllDepthMask) (GLboolean flag);
static void     (APIENTRY * dllDepthRange) (GLclampd zNear, GLclampd zFar);
static void     (APIENTRY * dllDisable) (GLenum cap);
static void     (APIENTRY * dllDrawArrays) (GLenum mode, GLint first, GLsizei count);
static void     (APIENTRY * dllDrawBuffer) (GLenum mode);
static void     (APIENTRY * dllDrawElements) (GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);
static void     (APIENTRY * dllEnable) (GLenum cap);
static void     (APIENTRY * dllFinish) (void);
static void     (APIENTRY * dllFlush) (void);
static void     (APIENTRY * dllFrontFace) (GLenum mode);
static void     (APIENTRY * dllGenTextures) (GLsizei n, GLuint * textures);
static void     (APIENTRY * dllGetBooleanv) (GLenum pname, GLboolean * params);
//static void     (APIENTRY * dllGetDoublev) (GLenum pname, GLdouble * params);

GLenum(APIENTRY * dllGetError) (void);
static void     (APIENTRY * dllGetFloatv) (GLenum pname, GLfloat * params);
static void     (APIENTRY * dllGetIntegerv) (GLenum pname, GLint * params);
const GLubyte  *(APIENTRY * dllGetString) (GLenum name);
static void     (APIENTRY * dllGetTexParameterfv) (GLenum target, GLenum pname, GLfloat * params);
static void     (APIENTRY * dllGetTexParameteriv) (GLenum target, GLenum pname, GLint * params);
static void     (APIENTRY * dllHint) (GLenum target, GLenum mode);

GLboolean(APIENTRY * dllIsEnabled) (GLenum cap);
GLboolean(APIENTRY * dllIsTexture) (GLuint texture);
static void     (APIENTRY * dllLineWidth) (GLfloat width);
static void     (APIENTRY * dllPolygonMode) (GLenum face, GLenum mode);
static void     (APIENTRY * dllPolygonOffset) (GLfloat factor, GLfloat units);
static void     (APIENTRY * dllReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type,
											GLvoid * pixels);
static void     (APIENTRY * dllScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
static void     (APIENTRY * dllStencilFunc) (GLenum func, GLint ref, GLuint mask);
static void     (APIENTRY * dllStencilMask) (GLuint mask);
static void     (APIENTRY * dllStencilOp) (GLenum fail, GLenum zfail, GLenum zpass);
static void     (APIENTRY * dllTexImage1D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border,
											GLenum format, GLenum type, const GLvoid * pixels);
static void     (APIENTRY * dllTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
											GLint border, GLenum format, GLenum type, const GLvoid * pixels);
static void     (APIENTRY * dllTexParameterf) (GLenum target, GLenum pname, GLfloat param);
static void     (APIENTRY * dllTexParameterfv) (GLenum target, GLenum pname, const GLfloat * params);
static void     (APIENTRY * dllTexParameteri) (GLenum target, GLenum pname, GLint param);
static void     (APIENTRY * dllTexParameteriv) (GLenum target, GLenum pname, const GLint * params);
static void     (APIENTRY * dllTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format,
											   GLenum type, const GLvoid * pixels);
static void     (APIENTRY * dllTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
											   GLsizei height, GLenum format, GLenum type, const GLvoid * pixels);
static void     (APIENTRY * dllViewport) (GLint x, GLint y, GLsizei width, GLsizei height);

static const char *BooleanToString(GLboolean b)
{
	if(b == GL_FALSE)
	{
		return "GL_FALSE";
	}
	else if(b == GL_TRUE)
	{
		return "GL_TRUE";
	}
	else
	{
		return "OUT OF RANGE FOR BOOLEAN";
	}
}

static const char *FuncToString(GLenum f)
{
	switch (f)
	{
		case GL_ALWAYS:
			return "GL_ALWAYS";
		case GL_NEVER:
			return "GL_NEVER";
		case GL_LEQUAL:
			return "GL_LEQUAL";
		case GL_LESS:
			return "GL_LESS";
		case GL_EQUAL:
			return "GL_EQUAL";
		case GL_GREATER:
			return "GL_GREATER";
		case GL_GEQUAL:
			return "GL_GEQUAL";
		case GL_NOTEQUAL:
			return "GL_NOTEQUAL";
		default:
			return "!!! UNKNOWN !!!";
	}
}

static const char *PrimToString(GLenum mode)
{
	static char     prim[1024];

	if(mode == GL_TRIANGLES)
	{
		strcpy(prim, "GL_TRIANGLES");
	}
	else if(mode == GL_TRIANGLE_STRIP)
	{
		strcpy(prim, "GL_TRIANGLE_STRIP");
	}
	else if(mode == GL_TRIANGLE_FAN)
	{
		strcpy(prim, "GL_TRIANGLE_FAN");
	}
	else if(mode == GL_QUADS)
	{
		strcpy(prim, "GL_QUADS");
	}
	else if(mode == GL_QUAD_STRIP)
	{
		strcpy(prim, "GL_QUAD_STRIP");
	}
	else if(mode == GL_POLYGON)
	{
		strcpy(prim, "GL_POLYGON");
	}
	else if(mode == GL_POINTS)
	{
		strcpy(prim, "GL_POINTS");
	}
	else if(mode == GL_LINES)
	{
		strcpy(prim, "GL_LINES");
	}
	else if(mode == GL_LINE_STRIP)
	{
		strcpy(prim, "GL_LINE_STRIP");
	}
	else if(mode == GL_LINE_LOOP)
	{
		strcpy(prim, "GL_LINE_LOOP");
	}
	else
	{
		sprintf(prim, "0x%x", mode);
	}

	return prim;
}

static const char *CapToString(GLenum cap)
{
	static char     buffer[1024];

	switch (cap)
	{
		case GL_TEXTURE_2D:
			return "GL_TEXTURE_2D";
		case GL_BLEND:
			return "GL_BLEND";
		case GL_DEPTH_TEST:
			return "GL_DEPTH_TEST";
		case GL_CULL_FACE:
			return "GL_CULL_FACE";
		case GL_CLIP_PLANE0:
			return "GL_CLIP_PLANE0";
		case GL_COLOR_ARRAY:
			return "GL_COLOR_ARRAY";
		case GL_TEXTURE_COORD_ARRAY:
			return "GL_TEXTURE_COORD_ARRAY";
		case GL_VERTEX_ARRAY:
			return "GL_VERTEX_ARRAY";
		case GL_ALPHA_TEST:
			return "GL_ALPHA_TEST";
		case GL_STENCIL_TEST:
			return "GL_STENCIL_TEST";
		default:
			sprintf(buffer, "0x%x", cap);
	}

	return buffer;
}

static const char *TypeToString(GLenum t)
{
	switch (t)
	{
		case GL_BYTE:
			return "GL_BYTE";
		case GL_UNSIGNED_BYTE:
			return "GL_UNSIGNED_BYTE";
		case GL_SHORT:
			return "GL_SHORT";
		case GL_UNSIGNED_SHORT:
			return "GL_UNSIGNED_SHORT";
		case GL_INT:
			return "GL_INT";
		case GL_UNSIGNED_INT:
			return "GL_UNSIGNED_INT";
		case GL_FLOAT:
			return "GL_FLOAT";
		case GL_DOUBLE:
			return "GL_DOUBLE";
		default:
			return "!!! UNKNOWN !!!";
	}
}

static void APIENTRY logBindTexture(GLenum target, GLuint texture)
{
	fprintf(glw_state.log_fp, "glBindTexture( 0x%x, %u )\n", target, texture);
	dllBindTexture(target, texture);
}

static void BlendToName(char *n, GLenum f)
{
	switch (f)
	{
		case GL_ONE:
			strcpy(n, "GL_ONE");
			break;
		case GL_ZERO:
			strcpy(n, "GL_ZERO");
			break;
		case GL_SRC_ALPHA:
			strcpy(n, "GL_SRC_ALPHA");
			break;
		case GL_ONE_MINUS_SRC_ALPHA:
			strcpy(n, "GL_ONE_MINUS_SRC_ALPHA");
			break;
		case GL_DST_COLOR:
			strcpy(n, "GL_DST_COLOR");
			break;
		case GL_ONE_MINUS_DST_COLOR:
			strcpy(n, "GL_ONE_MINUS_DST_COLOR");
			break;
		case GL_DST_ALPHA:
			strcpy(n, "GL_DST_ALPHA");
			break;
		default:
			sprintf(n, "0x%x", f);
	}
}
static void APIENTRY logBlendFunc(GLenum sfactor, GLenum dfactor)
{
	char            sf[128], df[128];

	BlendToName(sf, sfactor);
	BlendToName(df, dfactor);

	fprintf(glw_state.log_fp, "glBlendFunc( %s, %s )\n", sf, df);
	dllBlendFunc(sfactor, dfactor);
}

static void APIENTRY logClear(GLbitfield mask)
{
	fprintf(glw_state.log_fp, "glClear( 0x%x = ", mask);

	if(mask & GL_COLOR_BUFFER_BIT)
	{
		fprintf(glw_state.log_fp, "GL_COLOR_BUFFER_BIT ");
	}
	if(mask & GL_DEPTH_BUFFER_BIT)
	{
		fprintf(glw_state.log_fp, "GL_DEPTH_BUFFER_BIT ");
	}
	if(mask & GL_STENCIL_BUFFER_BIT)
	{
		fprintf(glw_state.log_fp, "GL_STENCIL_BUFFER_BIT ");
	}
	if(mask & GL_ACCUM_BUFFER_BIT)
	{
		fprintf(glw_state.log_fp, "GL_ACCUM_BUFFER_BIT ");
	}

	fprintf(glw_state.log_fp, ")\n");
	dllClear(mask);
}

static void APIENTRY logClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	fprintf(glw_state.log_fp, "glClearColor\n");
	dllClearColor(red, green, blue, alpha);
}

static void APIENTRY logClearDepth(GLclampd depth)
{
	fprintf(glw_state.log_fp, "glClearDepth( %f )\n", (float)depth);
	dllClearDepth(depth);
}

static void APIENTRY logClearStencil(GLint s)
{
	fprintf(glw_state.log_fp, "glClearStencil( %d )\n", s);
	dllClearStencil(s);
}



#define SIG( x ) fprintf( glw_state.log_fp, x "\n" )


static void APIENTRY logColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	SIG("glColorMask");
	dllColorMask(red, green, blue, alpha);
}

static void APIENTRY logCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
	SIG("glCopyPixels");
	dllCopyPixels(x, y, width, height, type);
}

static void APIENTRY logCopyTexImage1D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width,
									   GLint border)
{
	SIG("glCopyTexImage1D");
	dllCopyTexImage1D(target, level, internalFormat, x, y, width, border);
}

static void APIENTRY logCopyTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width,
									   GLsizei height, GLint border)
{
	SIG("glCopyTexImage2D");
	dllCopyTexImage2D(target, level, internalFormat, x, y, width, height, border);
}

static void APIENTRY logCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	SIG("glCopyTexSubImage1D");
	dllCopyTexSubImage1D(target, level, xoffset, x, y, width);
}

static void APIENTRY logCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y,
										  GLsizei width, GLsizei height)
{
	SIG("glCopyTexSubImage2D");
	dllCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

static void APIENTRY logCullFace(GLenum mode)
{
	fprintf(glw_state.log_fp, "glCullFace( %s )\n", (mode == GL_FRONT) ? "GL_FRONT" : "GL_BACK");
	dllCullFace(mode);
}

static void APIENTRY logDeleteTextures(GLsizei n, const GLuint * textures)
{
	SIG("glDeleteTextures");
	dllDeleteTextures(n, textures);
}

static void APIENTRY logDepthFunc(GLenum func)
{
	fprintf(glw_state.log_fp, "glDepthFunc( %s )\n", FuncToString(func));
	dllDepthFunc(func);
}

static void APIENTRY logDepthMask(GLboolean flag)
{
	fprintf(glw_state.log_fp, "glDepthMask( %s )\n", BooleanToString(flag));
	dllDepthMask(flag);
}

static void APIENTRY logDepthRange(GLclampd zNear, GLclampd zFar)
{
	fprintf(glw_state.log_fp, "glDepthRange( %f, %f )\n", (float)zNear, (float)zFar);
	dllDepthRange(zNear, zFar);
}

static void APIENTRY logDisable(GLenum cap)
{
	fprintf(glw_state.log_fp, "glDisable( %s )\n", CapToString(cap));
	dllDisable(cap);
}

static void APIENTRY logDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	SIG("glDrawArrays");
	dllDrawArrays(mode, first, count);
}

static void APIENTRY logDrawBuffer(GLenum mode)
{
	SIG("glDrawBuffer");
	dllDrawBuffer(mode);
}

static void APIENTRY logDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
	fprintf(glw_state.log_fp, "glDrawElements( %s, %d, %s, MEM )\n", PrimToString(mode), count, TypeToString(type));
	dllDrawElements(mode, count, type, indices);
}

static void APIENTRY logEnable(GLenum cap)
{
	fprintf(glw_state.log_fp, "glEnable( %s )\n", CapToString(cap));
	dllEnable(cap);
}

static void APIENTRY logFinish(void)
{
	SIG("glFinish");
	dllFinish();
}

static void APIENTRY logFlush(void)
{
	SIG("glFlush");
	dllFlush();
}

static void APIENTRY logFrontFace(GLenum mode)
{
	SIG("glFrontFace");
	dllFrontFace(mode);
}

static void APIENTRY logGenTextures(GLsizei n, GLuint * textures)
{
	SIG("glGenTextures");
	dllGenTextures(n, textures);
}

static void APIENTRY logGetBooleanv(GLenum pname, GLboolean * params)
{
	SIG("glGetBooleanv");
	dllGetBooleanv(pname, params);
}

static GLenum APIENTRY logGetError(void)
{
	SIG("glGetError");
	return dllGetError();
}

static void APIENTRY logGetFloatv(GLenum pname, GLfloat * params)
{
	SIG("glGetFloatv");
	dllGetFloatv(pname, params);
}

static void APIENTRY logGetIntegerv(GLenum pname, GLint * params)
{
	SIG("glGetIntegerv");
	dllGetIntegerv(pname, params);
}

static const GLubyte *APIENTRY logGetString(GLenum name)
{
	SIG("glGetString");
	return dllGetString(name);
}

static void APIENTRY logGetTexParameterfv(GLenum target, GLenum pname, GLfloat * params)
{
	SIG("glGetTexParameterfv");
	dllGetTexParameterfv(target, pname, params);
}

static void APIENTRY logGetTexParameteriv(GLenum target, GLenum pname, GLint * params)
{
	SIG("glGetTexParameteriv");
	dllGetTexParameteriv(target, pname, params);
}

static void APIENTRY logHint(GLenum target, GLenum mode)
{
	fprintf(glw_state.log_fp, "glHint( 0x%x, 0x%x )\n", target, mode);
	dllHint(target, mode);
}

static GLboolean APIENTRY logIsEnabled(GLenum cap)
{
	SIG("glIsEnabled");
	return dllIsEnabled(cap);
}

static GLboolean APIENTRY logIsTexture(GLuint texture)
{
	SIG("glIsTexture");
	return dllIsTexture(texture);
}

static void APIENTRY logLineWidth(GLfloat width)
{
	SIG("glLineWidth");
	dllLineWidth(width);
}

static void APIENTRY logPolygonMode(GLenum face, GLenum mode)
{
	fprintf(glw_state.log_fp, "glPolygonMode( 0x%x, 0x%x )\n", face, mode);
	dllPolygonMode(face, mode);
}

static void APIENTRY logPolygonOffset(GLfloat factor, GLfloat units)
{
	SIG("glPolygonOffset");
	dllPolygonOffset(factor, units);
}

static void APIENTRY logReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels)
{
	SIG("glReadPixels");
	dllReadPixels(x, y, width, height, format, type, pixels);
}

static void APIENTRY logScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
	fprintf(glw_state.log_fp, "glScissor( %d, %d, %d, %d )\n", x, y, width, height);
	dllScissor(x, y, width, height);
}

static void APIENTRY logStencilFunc(GLenum func, GLint ref, GLuint mask)
{
	SIG("glStencilFunc");
	dllStencilFunc(func, ref, mask);
}

static void APIENTRY logStencilMask(GLuint mask)
{
	SIG("glStencilMask");
	dllStencilMask(mask);
}

static void APIENTRY logStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	SIG("glStencilOp");
	dllStencilOp(fail, zfail, zpass);
}

static void APIENTRY logTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format,
								   GLenum type, const void *pixels)
{
	SIG("glTexImage1D");
	dllTexImage1D(target, level, internalformat, width, border, format, type, pixels);
}
static void APIENTRY logTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border,
								   GLenum format, GLenum type, const void *pixels)
{
	SIG("glTexImage2D");
	dllTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

static void APIENTRY logTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
	fprintf(glw_state.log_fp, "glTexParameterf( 0x%x, 0x%x, %f )\n", target, pname, param);
	dllTexParameterf(target, pname, param);
}

static void APIENTRY logTexParameterfv(GLenum target, GLenum pname, const GLfloat * params)
{
	SIG("glTexParameterfv");
	dllTexParameterfv(target, pname, params);
}

static void APIENTRY logTexParameteri(GLenum target, GLenum pname, GLint param)
{
	fprintf(glw_state.log_fp, "glTexParameteri( 0x%x, 0x%x, 0x%x )\n", target, pname, param);
	dllTexParameteri(target, pname, param);
}

static void APIENTRY logTexParameteriv(GLenum target, GLenum pname, const GLint * params)
{
	SIG("glTexParameteriv");
	dllTexParameteriv(target, pname, params);
}

static void APIENTRY logTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type,
									  const void *pixels)
{
	SIG("glTexSubImage1D");
	dllTexSubImage1D(target, level, xoffset, width, format, type, pixels);
}

static void APIENTRY logTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
									  GLenum format, GLenum type, const void *pixels)
{
	SIG("glTexSubImage2D");
	dllTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

static void APIENTRY logViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	fprintf(glw_state.log_fp, "glViewport( %d, %d, %d, %d )\n", x, y, width, height);
	dllViewport(x, y, width, height);
}


/*
** QGL_Shutdown
**
** Unloads the specified DLL then nulls out all the proc pointers.  This
** is only called during a hard shutdown of the OGL subsystem (e.g. vid_restart).
*/
// *INDENT-OFF*
void QGL_Shutdown(void)
{
	ri.Printf(PRINT_ALL, "...shutting down QGL\n");

	if(glw_state.hinstOpenGL)
	{
		ri.Printf(PRINT_ALL, "...unloading OpenGL DLL\n");
		FreeLibrary(glw_state.hinstOpenGL);
	}

	glw_state.hinstOpenGL = NULL;

	qglBindTexture               = NULL;
	qglBlendFunc                 = NULL;
	qglClear                     = NULL;
	qglClearColor                = NULL;
	qglClearDepth                = NULL;
	qglClearStencil              = NULL;
	qglColorMask                 = NULL;
	qglCopyPixels                = NULL;
	qglCopyTexImage1D            = NULL;
	qglCopyTexImage2D            = NULL;
	qglCopyTexSubImage1D         = NULL;
	qglCopyTexSubImage2D         = NULL;
	qglCullFace                  = NULL;
	qglDeleteTextures            = NULL;
	qglDepthFunc                 = NULL;
	qglDepthMask                 = NULL;
	qglDepthRange                = NULL;
	qglDisable                   = NULL;
	qglDrawArrays                = NULL;
	qglDrawBuffer                = NULL;
	qglDrawElements              = NULL;
	qglEnable                    = NULL;
	qglFinish                    = NULL;
	qglFlush                     = NULL;
	qglFrontFace                 = NULL;
	qglGenTextures               = NULL;
	qglGetBooleanv               = NULL;
	qglGetError                  = NULL;
	qglGetFloatv                 = NULL;
	qglGetIntegerv               = NULL;
	qglGetString                 = NULL;
	qglGetTexParameterfv         = NULL;
	qglGetTexParameteriv         = NULL;
	qglHint                      = NULL;
	qglIsEnabled                 = NULL;
	qglIsTexture                 = NULL;
	qglLineWidth                 = NULL;
	qglPolygonMode               = NULL;
	qglPolygonOffset             = NULL;
	qglReadPixels                = NULL;
	qglScissor                   = NULL;
	qglStencilFunc               = NULL;
	qglStencilMask               = NULL;
	qglStencilOp                 = NULL;
	qglTexImage1D                = NULL;
	qglTexImage2D                = NULL;
	qglTexParameterf             = NULL;
	qglTexParameterfv            = NULL;
	qglTexParameteri             = NULL;
	qglTexParameteriv            = NULL;
	qglTexSubImage1D             = NULL;
	qglTexSubImage2D             = NULL;
	qglViewport                  = NULL;

	qwglCopyContext = NULL;
	qwglCreateContext = NULL;
	qwglCreateLayerContext = NULL;
	qwglDeleteContext = NULL;
	qwglDescribeLayerPlane = NULL;
	qwglGetCurrentContext = NULL;
	qwglGetCurrentDC = NULL;
	qwglGetLayerPaletteEntries = NULL;
	qwglGetProcAddress = NULL;
	qwglMakeCurrent = NULL;
	qwglRealizeLayerPalette = NULL;
	qwglSetLayerPaletteEntries = NULL;
	qwglShareLists = NULL;
	qwglSwapLayerBuffers = NULL;
	qwglUseFontBitmaps = NULL;
	qwglUseFontOutlines = NULL;

	qwglChoosePixelFormat = NULL;
	qwglDescribePixelFormat = NULL;
	qwglGetPixelFormat = NULL;
	qwglSetPixelFormat = NULL;
	qwglSwapBuffers = NULL;
}
// *INDENT-ON*

#ifndef __GNUC__
#   pragma warning (disable : 4113 4133 4047 )
#endif
#   define GPA( a ) GetProcAddress( glw_state.hinstOpenGL, a )

/*
** QGL_Init
**
** This is responsible for binding our qgl function pointers to
** the appropriate GL stuff.  In Windows this means doing a
** LoadLibrary and a bunch of calls to GetProcAddress.  On other
** operating systems we need to do the right thing, whatever that
** might be.
*/
qboolean QGL_Init(const char *dllname)
{
	char            systemDir[1024];
	char            libName[1024];

	GetSystemDirectory(systemDir, sizeof(systemDir));

	assert(glw_state.hinstOpenGL == 0);

	ri.Printf(PRINT_ALL, "...initializing QGL\n");

	if(dllname[0] != '!' && strstr("dllname", ".dll") == NULL)
	{
		Com_sprintf(libName, sizeof(libName), "%s\\%s", systemDir, dllname);
	}
	else
	{
		Q_strncpyz(libName, dllname, sizeof(libName));
	}

	ri.Printf(PRINT_ALL, "...calling LoadLibrary( '%s.dll' ): ", libName);

	if((glw_state.hinstOpenGL = LoadLibrary(dllname)) == 0)
	{
		ri.Printf(PRINT_ALL, "failed\n");
		return qfalse;
	}
	ri.Printf(PRINT_ALL, "succeeded\n");

	qglBindTexture               = dllBindTexture = GPA( "glBindTexture" );
	qglBlendFunc                 = dllBlendFunc = GPA( "glBlendFunc" );
	qglClear                     = dllClear = GPA( "glClear" );
	qglClearColor                = dllClearColor = GPA( "glClearColor" );
	qglClearDepth                = dllClearDepth = GPA( "glClearDepth" );
	qglClearStencil              = dllClearStencil = GPA( "glClearStencil" );
	qglColorMask                 = dllColorMask = GPA( "glColorMask" );
	qglCopyPixels                = dllCopyPixels = GPA( "glCopyPixels" );
	qglCopyTexImage1D            = dllCopyTexImage1D = GPA( "glCopyTexImage1D" );
	qglCopyTexImage2D            = dllCopyTexImage2D = GPA( "glCopyTexImage2D" );
	qglCopyTexSubImage1D         = dllCopyTexSubImage1D = GPA( "glCopyTexSubImage1D" );
	qglCopyTexSubImage2D         = dllCopyTexSubImage2D = GPA( "glCopyTexSubImage2D" );
	qglCullFace                  = dllCullFace = GPA( "glCullFace" );
	qglDeleteTextures            = dllDeleteTextures = GPA( "glDeleteTextures" );
	qglDepthFunc                 = dllDepthFunc = GPA( "glDepthFunc" );
	qglDepthMask                 = dllDepthMask = GPA( "glDepthMask" );
	qglDepthRange                = dllDepthRange = GPA( "glDepthRange" );
	qglDisable                   = dllDisable = GPA( "glDisable" );
	qglDrawArrays                = dllDrawArrays = GPA( "glDrawArrays" );
	qglDrawBuffer                = dllDrawBuffer = GPA( "glDrawBuffer" );
	qglDrawElements              = dllDrawElements = GPA( "glDrawElements" );
	qglEnable                    = dllEnable                    = GPA( "glEnable" );
	qglFinish                    = dllFinish                    = GPA( "glFinish" );
	qglFlush                     = dllFlush                     = GPA( "glFlush" );
	qglFrontFace                 = dllFrontFace                 = GPA( "glFrontFace" );
	qglGenTextures               = dllGenTextures               = GPA( "glGenTextures" );
	qglGetBooleanv               = dllGetBooleanv               = GPA( "glGetBooleanv" );
	qglGetError                  = dllGetError                  = GPA( "glGetError" );
	qglGetFloatv                 = dllGetFloatv                 = GPA( "glGetFloatv" );
	qglGetIntegerv               = dllGetIntegerv               = GPA( "glGetIntegerv" );
	qglGetString                 = dllGetString                 = GPA( "glGetString" );
	qglGetTexParameterfv         = dllGetTexParameterfv         = GPA( "glGetTexParameterfv" );
	qglGetTexParameteriv         = dllGetTexParameteriv         = GPA( "glGetTexParameteriv" );
	qglHint                      = dllHint                      = GPA( "glHint" );
	qglIsEnabled                 = dllIsEnabled                 = GPA( "glIsEnabled" );
	qglIsTexture                 = dllIsTexture                 = GPA( "glIsTexture" );
	qglLineWidth                 = dllLineWidth                 = GPA( "glLineWidth" );
	qglPolygonMode               = dllPolygonMode               = GPA( "glPolygonMode" );
	qglPolygonOffset             = dllPolygonOffset             = GPA( "glPolygonOffset" );
	qglReadPixels                = dllReadPixels                = GPA( "glReadPixels" );
	qglScissor                   = dllScissor                   = GPA( "glScissor" );
	qglStencilFunc               = dllStencilFunc               = GPA( "glStencilFunc" );
	qglStencilMask               = dllStencilMask               = GPA( "glStencilMask" );
	qglStencilOp                 = dllStencilOp                 = GPA( "glStencilOp" );
	qglTexImage1D                = dllTexImage1D                = GPA( "glTexImage1D" );
	qglTexImage2D                = dllTexImage2D                = GPA( "glTexImage2D" );
	qglTexParameterf             = dllTexParameterf             = GPA( "glTexParameterf" );
	qglTexParameterfv            = dllTexParameterfv            = GPA( "glTexParameterfv" );
	qglTexParameteri             = dllTexParameteri             = GPA( "glTexParameteri" );
	qglTexParameteriv            = dllTexParameteriv            = GPA( "glTexParameteriv" );
	qglTexSubImage1D             = dllTexSubImage1D             = GPA( "glTexSubImage1D" );
	qglTexSubImage2D             = dllTexSubImage2D             = GPA( "glTexSubImage2D" );
	qglViewport                  = dllViewport                  = GPA( "glViewport" );

	qwglCopyContext = GPA("wglCopyContext");
	qwglCreateContext = GPA("wglCreateContext");
	qwglCreateLayerContext = GPA("wglCreateLayerContext");
	qwglDeleteContext = GPA("wglDeleteContext");
	qwglDescribeLayerPlane = GPA("wglDescribeLayerPlane");
	qwglGetCurrentContext = GPA("wglGetCurrentContext");
	qwglGetCurrentDC = GPA("wglGetCurrentDC");
	qwglGetLayerPaletteEntries = GPA("wglGetLayerPaletteEntries");
	qwglGetProcAddress = GPA("wglGetProcAddress");
	qwglMakeCurrent = GPA("wglMakeCurrent");
	qwglRealizeLayerPalette = GPA("wglRealizeLayerPalette");
	qwglSetLayerPaletteEntries = GPA("wglSetLayerPaletteEntries");
	qwglShareLists = GPA("wglShareLists");
	qwglSwapLayerBuffers = GPA("wglSwapLayerBuffers");
	qwglUseFontBitmaps = GPA("wglUseFontBitmapsA");
	qwglUseFontOutlines = GPA("wglUseFontOutlinesA");

	qwglChoosePixelFormat = GPA("wglChoosePixelFormat");
	qwglDescribePixelFormat = GPA("wglDescribePixelFormat");
	qwglGetPixelFormat = GPA("wglGetPixelFormat");
	qwglSetPixelFormat = GPA("wglSetPixelFormat");
	qwglSwapBuffers = GPA("wglSwapBuffers");

	qwglSwapIntervalEXT = 0;
	qglActiveTextureARB = 0;

	qwglGetDeviceGammaRamp3DFX = NULL;
	qwglSetDeviceGammaRamp3DFX = NULL;

	// check logging
	QGL_EnableLogging(r_logFile->integer);

	return qtrue;
}
// *INDENT-ON*

void QGL_EnableLogging(qboolean enable)
{
	// fixed for new countdown
	static qboolean isEnabled = qfalse;	// init

	// return if we're already active
	if(isEnabled && enable)
	{
		// decrement log counter and stop if it has reached 0
		ri.Cvar_Set("r_logFile", va("%d", r_logFile->integer - 1));
		if(r_logFile->integer)
		{
			return;
		}
		enable = qfalse;
	}

	// return if we're already disabled
	if(!enable && !isEnabled)
	{
		return;
	}

	isEnabled = enable;

	if(enable)
	{
		if(!glw_state.log_fp)
		{
			struct tm      *newtime;
			time_t          aclock;
			char            buffer[1024];
			cvar_t         *basedir;

			time(&aclock);
			newtime = localtime(&aclock);

			asctime(newtime);

			basedir = ri.Cvar_Get("fs_basepath", "", 0);
			Com_sprintf(buffer, sizeof(buffer), "%s/gl.log", basedir->string);
			glw_state.log_fp = fopen(buffer, "wt");

			fprintf(glw_state.log_fp, "%s\n", asctime(newtime));
		}

		qglBindTexture               = logBindTexture;
		qglBlendFunc                 = logBlendFunc;
		qglClear                     = logClear;
		qglClearColor                = logClearColor;
		qglClearDepth                = logClearDepth;
		qglClearStencil              = logClearStencil;
		qglColorMask                 = logColorMask;
		qglCopyPixels                = logCopyPixels;
		qglCopyTexImage1D            = logCopyTexImage1D;
		qglCopyTexImage2D            = logCopyTexImage2D;
		qglCopyTexSubImage1D         = logCopyTexSubImage1D;
		qglCopyTexSubImage2D         = logCopyTexSubImage2D;
		qglCullFace                  = logCullFace;
		qglDeleteTextures            = logDeleteTextures;
		qglDepthFunc                 = logDepthFunc;
		qglDepthMask                 = logDepthMask;
		qglDepthRange                = logDepthRange;
		qglDisable                   = logDisable;
		qglDrawArrays                = logDrawArrays;
		qglDrawBuffer                = logDrawBuffer;
		qglDrawElements              = logDrawElements;
		qglEnable                    = logEnable;
		qglFinish                    = logFinish;
		qglFlush                     = logFlush;
		qglFrontFace                 = logFrontFace;
		qglGenTextures               = logGenTextures;
		qglGetBooleanv               = logGetBooleanv;
		qglGetError                  = logGetError;
		qglGetFloatv                 = logGetFloatv;
		qglGetIntegerv               = logGetIntegerv;
		qglGetString                 = logGetString;
		qglGetTexParameterfv         = logGetTexParameterfv;
		qglGetTexParameteriv         = logGetTexParameteriv;
		qglHint                      = logHint;
		qglIsEnabled                 = logIsEnabled;
		qglIsTexture                 = logIsTexture;
		qglLineWidth                 = logLineWidth;
		qglPolygonMode               = logPolygonMode;
		qglPolygonOffset             = logPolygonOffset;
		qglReadPixels                = logReadPixels;
		qglScissor                   = logScissor;
		qglStencilFunc               = logStencilFunc;
		qglStencilMask               = logStencilMask;
		qglStencilOp                 = logStencilOp;
		qglTexImage1D                = logTexImage1D;
		qglTexImage2D                = logTexImage2D;
		qglTexParameterf             = logTexParameterf;
		qglTexParameterfv            = logTexParameterfv;
		qglTexParameteri             = logTexParameteri;
		qglTexParameteriv            = logTexParameteriv;
		qglTexSubImage1D             = logTexSubImage1D;
		qglTexSubImage2D             = logTexSubImage2D;
		qglViewport                  = logViewport;
	}
	else
	{
		if(glw_state.log_fp)
		{
			fprintf(glw_state.log_fp, "*** CLOSING LOG ***\n");
			fclose(glw_state.log_fp);
			glw_state.log_fp = NULL;
		}
		qglBindTexture               = dllBindTexture;
		qglBlendFunc                 = dllBlendFunc;
		qglClear                     = dllClear;
		qglClearColor                = dllClearColor;
		qglClearDepth                = dllClearDepth;
		qglClearStencil              = dllClearStencil;
		qglColorMask                 = dllColorMask;
		qglCopyPixels                = dllCopyPixels;
		qglCopyTexImage1D            = dllCopyTexImage1D;
		qglCopyTexImage2D            = dllCopyTexImage2D;
		qglCopyTexSubImage1D         = dllCopyTexSubImage1D;
		qglCopyTexSubImage2D         = dllCopyTexSubImage2D;
		qglCullFace                  = dllCullFace;
		qglDeleteTextures            = dllDeleteTextures;
		qglDepthFunc                 = dllDepthFunc;
		qglDepthMask                 = dllDepthMask;
		qglDepthRange                = dllDepthRange;
		qglDisable                   = dllDisable;
		qglDrawArrays                = dllDrawArrays;
		qglDrawBuffer                = dllDrawBuffer;
		qglDrawElements              = dllDrawElements;
		qglEnable                    = dllEnable;
		qglFinish                    = dllFinish;
		qglFlush                     = dllFlush;
		qglFrontFace                 = dllFrontFace;
		qglGenTextures               = dllGenTextures;
		qglGetBooleanv               = dllGetBooleanv;
		qglGetError                  = dllGetError;
		qglGetFloatv                 = dllGetFloatv;
		qglGetIntegerv               = dllGetIntegerv;
		qglGetString                 = dllGetString;
		qglGetTexParameterfv         = dllGetTexParameterfv;
		qglGetTexParameteriv         = dllGetTexParameteriv;
		qglHint                      = dllHint;
		qglIsEnabled                 = dllIsEnabled;
		qglIsTexture                 = dllIsTexture;
		qglLineWidth                 = dllLineWidth;
		qglPolygonMode               = dllPolygonMode;
		qglPolygonOffset             = dllPolygonOffset;
		qglReadPixels                = dllReadPixels;
		qglScissor                   = dllScissor;
		qglStencilFunc               = dllStencilFunc;
		qglStencilMask               = dllStencilMask;
		qglStencilOp                 = dllStencilOp;
		qglTexImage1D                = dllTexImage1D;
		qglTexImage2D                = dllTexImage2D;
		qglTexParameterf             = dllTexParameterf;
		qglTexParameterfv            = dllTexParameterfv;
		qglTexParameteri             = dllTexParameteri;
		qglTexParameteriv            = dllTexParameteriv;
		qglTexSubImage1D             = dllTexSubImage1D ;
		qglTexSubImage2D             = dllTexSubImage2D;
		qglViewport                  = dllViewport;
	}
}
// *INDENT-ON*

#ifndef __GNUC__
#pragma warning (default : 4113 4133 4047 )
#endif
