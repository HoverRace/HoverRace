// VideoBuffer.cpp
//
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
//
// Licensed under GrokkSoft HoverRace SourceCode License v1.0(the "License");
// you may not use this file except in compliance with the License.
//
// A copy of the license should have been attached to the package from which
// you have taken this file. If you can not find the license you can not use
// this file.
//
//
// The author makes no representations about the suitability of
// this software for any purpose.  It is provided "as is" "AS IS",
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
//
// See the License for the specific language governing permissions
// and limitations under the License.
//

#include "stdafx.h"

#include "VideoBuffer.h"
#include "Bitmap.h"
#include "ColorPalette.h"
#include "GpuSceneRenderer.h"

#include "../Util/Profiler.h"
#include "../Util/Config.h"
#include "../Util/WorldCoordinates.h"

#include <stddef.h>
#include <math.h>
#include <gl/GL.h>

#pragma comment(lib, "opengl32.lib")

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif
#ifndef GL_TEXTURE1
#define GL_TEXTURE1 0x84C1
#endif
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif
#ifndef GL_INFO_LOG_LENGTH
#define GL_INFO_LOG_LENGTH 0x8B84
#endif
#ifndef GL_LUMINANCE8
#define GL_LUMINANCE8 0x8040
#endif
#ifndef GL_FRAMEBUFFER_SRGB
#define GL_FRAMEBUFFER_SRGB 0x8DB9
#endif

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;

