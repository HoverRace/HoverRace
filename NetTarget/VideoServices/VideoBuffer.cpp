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
#include "ColorPalette.h"

#include "../Util/Profiler.h"
#include "../Util/Config.h"

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

namespace {
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
	mClipper = NULL;
	mBackPalette = NULL;
	mPackedPalette = NULL;

	mModeSettingInProgress = FALSE;
	mFullScreen = FALSE;

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

	// backported from newer VideoBuffer.cpp
	// Load DirectDraw.
	// As of the June 2010 update of the DirectX SDK, ddraw.lib is no longer
	// included; however, it is possible to still load DirectDraw manually.
	directDrawInst = LoadLibrary("ddraw.dll");

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

	delete[]mBackPalette;
	delete[]mPackedPalette;

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

void MR_VideoBuffer::ResetWindowedPresentFallback()
{
	mUseGdiWindowedPresentFallback = FALSE;
	mWindowedInvalidRectStreak = 0;
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
	if(dibBpp == 0 || mXRes <= 0 || mYRes <= 0) {
		gRenderLogStats.gdiPresentFailures++;
		PRINT_LOG("GDI windowed present fallback invalid geometry size=%dx%d bpp=%lu",
			mXRes, mYRes, mBpp);
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
			0, 0, mXRes, mYRes,
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

	// do it only if it is safe
	if(!mSpecialWindowMode && !mFullScreen && !mModeSettingInProgress && (mDirectDraw != NULL)) {
		// Retrieve current mode info
		mSpecialModeXRes = GetSystemMetrics(SM_CXSCREEN);
		mSpecialModeYRes = GetSystemMetrics(SM_CYSCREEN);

		GetWindowRect(mWindow, &mOriginalPos);

		if(DD_CALL(mDirectDraw->SetCooperativeLevel(mWindow, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX | DDSCL_ALLOWREBOOT | DDSCL_NOWINDOWCHANGES)) == DD_OK) {
			if(DD_CALL(mDirectDraw->SetDisplayMode(mSpecialModeXRes, mSpecialModeYRes, colorBits)) == DD_OK) {
				mSpecialWindowMode = TRUE;

				// Resize window to it's original position
												  // This parameter have no effect when used here(I dont know why?? ask Bill)
				SetWindowPos(mWindow, HWND_NOTOPMOST,
					mOriginalPos.left, mOriginalPos.top, mOriginalPos.right - mOriginalPos.left, mOriginalPos.bottom - mOriginalPos.top, SWP_SHOWWINDOW /*SWP_NOACTIVATE */ );
			}
			else {
				DD_CALL(mDirectDraw->SetCooperativeLevel(mWindow, DDSCL_NORMAL));

				// It failled BOF
				ASSERT(FALSE);
			}
		}

	}

	return mSpecialWindowMode;
}

void MR_VideoBuffer::DeleteInternalSurfaces()
{
	PRINT_LOG("DeleteInternalSurfaces");

	ASSERT(mBuffer == NULL);					  // should be unlock

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

	if(mDirectDraw != NULL) {
		// Initialize with system colors (Ignore errors)
		HDC hdc = GetDC(NULL);
		if(GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) {
			// get the current windows colors.
			GetSystemPaletteEntries(hdc, 0, 256, lPalette);
		}
		else {
			// ASSERT( FALSE );
		}
		ReleaseDC(NULL, hdc);

		// Add out own entries
		PALETTEENTRY *lOurEntries = MR_GetColors(1.0 / mGamma, mContrast * mBrightness, mBrightness - (mContrast * mBrightness));

		for(lCounter = 0; lCounter < MR_BASIC_COLORS; lCounter++) {
			lPalette[MR_RESERVED_COLORS_BEGINNING + lCounter] = lOurEntries[lCounter];
												  //|*/PC_EXPLICIT; //lPalette[ 0 ].peFlags;
			lPalette[MR_RESERVED_COLORS_BEGINNING + lCounter].peFlags = PC_NOCOLLAPSE;
		}
		delete[]lOurEntries;

		if(mBackPalette != NULL) {
			for(lCounter = 0; lCounter < MR_BACK_COLORS; lCounter++) {
				lPalette[MR_RESERVED_COLORS_BEGINNING + MR_BASIC_COLORS + lCounter] = MR_ConvertColor(mBackPalette[lCounter * 3], mBackPalette[lCounter * 3 + 1], mBackPalette[lCounter * 3 + 2], 1.0 / mGamma, mContrast * mBrightness, mBrightness - (mContrast * mBrightness));
												  //|*/PC_EXPLICIT; //lPalette[ 0 ].peFlags;
				lPalette[MR_RESERVED_COLORS_BEGINNING + MR_BASIC_COLORS + lCounter].peFlags = PC_NOCOLLAPSE;

			}
		}

		for(lCounter = 0; lCounter < MR_RESERVED_COLORS_BEGINNING; lCounter++) {
			lPalette[lCounter].peFlags = 0;		  //PC_NOCOLLAPSE; //lPalette[ 0 ].peFlags;
		}

		// Generate the packed palette.
		if(mBpp > 8) {
			if(mPackedPalette == NULL) {
				mPackedPalette = new DWORD[256];
			}
			for(int i = 0; i < 256; i++) {
				mPackedPalette[i] = PackRGB(lPalette[i].peRed, lPalette[i].peGreen, lPalette[i].peBlue);
				PRINT_LOG("Palette entry %d is %08xd", i, mPackedPalette[i]);
			}
		}
		// Create the palette
		if(DD_CALL(mDirectDraw->CreatePalette(DDPCAPS_8BIT /*|DDPCAPS_ALLOW256 */ , lPalette, &mPalette, NULL)) != DD_OK) {
		ASSERT(FALSE);
		mPalette = NULL;
	}
	// Assign the palette to the existing buffers
	// AssignPalette();
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

void MR_VideoBuffer::AssignPalette()
{
	PRINT_LOG("AssignPalette");

	// Currently only work in 8bit mode
	if(mBpp != 8) {
		PRINT_LOG("AssignPalette skipped because bpp=%d", mBpp);
		return;
	}

	if((mFrontBuffer != NULL) && (mPalette != NULL)) {
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

	/*
	   if( mSpecialMode && (GetActiveWindow()==mWindow ))
	   {
	   mFullScreen = TRUE;
	   }
	 */

	if(mDirectDraw && mFullScreen) {
		mFullScreen = FALSE;

		if(!mSpecialWindowMode) {
			DD_CALL(mDirectDraw->RestoreDisplayMode());

			if(DD_CALL(mDirectDraw->SetCooperativeLevel(mWindow, DDSCL_NORMAL)) != DD_OK) {
				ASSERT(FALSE);
			}
		}
		else {
			DD_CALL(mDirectDraw->SetCooperativeLevel(mWindow, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX | DDSCL_ALLOWREBOOT | DDSCL_NOWINDOWCHANGES));

			if(DD_CALL(mDirectDraw->SetDisplayMode(mSpecialModeXRes, mSpecialModeYRes, 8)) == DD_OK) {
				/*
				   if( DD_CALL( mDirectDraw->SetCooperativeLevel( mWindow, DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX|DDSCL_ALLOWREBOOT|DDSCL_NOWINDOWCHANGES )) != DD_OK )
				   {
				   ASSERT( FALSE );
				   }
				 */
			}
			else {
				// ASSERT( FALSE );

				mSpecialWindowMode = FALSE;

				DD_CALL(mDirectDraw->RestoreDisplayMode());

				if(DD_CALL(mDirectDraw->SetCooperativeLevel(mWindow, DDSCL_NORMAL)) != DD_OK) {
					ASSERT(FALSE);
				}
			}

		}

		// if( mFullScreen )
		{
			// Adjust the window position
			SetWindowLong(mWindow, GWL_EXSTYLE, mOriginalExStyle);
			SetWindowLong(mWindow, GWL_STYLE, mOriginalStyle);

			// SetForegroundWindow( mWindow );

			SetWindowPos(mWindow, HWND_NOTOPMOST,  // This parameter have no effect when used here(I dont know why?? ask Bill)
				mOriginalPos.left, mOriginalPos.top, mOriginalPos.right - mOriginalPos.left, mOriginalPos.bottom - mOriginalPos.top, SWP_SHOWWINDOW /*SWP_NOACTIVATE */ );

			// We're back in windowed mode, so use native color depth.
			mBpp = mNativeBpp;

			CreatePalette(mGamma, mContrast, mBrightness);
		}
	}
}

BOOL MR_VideoBuffer::SetVideoMode()
{
	PRINT_LOG("SetVideoMode(Window)");

	BOOL lReturnValue;
	DDSURFACEDESC lSurfaceDesc;
	RECT windowRect;

	ASSERT(!mModeSettingInProgress);

	mModeSettingInProgress = TRUE;
	ResetWindowedPresentFallback();

	if(GetWindowRect(mWindow, &windowRect)) {
		LogRect("SetVideoMode(Window) window", windowRect);
	}

	ReturnToWindowsResolution();
	mRequestedAdapterGuidValid = FALSE;
	PRINT_LOG("SetVideoMode(Window) forcing default DirectDraw adapter");
	lReturnValue = InitDirectDraw();
	// Retrieve the window size
	if(lReturnValue) {
		RECT lRect;

		lReturnValue = GetClientRect(mWindow, &lRect);

		ASSERT(lReturnValue);

		mXRes = lRect.right;
		mYRes = lRect.bottom;
		mLineLen = mXRes;
	}

	if(lReturnValue) {
		POINT lPoint = { 0, 0 };

		lReturnValue = ClientToScreen(mWindow, &lPoint);

		mX0 = lPoint.x;
		mY0 = lPoint.y;

		ASSERT(lReturnValue);
	}

	// Create a front buffer
	if(lReturnValue) {
		// Ask specificcly for a 8 bit per pixel mode
		memset(&lSurfaceDesc, 0, sizeof(lSurfaceDesc));
		lSurfaceDesc.dwSize = sizeof(lSurfaceDesc);
		lSurfaceDesc.dwFlags = DDSD_CAPS /*|DDSD_PIXELFORMAT */ ;

		lSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

		/*
		   lSurfaceDesc.ddpfPixelFormat.dwSize  = sizeof( lSurfaceDesc.ddpfPixelFormat );
		   lSurfaceDesc.ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED8|DDPF_RGB;
		   lSurfaceDesc.ddpfPixelFormat.dwRGBBitCount = 8;
		 */

		if(DD_CALL(mDirectDraw->CreateSurface(&lSurfaceDesc, &mFrontBuffer, NULL)) != DD_OK) {
			// ASSERT( FALSE );
			lReturnValue = FALSE;
		}
		else {
			// We're running windowed now, so we need to use the
			// native color depth.
			mBpp = mNativeBpp;
		}
	}

	if(lReturnValue) {
		// Create the working surface
		memset(&lSurfaceDesc, 0, sizeof(lSurfaceDesc));
		lSurfaceDesc.dwSize = sizeof(lSurfaceDesc);
		lSurfaceDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH /*|DDSD_PIXELFORMAT */ ;

		lSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		lSurfaceDesc.dwHeight = mYRes;
		lSurfaceDesc.dwWidth = mXRes;

		/*
		   lSurfaceDesc.ddpfPixelFormat.dwSize  = sizeof( lSurfaceDesc.ddpfPixelFormat );
		   lSurfaceDesc.ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED8|DDPF_RGB;
		   lSurfaceDesc.ddpfPixelFormat.dwRGBBitCount = 8;
		 */

		if(DD_CALL(mDirectDraw->CreateSurface(&lSurfaceDesc, &mBackBuffer, NULL)) != DD_OK) {
			// ASSERT( FALSE ); // Probably a bad video mode (not 8bit/pixel)
			// Retry but not is system memory this time

			lSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN /*| DDSCAPS_SYSTEMMEMORY */ ;

			if(DD_CALL(mDirectDraw->CreateSurface(&lSurfaceDesc, &mBackBuffer, NULL)) != DD_OK) {
				// ASSERT( FALSE ); // Probably a bad video mode (not 8bit/pixel)
				lReturnValue = FALSE;
			}
		}
	}

	if(lReturnValue) {
		// Create a clipper
		if(DD_CALL(mDirectDraw->CreateClipper(0, &mClipper, NULL)) != DD_OK) {
			ASSERT(FALSE);
			lReturnValue = FALSE;
		}
	}

	if(lReturnValue) {
		// Attatch it to the current window
		if(DD_CALL(mClipper->SetHWnd(0, mWindow)) != DD_OK) {
			ASSERT(FALSE);
			lReturnValue = FALSE;
		}
	}

	if(lReturnValue) {
		// Attatch it to the current window
		if(DD_CALL(mFrontBuffer->SetClipper(mClipper)) != DD_OK) {
			ASSERT(FALSE);
			lReturnValue = FALSE;
		}
	}

	if(lReturnValue) {
		// Create a local memory ZBuffer
		// We do not use DirectDrawZBuffer for now

		mZBuffer = new MR_UInt16[mXRes * mYRes];

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

	// Set afull screen video mode
	HRESULT lErrorCode;
	BOOL lReturnValue;
	DDSURFACEDESC lSurfaceDesc;
	// DDCAPS          lDDCaps;
	MR_Config *cfg = MR_Config::GetInstance();

	DWORD lReqBpp = cfg->video.nativeBppFullscreen ? mNativeBpp : 8;

	ASSERT(!mModeSettingInProgress);

	mModeSettingInProgress = TRUE;
	ResetWindowedPresentFallback();

	PrepareDesktopFullscreen(NULL);
	lReturnValue = InitDirectDraw();

	if(lReturnValue) {
		DeleteInternalSurfaces();

		if(!mFullScreen) {
			// Save current position and style to be able to restore the current mode
			mOriginalExStyle = GetWindowLong(mWindow, GWL_EXSTYLE);
			mOriginalStyle = GetWindowLong(mWindow, GWL_STYLE);
			GetWindowRect(mWindow, &mOriginalPos);
		}
		// Make the window a non-borderwindow
		// SetWindowLong( mWindow, GWL_STYLE, mOriginalStyle & ~(WS_THICKFRAME ) );

		if(DD_CALL(mDirectDraw->SetCooperativeLevel(mWindow, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX | DDSCL_ALLOWREBOOT /*|DDSCL_NOWINDOWCHANGES */ )) != DD_OK) {
			ASSERT(FALSE);
			lReturnValue = FALSE;
		}
	}

// Retrieve the window size
if(lReturnValue) {
	mXRes = pXRes;
	mYRes = pYRes;
	mLineLen = mXRes;

	mFullScreen = TRUE;
}

if(lReturnValue) {
	// ASSERT( FALSE );

	if(DD_CALL(mDirectDraw->SetDisplayMode(pXRes, pYRes, lReqBpp)) != DD_OK) {
		lReturnValue = FALSE;
		ASSERT(FALSE);
	}
}

if(lReturnValue) {
	// Resize to full screen
	ShowWindow(mWindow, SW_MAXIMIZE);

}

// Create a front buffer
if(lReturnValue) {
	memset(&lSurfaceDesc, 0, sizeof(lSurfaceDesc));
	lSurfaceDesc.dwSize = sizeof(lSurfaceDesc);

	lSurfaceDesc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	lSurfaceDesc.dwBackBufferCount = 1;
	lSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_SYSTEMMEMORY;

	if((lErrorCode = DD_CALL(mDirectDraw->CreateSurface(&lSurfaceDesc, &mFrontBuffer, NULL))) != DD_OK) {
		ASSERT(FALSE);

		lSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX /*|DDSCAPS_SYSTEMMEMORY */ ;

		if((lErrorCode = DD_CALL(mDirectDraw->CreateSurface(&lSurfaceDesc, &mFrontBuffer, NULL))) != DD_OK) {
			ASSERT(FALSE);
			lReturnValue = FALSE;
		}
	}

	if(lReturnValue) {
		// Create (retrieve already created) the working surface
		DDSCAPS lDDSCaps;

		lDDSCaps.dwCaps = DDSCAPS_BACKBUFFER;

		if(DD_CALL(mFrontBuffer->GetAttachedSurface(&lDDSCaps, &mBackBuffer)) != DD_OK) {
			ASSERT(FALSE);
			lReturnValue = FALSE;
		}
	}

	if(lReturnValue) {
		mBpp = lReqBpp;
	}
}

if(lReturnValue) {
	// Create a local memory ZBuffer
	// We do not use DirectDrawZBuffer for now

	mZBuffer = new MR_UInt16[mXRes * mYRes];

}

if(!lReturnValue) {
	ReturnToWindowsResolution();
}

// AssignPalette();

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

BOOL MR_VideoBuffer::Lock()
{
	MR_SAMPLE_CONTEXT("LockVideoBuffer");

	BOOL lReturnValue = TRUE;

	gRenderLogStats.lockCount++;

	ASSERT(mBuffer == NULL);
	ASSERT(mDirectDraw != NULL);

	if(mIconMode) {
		lReturnValue = FALSE;
	}

	if(mBackBuffer == NULL) {
		// ASSERT( FALSE ); // It is possible but I want to know when it append
		// No surface
		lReturnValue = FALSE;
	}
	// Restore lost buffers (I have to do that but I don't know why
	if(lReturnValue) {
		if(DD_CALL(mFrontBuffer->IsLost()) == DDERR_SURFACELOST) {
			if(DD_CALL(mFrontBuffer->Restore()) != DD_OK) {
				ASSERT(FALSE);
				lReturnValue = FALSE;
			}
		}
		if(DD_CALL(mBackBuffer->IsLost()) == DDERR_SURFACELOST) {
			if(DD_CALL(mBackBuffer->Restore()) != DD_OK) {
				ASSERT(FALSE);
				lReturnValue = FALSE;
			}
		}

	}
	// Do the lock
	if(lReturnValue) {
		if(!gDebugMode && (mBpp == 8)) {
			DDSURFACEDESC lSurfaceDesc;

			lSurfaceDesc.dwSize = sizeof(lSurfaceDesc);

			if(DD_CALL(mBackBuffer->Lock(NULL, &lSurfaceDesc, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL)) != DD_OK) {
				ASSERT(FALSE);
				lReturnValue = FALSE;
			}
			else {
				mLineLen = lSurfaceDesc.lPitch;
				mBuffer = (MR_UInt8 *) lSurfaceDesc.lpSurface;
			}
		}
		else {
			// Render indirectly if debugging or not in 256-color mode.
			mBuffer = new MR_UInt8[mXRes * mYRes];
			mLineLen = mXRes;
		}
	}

	if(!lReturnValue) {
		gRenderLogStats.lockFailures++;
	}

	return lReturnValue;
}

void MR_VideoBuffer::Unlock()
{
	MR_SAMPLE_CONTEXT("UnlockVideoBuffer");
	BOOL unlockFailed = FALSE;

	gRenderLogStats.unlockCount++;

	ASSERT(mBuffer != NULL);
	ASSERT(mDirectDraw != NULL);
	ASSERT(mBackBuffer != NULL);

	if(!gDebugMode && (mBpp == 8)) {
		if(DD_CALL(mBackBuffer->Unlock(NULL)) != DD_OK) {
			ASSERT(FALSE);
			unlockFailed = TRUE;
		}
		mBuffer = NULL;
	}
	else {
		// Lock the back buffer and copy mBuffer
		if(DD_CALL(mFrontBuffer->IsLost()) == DDERR_SURFACELOST) {
			if(DD_CALL(mFrontBuffer->Restore()) != DD_OK) {
				// ASSERT( FALSE );
			}
		}
		if(DD_CALL(mBackBuffer->IsLost()) == DDERR_SURFACELOST) {
			if(DD_CALL(mBackBuffer->Restore()) != DD_OK) {
				// ASSERT( FALSE );
			}
		}

		DDSURFACEDESC lSurfaceDesc;

		lSurfaceDesc.dwSize = sizeof(lSurfaceDesc);

		if(DD_CALL(mBackBuffer->Lock(NULL, &lSurfaceDesc, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL)) != DD_OK) {
			// ASSERT( FALSE );
			unlockFailed = TRUE;
		}
		else {
			int lLineLen = lSurfaceDesc.lPitch;
			MR_UInt8 *lDest = (MR_UInt8 *) lSurfaceDesc.lpSurface;
			MR_UInt8 *lSrc = mBuffer;

			MR_SAMPLE_START(CopyVideoBuffer, "CopyVideoBuffer");

			if(mBpp <= 8) {
				for(int lCounter = 0; lCounter < mYRes; lCounter++) {
					memcpy(lDest, lSrc, mXRes);
					lDest += lLineLen;
					lSrc += mLineLen;
				}
			}
			else if(mBpp <= 16) {
				for(int lCounter = 0; lCounter < mYRes; lCounter++) {
					MR_UInt16 *lRDest = reinterpret_cast < MR_UInt16 * >(lDest);
					for(int lx = 0; lx < mXRes; ++lx) {
						*lRDest++ = static_cast < MR_UInt16 > (mPackedPalette[lSrc[lx]]);
					}
					lDest += lLineLen;
					lSrc += mLineLen;
				}
			}
			else if(mBpp <= 24) {
				// Note: Untested!
				for(int lCounter = 0; lCounter < mYRes; lCounter++) {
					MR_UInt8 *lRDest = lDest;
					for(int lx = 0; lx < mXRes; ++lx) {
						DWORD lColor = mPackedPalette[lSrc[lx]];
						*lRDest++ = static_cast < MR_UInt8 > ((lColor >> 16) & 0xff);
						*lRDest++ = static_cast < MR_UInt8 > ((lColor >> 8) & 0xff);
						*lRDest++ = static_cast < MR_UInt8 > (lColor & 0xff);
					}
					lDest += lLineLen;
					lSrc += mLineLen;
				}
			}
			else if(mBpp <= 32) {
				for(int lCounter = 0; lCounter < mYRes; lCounter++) {
					MR_UInt32 *lRDest = reinterpret_cast < MR_UInt32 * >(lDest);
					for(int lx = 0; lx < mXRes; ++lx) {
						*lRDest++ = mPackedPalette[lSrc[lx]];
					}
					lDest += lLineLen;
					lSrc += mLineLen;
				}
			}

			MR_SAMPLE_END(CopyVideoBuffer);

		}

		// Unlock
		if(DD_CALL(mBackBuffer->Unlock(NULL)) != DD_OK) {
			// ASSERT( FALSE );
			unlockFailed = TRUE;
		}

		delete[]mBuffer;
		mBuffer = NULL;
	}

	if(unlockFailed) {
		gRenderLogStats.unlockFailures++;
	}

	Flip();
}

void MR_VideoBuffer::Flip()
{
	HRESULT lErrorCode;

	gRenderLogStats.flipCount++;

	ASSERT(mBuffer == NULL);
	ASSERT(mDirectDraw != NULL);
	ASSERT(mFrontBuffer != NULL);

	if(mFullScreen) {
		if(DD_CALL(mFrontBuffer->Flip(NULL, DDFLIP_WAIT)) != DD_OK) {
			// ASSERT( FALSE );
			gRenderLogStats.flipFailures++;
		}

	}
	else {
		// We are in a window, use normal blitting
		if(mUseGdiWindowedPresentFallback) {
			if(!PresentWindowedWithGdi()) {
				gRenderLogStats.flipFailures++;
			}
			return;
		}

		int lX0 = mFullScreen ? 0 : mX0;
		int lY0 = mFullScreen ? 0 : mY0;

		RECT lDestRectangle = { lX0, lY0, lX0 + mXRes, lY0 + mYRes };
		RECT lSrcRectangle = { 0, 0, mXRes, mYRes };

		lErrorCode = DD_CALL(mFrontBuffer->Blt(&lDestRectangle, mBackBuffer, &lSrcRectangle, DDBLT_WAIT, NULL));

		if(lErrorCode != DD_OK) {
			if(lErrorCode == DDERR_INVALIDRECT) {
				gRenderLogStats.invalidRectFlipFailures++;
				mWindowedInvalidRectStreak++;

				if(mWindowedInvalidRectStreak >= 3) {
					mUseGdiWindowedPresentFallback = TRUE;
					gRenderLogStats.gdiFallbackActivations++;
					PRINT_LOG("Activating GDI windowed present fallback after %d consecutive DDERR_INVALIDRECT failures origin=%d,%d size=%dx%d",
						mWindowedInvalidRectStreak, mX0, mY0, mXRes, mYRes);
					if(PresentWindowedWithGdi()) {
						return;
					}
				}
			}
			else {
				mWindowedInvalidRectStreak = 0;
			}

			// ASSERT( FALSE );
			gRenderLogStats.flipFailures++;
		}
		else {
			mWindowedInvalidRectStreak = 0;
		}
	}
}

void MR_VideoBuffer::Clear(MR_UInt8 pColor)
{
	ASSERT(mBuffer != NULL);
	ASSERT(mDirectDraw != NULL);
	ASSERT(mBackBuffer != NULL);

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