typedef void (APIENTRY *PFNGLACTIVETEXTUREPROC)(GLenum texture);
typedef GLuint (APIENTRY *PFNGLCREATESHADERPROC)(GLenum type);
typedef void (APIENTRY *PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
typedef void (APIENTRY *PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void (APIENTRY *PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRY *PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef GLuint (APIENTRY *PFNGLCREATEPROGRAMPROC)(void);
typedef void (APIENTRY *PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void (APIENTRY *PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void (APIENTRY *PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRY *PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef void (APIENTRY *PFNGLUSEPROGRAMPROC)(GLuint program);
typedef GLint (APIENTRY *PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar *name);
typedef void (APIENTRY *PFNGLUNIFORM1IPROC)(GLint location, GLint v0);
typedef void (APIENTRY *PFNGLDELETESHADERPROC)(GLuint shader);
typedef void (APIENTRY *PFNGLDELETEPROGRAMPROC)(GLuint program);

namespace {
	struct MR_GpuSceneProjectedVertex
	{
		GLfloat mX;
		GLfloat mY;
		BOOL mVisible;
	};

	struct MR_GpuSceneCameraVertex
	{
		GLdouble mX;
		GLdouble mY;
		GLdouble mZ;
		BOOL mVisible;
	};

	struct MR_GpuSceneTexturedVertex
	{
		MR_GpuSceneCameraVertex mCamera;
		GLfloat mU;
		GLfloat mV;
	};

	struct MR_GpuSceneColoredVertex
	{
		MR_GpuSceneCameraVertex mCamera;
	};

	static BOOL TransformGpuSceneVertexToCameraSpace(const MR_GpuSceneFrame &pFrame,
		const MR_3DCoordinate &pWorldVertex, MR_GpuSceneCameraVertex &pCameraVertex)
	{
		const double lCos = static_cast<double>(MR_Cos[pFrame.mOrientation]) / MR_TRIGO_FRACT;
		const double lSin = static_cast<double>(MR_Sin[pFrame.mOrientation]) / MR_TRIGO_FRACT;
		const double lDX = static_cast<double>(pWorldVertex.mX - pFrame.mCameraPosition.mX);
		const double lDY = static_cast<double>(pWorldVertex.mY - pFrame.mCameraPosition.mY);
		const double lDZ = static_cast<double>(pWorldVertex.mZ - pFrame.mCameraPosition.mZ);
		const double lCameraX = lDX * lCos + lDY * lSin;
		const double lCameraY = -lDX * lSin + lDY * lCos;
		const double lCameraZ = lDZ;
		const double lNearPlane = max(1.0, static_cast<double>(pFrame.mPlanDist));

		pCameraVertex.mVisible = FALSE;
		pCameraVertex.mX = 0.0;
		pCameraVertex.mY = 0.0;
		pCameraVertex.mZ = 0.0;

		if(lCameraX < lNearPlane) {
			return FALSE;
		}

		// Map HoverRace camera space to OpenGL eye space:
		// +X right, +Y up, and looking down -Z.
		pCameraVertex.mX = -lCameraY;
		pCameraVertex.mY = lCameraZ;
		pCameraVertex.mZ = -lCameraX;
		pCameraVertex.mVisible = TRUE;
		return TRUE;
	}

	static void ApplyGpuScenePositionMatrix(const MR_GpuScenePositionMatrix &pMatrix,
		const MR_3DCoordinate &pSrc, MR_3DCoordinate &pDest)
	{
		pDest.mX = pMatrix.mDisplacement.mX
			+ MulDiv(pSrc.mX, pMatrix.mRotation[0][0], MR_TRIGO_FRACT)
			+ MulDiv(pSrc.mY, pMatrix.mRotation[0][1], MR_TRIGO_FRACT);
		pDest.mY = pMatrix.mDisplacement.mY
			+ MulDiv(pSrc.mX, pMatrix.mRotation[1][0], MR_TRIGO_FRACT)
			+ MulDiv(pSrc.mY, pMatrix.mRotation[1][1], MR_TRIGO_FRACT);
		pDest.mZ = pMatrix.mDisplacement.mZ + pSrc.mZ;
	}

	static BOOL ProjectGpuSceneVertex(const MR_GpuSceneFrame &pFrame,
		const MR_3DCoordinate &pWorldVertex, MR_GpuSceneProjectedVertex &pProjected)
	{
		MR_GpuSceneCameraVertex lCameraVertex;
		const double lWidth = static_cast<double>(pFrame.mViewport.right - pFrame.mViewport.left);
		const double lHeight = static_cast<double>(pFrame.mViewport.bottom - pFrame.mViewport.top);

		pProjected.mVisible = FALSE;
		pProjected.mX = 0.0f;
		pProjected.mY = 0.0f;

		if((lWidth <= 0.0) || (lHeight <= 0.0) || (pFrame.mPlanHW == 0) || (pFrame.mPlanVW == 0)) {
			return FALSE;
		}
		if(!TransformGpuSceneVertexToCameraSpace(pFrame, pWorldVertex, lCameraVertex)) {
			return FALSE;
		}

		const double lCameraX = -lCameraVertex.mZ;
		const double lCameraY = -lCameraVertex.mX;
		const double lCameraZ = lCameraVertex.mY;
		const double lScreenX =
			((-lCameraY) * lWidth * pFrame.mPlanDist) / (lCameraX * pFrame.mPlanHW * 2.0) +
			(lWidth / 2.0);
		const double lScreenY =
			(-(lCameraZ) * lHeight * pFrame.mPlanDist) / (lCameraX * pFrame.mPlanVW * 2.0) +
			(lHeight / 2.0) + pFrame.mScroll;

		pProjected.mX = static_cast<GLfloat>((2.0 * lScreenX / lWidth) - 1.0);
		pProjected.mY = static_cast<GLfloat>(1.0 - (2.0 * lScreenY / lHeight));
		pProjected.mVisible = TRUE;
		return TRUE;
	}

	static BOOL ProjectGpuSceneCameraVertex(const MR_GpuSceneFrame &pFrame,
		const MR_GpuSceneCameraVertex &pCameraVertex, MR_GpuSceneProjectedVertex &pProjected)
	{
		const double lWidth = static_cast<double>(pFrame.mViewport.right - pFrame.mViewport.left);
		const double lHeight = static_cast<double>(pFrame.mViewport.bottom - pFrame.mViewport.top);
		const double lCameraX = -pCameraVertex.mZ;
		const double lCameraY = -pCameraVertex.mX;
		const double lCameraZ = pCameraVertex.mY;
		const double lScreenX =
			((-lCameraY) * lWidth * pFrame.mPlanDist) / (lCameraX * pFrame.mPlanHW * 2.0) +
			(lWidth / 2.0);
		const double lScreenY =
			(-(lCameraZ) * lHeight * pFrame.mPlanDist) / (lCameraX * pFrame.mPlanVW * 2.0) +
			(lHeight / 2.0) + pFrame.mScroll;

		pProjected.mX = static_cast<GLfloat>((2.0 * lScreenX / lWidth) - 1.0);
		pProjected.mY = static_cast<GLfloat>(1.0 - (2.0 * lScreenY / lHeight));
		pProjected.mVisible = TRUE;
		return TRUE;
	}

	static MR_3DCoordinate LerpGpuSceneCoordinate(const MR_3DCoordinate &p0,
		const MR_3DCoordinate &p1, double pT)
	{
		return MR_3DCoordinate(
			static_cast<MR_Int32>(p0.mX + ((p1.mX - p0.mX) * pT)),
			static_cast<MR_Int32>(p0.mY + ((p1.mY - p0.mY) * pT)),
			static_cast<MR_Int32>(p0.mZ + ((p1.mZ - p0.mZ) * pT)));
	}

	static GLdouble LerpGpuSceneDouble(GLdouble p0, GLdouble p1, GLdouble pT)
	{
		return p0 + ((p1 - p0) * pT);
	}

	static BOOL ClipGpuSceneWallEdgeToNearPlane(MR_GpuSceneCameraVertex &pStart,
		MR_GpuSceneCameraVertex &pEnd, GLdouble pNearPlaneZ, GLdouble &pT)
	{
		const BOOL lStartInside = (pStart.mZ <= pNearPlaneZ);
		const BOOL lEndInside = (pEnd.mZ <= pNearPlaneZ);

		pT = 0.0;

		if(lStartInside && lEndInside) {
			return TRUE;
		}
		if(!lStartInside && !lEndInside) {
			return FALSE;
		}

		const GLdouble lDenominator = pEnd.mZ - pStart.mZ;
		if(lDenominator == 0.0) {
			return FALSE;
		}

		pT = (pNearPlaneZ - pStart.mZ) / lDenominator;
		if(pT < 0.0) {
			pT = 0.0;
		}
		else if(pT > 1.0) {
			pT = 1.0;
		}

		MR_GpuSceneCameraVertex lClipped;
		lClipped.mX = LerpGpuSceneDouble(pStart.mX, pEnd.mX, pT);
		lClipped.mY = LerpGpuSceneDouble(pStart.mY, pEnd.mY, pT);
		lClipped.mZ = pNearPlaneZ;
		lClipped.mVisible = TRUE;

		if(!lStartInside) {
			pStart = lClipped;
		}
		else {
			pEnd = lClipped;
		}

		return TRUE;
	}

	static void ClipGpuSceneTexturedPolygonToNearPlane(
		std::vector<MR_GpuSceneTexturedVertex> &pVertices, GLdouble pNearPlaneZ)
	{
		if(pVertices.empty()) {
			return;
		}

		std::vector<MR_GpuSceneTexturedVertex> lClipped;
		lClipped.reserve(pVertices.size() + 4);

		for(size_t lVertex = 0; lVertex < pVertices.size(); lVertex++) {
			const MR_GpuSceneTexturedVertex &lCurrent = pVertices[lVertex];
			const MR_GpuSceneTexturedVertex &lNext = pVertices[(lVertex + 1) % pVertices.size()];
			const BOOL lCurrentInside = lCurrent.mCamera.mVisible && (lCurrent.mCamera.mZ <= pNearPlaneZ);
			const BOOL lNextInside = lNext.mCamera.mVisible && (lNext.mCamera.mZ <= pNearPlaneZ);

			if(lCurrentInside) {
				lClipped.push_back(lCurrent);
			}

			if(lCurrentInside != lNextInside) {
				const GLdouble lDenominator = lNext.mCamera.mZ - lCurrent.mCamera.mZ;
				if(lDenominator != 0.0) {
					GLdouble lT = (pNearPlaneZ - lCurrent.mCamera.mZ) / lDenominator;
					if(lT < 0.0) {
						lT = 0.0;
					}
					else if(lT > 1.0) {
						lT = 1.0;
					}

					MR_GpuSceneTexturedVertex lClippedVertex;
					lClippedVertex.mCamera.mX = LerpGpuSceneDouble(lCurrent.mCamera.mX, lNext.mCamera.mX, lT);
					lClippedVertex.mCamera.mY = LerpGpuSceneDouble(lCurrent.mCamera.mY, lNext.mCamera.mY, lT);
					lClippedVertex.mCamera.mZ = pNearPlaneZ;
					lClippedVertex.mCamera.mVisible = TRUE;
					lClippedVertex.mU = static_cast<GLfloat>(LerpGpuSceneDouble(lCurrent.mU, lNext.mU, lT));
					lClippedVertex.mV = static_cast<GLfloat>(LerpGpuSceneDouble(lCurrent.mV, lNext.mV, lT));
					lClipped.push_back(lClippedVertex);
				}
			}
		}

		pVertices.swap(lClipped);
	}

	static void ClipGpuSceneColoredPolygonToNearPlane(
		std::vector<MR_GpuSceneColoredVertex> &pVertices, GLdouble pNearPlaneZ)
	{
		if(pVertices.empty()) {
			return;
		}

		std::vector<MR_GpuSceneColoredVertex> lClipped;
		lClipped.reserve(pVertices.size() + 4);

		for(size_t lVertex = 0; lVertex < pVertices.size(); lVertex++) {
			const MR_GpuSceneColoredVertex &lCurrent = pVertices[lVertex];
			const MR_GpuSceneColoredVertex &lNext = pVertices[(lVertex + 1) % pVertices.size()];
			const BOOL lCurrentInside = lCurrent.mCamera.mVisible && (lCurrent.mCamera.mZ <= pNearPlaneZ);
			const BOOL lNextInside = lNext.mCamera.mVisible && (lNext.mCamera.mZ <= pNearPlaneZ);

			if(lCurrentInside) {
				lClipped.push_back(lCurrent);
			}

			if(lCurrentInside != lNextInside) {
				const GLdouble lDenominator = lNext.mCamera.mZ - lCurrent.mCamera.mZ;
				if(lDenominator != 0.0) {
					GLdouble lT = (pNearPlaneZ - lCurrent.mCamera.mZ) / lDenominator;
					if(lT < 0.0) {
						lT = 0.0;
					}
					else if(lT > 1.0) {
						lT = 1.0;
					}

					MR_GpuSceneColoredVertex lClippedVertex;
					lClippedVertex.mCamera.mX = LerpGpuSceneDouble(lCurrent.mCamera.mX, lNext.mCamera.mX, lT);
					lClippedVertex.mCamera.mY = LerpGpuSceneDouble(lCurrent.mCamera.mY, lNext.mCamera.mY, lT);
					lClippedVertex.mCamera.mZ = pNearPlaneZ;
					lClippedVertex.mCamera.mVisible = TRUE;
					lClipped.push_back(lClippedVertex);
				}
			}
		}

		pVertices.swap(lClipped);
	}
}

// Debug flag
#ifdef _DEBUG
static const BOOL gDebugMode = TRUE;
#else
static const BOOL gDebugMode = FALSE;
#endif

// Video card debuging traces

// #define _CARD_DEBUG

#ifdef _CARD_DEBUG

#include <Mmsystem.h>

static FILE *gOutputFile = NULL;

#define OPEN_LOG() \
if( gOutputFile == NULL ) \
{ \
	gOutputFile = fopen( "Video.log", "a" ); \
} \

#define CLOSE_LOG() \
if( gOutputFile != NULL ) \
{ \
	fclose( gOutputFile ); \
	gOutputFile = NULL; \
} \

		static void PrintTimeAndLine(int pLine)
		{
			if(gOutputFile != NULL) {
				fprintf(gOutputFile, "%6d %4d : ", (int) timeGetTime(), pLine);
			}
		}

static int Assert(int pCondition, int pLine)
{
	if(!pCondition && (gOutputFile != NULL)) {
		fprintf(gOutputFile, "%6d %4d : ASSERT FAILED", (int) timeGetTime(), pLine);
	}

	return pCondition;
}

static void PrintLog(const char *pFormat, ...)
{
	va_list lParamList;

	va_start(lParamList, pFormat);

	if(gOutputFile != NULL) {
		vfprintf(gOutputFile, pFormat, lParamList);
		fprintf(gOutputFile, "\n");
		fflush(gOutputFile);
	}
}

int DDrawCall(int pFuncResult, int pLine)
{
	const char *lErrStr = "DDERR_<other>";

	switch (pFuncResult) {
		case DD_OK:
			lErrStr = "DD_OK";
			break;

		case DDERR_INVALIDPARAMS:
			lErrStr = "DDERR_INVALIDPARAMS";
			break;

		case DDERR_INVALIDOBJECT:
			lErrStr = "DDERR_INVALIDOBJECT";
			break;

		case DDERR_SURFACELOST:
			lErrStr = "DDERR_SURFACELOST";
			break;

		case DDERR_SURFACEBUSY:
			lErrStr = "DDERR_SURFACEBUSY";
			break;

		case DDERR_GENERIC:
			lErrStr = "DDERR_GENERIC";
			break;

		case DDERR_WASSTILLDRAWING:
			lErrStr = "DDERR_WASSTILLDRAWING";
			break;

		case DDERR_UNSUPPORTED:
			lErrStr = "DDERR_UNSUPPORTED";
			break;

		case DDERR_NOTFLIPPABLE:
			lErrStr = "DDERR_NOTFLIPPABLE";
			break;

		case DDERR_NOFLIPHW:
			lErrStr = "DDERR_NOFLIPHW";
			break;

		case DDERR_INVALIDMODE:
			lErrStr = "DDERR_INVALIDMODE";
			break;

		case DDERR_LOCKEDSURFACES:
			lErrStr = "DDERR_LOCKEDSURFACES";
			break;

			/*
			   case DDERR_WASSTILLDRAWING:
			   lErrStr = "DDERR_WASSTILLDRAWING";
			   break;
			 */

		case DDERR_NOEXCLUSIVEMODE:
			lErrStr = "DDERR_NOEXCLUSIVEMODE";
			break;

			/*
			   case DDERR_INVALIDPARAMS:
			   lErrStr = "DDERR_INVALIDPARAMS";
			   break;
			 */

		case DDERR_OUTOFVIDEOMEMORY:
			lErrStr = "DDERR_OUTOFVIDEOMEMORY";
			break;

		case DDERR_NODIRECTDRAWHW:
			lErrStr = "DDERR_NODIRECTDRAWHW";
			break;

		case DDERR_NOCOOPERATIVELEVELSET:
			lErrStr = "DDERR_NOCOOPERATIVELEVELSET";
			break;

		case DDERR_INVALIDCAPS:
			lErrStr = "DDERR_INVALIDCAPS";
			break;

		case DDERR_INVALIDPIXELFORMAT:
			lErrStr = "DDERR_INVALIDPIXELFORMAT";
			break;

		case DDERR_NOALPHAHW:
			lErrStr = "DDERR_NOALPHAHW";
			break;

			/*
			   case DDERR_NOFLIPHW:
			   lErrStr = "DDERR_NOFLIPHW";
			   break;
			 */

		case DDERR_NOZBUFFERHW:
			lErrStr = "DDERR_NOZBUFFERHW";
			break;

			/*
			   case DDERR_NOEXCLUSIVEMODE:
			   lErrStr = "DDERR_NOEXCLUSIVEMODE";
			   break;
			 */

		case DDERR_OUTOFMEMORY:
			lErrStr = "DDERR_OUTOFMEMORY";
			break;

		case DDERR_PRIMARYSURFACEALREADYEXISTS:
			lErrStr = "DDERR_PRIMARYSURFACEALREADYEXISTS";
			break;

		case DDERR_NOEMULATION:
			lErrStr = "DDERR_NOEMULATION";
			break;

		case DDERR_INCOMPATIBLEPRIMARY:
			lErrStr = "DDERR_INCOMPATIBLEPRIMARY";
			break;
	}

	if(gOutputFile != NULL) {
		fprintf(gOutputFile, "%6d %4d : %d %s\n", (int) timeGetTime(), pLine, pFuncResult, lErrStr);
		fflush(gOutputFile);
	}

	return pFuncResult;
}

#define PRINT_LOG   PrintTimeAndLine( __LINE__ );PrintLog

#define DD_CALL( pFunc )   DDrawCall( pFunc, __LINE__ )

#ifdef ASSERT
#undef ASSERT
#endif

#define ASSERT( pCondition ) Assert( pCondition, __LINE__ )

#else

static FILE *gOutputFile = NULL;
static char gOutputPath[MAX_PATH] = { 0 };
static BOOL gLogSessionOpen = FALSE;

static void BuildDebugLogPath(char *buffer, size_t bufferSize)
{
	if(bufferSize == 0) {
		return;
	}

	buffer[0] = '\0';

	char appData[MAX_PATH] = { 0 };
	DWORD len = GetEnvironmentVariableA("APPDATA", appData, sizeof(appData));
	if((len > 0) && (len < sizeof(appData))) {
		_snprintf(buffer, bufferSize - 1, "%s\\HoverRace.com", appData);
		buffer[bufferSize - 1] = '\0';
		CreateDirectoryA(buffer, NULL);

		_snprintf(buffer, bufferSize - 1, "%s\\HoverRace.com\\HoverRace", appData);
		buffer[bufferSize - 1] = '\0';
		CreateDirectoryA(buffer, NULL);

		_snprintf(buffer, bufferSize - 1, "%s\\HoverRace.com\\HoverRace\\Video.log", appData);
		buffer[bufferSize - 1] = '\0';
		return;
	}

	_snprintf(buffer, bufferSize - 1, "Video.log");
	buffer[bufferSize - 1] = '\0';
}

static void EnsureDebugLog()
{
	if(gOutputFile == NULL) {
		if(gOutputPath[0] == '\0') {
			BuildDebugLogPath(gOutputPath, sizeof(gOutputPath));
		}
		gOutputFile = fopen(gOutputPath, "a");
		if((gOutputFile != NULL) && !gLogSessionOpen) {
			SYSTEMTIME st;
			char exePath[MAX_PATH] = { 0 };
			GetLocalTime(&st);
			GetModuleFileNameA(NULL, exePath, sizeof(exePath));
			fprintf(gOutputFile,
				"\n========== VIDEO LOG SESSION START %04d-%02d-%02d %02d:%02d:%02d pid=%lu build=%s ==========\n",
				st.wYear, st.wMonth, st.wDay,
				st.wHour, st.wMinute, st.wSecond,
				GetCurrentProcessId(),
#ifdef _DEBUG
				"Debug"
#else
				"Release"
#endif
			);
			fprintf(gOutputFile, "exe=%s\n", exePath);
			fprintf(gOutputFile, "log=%s\n", gOutputPath);
			fflush(gOutputFile);
			gLogSessionOpen = TRUE;
		}
	}
}

static void CloseDebugLog()
{
	if(gOutputFile != NULL) {
		if(gLogSessionOpen) {
			SYSTEMTIME st;
			GetLocalTime(&st);
			fprintf(gOutputFile,
				"========== VIDEO LOG SESSION END %04d-%02d-%02d %02d:%02d:%02d ==========\n",
				st.wYear, st.wMonth, st.wDay,
				st.wHour, st.wMinute, st.wSecond);
			fflush(gOutputFile);
			gLogSessionOpen = FALSE;
		}
		fclose(gOutputFile);
		gOutputFile = NULL;
	}
}

void PrintLog(const char *pFormat, ...)
{
	char buffer[1024];
	va_list lParamList;

	va_start(lParamList, pFormat);
	_vsnprintf(buffer, sizeof(buffer) - 1, pFormat, lParamList);
	buffer[sizeof(buffer) - 1] = '\0';
	va_end(lParamList);

	EnsureDebugLog();
	if(gOutputFile != NULL) {
		fprintf(gOutputFile, "%10lu %s\n", GetTickCount(), buffer);
		fflush(gOutputFile);
	}

	OutputDebugStringA(buffer);
	OutputDebugStringA("\n");
}

static int DDrawCall(int pFuncResult, int pLine)
{
	if(pFuncResult != DD_OK) {
		PrintLog("DDRAW line=%d hr=%d", pLine, pFuncResult);
	}

	return pFuncResult;
}

#define OPEN_LOG() EnsureDebugLog()
#define CLOSE_LOG() CloseDebugLog()
#define PRINT_LOG          PrintLog
#define DD_CALL( pFunc )   DDrawCall( pFunc, __LINE__ )
#endif

struct MR_OpenGLState
{
	struct CachedBitmapTexture
	{
		const MR_Bitmap *bitmap;
		int subBitmap;
		GLuint texture;
		int width;
		int height;

		CachedBitmapTexture() :
			bitmap(NULL), subBitmap(0), texture(0), width(0), height(0)
		{
		}
	};

	HDC windowDc;
	HGLRC context;
	GLuint frameTexture;
	GLuint paletteTexture;
	GLuint shaderProgram;
	GLuint vertexShader;
	GLuint fragmentShader;
	GLint indexUniform;
	GLint paletteUniform;
	int textureWidth;
	int textureHeight;
	BOOL shaderReady;
	std::vector<MR_UInt8> rgbaFallback;
	std::vector<CachedBitmapTexture> cachedBitmapTextures;

	MR_OpenGLState() :
		windowDc(NULL), context(NULL),
		frameTexture(0), paletteTexture(0),
		shaderProgram(0), vertexShader(0), fragmentShader(0),
		indexUniform(-1), paletteUniform(-1),
		textureWidth(0), textureHeight(0),
		shaderReady(FALSE)
	{
	}
};

static MR_UInt8 ApplyLegacyPresentationCurve(MR_UInt8 value)
{
	const double normalized = value / 255.0;
	const double corrected = pow(normalized, 2.2);
	int scaled = static_cast<int>((corrected * 255.0) + 0.5);
	if(scaled < 0) {
		scaled = 0;
	}
	if(scaled > 255) {
		scaled = 255;
	}
	return static_cast<MR_UInt8>(scaled);
}

namespace {
	enum OpenGLShaderBringupMode
	{
		OGL_SHADER_BRINGUP_OFF = 0,
		OGL_SHADER_BRINGUP_COMPILE = 1,
		OGL_SHADER_BRINGUP_UPLOADS = 2,
		OGL_SHADER_BRINGUP_DRAW = 3
	};

	static OpenGLShaderBringupMode GetOpenGLShaderBringupMode()
	{
		static BOOL initialized = FALSE;
		static OpenGLShaderBringupMode mode = OGL_SHADER_BRINGUP_DRAW;
		if(!initialized) {
			char buffer[32] = {0};
			DWORD len = GetEnvironmentVariableA("HOVERRACE_OPENGL_SHADER_STAGE",
				buffer, sizeof(buffer));
			if(len > 0 && len < sizeof(buffer)) {
				if(_stricmp(buffer, "compile") == 0) {
					mode = OGL_SHADER_BRINGUP_COMPILE;
				}
				else if(_stricmp(buffer, "uploads") == 0) {
					mode = OGL_SHADER_BRINGUP_UPLOADS;
				}
				else if(_stricmp(buffer, "draw") == 0 || _stricmp(buffer, "indexed_shader") == 0) {
					mode = OGL_SHADER_BRINGUP_DRAW;
				}
			}
			initialized = TRUE;
		}
		return mode;
	}

	static const char *GetOpenGLShaderBringupModeName(OpenGLShaderBringupMode mode)
	{
		switch(mode) {
			case OGL_SHADER_BRINGUP_COMPILE: return "compile";
			case OGL_SHADER_BRINGUP_UPLOADS: return "uploads";
			case OGL_SHADER_BRINGUP_DRAW: return "draw";
			case OGL_SHADER_BRINGUP_OFF:
			default:
				return "off";
		}
	}

	struct GLFunctions
	{
		PFNGLACTIVETEXTUREPROC ActiveTexture;
		PFNGLCREATESHADERPROC CreateShader;
		PFNGLSHADERSOURCEPROC ShaderSource;
		PFNGLCOMPILESHADERPROC CompileShader;
		PFNGLGETSHADERIVPROC GetShaderiv;
		PFNGLGETSHADERINFOLOGPROC GetShaderInfoLog;
		PFNGLCREATEPROGRAMPROC CreateProgram;
		PFNGLATTACHSHADERPROC AttachShader;
		PFNGLLINKPROGRAMPROC LinkProgram;
		PFNGLGETPROGRAMIVPROC GetProgramiv;
		PFNGLGETPROGRAMINFOLOGPROC GetProgramInfoLog;
		PFNGLUSEPROGRAMPROC UseProgram;
		PFNGLGETUNIFORMLOCATIONPROC GetUniformLocation;
		PFNGLUNIFORM1IPROC Uniform1i;
		PFNGLDELETESHADERPROC DeleteShader;
		PFNGLDELETEPROGRAMPROC DeleteProgram;
		BOOL loaded;

		GLFunctions() :
			ActiveTexture(NULL), CreateShader(NULL), ShaderSource(NULL),
			CompileShader(NULL), GetShaderiv(NULL), GetShaderInfoLog(NULL),
			CreateProgram(NULL), AttachShader(NULL), LinkProgram(NULL),
			GetProgramiv(NULL), GetProgramInfoLog(NULL), UseProgram(NULL),
			GetUniformLocation(NULL), Uniform1i(NULL), DeleteShader(NULL),
			DeleteProgram(NULL), loaded(FALSE)
		{
		}
	};

	GLFunctions gGL;

	static PROC LoadOpenGLProc(const char *name)
	{
		PROC proc = wglGetProcAddress(name);
		if(proc == NULL || proc == (PROC)1 || proc == (PROC)2 ||
			proc == (PROC)3 || proc == (PROC)-1)
		{
			HMODULE glModule = GetModuleHandleA("opengl32.dll");
			if(glModule == NULL) {
				glModule = LoadLibraryA("opengl32.dll");
			}
			if(glModule != NULL) {
				proc = GetProcAddress(glModule, name);
			}
		}
		return proc;
	}

	static int FindWindowOpenGLPixelFormat(HDC windowDc)
	{
		if(windowDc == NULL) {
			return 0;
		}

		PIXELFORMATDESCRIPTOR firstPfd;
		memset(&firstPfd, 0, sizeof(firstPfd));
		const int maxFormats = DescribePixelFormat(windowDc, 1,
			sizeof(firstPfd), &firstPfd);
		PRINT_LOG("FindWindowOpenGLPixelFormat maxFormats=%d", maxFormats);
		if(maxFormats <= 0) {
			PRINT_LOG("FindWindowOpenGLPixelFormat initial DescribePixelFormat failed err=%lu",
				GetLastError());
			return 0;
		}

		int bestFormat = 0;
		int bestScore = -1;
		for(int format = 1; format <= maxFormats; format++) {
			PIXELFORMATDESCRIPTOR pfd;
			memset(&pfd, 0, sizeof(pfd));
			if(DescribePixelFormat(windowDc, format, sizeof(pfd), &pfd) == 0) {
				if(format <= 4) {
					PRINT_LOG("FindWindowOpenGLPixelFormat DescribePixelFormat failed format=%d err=%lu",
						format, GetLastError());
				}
				continue;
			}

			if((pfd.dwFlags & PFD_DRAW_TO_WINDOW) == 0) {
				continue;
			}
			if((pfd.dwFlags & PFD_SUPPORT_OPENGL) == 0) {
				continue;
			}
			if(pfd.iPixelType != PFD_TYPE_RGBA) {
				continue;
			}

			int score = 0;
			if((pfd.dwFlags & PFD_DOUBLEBUFFER) != 0) {
				score += 1000;
			}
			score += pfd.cColorBits;
			score += pfd.cDepthBits;
			if(pfd.iLayerType == PFD_MAIN_PLANE) {
				score += 100;
			}

			if(score > bestScore) {
				bestScore = score;
				bestFormat = format;
				if(bestScore > 0) {
					PRINT_LOG("FindWindowOpenGLPixelFormat candidate format=%d flags=0x%08lx color=%u depth=%u score=%d",
						format, (unsigned long) pfd.dwFlags, (unsigned) pfd.cColorBits,
						(unsigned) pfd.cDepthBits, score);
				}
			}
		}

		PRINT_LOG("FindWindowOpenGLPixelFormat result format=%d score=%d", bestFormat, bestScore);
		return bestFormat;
	}

	static BOOL LogOpenGLErrors(const char *stage)
	{
		BOOL success = TRUE;
		GLenum err = glGetError();
		while(err != GL_NO_ERROR) {
			success = FALSE;
			PRINT_LOG("OpenGL error stage=%s glerr=0x%04x", stage, (unsigned) err);
			err = glGetError();
		}
		return success;
	}

	static BOOL LoadOpenGLFunctions()
	{
		if(gGL.loaded) {
			return TRUE;
		}

		gGL.ActiveTexture = reinterpret_cast<PFNGLACTIVETEXTUREPROC>(LoadOpenGLProc("glActiveTexture"));
		gGL.CreateShader = reinterpret_cast<PFNGLCREATESHADERPROC>(LoadOpenGLProc("glCreateShader"));
		gGL.ShaderSource = reinterpret_cast<PFNGLSHADERSOURCEPROC>(LoadOpenGLProc("glShaderSource"));
		gGL.CompileShader = reinterpret_cast<PFNGLCOMPILESHADERPROC>(LoadOpenGLProc("glCompileShader"));
		gGL.GetShaderiv = reinterpret_cast<PFNGLGETSHADERIVPROC>(LoadOpenGLProc("glGetShaderiv"));
		gGL.GetShaderInfoLog = reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>(LoadOpenGLProc("glGetShaderInfoLog"));
		gGL.CreateProgram = reinterpret_cast<PFNGLCREATEPROGRAMPROC>(LoadOpenGLProc("glCreateProgram"));
		gGL.AttachShader = reinterpret_cast<PFNGLATTACHSHADERPROC>(LoadOpenGLProc("glAttachShader"));
		gGL.LinkProgram = reinterpret_cast<PFNGLLINKPROGRAMPROC>(LoadOpenGLProc("glLinkProgram"));
		gGL.GetProgramiv = reinterpret_cast<PFNGLGETPROGRAMIVPROC>(LoadOpenGLProc("glGetProgramiv"));
		gGL.GetProgramInfoLog = reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>(LoadOpenGLProc("glGetProgramInfoLog"));
		gGL.UseProgram = reinterpret_cast<PFNGLUSEPROGRAMPROC>(LoadOpenGLProc("glUseProgram"));
		gGL.GetUniformLocation = reinterpret_cast<PFNGLGETUNIFORMLOCATIONPROC>(LoadOpenGLProc("glGetUniformLocation"));
		gGL.Uniform1i = reinterpret_cast<PFNGLUNIFORM1IPROC>(LoadOpenGLProc("glUniform1i"));
		gGL.DeleteShader = reinterpret_cast<PFNGLDELETESHADERPROC>(LoadOpenGLProc("glDeleteShader"));
		gGL.DeleteProgram = reinterpret_cast<PFNGLDELETEPROGRAMPROC>(LoadOpenGLProc("glDeleteProgram"));

		gGL.loaded =
			(gGL.ActiveTexture != NULL) &&
			(gGL.CreateShader != NULL) &&
			(gGL.ShaderSource != NULL) &&
			(gGL.CompileShader != NULL) &&
			(gGL.GetShaderiv != NULL) &&
			(gGL.GetShaderInfoLog != NULL) &&
			(gGL.CreateProgram != NULL) &&
			(gGL.AttachShader != NULL) &&
			(gGL.LinkProgram != NULL) &&
			(gGL.GetProgramiv != NULL) &&
			(gGL.GetProgramInfoLog != NULL) &&
			(gGL.UseProgram != NULL) &&
			(gGL.GetUniformLocation != NULL) &&
			(gGL.Uniform1i != NULL) &&
			(gGL.DeleteShader != NULL) &&
			(gGL.DeleteProgram != NULL);

		return gGL.loaded;
	}

	struct RenderLogStats
	{
		unsigned long lockCount;
		unsigned long unlockCount;
		unsigned long flipCount;
		unsigned long lockFailures;
		unsigned long unlockFailures;
		unsigned long flipFailures;
		unsigned long invalidRectFlipFailures;
		unsigned long gdiPresentCount;
		unsigned long gdiPresentFailures;
		unsigned long gdiFallbackActivations;

		RenderLogStats() :
			lockCount(0), unlockCount(0), flipCount(0),
			lockFailures(0), unlockFailures(0), flipFailures(0),
			invalidRectFlipFailures(0),
			gdiPresentCount(0), gdiPresentFailures(0),
			gdiFallbackActivations(0)
		{
		}
	};

	RenderLogStats gRenderLogStats;

	void ResetRenderLogStats()
	{
		gRenderLogStats = RenderLogStats();
	}

	void LogRenderStats(const char *label)
	{
		PRINT_LOG("%s renderStats lock=%lu unlock=%lu flip=%lu lockFail=%lu unlockFail=%lu flipFail=%lu invalidRectFlipFail=%lu gdiPresent=%lu gdiPresentFail=%lu gdiFallbackActivations=%lu",
			label,
			gRenderLogStats.lockCount,
			gRenderLogStats.unlockCount,
			gRenderLogStats.flipCount,
			gRenderLogStats.lockFailures,
			gRenderLogStats.unlockFailures,
			gRenderLogStats.flipFailures,
			gRenderLogStats.invalidRectFlipFailures,
			gRenderLogStats.gdiPresentCount,
			gRenderLogStats.gdiPresentFailures,
			gRenderLogStats.gdiFallbackActivations);
	}

	void FormatGuid(const GUID *guid, char *buffer, size_t bufferSize)
	{
		if((guid == NULL) || (bufferSize == 0)) {
			return;
		}

		_snprintf(buffer, bufferSize - 1,
			"{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
			guid->Data1, guid->Data2, guid->Data3,
			guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
			guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
		buffer[bufferSize - 1] = '\0';
	}

	void LogRect(const char *label, const RECT &rect)
	{
		PRINT_LOG("%s rect=(%ld,%ld)-(%ld,%ld) size=%ldx%ld",
			label,
			rect.left, rect.top, rect.right, rect.bottom,
			rect.right - rect.left, rect.bottom - rect.top);
	}

	void LogMonitor(const char *label, HMONITOR monitor)
	{
		if(monitor == NULL) {
			PRINT_LOG("%s monitor=NULL", label);
			return;
		}

		MONITORINFOEX monitorInfo;
		memset(&monitorInfo, 0, sizeof(monitorInfo));
		monitorInfo.cbSize = sizeof(monitorInfo);
		if(GetMonitorInfo(monitor, &monitorInfo)) {
			PRINT_LOG("%s monitor=%p device=%s primary=%d rect=(%ld,%ld)-(%ld,%ld)",
				label,
				monitor,
				monitorInfo.szDevice,
				(monitorInfo.dwFlags & MONITORINFOF_PRIMARY) != 0,
				monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.top,
				monitorInfo.rcMonitor.right,
				monitorInfo.rcMonitor.bottom);
		}
		else {
			PRINT_LOG("%s monitor=%p GetMonitorInfo failed err=%lu",
				label, monitor, GetLastError());
		}
	}

	struct DDAdapterSearchContext
	{
		HMONITOR monitor;
		BOOL hasGuid;
		GUID guid;

		DDAdapterSearchContext(HMONITOR pMonitor) :
			monitor(pMonitor), hasGuid(FALSE)
		{
			memset(&guid, 0, sizeof(guid));
		}
	};

	BOOL CALLBACK FindMonitorAdapterCallback(GUID FAR *lpGUID, LPSTR lpDriverDescription,
		LPSTR lpDriverName, LPVOID lpContext, HMONITOR hMonitor)
	{
		DDAdapterSearchContext *ctx =
			reinterpret_cast<DDAdapterSearchContext*>(lpContext);
		(void) lpDriverDescription;
		(void) lpDriverName;

		if(ctx->monitor == hMonitor) {
			char guidBuffer[64] = {0};
			if(lpGUID != NULL) {
				FormatGuid(lpGUID, guidBuffer, sizeof(guidBuffer));
			}
			PRINT_LOG("DD adapter match driver=%s desc=%s guid=%s hMonitor=%p",
				(lpDriverName != NULL) ? lpDriverName : "<null>",
				(lpDriverDescription != NULL) ? lpDriverDescription : "<null>",
				(lpGUID != NULL) ? guidBuffer : "<default>",
				hMonitor);
			LogMonitor("DD adapter match monitor", hMonitor);

			if(lpGUID != NULL) {
				ctx->guid = *lpGUID;
				ctx->hasGuid = TRUE;
			}
			return DDENUMRET_CANCEL;
		}

		if(hMonitor != NULL) {
			char guidBuffer[64] = {0};
			if(lpGUID != NULL) {
				FormatGuid(lpGUID, guidBuffer, sizeof(guidBuffer));
			}
			PRINT_LOG("DD adapter skip driver=%s desc=%s guid=%s hMonitor=%p",
				(lpDriverName != NULL) ? lpDriverName : "<null>",
				(lpDriverDescription != NULL) ? lpDriverDescription : "<null>",
				(lpGUID != NULL) ? guidBuffer : "<default>",
				hMonitor);
		}

		return DDENUMRET_OK;
	}

	HMONITOR FindFullscreenMonitor(HWND window)
	{
		RECT windowRect;
		if(GetWindowRect(window, &windowRect)) {
			POINT center;
			center.x = windowRect.left + ((windowRect.right - windowRect.left) / 2);
			center.y = windowRect.top + ((windowRect.bottom - windowRect.top) / 2);
			return MonitorFromPoint(center, MONITOR_DEFAULTTONEAREST);
		}

		return MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
	}

	static BOOL MakeOpenGLCurrent(MR_OpenGLState *state)
	{
		return (state != NULL) && (state->windowDc != NULL) &&
			(state->context != NULL) &&
			(wglMakeCurrent(state->windowDc, state->context) == TRUE);
	}

	static void ReleaseOpenGLCurrent()
	{
		wglMakeCurrent(NULL, NULL);
	}

	static GLuint CompileShader(GLenum shaderType, const char *source)
	{
		GLuint shader = gGL.CreateShader(shaderType);
		if(shader == 0) {
			return 0;
		}

		gGL.ShaderSource(shader, 1, &source, NULL);
		gGL.CompileShader(shader);

		GLint compiled = GL_FALSE;
		gGL.GetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if(compiled != GL_TRUE) {
			GLint logLength = 0;
			gGL.GetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
			if(logLength > 1) {
				std::vector<char> logBuffer(logLength + 1, '\0');
				gGL.GetShaderInfoLog(shader, logLength, NULL, &logBuffer[0]);
				PRINT_LOG("OpenGL shader compile failed type=%u log=%s",
					(unsigned) shaderType, &logBuffer[0]);
			}
			gGL.DeleteShader(shader);
			return 0;
		}

		return shader;
	}

	static BOOL BuildPaletteShader(MR_OpenGLState *state)
	{
		static const char *VERTEX_SHADER =
			"varying vec2 vTexCoord;\n"
			"void main()\n"
			"{\n"
			"    gl_Position = gl_Vertex;\n"
			"    vTexCoord = gl_MultiTexCoord0.st;\n"
			"}\n";

		static const char *FRAGMENT_SHADER =
			"uniform sampler2D uIndexTex;\n"
			"uniform sampler2D uPaletteTex;\n"
			"varying vec2 vTexCoord;\n"
			"void main()\n"
			"{\n"
			"    float idx = texture2D(uIndexTex, vTexCoord).r;\n"
			"    float paletteX = ((idx * 255.0) + 0.5) / 256.0;\n"
			"    gl_FragColor = texture2D(uPaletteTex, vec2(paletteX, 0.5));\n"
			"}\n";

		if((state == NULL) || !LoadOpenGLFunctions()) {
			return FALSE;
		}

		state->vertexShader = CompileShader(GL_VERTEX_SHADER, VERTEX_SHADER);
		state->fragmentShader = CompileShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER);
		if(state->vertexShader == 0 || state->fragmentShader == 0) {
			return FALSE;
		}

		state->shaderProgram = gGL.CreateProgram();
		if(state->shaderProgram == 0) {
			return FALSE;
		}

		gGL.AttachShader(state->shaderProgram, state->vertexShader);
		gGL.AttachShader(state->shaderProgram, state->fragmentShader);
		gGL.LinkProgram(state->shaderProgram);

		GLint linked = GL_FALSE;
		gGL.GetProgramiv(state->shaderProgram, GL_LINK_STATUS, &linked);
		if(linked != GL_TRUE) {
			GLint logLength = 0;
			gGL.GetProgramiv(state->shaderProgram, GL_INFO_LOG_LENGTH, &logLength);
			if(logLength > 1) {
				std::vector<char> logBuffer(logLength + 1, '\0');
				gGL.GetProgramInfoLog(state->shaderProgram, logLength, NULL, &logBuffer[0]);
				PRINT_LOG("OpenGL shader link failed log=%s", &logBuffer[0]);
			}
			return FALSE;
		}

		state->indexUniform = gGL.GetUniformLocation(state->shaderProgram, "uIndexTex");
		state->paletteUniform = gGL.GetUniformLocation(state->shaderProgram, "uPaletteTex");
		state->shaderReady = TRUE;
		return TRUE;
	}
}

// Computes the run length and shift of the block of ones in a bitmask.
// Example: If the mask is "00011100" then mSize=3 and mShift=2.
void MR_VideoBuffer::Channel::SetMask(DWORD mask)
{
	mShift = 0;
	mSize = 0;

	// Count the zeros on right hand side.
	while(!(mask & 1L)) {
		mask >>= 1;
		mShift++;
	}

	// Count the ones.
	while(mask & 1L) {
		mask >>= 1;
		mSize++;
	}
}

DWORD MR_VideoBuffer::Channel::GetMask() const
{
	if(mSize == 0) {
		return 0;
	}

	if(mSize >= 32) {
		return 0xffffffffUL;
	}

	return (((DWORD) 1 << mSize) - 1) << mShift;
}

// Packs a value into the bitmask for this channel.
DWORD MR_VideoBuffer::Channel::Pack(DWORD intensity) const
{
	intensity >>= (8 - mSize);
	intensity <<= mShift;
	return intensity;
}

MR_VideoBuffer::MR_VideoBuffer(HWND pWindow, double pGamma, double pContrast, double pBrightness)
{
	OPEN_LOG();
	ResetRenderLogStats();
	PRINT_LOG("VIDEO_BUFFER_CREATION");

	ASSERT(pWindow != NULL);

	mWindow = pWindow;
	mDirectDraw = NULL;
	mFrontBuffer = NULL;
	mBackBuffer = NULL;
	mPalette = NULL;
	mZBuffer = NULL;
	mBuffer = NULL;
	mRenderSurface = NULL;
	mClipper = NULL;
	mBackPalette = NULL;
	mPackedPalette = NULL;
	mPaletteTexture = NULL;
	mPaletteDirty = FALSE;
	mOpenGLState = NULL;
	mGpuSceneRenderer = NULL;
	mOpenGLPresentFailureLogCount = 0;
	mOpenGLFrameTraceLogCount = 0;
	mOpenGLLoggedPresentPath = FALSE;
	mStagePerfAccumulatedClearMs = 0;
	mStagePerfAccumulatedBackgroundMs = 0;
	mStagePerfAccumulatedClearZMs = 0;
	mStagePerfAccumulatedFloorMs = 0;
	mStagePerfAccumulatedWallMs = 0;
	mStagePerfAccumulatedWallSetupMs = 0;
	mStagePerfAccumulatedWallLoopMs = 0;
	mStagePerfAccumulatedActorMs = 0;
	mStagePerfMaxClearMs = 0;
	mStagePerfMaxBackgroundMs = 0;
	mStagePerfMaxClearZMs = 0;
	mStagePerfMaxFloorMs = 0;
	mStagePerfMaxWallMs = 0;
	mStagePerfMaxWallSetupMs = 0;
	mStagePerfMaxWallLoopMs = 0;
	mStagePerfMaxActorMs = 0;
	mStagePerfSampleCount = 0;

	mModeSettingInProgress = FALSE;
	mFullScreen = FALSE;
	mX0 = 0;
	mY0 = 0;
	mDisplayXRes = 0;
	mDisplayYRes = 0;
	mXRes = 0;
	mYRes = 0;
	mLineLen = 0;
	mRenderScalePercent = 100;

	mBpp = 0;
	mNativeBpp = 0;

	mIconMode = IsIconic(pWindow);

	mGamma = pGamma;
	mContrast = pContrast;
	mBrightness = pBrightness;

	mSpecialWindowMode = FALSE;
	SetRect(&mFullscreenRect, 0, 0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN));
	mRequestedAdapterGuidValid = FALSE;
	mCurrentAdapterGuidValid = FALSE;
	mUseGdiWindowedPresentFallback = FALSE;
	mWindowedInvalidRectStreak = 0;
	mWindowedPresentFailureStreak = 0;
	mWindowedPresentFailureHr = DD_OK;

	// backported from newer VideoBuffer.cpp
	// Load DirectDraw.
	// As of the June 2010 update of the DirectX SDK, ddraw.lib is no longer
	// included; however, it is possible to still load DirectDraw manually.
	directDrawInst = LoadLibrary("ddraw.dll");
	mGpuSceneRenderer = new MR_GpuSceneRenderer(this);

	/*
	   if( !SetVideoMode() )
	   {

	   }
	 */
}

MR_VideoBuffer::~MR_VideoBuffer()
{
	//   mFullScreen        = TRUE;
	//   mSpecialWindowMode = FALSE;   // force real windows resolution
	ReturnToWindowsResolution();
	ReleaseDirectDraw();
	delete mGpuSceneRenderer;
	mGpuSceneRenderer = NULL;

	delete[]mBackPalette;
	delete[]mPackedPalette;
	delete[]mPaletteTexture;

	LogRenderStats("VIDEO_BUFFER_DESTRUCTION");
	PRINT_LOG("VIDEO_BUFFER_DESTRUCTION\n\n");
	CLOSE_LOG();

}

DWORD MR_VideoBuffer::PackRGB(DWORD r, DWORD g, DWORD b)
{
	return mRChan.Pack(r) | mGChan.Pack(g) | mBChan.Pack(b);
}

void MR_VideoBuffer::SetRequestedAdapterForMonitor(HMONITOR pMonitor)
{
	mRequestedAdapterGuidValid = FALSE;
	LogMonitor("SetRequestedAdapterForMonitor", pMonitor);

	if((pMonitor == NULL) || (directDrawInst == NULL)) {
		PRINT_LOG("SetRequestedAdapterForMonitor using default adapter");
		return;
	}

	LPDIRECTDRAWENUMERATEEX enumerateEx =
		(LPDIRECTDRAWENUMERATEEX)GetProcAddress(directDrawInst, "DirectDrawEnumerateExA");
	if(enumerateEx != NULL) {
		DDAdapterSearchContext ctx(pMonitor);
		if(enumerateEx(FindMonitorAdapterCallback, &ctx,
			DDENUM_ATTACHEDSECONDARYDEVICES) == DD_OK && ctx.hasGuid)
		{
			mRequestedAdapterGuid = ctx.guid;
			mRequestedAdapterGuidValid = TRUE;
		}
	}

	if(mRequestedAdapterGuidValid) {
		char guidBuffer[64] = {0};
		FormatGuid(&mRequestedAdapterGuid, guidBuffer, sizeof(guidBuffer));
		PRINT_LOG("Requested DirectDraw adapter guid=%s", guidBuffer);
	}
	else {
		PRINT_LOG("Requested DirectDraw adapter guid=<default>");
	}
}

BOOL MR_VideoBuffer::PrepareWindowedAdapter()
{
	RECT windowRect;
	if(GetWindowRect(mWindow, &windowRect)) {
		LogRect("PrepareWindowedAdapter window", windowRect);
	}
	SetRequestedAdapterForMonitor(FindFullscreenMonitor(mWindow));
	return TRUE;
}

BOOL MR_VideoBuffer::IsCurrentAdapterRequested() const
{
	if(mRequestedAdapterGuidValid != mCurrentAdapterGuidValid) {
		return FALSE;
	}

	return !mRequestedAdapterGuidValid ||
		IsEqualGUID(mRequestedAdapterGuid, mCurrentAdapterGuid);
}

void MR_VideoBuffer::ComputeRenderResolution(int pDisplayXRes, int pDisplayYRes,
	int &pRenderXRes, int &pRenderYRes) const
{
	pRenderXRes = pDisplayXRes;
	pRenderYRes = pDisplayYRes;
}

void MR_VideoBuffer::ResetWindowedPresentFallback()
{
	mUseGdiWindowedPresentFallback = FALSE;
	mWindowedInvalidRectStreak = 0;
	mWindowedPresentFailureStreak = 0;
	mWindowedPresentFailureHr = DD_OK;
}

BOOL MR_VideoBuffer::PresentWindowedWithGdi()
{
	struct DibBufferInfo
	{
		BITMAPINFOHEADER header;
		DWORD masks[3];
		RGBQUAD colors[256];
	};

	if((mWindow == NULL) || (mBackBuffer == NULL)) {
		gRenderLogStats.gdiPresentFailures++;
		PRINT_LOG("GDI windowed present fallback unavailable window=%p backBuffer=%p",
			mWindow, mBackBuffer);
		return FALSE;
	}

	if(mBpp <= 8) {
		gRenderLogStats.gdiPresentFailures++;
		PRINT_LOG("GDI windowed present fallback unsupported bpp=%lu", mBpp);
		return FALSE;
	}

	const WORD dibBpp =
		(mBpp <= 16) ? 16 :
		((mBpp <= 24) ? 24 : 32);
	if(dibBpp == 0 || mXRes <= 0 || mYRes <= 0 || mDisplayXRes <= 0 || mDisplayYRes <= 0) {
		gRenderLogStats.gdiPresentFailures++;
		PRINT_LOG("GDI windowed present fallback invalid geometry render=%dx%d display=%dx%d bpp=%lu",
			mXRes, mYRes, mDisplayXRes, mDisplayYRes, mBpp);
		return FALSE;
	}

	DDSURFACEDESC surfaceDesc;
	memset(&surfaceDesc, 0, sizeof(surfaceDesc));
	surfaceDesc.dwSize = sizeof(surfaceDesc);
	if(DD_CALL(mBackBuffer->Lock(NULL, &surfaceDesc,
		DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL)) != DD_OK)
	{
		gRenderLogStats.gdiPresentFailures++;
		PRINT_LOG("GDI windowed present fallback could not lock back buffer");
		return FALSE;
	}

	const int sourceStride = surfaceDesc.lPitch;
	const int packedStride = ((mXRes * dibBpp + 31) / 32) * 4;
	const int copyStride = (sourceStride < packedStride) ? sourceStride : packedStride;
	const MR_UInt8 *surfaceBits =
		reinterpret_cast<const MR_UInt8*>(surfaceDesc.lpSurface);
	MR_UInt8 *packedBits = NULL;
	const void *dibBits = surfaceBits;

	if(sourceStride != packedStride) {
		packedBits = new MR_UInt8[packedStride * mYRes];
		memset(packedBits, 0, packedStride * mYRes);
		for(int y = 0; y < mYRes; y++) {
			memcpy(packedBits + (packedStride * y),
				surfaceBits + (sourceStride * y),
				copyStride);
		}
		dibBits = packedBits;
	}

	DibBufferInfo dibInfo;
	memset(&dibInfo, 0, sizeof(dibInfo));
	dibInfo.header.biSize = sizeof(dibInfo.header);
	dibInfo.header.biWidth = mXRes;
	dibInfo.header.biHeight = -mYRes;
	dibInfo.header.biPlanes = 1;
	dibInfo.header.biBitCount = dibBpp;
	dibInfo.header.biSizeImage = packedStride * mYRes;

	if(dibBpp == 16 || dibBpp == 32) {
		dibInfo.header.biCompression = BI_BITFIELDS;
		dibInfo.masks[0] = mRChan.GetMask();
		dibInfo.masks[1] = mGChan.GetMask();
		dibInfo.masks[2] = mBChan.GetMask();
	}
	else {
		dibInfo.header.biCompression = BI_RGB;
	}

	HDC windowDc = GetDC(mWindow);
	BOOL success = FALSE;
	DWORD lastError = 0;
	gRenderLogStats.gdiPresentCount++;

	if(windowDc == NULL) {
		lastError = GetLastError();
		gRenderLogStats.gdiPresentFailures++;
		PRINT_LOG("GDI windowed present fallback GetDC failed err=%lu", lastError);
	}
	else {
		SetStretchBltMode(windowDc, COLORONCOLOR);
		int result = StretchDIBits(windowDc,
			0, 0, mDisplayXRes, mDisplayYRes,
			0, 0, mXRes, mYRes,
			dibBits,
			reinterpret_cast<BITMAPINFO*>(&dibInfo),
			DIB_RGB_COLORS,
			SRCCOPY);
		if(result == GDI_ERROR) {
			lastError = GetLastError();
			gRenderLogStats.gdiPresentFailures++;
			PRINT_LOG("GDI windowed present fallback StretchDIBits failed err=%lu stride=%d packedStride=%d bpp=%u",
				lastError, sourceStride, packedStride, dibBpp);
		}
		else {
			success = TRUE;
		}
		ReleaseDC(mWindow, windowDc);
	}

	if(DD_CALL(mBackBuffer->Unlock(NULL)) != DD_OK) {
		gRenderLogStats.gdiPresentFailures++;
		PRINT_LOG("GDI windowed present fallback unlock failed");
		success = FALSE;
	}

	delete[]packedBits;
	return success;
}

BOOL MR_VideoBuffer::InitOpenGL()
{
	PRINT_LOG("InitOpenGL");

	if(mOpenGLState == NULL) {
		PRINT_LOG("InitOpenGL alloc state");
		mOpenGLState = new MR_OpenGLState();
	}

	if(mOpenGLState->windowDc == NULL) {
		PRINT_LOG("InitOpenGL GetDC begin hwnd=%p", mWindow);
		mOpenGLState->windowDc = GetDC(mWindow);
		if(mOpenGLState->windowDc == NULL) {
			PRINT_LOG("InitOpenGL GetDC failed err=%lu", GetLastError());
			return FALSE;
		}
		PRINT_LOG("InitOpenGL GetDC ok dc=%p", mOpenGLState->windowDc);

		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(pfd));
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 16;
		pfd.iLayerType = PFD_MAIN_PLANE;

		PRINT_LOG("InitOpenGL GetPixelFormat begin");
		int pixelFormat = GetPixelFormat(mOpenGLState->windowDc);
		PRINT_LOG("InitOpenGL GetPixelFormat=%d", pixelFormat);
		if(pixelFormat == 0) {
			PRINT_LOG("InitOpenGL FindWindowOpenGLPixelFormat begin");
			pixelFormat = FindWindowOpenGLPixelFormat(mOpenGLState->windowDc);
			if(pixelFormat == 0) {
				PRINT_LOG("InitOpenGL FindWindowOpenGLPixelFormat failed err=%lu", GetLastError());
				return FALSE;
			}
			PRINT_LOG("InitOpenGL FindWindowOpenGLPixelFormat ok format=%d", pixelFormat);
			if(DescribePixelFormat(mOpenGLState->windowDc, pixelFormat, sizeof(pfd), &pfd) == 0) {
				PRINT_LOG("InitOpenGL DescribePixelFormat failed err=%lu format=%d",
					GetLastError(), pixelFormat);
				return FALSE;
			}
			PRINT_LOG("InitOpenGL SetPixelFormat begin");
			if(!SetPixelFormat(mOpenGLState->windowDc, pixelFormat, &pfd)) {
				PRINT_LOG("InitOpenGL SetPixelFormat failed err=%lu", GetLastError());
				return FALSE;
			}
			PRINT_LOG("InitOpenGL SetPixelFormat ok");
		}
	}

	if(mOpenGLState->context == NULL) {
		PRINT_LOG("InitOpenGL wglCreateContext begin dc=%p", mOpenGLState->windowDc);
		mOpenGLState->context = wglCreateContext(mOpenGLState->windowDc);
		if(mOpenGLState->context == NULL) {
			PRINT_LOG("InitOpenGL wglCreateContext failed err=%lu", GetLastError());
			return FALSE;
		}
		PRINT_LOG("InitOpenGL wglCreateContext ok context=%p", mOpenGLState->context);
	}

	PRINT_LOG("InitOpenGL wglMakeCurrent begin dc=%p context=%p",
		mOpenGLState->windowDc, mOpenGLState->context);
	if(!MakeOpenGLCurrent(mOpenGLState)) {
		PRINT_LOG("InitOpenGL wglMakeCurrent failed err=%lu", GetLastError());
		return FALSE;
	}

	PRINT_LOG("InitOpenGL current dc=%p context=%p", mOpenGLState->windowDc,
		mOpenGLState->context);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glDisable(GL_DITHER);
	glDisable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_TEXTURE_2D);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glColor4ub(255, 255, 255, 255);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	mOpenGLLoggedPresentPath = FALSE;

	if(mOpenGLState->frameTexture == 0) {
		glGenTextures(1, &mOpenGLState->frameTexture);
	}
	if(mOpenGLState->paletteTexture == 0) {
		glGenTextures(1, &mOpenGLState->paletteTexture);
	}

	{
		const OpenGLShaderBringupMode shaderMode = GetOpenGLShaderBringupMode();
		PRINT_LOG("InitOpenGL shaderStage=%s", GetOpenGLShaderBringupModeName(shaderMode));
		if(shaderMode != OGL_SHADER_BRINGUP_OFF && !mOpenGLState->shaderReady) {
			if(!BuildPaletteShader(mOpenGLState)) {
				PRINT_LOG("InitOpenGL indexed shader build failed; falling back to cpu_rgb");
				mOpenGLState->shaderReady = FALSE;
			}
		}
		else if(shaderMode == OGL_SHADER_BRINGUP_OFF) {
			mOpenGLState->shaderReady = FALSE;
		}
	}

	PRINT_LOG("InitOpenGL shaderReady=%d frameTexture=%u paletteTexture=%u",
		(int) mOpenGLState->shaderReady,
		(unsigned) mOpenGLState->frameTexture,
		(unsigned) mOpenGLState->paletteTexture);
	LogOpenGLErrors("InitOpenGL");
	ReleaseOpenGLCurrent();

	mBpp = 32;
	if(mNativeBpp == 0) {
		mNativeBpp = 32;
	}

	return TRUE;
}

void MR_VideoBuffer::ReleaseOpenGL()
{
	if(mOpenGLState == NULL) {
		return;
	}

	if((mOpenGLState->context != NULL) && MakeOpenGLCurrent(mOpenGLState)) {
		if(mOpenGLState->frameTexture != 0) {
			glDeleteTextures(1, &mOpenGLState->frameTexture);
			mOpenGLState->frameTexture = 0;
		}
		if(mOpenGLState->paletteTexture != 0) {
			glDeleteTextures(1, &mOpenGLState->paletteTexture);
			mOpenGLState->paletteTexture = 0;
		}
		for(size_t lIndex = 0; lIndex < mOpenGLState->cachedBitmapTextures.size(); lIndex++) {
			if(mOpenGLState->cachedBitmapTextures[lIndex].texture != 0) {
				glDeleteTextures(1, &mOpenGLState->cachedBitmapTextures[lIndex].texture);
			}
		}
		mOpenGLState->cachedBitmapTextures.clear();
		if(mOpenGLState->shaderProgram != 0 && gGL.DeleteProgram != NULL) {
			gGL.DeleteProgram(mOpenGLState->shaderProgram);
			mOpenGLState->shaderProgram = 0;
		}
		if(mOpenGLState->vertexShader != 0 && gGL.DeleteShader != NULL) {
			gGL.DeleteShader(mOpenGLState->vertexShader);
			mOpenGLState->vertexShader = 0;
		}
		if(mOpenGLState->fragmentShader != 0 && gGL.DeleteShader != NULL) {
			gGL.DeleteShader(mOpenGLState->fragmentShader);
			mOpenGLState->fragmentShader = 0;
		}
		wglMakeCurrent(NULL, NULL);
	}

	if(mOpenGLState->context != NULL) {
		wglDeleteContext(mOpenGLState->context);
		mOpenGLState->context = NULL;
	}
	if(mOpenGLState->windowDc != NULL) {
		ReleaseDC(mWindow, mOpenGLState->windowDc);
		mOpenGLState->windowDc = NULL;
	}

	delete mOpenGLState;
	mOpenGLState = NULL;
}

BOOL MR_VideoBuffer::EnsureOpenGLResources()
{
	OpenGLShaderBringupMode shaderMode = GetOpenGLShaderBringupMode();
	BOOL useShaderUploads = (mOpenGLState != NULL) &&
		mOpenGLState->shaderReady &&
		(shaderMode == OGL_SHADER_BRINGUP_UPLOADS || shaderMode == OGL_SHADER_BRINGUP_DRAW);

	if(!InitOpenGL()) {
		return FALSE;
	}

	if(!MakeOpenGLCurrent(mOpenGLState)) {
		PRINT_LOG("EnsureOpenGLResources wglMakeCurrent failed err=%lu", GetLastError());
		return FALSE;
	}

	glBindTexture(GL_TEXTURE_2D, mOpenGLState->frameTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	useShaderUploads = mOpenGLState->shaderReady &&
		(shaderMode == OGL_SHADER_BRINGUP_UPLOADS || shaderMode == OGL_SHADER_BRINGUP_DRAW);
	if(useShaderUploads) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8, mXRes, mYRes, 0,
			GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

		glBindTexture(GL_TEXTURE_2D, mOpenGLState->paletteTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, mPaletteTexture);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mXRes, mYRes, 0,
			GL_RGB, GL_UNSIGNED_BYTE, NULL);
		mOpenGLState->rgbaFallback.resize(mXRes * mYRes * 3);
	}

	mOpenGLState->textureWidth = mXRes;
	mOpenGLState->textureHeight = mYRes;
	mPaletteDirty = TRUE;
	mOpenGLPresentFailureLogCount = 0;
	mOpenGLFrameTraceLogCount = 0;
	mOpenGLLoggedPresentPath = FALSE;
	PRINT_LOG("EnsureOpenGLResources shaderReady=%d shaderUploads=%d render=%dx%d display=%dx%d",
		(int) mOpenGLState->shaderReady, (int) useShaderUploads,
		mXRes, mYRes, mDisplayXRes, mDisplayYRes);
	LogOpenGLErrors("EnsureOpenGLResources");
	ReleaseOpenGLCurrent();
	return TRUE;
}

BOOL MR_VideoBuffer::PresentOpenGL()
{
	OpenGLShaderBringupMode shaderMode = GetOpenGLShaderBringupMode();
	BOOL useShaderPath = mOpenGLState != NULL &&
		mOpenGLState->shaderReady &&
		(shaderMode == OGL_SHADER_BRINGUP_UPLOADS || shaderMode == OGL_SHADER_BRINGUP_DRAW);

	if((mOpenGLState == NULL) || (mRenderSurface == NULL)) {
		if(mOpenGLPresentFailureLogCount < 10) {
			PRINT_LOG("PresentOpenGL missing state gl=%p render=%p",
				mOpenGLState, mRenderSurface);
		}
		mOpenGLPresentFailureLogCount++;
		return FALSE;
	}

	if(!mOpenGLLoggedPresentPath) {
		PRINT_LOG("OpenGL present path=%s shaderStage=%s",
			useShaderPath ? "indexed_shader" : "cpu_rgb",
			GetOpenGLShaderBringupModeName(shaderMode));
		mOpenGLLoggedPresentPath = TRUE;
	}

recompute_path:
	useShaderPath = mOpenGLState->shaderReady &&
		(shaderMode == OGL_SHADER_BRINGUP_UPLOADS || shaderMode == OGL_SHADER_BRINGUP_DRAW);

	if(!MakeOpenGLCurrent(mOpenGLState)) {
		if(mOpenGLPresentFailureLogCount < 10) {
			PRINT_LOG("PresentOpenGL wglMakeCurrent failed err=%lu dc=%p context=%p",
				GetLastError(), mOpenGLState->windowDc, mOpenGLState->context);
		}
		mOpenGLPresentFailureLogCount++;
		return FALSE;
	}

	if(mPaletteDirty && useShaderPath &&
		(shaderMode == OGL_SHADER_BRINGUP_UPLOADS || shaderMode == OGL_SHADER_BRINGUP_DRAW))
	{
		glBindTexture(GL_TEXTURE_2D, mOpenGLState->paletteTexture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1,
			GL_RGBA, GL_UNSIGNED_BYTE, mPaletteTexture);
		if(!LogOpenGLErrors("PresentOpenGL palette upload")) {
			PRINT_LOG("OpenGL present shader fallback -> cpu_rgb reason=palette_upload");
			mOpenGLState->shaderReady = FALSE;
			glBindTexture(GL_TEXTURE_2D, mOpenGLState->frameTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mXRes, mYRes, 0,
				GL_RGB, GL_UNSIGNED_BYTE, NULL);
			mOpenGLState->rgbaFallback.resize(
				static_cast<size_t>(mXRes) * static_cast<size_t>(mYRes) * 3);
			mPaletteDirty = FALSE;
			if(gGL.UseProgram != NULL) {
				gGL.UseProgram(0);
			}
			mOpenGLLoggedPresentPath = FALSE;
			ReleaseOpenGLCurrent();
			goto recompute_path;
		}
		mPaletteDirty = FALSE;
	}

	glBindTexture(GL_TEXTURE_2D, mOpenGLState->frameTexture);
	if(useShaderPath &&
		(shaderMode == OGL_SHADER_BRINGUP_UPLOADS || shaderMode == OGL_SHADER_BRINGUP_DRAW))
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mXRes, mYRes,
			GL_LUMINANCE, GL_UNSIGNED_BYTE, mRenderSurface);
		if(!LogOpenGLErrors("PresentOpenGL indexed upload")) {
			PRINT_LOG("OpenGL present shader fallback -> cpu_rgb reason=indexed_upload");
			mOpenGLState->shaderReady = FALSE;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mXRes, mYRes, 0,
				GL_RGB, GL_UNSIGNED_BYTE, NULL);
			mOpenGLState->rgbaFallback.resize(
				static_cast<size_t>(mXRes) * static_cast<size_t>(mYRes) * 3);
			if(gGL.UseProgram != NULL) {
				gGL.UseProgram(0);
			}
			mOpenGLLoggedPresentPath = FALSE;
			ReleaseOpenGLCurrent();
			goto recompute_path;
		}
	}
	else {
		size_t pixelCount = static_cast<size_t>(mXRes) * static_cast<size_t>(mYRes);
		if(mOpenGLState->rgbaFallback.size() != pixelCount * 3) {
			mOpenGLState->rgbaFallback.resize(pixelCount * 3);
		}
		for(size_t i = 0; i < pixelCount; i++) {
			const MR_UInt8 idx = mRenderSurface[i];
			mOpenGLState->rgbaFallback[(i * 3) + 0] = mPaletteTexture[(idx * 4) + 0];
			mOpenGLState->rgbaFallback[(i * 3) + 1] = mPaletteTexture[(idx * 4) + 1];
			mOpenGLState->rgbaFallback[(i * 3) + 2] = mPaletteTexture[(idx * 4) + 2];
		}
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mXRes, mYRes,
			GL_RGB, GL_UNSIGNED_BYTE, &mOpenGLState->rgbaFallback[0]);
		if(!LogOpenGLErrors("PresentOpenGL rgba upload")) {
			mOpenGLPresentFailureLogCount++;
			ReleaseOpenGLCurrent();
			return FALSE;
		}
	}

	glViewport(0, 0, mDisplayXRes, mDisplayYRes);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_FRAMEBUFFER_SRGB);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glColor4ub(255, 255, 255, 255);
	glClear(GL_COLOR_BUFFER_BIT);

	if(useShaderPath && shaderMode == OGL_SHADER_BRINGUP_DRAW) {
		gGL.UseProgram(mOpenGLState->shaderProgram);
		gGL.ActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mOpenGLState->frameTexture);
		gGL.Uniform1i(mOpenGLState->indexUniform, 0);
		gGL.ActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mOpenGLState->paletteTexture);
		gGL.Uniform1i(mOpenGLState->paletteUniform, 1);
		gGL.ActiveTexture(GL_TEXTURE0);
	}
	else {
		if(gGL.UseProgram != NULL) {
			gGL.UseProgram(0);
		}
	}

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, -1.0f);
	glEnd();
	if(!LogOpenGLErrors("PresentOpenGL draw")) {
		if(useShaderPath) {
			PRINT_LOG("OpenGL present shader fallback -> cpu_rgb reason=draw");
			mOpenGLState->shaderReady = FALSE;
			glBindTexture(GL_TEXTURE_2D, mOpenGLState->frameTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mXRes, mYRes, 0,
				GL_RGB, GL_UNSIGNED_BYTE, NULL);
			mOpenGLState->rgbaFallback.resize(
				static_cast<size_t>(mXRes) * static_cast<size_t>(mYRes) * 3);
			if(gGL.UseProgram != NULL) {
				gGL.UseProgram(0);
			}
			mOpenGLLoggedPresentPath = FALSE;
			ReleaseOpenGLCurrent();
			goto recompute_path;
		}
		mOpenGLPresentFailureLogCount++;
		ReleaseOpenGLCurrent();
		return FALSE;
	}

	if(useShaderPath && shaderMode == OGL_SHADER_BRINGUP_DRAW) {
		gGL.UseProgram(0);
	}

	RenderGpuSceneOverlay();

	if(!SwapBuffers(mOpenGLState->windowDc)) {
		if(mOpenGLPresentFailureLogCount < 10) {
			PRINT_LOG("PresentOpenGL SwapBuffers failed err=%lu dc=%p",
				GetLastError(), mOpenGLState->windowDc);
		}
		mOpenGLPresentFailureLogCount++;
		ReleaseOpenGLCurrent();
		return FALSE;
	}

	mOpenGLPresentFailureLogCount = 0;
	ReleaseOpenGLCurrent();
	return TRUE;
}

BOOL MR_VideoBuffer::InitDirectDraw()
{
	PRINT_LOG("InitDirectDraw");

	BOOL lReturnValue = TRUE;
	char requestedGuidBuffer[64] = {0};
	char currentGuidBuffer[64] = {0};

	if(mRequestedAdapterGuidValid) {
		FormatGuid(&mRequestedAdapterGuid, requestedGuidBuffer, sizeof(requestedGuidBuffer));
	}
	if(mCurrentAdapterGuidValid) {
		FormatGuid(&mCurrentAdapterGuid, currentGuidBuffer, sizeof(currentGuidBuffer));
	}

	PRINT_LOG("InitDirectDraw requestedValid=%d requested=%s currentValid=%d current=%s existingDD=%p fullscreen=%d",
		mRequestedAdapterGuidValid,
		mRequestedAdapterGuidValid ? requestedGuidBuffer : "<default>",
		mCurrentAdapterGuidValid,
		mCurrentAdapterGuidValid ? currentGuidBuffer : "<default>",
		mDirectDraw,
		mFullScreen);

	if((mDirectDraw != NULL) && !IsCurrentAdapterRequested()) {
		PRINT_LOG("InitDirectDraw releasing DirectDraw because adapter request changed");
		if(mFullScreen) {
			DD_CALL(mDirectDraw->RestoreDisplayMode());
			DD_CALL(mDirectDraw->SetCooperativeLevel(mWindow, DDSCL_NORMAL));
			mFullScreen = FALSE;
		}

		ReleaseDirectDraw();
	}

	if(mDirectDraw == NULL) {
		typedef HRESULT (WINAPI* LPDIRECTDRAWCREATE)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);
		LPDIRECTDRAWCREATE directDrawCreate = (LPDIRECTDRAWCREATE)GetProcAddress(directDrawInst, "DirectDrawCreate");
		GUID FAR *requestedGuid = mRequestedAdapterGuidValid ? &mRequestedAdapterGuid : NULL;

		if (directDrawCreate == NULL) {
			PRINT_LOG("InitDirectDraw DirectDrawCreate export missing");
			return false;
		}

		if(DD_CALL(directDrawCreate(requestedGuid, &mDirectDraw, NULL)) != DD_OK) {
			if((requestedGuid != NULL) &&
				(DD_CALL(directDrawCreate(NULL, &mDirectDraw, NULL)) == DD_OK))
			{
				char guidBuffer[64] = {0};
				FormatGuid(&mRequestedAdapterGuid, guidBuffer, sizeof(guidBuffer));
				PRINT_LOG("InitDirectDraw fell back to default adapter after failing guid=%s", guidBuffer);
				mRequestedAdapterGuidValid = FALSE;
			}
			else {
				PRINT_LOG("InitDirectDraw failed to create DirectDraw requested=%s",
					(requestedGuid != NULL) ? requestedGuidBuffer : "<default>");
				ASSERT(FALSE);
				lReturnValue = false;
			}
		}
		if(lReturnValue) {
			mCurrentAdapterGuidValid = mRequestedAdapterGuidValid;
			if(mCurrentAdapterGuidValid) {
				mCurrentAdapterGuid = mRequestedAdapterGuid;
			}

			PRINT_LOG("InitDirectDraw created DirectDraw=%p currentAdapter=%s",
				mDirectDraw,
				mCurrentAdapterGuidValid ? requestedGuidBuffer : "<default>");

			if(DD_CALL(mDirectDraw->SetCooperativeLevel(mWindow, DDSCL_NORMAL)) != DD_OK) {
				PRINT_LOG("InitDirectDraw SetCooperativeLevel(DDSCL_NORMAL) failed");
				ASSERT(FALSE);
				lReturnValue = false;
			}
		}
	}

	if(lReturnValue) {
		// Keep track of the native pixel format so we can blit later.
		DDSURFACEDESC lSurfaceDesc;
		memset(&lSurfaceDesc, 0, sizeof(lSurfaceDesc));
		lSurfaceDesc.dwSize = sizeof(lSurfaceDesc);

		if(DD_CALL(mDirectDraw->GetDisplayMode(&lSurfaceDesc)) != DD_OK) {
			lReturnValue = FALSE;
		}
		else {
			lReturnValue = ProcessCurrentBpp(lSurfaceDesc.ddpfPixelFormat);
			if(lReturnValue) {
				// We make the assumption that the desktop color depth
				// won't change while we're running.
				if(mNativeBpp == 0)
					mNativeBpp = mBpp;
			}
		}
	}

	if(mPalette == NULL) {
		// Create a palette
		CreatePalette(mGamma, mContrast, mBrightness);
	}

	return lReturnValue;
}

void MR_VideoBuffer::ReleaseDirectDraw()
{
	DeleteInternalSurfaces();
	ReleaseOpenGL();

	if(mPalette != NULL) {
		mPalette->Release();
		mPalette = NULL;
	}

	if(mDirectDraw != NULL) {
		mDirectDraw->Release();
		mDirectDraw = NULL;
	}

	mCurrentAdapterGuidValid = FALSE;
}

BOOL MR_VideoBuffer::ProcessCurrentBpp(const DDPIXELFORMAT & lFormat)
{
	if(lFormat.dwFlags & DDPF_PALETTEINDEXED8) {
		mBpp = 8;
		PRINT_LOG("BPP: Indexed");
		// Don't need to process the pixel format since we
		// can just copy the bits.
	}
	else {
		mBpp = lFormat.dwRGBBitCount;
		PRINT_LOG("BPP: %d", mBpp);
		if(mBpp < 8 || mBpp > 32) {
			// Interesting!  Not quite sure how to deal with this.
			PRINT_LOG("Unhandled RGB bpp: %d", mBpp);
			return FALSE;
		}
		else {
			mRChan.SetMask(lFormat.dwRBitMask);
			mGChan.SetMask(lFormat.dwGBitMask);
			mBChan.SetMask(lFormat.dwBBitMask);
		}
	}
	return TRUE;
}

BOOL MR_VideoBuffer::TryToSetColorMode(int colorBits)
{
	(void) colorBits;
	return FALSE;
}

void MR_VideoBuffer::DeleteInternalSurfaces()
{
	PRINT_LOG("DeleteInternalSurfaces");

	ASSERT(mBuffer == NULL);					  // should be unlock

	if((mOpenGLState != NULL) && MakeOpenGLCurrent(mOpenGLState)) {
		if(mOpenGLState->frameTexture != 0) {
			glDeleteTextures(1, &mOpenGLState->frameTexture);
			mOpenGLState->frameTexture = 0;
		}
		if(mOpenGLState->paletteTexture != 0) {
			glDeleteTextures(1, &mOpenGLState->paletteTexture);
			mOpenGLState->paletteTexture = 0;
		}
		for(size_t lIndex = 0; lIndex < mOpenGLState->cachedBitmapTextures.size(); lIndex++) {
			if(mOpenGLState->cachedBitmapTextures[lIndex].texture != 0) {
				glDeleteTextures(1, &mOpenGLState->cachedBitmapTextures[lIndex].texture);
			}
		}
		mOpenGLState->cachedBitmapTextures.clear();
		mOpenGLState->textureWidth = 0;
		mOpenGLState->textureHeight = 0;
		mOpenGLState->rgbaFallback.clear();
	}

	if(mDirectDraw != NULL) {
		if(mBackBuffer != NULL) {
			DD_CALL(mBackBuffer->Release());
			mBackBuffer = NULL;
		}

		if(mFrontBuffer != NULL) {
			DD_CALL(mFrontBuffer->Release());
			mFrontBuffer = NULL;
		}

		if(mClipper != NULL) {
			DD_CALL(mClipper->Release());
			mClipper = NULL;
		}
	}
	delete[]mZBuffer;
	mZBuffer = NULL;
	delete[]mRenderSurface;
	mRenderSurface = NULL;
	mBuffer = NULL;
}

void MR_VideoBuffer::CreatePalette(double pGamma, double pContrast, double pBrightness)
{
	PRINT_LOG("CreatePalette");

	PALETTEENTRY lPalette[256];

	int lCounter;

	mGamma = pGamma;
	mContrast = pContrast;
	mBrightness = pBrightness;

	if(mGamma < 0.2) {
		mGamma = 0.2;
	}

	if(mGamma > 4.0) {
		mGamma = 4.0;
	}

	if(mContrast > 1.0) {
		mContrast = 1.0;
	}

	if(mContrast < 0.3) {
		mContrast = 0.3;
	}

	if(mBrightness > 1) {
		mBrightness = 1.0;
	}

	if(mBrightness < 0.3) {
		mBrightness = 0.3;
	}
	// Clean existing pallette
	if(mPalette != NULL) {
		mPalette->Release();
		mPalette = NULL;
	}

	// Initialize with system colors (Ignore errors)
	HDC hdc = GetDC(NULL);
	if(GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) {
		// get the current windows colors.
		GetSystemPaletteEntries(hdc, 0, 256, lPalette);
	}
	ReleaseDC(NULL, hdc);

	// Add our own entries.
	PALETTEENTRY *lOurEntries = MR_GetColors(1.0 / mGamma, mContrast * mBrightness, mBrightness - (mContrast * mBrightness));

	for(lCounter = 0; lCounter < MR_BASIC_COLORS; lCounter++) {
		lPalette[MR_RESERVED_COLORS_BEGINNING + lCounter] = lOurEntries[lCounter];
		lPalette[MR_RESERVED_COLORS_BEGINNING + lCounter].peFlags = PC_NOCOLLAPSE;
	}
	delete[]lOurEntries;

	if(mBackPalette != NULL) {
		for(lCounter = 0; lCounter < MR_BACK_COLORS; lCounter++) {
			lPalette[MR_RESERVED_COLORS_BEGINNING + MR_BASIC_COLORS + lCounter] =
				MR_ConvertColor(mBackPalette[lCounter * 3], mBackPalette[lCounter * 3 + 1], mBackPalette[lCounter * 3 + 2],
					1.0 / mGamma, mContrast * mBrightness, mBrightness - (mContrast * mBrightness));
			lPalette[MR_RESERVED_COLORS_BEGINNING + MR_BASIC_COLORS + lCounter].peFlags = PC_NOCOLLAPSE;
		}
	}

	for(lCounter = 0; lCounter < MR_RESERVED_COLORS_BEGINNING; lCounter++) {
		lPalette[lCounter].peFlags = 0;
	}

	if(mPackedPalette == NULL) {
		mPackedPalette = new DWORD[256];
	}
	if(mPaletteTexture == NULL) {
		mPaletteTexture = new MR_UInt8[256 * 4];
	}

	for(int i = 0; i < 256; i++) {
		mPackedPalette[i] = PackRGB(lPalette[i].peRed, lPalette[i].peGreen, lPalette[i].peBlue);
		mPaletteTexture[(i * 4) + 0] = ApplyLegacyPresentationCurve(lPalette[i].peRed);
		mPaletteTexture[(i * 4) + 1] = ApplyLegacyPresentationCurve(lPalette[i].peGreen);
		mPaletteTexture[(i * 4) + 2] = ApplyLegacyPresentationCurve(lPalette[i].peBlue);
		mPaletteTexture[(i * 4) + 3] = 255;
	}
	mPaletteDirty = TRUE;

	if(mDirectDraw != NULL) {
		if(DD_CALL(mDirectDraw->CreatePalette(DDPCAPS_8BIT, lPalette, &mPalette, NULL)) != DD_OK) {
			ASSERT(FALSE);
			mPalette = NULL;
		}
	}

}

void MR_VideoBuffer::GetPaletteAttrib(double &pGamma, double &pContrast, double &pBrightness)
{
	pGamma = mGamma;
	pContrast = mContrast;
	pBrightness = mBrightness;
}

void MR_VideoBuffer::SetBackPalette(MR_UInt8 * pPalette)
{
	delete[]mBackPalette;
	mBackPalette = pPalette;

	CreatePalette(mGamma, mContrast, mBrightness);
}

MR_GpuSceneRenderer *MR_VideoBuffer::GetGpuSceneRenderer() const
{
	return mGpuSceneRenderer;
}

void MR_VideoBuffer::SetOpenGLColorFromPaletteIndex(MR_UInt8 pColorIndex, unsigned char pAlpha) const
{
	if(mPaletteTexture != NULL) {
		const int lOffset = pColorIndex * 4;
		glColor4ub(mPaletteTexture[lOffset + 0], mPaletteTexture[lOffset + 1],
			mPaletteTexture[lOffset + 2], pAlpha);
	}
	else {
		glColor4ub(pColorIndex, pColorIndex, pColorIndex, pAlpha);
	}
}

unsigned int MR_VideoBuffer::GetOrCreateGpuBitmapTexture(const MR_Bitmap *pBitmap, int pSubBitmap,
	int &pWidth, int &pHeight)
{
	pWidth = 0;
	pHeight = 0;

	if((mOpenGLState == NULL) || (pBitmap == NULL)) {
		return 0;
	}
	if((pSubBitmap < 0) || (pSubBitmap >= pBitmap->GetNbSubBitmap())) {
		return 0;
	}

	for(size_t lIndex = 0; lIndex < mOpenGLState->cachedBitmapTextures.size(); lIndex++) {
		MR_OpenGLState::CachedBitmapTexture &lEntry = mOpenGLState->cachedBitmapTextures[lIndex];
		if((lEntry.bitmap == pBitmap) && (lEntry.subBitmap == pSubBitmap)) {
			pWidth = lEntry.width;
			pHeight = lEntry.height;
			return lEntry.texture;
		}
	}

	pWidth = pBitmap->GetXRes(pSubBitmap);
	pHeight = pBitmap->GetYRes(pSubBitmap);
	if((pWidth <= 0) || (pHeight <= 0)) {
		return 0;
	}

	const MR_UInt8 *lSource = pBitmap->GetBuffer(pSubBitmap);
	if(lSource == NULL) {
		return 0;
	}

	std::vector<MR_UInt8> lRgbBuffer(static_cast<size_t>(pWidth) * static_cast<size_t>(pHeight) * 3);
	for(int lY = 0; lY < pHeight; lY++) {
		for(int lX = 0; lX < pWidth; lX++) {
			const MR_UInt8 lIndex = lSource[lX * pHeight + lY];
			const size_t lDestOffset =
				(static_cast<size_t>(lY) * static_cast<size_t>(pWidth) + static_cast<size_t>(lX)) * 3;
			lRgbBuffer[lDestOffset + 0] = mPaletteTexture[(lIndex * 4) + 0];
			lRgbBuffer[lDestOffset + 1] = mPaletteTexture[(lIndex * 4) + 1];
			lRgbBuffer[lDestOffset + 2] = mPaletteTexture[(lIndex * 4) + 2];
		}
	}

	GLuint lTexture = 0;
	glGenTextures(1, &lTexture);
	if(lTexture == 0) {
		return 0;
	}

	glBindTexture(GL_TEXTURE_2D, lTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pWidth, pHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE, &lRgbBuffer[0]);
	if(!LogOpenGLErrors("CreateGpuBitmapTexture")) {
		glDeleteTextures(1, &lTexture);
		return 0;
	}

	MR_OpenGLState::CachedBitmapTexture lEntry;
	lEntry.bitmap = pBitmap;
	lEntry.subBitmap = pSubBitmap;
	lEntry.texture = lTexture;
	lEntry.width = pWidth;
	lEntry.height = pHeight;
	mOpenGLState->cachedBitmapTextures.push_back(lEntry);

	return lTexture;
}

void MR_VideoBuffer::RenderGpuSceneOverlay()
{
	if((mGpuSceneRenderer == NULL) || !mGpuSceneRenderer->IsEnabled()) {
		return;
	}

	const MR_GpuSceneFrame &lFrame = mGpuSceneRenderer->GetFrame();
	if(lFrame.mViewport.right <= lFrame.mViewport.left
		|| lFrame.mViewport.bottom <= lFrame.mViewport.top) {
		return;
	}

	if(lFrame.mWalls.empty()
		&& lFrame.mBitmapPatches.empty() && lFrame.mColorPatches.empty()) {
		return;
	}

	if(gGL.UseProgram != NULL) {
		gGL.UseProgram(0);
	}
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glLineWidth(1.5f);

	const GLdouble lNearPlane = max(1.0, static_cast<GLdouble>(lFrame.mPlanDist));
	const GLdouble lFarPlane = max(lNearPlane + 1.0, 2000000.0);
	const GLdouble lScrollNdc = -2.0 * static_cast<GLdouble>(lFrame.mScroll) /
		max(1, lFrame.mViewport.bottom - lFrame.mViewport.top);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glFrustum(-static_cast<GLdouble>(lFrame.mPlanHW),
		static_cast<GLdouble>(lFrame.mPlanHW),
		-static_cast<GLdouble>(lFrame.mPlanVW),
		static_cast<GLdouble>(lFrame.mPlanVW),
		lNearPlane,
		lFarPlane);
	glTranslated(0.0, lScrollNdc, 0.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	for(size_t lWallIndex = 0; lWallIndex < lFrame.mWalls.size(); lWallIndex++) {
		const MR_GpuSceneWall &lWall = lFrame.mWalls[lWallIndex];
		const MR_3DCoordinate lTopLeft = lWall.mUpperLeft;
		const MR_3DCoordinate lTopRight(lWall.mLowerRight.mX, lWall.mLowerRight.mY, lWall.mUpperLeft.mZ);
		const MR_3DCoordinate lBottomRight = lWall.mLowerRight;
		const MR_3DCoordinate lBottomLeft(lWall.mUpperLeft.mX, lWall.mUpperLeft.mY, lWall.mLowerRight.mZ);
		const double lWallHeight = static_cast<double>(lWall.mUpperLeft.mZ - lWall.mLowerRight.mZ);
		const double lWallLength = max(1.0, static_cast<double>(lWall.mLen));
		int lSegmentCount = max(1, min(64, lWall.mLen / 2000));
		MR_GpuSceneProjectedVertex lEndProjected[4];
		BOOL lAnyEndVisible = TRUE;

		if(!ProjectGpuSceneVertex(lFrame, lTopLeft, lEndProjected[0])
			|| !ProjectGpuSceneVertex(lFrame, lTopRight, lEndProjected[1])
			|| !ProjectGpuSceneVertex(lFrame, lBottomRight, lEndProjected[2])
			|| !ProjectGpuSceneVertex(lFrame, lBottomLeft, lEndProjected[3])) {
			lAnyEndVisible = FALSE;
		}
		if(lAnyEndVisible) {
			const double lProjectedWidth =
				fabs(static_cast<double>(lEndProjected[1].mX - lEndProjected[0].mX))
				* (lFrame.mViewport.right - lFrame.mViewport.left) * 0.5;
			lSegmentCount = max(lSegmentCount, min(64, static_cast<int>(lProjectedWidth / 96.0)));
		}

		if((lWall.mPrimaryBitmap == NULL) || (lWallHeight <= 0.0)) {
			continue;
		}

		glEnable(GL_TEXTURE_2D);
		for(int lSegment = 0; lSegment < lSegmentCount; lSegment++) {
			const double lT0 = static_cast<double>(lSegment) / lSegmentCount;
			const double lT1 = static_cast<double>(lSegment + 1) / lSegmentCount;
			MR_3DCoordinate lVertices[4];
			MR_GpuSceneCameraVertex lCameraVertices[4];
			BOOL lVisible = TRUE;

			lVertices[0] = LerpGpuSceneCoordinate(lTopLeft, lTopRight, lT0);
			lVertices[1] = LerpGpuSceneCoordinate(lTopLeft, lTopRight, lT1);
			lVertices[2] = LerpGpuSceneCoordinate(lBottomLeft, lBottomRight, lT1);
			lVertices[3] = LerpGpuSceneCoordinate(lBottomLeft, lBottomRight, lT0);

			for(int lVertex = 0; lVertex < 4; lVertex++) {
				if(!TransformGpuSceneVertexToCameraSpace(lFrame, lVertices[lVertex], lCameraVertices[lVertex])) {
					lVisible = FALSE;
					break;
				}
			}

			if(!lVisible) {
				continue;
			}

			GLdouble lLeftClipT = 0.0;
			GLdouble lRightClipT = 1.0;
			const BOOL lLeftInside = (lCameraVertices[0].mZ <= -lNearPlane);
			const BOOL lRightInside = (lCameraVertices[1].mZ <= -lNearPlane);

			if(!lLeftInside && !lRightInside) {
				continue;
			}
			if(lLeftInside != lRightInside) {
				const GLdouble lDenominator = lCameraVertices[1].mZ - lCameraVertices[0].mZ;
				if(lDenominator == 0.0) {
					continue;
				}

				GLdouble lClipT = (-lNearPlane - lCameraVertices[0].mZ) / lDenominator;
				if(lClipT < 0.0) {
					lClipT = 0.0;
				}
				else if(lClipT > 1.0) {
					lClipT = 1.0;
				}

				if(!lLeftInside) {
					lLeftClipT = lClipT;
					lCameraVertices[0].mX = LerpGpuSceneDouble(lCameraVertices[0].mX, lCameraVertices[1].mX, lClipT);
					lCameraVertices[0].mY = LerpGpuSceneDouble(lCameraVertices[0].mY, lCameraVertices[1].mY, lClipT);
					lCameraVertices[0].mZ = -lNearPlane;
					lCameraVertices[3].mX = LerpGpuSceneDouble(lCameraVertices[3].mX, lCameraVertices[2].mX, lClipT);
					lCameraVertices[3].mY = LerpGpuSceneDouble(lCameraVertices[3].mY, lCameraVertices[2].mY, lClipT);
					lCameraVertices[3].mZ = -lNearPlane;
				}
				else {
					lRightClipT = lClipT;
					lCameraVertices[1].mX = LerpGpuSceneDouble(lCameraVertices[0].mX, lCameraVertices[1].mX, lClipT);
					lCameraVertices[1].mY = LerpGpuSceneDouble(lCameraVertices[0].mY, lCameraVertices[1].mY, lClipT);
					lCameraVertices[1].mZ = -lNearPlane;
					lCameraVertices[2].mX = LerpGpuSceneDouble(lCameraVertices[3].mX, lCameraVertices[2].mX, lClipT);
					lCameraVertices[2].mY = LerpGpuSceneDouble(lCameraVertices[3].mY, lCameraVertices[2].mY, lClipT);
					lCameraVertices[2].mZ = -lNearPlane;
				}
			}

			MR_GpuSceneProjectedVertex lProjectedVertices[4];
			for(int lVertex = 0; lVertex < 4; lVertex++) {
				ProjectGpuSceneCameraVertex(lFrame, lCameraVertices[lVertex], lProjectedVertices[lVertex]);
			}

			{
				const GLdouble lEdge1X = lCameraVertices[1].mX - lCameraVertices[0].mX;
				const GLdouble lEdge1Y = lCameraVertices[1].mY - lCameraVertices[0].mY;
				const GLdouble lEdge1Z = lCameraVertices[1].mZ - lCameraVertices[0].mZ;
				const GLdouble lEdge2X = lCameraVertices[3].mX - lCameraVertices[0].mX;
				const GLdouble lEdge2Y = lCameraVertices[3].mY - lCameraVertices[0].mY;
				const GLdouble lEdge2Z = lCameraVertices[3].mZ - lCameraVertices[0].mZ;
				const GLdouble lNormalX = (lEdge1Y * lEdge2Z) - (lEdge1Z * lEdge2Y);
				const GLdouble lNormalY = (lEdge1Z * lEdge2X) - (lEdge1X * lEdge2Z);
				const GLdouble lNormalZ = (lEdge1X * lEdge2Y) - (lEdge1Y * lEdge2X);
				const GLdouble lFacing = (lNormalX * lCameraVertices[0].mX)
					+ (lNormalY * lCameraVertices[0].mY)
					+ (lNormalZ * lCameraVertices[0].mZ);

				if(lFacing <= 0.0) {
					continue;
				}
			}

			int lWallTextureWidth = 0;
			int lWallTextureHeight = 0;
			GLuint lWallTexture = 0;
			int lSegmentProjectedHeight = max(1,
				static_cast<int>(max(
					fabs(static_cast<double>(lProjectedVertices[3].mY - lProjectedVertices[0].mY)),
					fabs(static_cast<double>(lProjectedVertices[2].mY - lProjectedVertices[1].mY)))
					* (lFrame.mViewport.bottom - lFrame.mViewport.top) * 0.5));
			const int lBitmapWidth = max(1, lWall.mPrimaryBitmap->GetWidth());
			const int lBitmapHeightMm = max(1, lWall.mPrimaryBitmap->GetHeight());
			int lBitmapRepeatCount =
				static_cast<int>((lWall.mLen + (lBitmapWidth / 2)) / lBitmapWidth);
			int lBitmapHeightRepeatCount =
				static_cast<int>((static_cast<int>(lWallHeight) + (lBitmapHeightMm / 2)) / lBitmapHeightMm);
			const BOOL lUseFittedHeight = (lWallHeight > lBitmapHeightMm);
			int lWallSubBitmap = 0;

			if(lBitmapRepeatCount < 1) {
				lBitmapRepeatCount = 1;
			}
			if(lBitmapHeightRepeatCount < 1) {
				lBitmapHeightRepeatCount = 1;
			}

			if(lWall.mPrimaryBitmap->GetNbSubBitmap() > 1) {
				int lBitmapTileProjectedHeight;

				if(lUseFittedHeight) {
					lBitmapTileProjectedHeight = max(1,
						(lSegmentProjectedHeight + (lBitmapHeightRepeatCount / 2)) / lBitmapHeightRepeatCount);
				}
				else {
					lBitmapTileProjectedHeight = max(1,
						MulDiv(lSegmentProjectedHeight, lBitmapHeightMm,
							max(1, static_cast<int>(lWallHeight))));
				}

				lWallSubBitmap = lWall.mPrimaryBitmap->GetBestBitmapForYRes(
					lBitmapTileProjectedHeight);
				if(lWallSubBitmap < 0) {
					lWallSubBitmap = 0;
				}
			}
			lWallTexture = GetOrCreateGpuBitmapTexture(lWall.mPrimaryBitmap, lWallSubBitmap,
				lWallTextureWidth, lWallTextureHeight);

			if(lWallTexture != 0) {
				if(lBitmapRepeatCount < 1) {
					lBitmapRepeatCount = 1;
				}
				if(lBitmapHeightRepeatCount < 1) {
					lBitmapHeightRepeatCount = 1;
				}

				const double lTotalURepeat = static_cast<double>(lBitmapRepeatCount);
				const double lU0 = lTotalURepeat * (lT0 + ((lT1 - lT0) * lLeftClipT));
				const double lU1 = lTotalURepeat * (lT0 + ((lT1 - lT0) * lRightClipT));
				const double lVRepeat = lUseFittedHeight
					? static_cast<double>(lBitmapHeightRepeatCount)
					: (lWallHeight / lBitmapHeightMm);
				const double lVTopLeft = 0.0;
				const double lVTopRight = 0.0;
				const double lVBottomLeft = lVRepeat;
				const double lVBottomRight = lVRepeat;

				glBindTexture(GL_TEXTURE_2D, lWallTexture);
				glColor4ub(255, 255, 255, 255);
				glBegin(GL_QUADS);
				glTexCoord2f(static_cast<GLfloat>(lU0), static_cast<GLfloat>(lVTopLeft)); glVertex3d(lCameraVertices[0].mX, lCameraVertices[0].mY, lCameraVertices[0].mZ);
				glTexCoord2f(static_cast<GLfloat>(lU1), static_cast<GLfloat>(lVTopRight)); glVertex3d(lCameraVertices[1].mX, lCameraVertices[1].mY, lCameraVertices[1].mZ);
				glTexCoord2f(static_cast<GLfloat>(lU1), static_cast<GLfloat>(lVBottomRight)); glVertex3d(lCameraVertices[2].mX, lCameraVertices[2].mY, lCameraVertices[2].mZ);
				glTexCoord2f(static_cast<GLfloat>(lU0), static_cast<GLfloat>(lVBottomLeft)); glVertex3d(lCameraVertices[3].mX, lCameraVertices[3].mY, lCameraVertices[3].mZ);
				glEnd();
			}
			else {
				SetOpenGLColorFromPaletteIndex(lWall.mPrimaryBitmap->GetPlainColor(), 96);
				glBegin(GL_QUADS);
				for(int lVertex = 0; lVertex < 4; lVertex++) {
					glVertex3d(lCameraVertices[lVertex].mX, lCameraVertices[lVertex].mY, lCameraVertices[lVertex].mZ);
				}
				glEnd();
			}
		}
		glDisable(GL_TEXTURE_2D);

	}

	for(size_t lPatchIndex = 0; lPatchIndex < lFrame.mBitmapPatches.size(); lPatchIndex++) {
		const MR_GpuScenePatchBitmap &lPatch = lFrame.mBitmapPatches[lPatchIndex];
		if((lPatch.mPatch == NULL) || (lPatch.mBitmap == NULL)) {
			continue;
		}

		const int lURes = lPatch.mPatch->GetURes();
		const int lVRes = lPatch.mPatch->GetVRes();
		if((lURes < 2) || (lVRes < 2)) {
			continue;
		}

		const MR_3DCoordinate *lNodeList = lPatch.mPatch->GetNodeList();
		if(lNodeList == NULL) {
			continue;
		}

		int lTextureWidth = 0;
		int lTextureHeight = 0;
		GLuint lTexture = GetOrCreateGpuBitmapTexture(lPatch.mBitmap, 0,
			lTextureWidth, lTextureHeight);
		if(lTexture == 0) {
			continue;
		}

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, lTexture);
		glColor4ub(255, 255, 255, 255);

		for(int lV = 0; lV < (lVRes - 1); lV++) {
			for(int lU = 0; lU < (lURes - 1); lU++) {
				const int lBase = lV * lURes + lU;
				const int lIndices[4] = {
					lBase,
					lBase + 1,
					lBase + lURes + 1,
					lBase + lURes
				};
				const GLfloat lUCoord[4] = {
					static_cast<GLfloat>(lU) / static_cast<GLfloat>(lURes - 1),
					static_cast<GLfloat>(lU + 1) / static_cast<GLfloat>(lURes - 1),
					static_cast<GLfloat>(lU + 1) / static_cast<GLfloat>(lURes - 1),
					static_cast<GLfloat>(lU) / static_cast<GLfloat>(lURes - 1)
				};
				const GLfloat lVCoord[4] = {
					static_cast<GLfloat>(lV) / static_cast<GLfloat>(lVRes - 1),
					static_cast<GLfloat>(lV) / static_cast<GLfloat>(lVRes - 1),
					static_cast<GLfloat>(lV + 1) / static_cast<GLfloat>(lVRes - 1),
					static_cast<GLfloat>(lV + 1) / static_cast<GLfloat>(lVRes - 1)
				};
				std::vector<MR_GpuSceneTexturedVertex> lVertices;
				lVertices.reserve(4);

				for(int lVertex = 0; lVertex < 4; lVertex++) {
					MR_3DCoordinate lWorldVertex;
					MR_GpuSceneTexturedVertex lTexturedVertex;
					ApplyGpuScenePositionMatrix(lPatch.mMatrix, lNodeList[lIndices[lVertex]], lWorldVertex);
					if(!TransformGpuSceneVertexToCameraSpace(lFrame, lWorldVertex, lTexturedVertex.mCamera)) {
						lTexturedVertex.mCamera.mVisible = FALSE;
					}
					lTexturedVertex.mU = lUCoord[lVertex];
					lTexturedVertex.mV = lVCoord[lVertex];
					lVertices.push_back(lTexturedVertex);
				}

				ClipGpuSceneTexturedPolygonToNearPlane(lVertices, -lNearPlane);
				if(lVertices.size() < 3) {
					continue;
				}

				glBegin(GL_POLYGON);
				for(size_t lVertex = 0; lVertex < lVertices.size(); lVertex++) {
					glTexCoord2f(lVertices[lVertex].mU, lVertices[lVertex].mV);
					glVertex3d(lVertices[lVertex].mCamera.mX,
						lVertices[lVertex].mCamera.mY,
						lVertices[lVertex].mCamera.mZ);
				}
				glEnd();
			}
		}
	}

	glDisable(GL_TEXTURE_2D);

	for(size_t lPatchIndex = 0; lPatchIndex < lFrame.mColorPatches.size(); lPatchIndex++) {
		const MR_GpuScenePatchColor &lPatch = lFrame.mColorPatches[lPatchIndex];
		if(lPatch.mPatch == NULL) {
			continue;
		}

		const int lURes = lPatch.mPatch->GetURes();
		const int lVRes = lPatch.mPatch->GetVRes();
		if((lURes < 2) || (lVRes < 2)) {
			continue;
		}

		const MR_3DCoordinate *lNodeList = lPatch.mPatch->GetNodeList();
		if(lNodeList == NULL) {
			continue;
		}

		SetOpenGLColorFromPaletteIndex(lPatch.mColor, 255);

		for(int lV = 0; lV < (lVRes - 1); lV++) {
			for(int lU = 0; lU < (lURes - 1); lU++) {
				const int lBase = lV * lURes + lU;
				const int lIndices[4] = {
					lBase,
					lBase + 1,
					lBase + lURes + 1,
					lBase + lURes
				};
				std::vector<MR_GpuSceneColoredVertex> lVertices;
				lVertices.reserve(4);

				for(int lVertex = 0; lVertex < 4; lVertex++) {
					MR_3DCoordinate lWorldVertex;
					MR_GpuSceneColoredVertex lColoredVertex;
					ApplyGpuScenePositionMatrix(lPatch.mMatrix, lNodeList[lIndices[lVertex]], lWorldVertex);
					if(!TransformGpuSceneVertexToCameraSpace(lFrame, lWorldVertex, lColoredVertex.mCamera)) {
						lColoredVertex.mCamera.mVisible = FALSE;
					}
					lVertices.push_back(lColoredVertex);
				}

				ClipGpuSceneColoredPolygonToNearPlane(lVertices, -lNearPlane);
				if(lVertices.size() < 3) {
					continue;
				}

				glBegin(GL_POLYGON);
				for(size_t lVertex = 0; lVertex < lVertices.size(); lVertex++) {
					glVertex3d(lVertices[lVertex].mCamera.mX,
						lVertices[lVertex].mCamera.mY,
						lVertices[lVertex].mCamera.mZ);
				}
				glEnd();
			}
		}
	}

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
}

void MR_VideoBuffer::LogPerformanceSample(DWORD pFrameAvgMs, DWORD pCpuAvgMs, DWORD pPresentAvgMs,
	DWORD pFrameMaxMs, DWORD pCpuMaxMs, DWORD pPresentMaxMs, int pSampleCount)
{
	PRINT_LOG("FrameTiming samples=%d avgFrame=%lu avgCpu=%lu avgPresent=%lu maxFrame=%lu maxCpu=%lu maxPresent=%lu mode=%s render=%dx%d display=%dx%d",
		pSampleCount,
		pFrameAvgMs,
		pCpuAvgMs,
		pPresentAvgMs,
		pFrameMaxMs,
		pCpuMaxMs,
		pPresentMaxMs,
		mFullScreen ? "fullscreen" : "windowed",
		mXRes,
		mYRes,
		mDisplayXRes,
		mDisplayYRes);
}

void MR_VideoBuffer::LogRenderStageSample(DWORD pClearMs, DWORD pBackgroundMs, DWORD pClearZMs, DWORD pFloorMs,
	DWORD pWallMs, DWORD pWallSetupMs, DWORD pWallLoopMs, DWORD pActorMs)
{
	enum { PERF_LOG_SAMPLE_COUNT = 120 };

	mStagePerfAccumulatedClearMs += pClearMs;
	mStagePerfAccumulatedBackgroundMs += pBackgroundMs;
	mStagePerfAccumulatedClearZMs += pClearZMs;
	mStagePerfAccumulatedFloorMs += pFloorMs;
	mStagePerfAccumulatedWallMs += pWallMs;
	mStagePerfAccumulatedWallSetupMs += pWallSetupMs;
	mStagePerfAccumulatedWallLoopMs += pWallLoopMs;
	mStagePerfAccumulatedActorMs += pActorMs;

	if(pClearMs > mStagePerfMaxClearMs) {
		mStagePerfMaxClearMs = pClearMs;
	}
	if(pBackgroundMs > mStagePerfMaxBackgroundMs) {
		mStagePerfMaxBackgroundMs = pBackgroundMs;
	}
	if(pClearZMs > mStagePerfMaxClearZMs) {
		mStagePerfMaxClearZMs = pClearZMs;
	}
	if(pFloorMs > mStagePerfMaxFloorMs) {
		mStagePerfMaxFloorMs = pFloorMs;
	}
	if(pWallMs > mStagePerfMaxWallMs) {
		mStagePerfMaxWallMs = pWallMs;
	}
	if(pWallSetupMs > mStagePerfMaxWallSetupMs) {
		mStagePerfMaxWallSetupMs = pWallSetupMs;
	}
	if(pWallLoopMs > mStagePerfMaxWallLoopMs) {
		mStagePerfMaxWallLoopMs = pWallLoopMs;
	}
	if(pActorMs > mStagePerfMaxActorMs) {
		mStagePerfMaxActorMs = pActorMs;
	}

	mStagePerfSampleCount++;
	if(mStagePerfSampleCount >= PERF_LOG_SAMPLE_COUNT) {
		PRINT_LOG("RenderStageTiming samples=%d avgClear=%lu avgBackground=%lu avgClearZ=%lu avgFloor=%lu avgWall=%lu avgWallSetup=%lu avgWallLoop=%lu avgActor=%lu maxClear=%lu maxBackground=%lu maxClearZ=%lu maxFloor=%lu maxWall=%lu maxWallSetup=%lu maxWallLoop=%lu maxActor=%lu render=%dx%d display=%dx%d",
			mStagePerfSampleCount,
			mStagePerfAccumulatedClearMs / mStagePerfSampleCount,
			mStagePerfAccumulatedBackgroundMs / mStagePerfSampleCount,
			mStagePerfAccumulatedClearZMs / mStagePerfSampleCount,
			mStagePerfAccumulatedFloorMs / mStagePerfSampleCount,
			mStagePerfAccumulatedWallMs / mStagePerfSampleCount,
			mStagePerfAccumulatedWallSetupMs / mStagePerfSampleCount,
			mStagePerfAccumulatedWallLoopMs / mStagePerfSampleCount,
			mStagePerfAccumulatedActorMs / mStagePerfSampleCount,
			mStagePerfMaxClearMs,
			mStagePerfMaxBackgroundMs,
			mStagePerfMaxClearZMs,
			mStagePerfMaxFloorMs,
			mStagePerfMaxWallMs,
			mStagePerfMaxWallSetupMs,
			mStagePerfMaxWallLoopMs,
			mStagePerfMaxActorMs,
			mXRes,
			mYRes,
			mDisplayXRes,
			mDisplayYRes);
		mStagePerfAccumulatedClearMs = 0;
		mStagePerfAccumulatedBackgroundMs = 0;
		mStagePerfAccumulatedClearZMs = 0;
		mStagePerfAccumulatedFloorMs = 0;
		mStagePerfAccumulatedWallMs = 0;
		mStagePerfAccumulatedWallSetupMs = 0;
		mStagePerfAccumulatedWallLoopMs = 0;
		mStagePerfAccumulatedActorMs = 0;
		mStagePerfMaxClearMs = 0;
		mStagePerfMaxBackgroundMs = 0;
		mStagePerfMaxClearZMs = 0;
		mStagePerfMaxFloorMs = 0;
		mStagePerfMaxWallMs = 0;
		mStagePerfMaxWallSetupMs = 0;
		mStagePerfMaxWallLoopMs = 0;
		mStagePerfMaxActorMs = 0;
		mStagePerfSampleCount = 0;
	}
}

void MR_VideoBuffer::AssignPalette()
{
	PRINT_LOG("AssignPalette");

	mPaletteDirty = TRUE;

	if((mBpp == 8) && (mFrontBuffer != NULL) && (mPalette != NULL)) {
		DD_CALL(mFrontBuffer->SetPalette(mPalette));
	}

	/*
	   if( (mBackBuffer != NULL)&&( mPalette!=NULL)&&!mFullScreen )
	   {
	   HRESULT lErrorCode = mBackBuffer->SetPalette( mPalette );
	   }
	 */

}

void MR_VideoBuffer::ReturnToWindowsResolution()
{
	PRINT_LOG("ReturnToWindowsResolution");

	DeleteInternalSurfaces();

	if(mFullScreen) {
		mFullScreen = FALSE;
		SetWindowLong(mWindow, GWL_EXSTYLE, mOriginalExStyle);
		SetWindowLong(mWindow, GWL_STYLE, mOriginalStyle);
		SetWindowPos(mWindow, HWND_NOTOPMOST,
			mOriginalPos.left, mOriginalPos.top,
			mOriginalPos.right - mOriginalPos.left,
			mOriginalPos.bottom - mOriginalPos.top,
			SWP_SHOWWINDOW | SWP_FRAMECHANGED);
		mBpp = 32;
		CreatePalette(mGamma, mContrast, mBrightness);
	}
}

BOOL MR_VideoBuffer::SetVideoMode()
{
	PRINT_LOG("SetVideoMode(Window)");

	BOOL lReturnValue = TRUE;
	RECT windowRect;

	ASSERT(!mModeSettingInProgress);

	mModeSettingInProgress = TRUE;
	ResetWindowedPresentFallback();

	if(GetWindowRect(mWindow, &windowRect)) {
		LogRect("SetVideoMode(Window) window", windowRect);
	}

	ReturnToWindowsResolution();
	DeleteInternalSurfaces();
	RECT lRect;
	lReturnValue = GetClientRect(mWindow, &lRect);
	if(lReturnValue) {
		mDisplayXRes = lRect.right;
		mDisplayYRes = lRect.bottom;
		mXRes = mDisplayXRes;
		mYRes = mDisplayYRes;
		mLineLen = mXRes;
		mRenderScalePercent = 100;
		mBpp = 32;
		if(mNativeBpp == 0) {
			mNativeBpp = 32;
		}
		PRINT_LOG("SetVideoMode(Window) display=%dx%d render=%dx%d scale=%d%%",
			mDisplayXRes, mDisplayYRes, mXRes, mYRes, mRenderScalePercent);
	}

	if(lReturnValue) {
		POINT lPoint = { 0, 0 };
		lReturnValue = ClientToScreen(mWindow, &lPoint);
		mX0 = lPoint.x;
		mY0 = lPoint.y;
	}

	if(lReturnValue) {
		delete[]mRenderSurface;
		mRenderSurface = new MR_UInt8[mXRes * mYRes];
		mZBuffer = new MR_UInt16[mXRes * mYRes];
		if(mRenderSurface == NULL || mZBuffer == NULL) {
			lReturnValue = FALSE;
		}
	}

	if(lReturnValue) {
		lReturnValue = EnsureOpenGLResources();
	}

	if(lReturnValue) {
		CreatePalette(mGamma, mContrast, mBrightness);
	}

	if(!lReturnValue) {
		DeleteInternalSurfaces();
	}
	// AssignPalette();

	mModeSettingInProgress = FALSE;

	return lReturnValue;
}

BOOL MR_VideoBuffer::SetVideoMode(int pXRes, int pYRes)
{
	PRINT_LOG("SetVideoMode %dx%d", pXRes, pYRes);

	BOOL lReturnValue = TRUE;
	RECT targetRect;

	ASSERT(!mModeSettingInProgress);

	mModeSettingInProgress = TRUE;
	ResetWindowedPresentFallback();

	lReturnValue = PrepareDesktopFullscreen(NULL);
	if(lReturnValue) {
		targetRect = mFullscreenRect;
		if(!mFullScreen) {
			mOriginalExStyle = GetWindowLong(mWindow, GWL_EXSTYLE);
			mOriginalStyle = GetWindowLong(mWindow, GWL_STYLE);
			GetWindowRect(mWindow, &mOriginalPos);
		}

		DeleteInternalSurfaces();

		SetWindowLong(mWindow, GWL_EXSTYLE, mOriginalExStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE | WS_EX_WINDOWEDGE));
		SetWindowLong(mWindow, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		SetWindowPos(mWindow, HWND_TOPMOST,
			targetRect.left, targetRect.top,
			targetRect.right - targetRect.left,
			targetRect.bottom - targetRect.top,
			SWP_SHOWWINDOW | SWP_FRAMECHANGED);

		mDisplayXRes = targetRect.right - targetRect.left;
		mDisplayYRes = targetRect.bottom - targetRect.top;
		mXRes = mDisplayXRes;
		mYRes = mDisplayYRes;
		mLineLen = mXRes;
		mRenderScalePercent = 100;
		mBpp = 32;
		if(mNativeBpp == 0) {
			mNativeBpp = 32;
		}
		mFullScreen = TRUE;
	}

	if(lReturnValue) {
		mX0 = targetRect.left;
		mY0 = targetRect.top;
		delete[]mRenderSurface;
		mRenderSurface = new MR_UInt8[mXRes * mYRes];
		mZBuffer = new MR_UInt16[mXRes * mYRes];
		if(mRenderSurface == NULL || mZBuffer == NULL) {
			lReturnValue = FALSE;
		}
	}

	if(lReturnValue) {
		lReturnValue = EnsureOpenGLResources();
	}
	if(lReturnValue) {
		CreatePalette(mGamma, mContrast, mBrightness);
	}

	if(!lReturnValue) {
		ReturnToWindowsResolution();
	}

	mModeSettingInProgress = FALSE;

	return lReturnValue;
}

BOOL MR_VideoBuffer::PrepareDesktopFullscreen(POINT *pResolution)
{
	mRequestedAdapterGuidValid = FALSE;
	RECT windowRect;
	if(GetWindowRect(mWindow, &windowRect)) {
		LogRect("PrepareDesktopFullscreen window", windowRect);
	}

	HMONITOR monitor = FindFullscreenMonitor(mWindow);
	if(monitor == NULL) {
		PRINT_LOG("PrepareDesktopFullscreen could not find monitor");
		return FALSE;
	}
	LogMonitor("PrepareDesktopFullscreen target", monitor);

	MONITORINFOEX monitorInfo;
	memset(&monitorInfo, 0, sizeof(monitorInfo));
	monitorInfo.cbSize = sizeof(monitorInfo);
	if(!GetMonitorInfo(monitor, &monitorInfo)) {
		return FALSE;
	}

	DEVMODE displayMode;
	memset(&displayMode, 0, sizeof(displayMode));
	displayMode.dmSize = sizeof(displayMode);
	if(!EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &displayMode)) {
		PRINT_LOG("PrepareDesktopFullscreen EnumDisplaySettings failed for %s", monitorInfo.szDevice);
		return FALSE;
	}

	PRINT_LOG("PrepareDesktopFullscreen device=%s resolution=%ldx%ld position=%ld,%ld",
		monitorInfo.szDevice,
		displayMode.dmPelsWidth,
		displayMode.dmPelsHeight,
		displayMode.dmPosition.x,
		displayMode.dmPosition.y);

	if(pResolution != NULL) {
		pResolution->x = displayMode.dmPelsWidth;
		pResolution->y = displayMode.dmPelsHeight;
	}

	mFullscreenRect = monitorInfo.rcMonitor;
	SetRequestedAdapterForMonitor(monitor);

	return TRUE;
}

BOOL MR_VideoBuffer::IsWindowMode() const
{
	return !mFullScreen;
}

BOOL MR_VideoBuffer::IsIconMode() const
{
	return mIconMode;
}

BOOL MR_VideoBuffer::IsModeSettingInProgress() const
{
	return mModeSettingInProgress;
}

int MR_VideoBuffer::GetXRes() const
{
	return mXRes;
}

int MR_VideoBuffer::GetYRes() const
{
	return mYRes;
}

int MR_VideoBuffer::GetDisplayXRes() const
{
	return mDisplayXRes;
}

int MR_VideoBuffer::GetDisplayYRes() const
{
	return mDisplayYRes;
}

int MR_VideoBuffer::GetLineLen() const
{
	return mLineLen;
}

int MR_VideoBuffer::GetZLineLen() const
{
	return mXRes;
}

MR_UInt8 *MR_VideoBuffer::GetBuffer()
{
	return mBuffer;
}

MR_UInt16 *MR_VideoBuffer::GetZBuffer()
{
	return mZBuffer;
}

int MR_VideoBuffer::GetXPixelMeter() const
{
	if(mFullScreen) {
		return mXRes * 3;
	}
	else {
		return 3 * GetSystemMetrics(SM_CXSCREEN);
	}
}

int MR_VideoBuffer::GetYPixelMeter() const
{
	if(mFullScreen) {
		return mYRes * 4;
	}
	else {
		return 4 * GetSystemMetrics(SM_CYSCREEN);
	}
}

int MR_VideoBuffer::GetRenderScalePercent() const
{
	return 100;
}

void MR_VideoBuffer::SetRenderScalePercent(int pPercent)
{
	(void) pPercent;
	if(mRenderScalePercent != 100) {
		PRINT_LOG("SetRenderScalePercent forcing native 100%%");
	}
	mRenderScalePercent = 100;
}

BOOL MR_VideoBuffer::Lock()
{
	MR_SAMPLE_CONTEXT("LockVideoBuffer");

	BOOL lReturnValue = TRUE;

	gRenderLogStats.lockCount++;

	ASSERT(mBuffer == NULL);

	if(mIconMode) {
		lReturnValue = FALSE;
	}

	if((mRenderSurface == NULL) || (mOpenGLState == NULL)) {
		lReturnValue = FALSE;
	}

	if(lReturnValue) {
		mBuffer = mRenderSurface;
		mLineLen = mXRes;
		if(mOpenGLFrameTraceLogCount < 10) {
			PRINT_LOG("Lock success buffer=%p lineLen=%d xres=%d yres=%d",
				mBuffer, mLineLen, mXRes, mYRes);
		}
	}

	if(!lReturnValue) {
		gRenderLogStats.lockFailures++;
		if(mOpenGLFrameTraceLogCount < 10) {
			PRINT_LOG("Lock failed icon=%d render=%p gl=%p",
				(int) mIconMode, mRenderSurface, mOpenGLState);
		}
	}

	return lReturnValue;
}

void MR_VideoBuffer::Unlock()
{
	MR_SAMPLE_CONTEXT("UnlockVideoBuffer");
	gRenderLogStats.unlockCount++;

	ASSERT(mBuffer != NULL);
	if(mOpenGLFrameTraceLogCount < 10) {
		PRINT_LOG("Unlock begin buffer=%p", mBuffer);
	}
	mBuffer = NULL;

	Flip();
}

void MR_VideoBuffer::Flip()
{
	gRenderLogStats.flipCount++;

	ASSERT(mBuffer == NULL);
	if(mOpenGLFrameTraceLogCount < 10) {
		PRINT_LOG("Flip begin count=%d", (int) gRenderLogStats.flipCount);
	}
	if(!PresentOpenGL()) {
		gRenderLogStats.flipFailures++;
	}
	if(mOpenGLFrameTraceLogCount < 10) {
		mOpenGLFrameTraceLogCount++;
	}
}

void MR_VideoBuffer::Clear(MR_UInt8 pColor)
{
	ASSERT(mBuffer != NULL);
	if(mOpenGLFrameTraceLogCount < 10) {
		PRINT_LOG("Clear color=%u lineLen=%d yres=%d", (unsigned) pColor, mLineLen, mYRes);
	}

	memset(mBuffer, pColor, mLineLen * mYRes);
}

void MR_VideoBuffer::EnterIconMode()
{
	PRINT_LOG("EnterIconMode");

	if(!mIconMode) {
		mIconMode = TRUE;

		/*
		   if( !mFullScreen )
		   {
		   mBeforeIconXRes = 0;
		   mBeforeIconYRes = 0;
		   }
		   else
		   {
		   // mBeforeIconXRes = 0;
		   // mBeforeIconYRes = 0;

		   mBeforeIconXRes = mXRes;
		   mBeforeIconYRes = mYRes;
		   }
		 */
	}
}

void MR_VideoBuffer::ExitIconMode()
{
	PRINT_LOG("ExitIconMode");

	if(mIconMode) {
		mIconMode = FALSE;

		/*
		   if( mBeforeIconXRes != 0 )
		   {
		   mFullScreen = TRUE;
		   SetVideoMode();

		   // SetFocus( mWindow );
		   // SetVideoMode( mBeforeIconXRes, mBeforeIconYRes );
		   }
		 */

	}
}
