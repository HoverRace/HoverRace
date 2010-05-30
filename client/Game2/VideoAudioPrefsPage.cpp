
// VideoAudioPrefsPage.h
// The "Video and Audio" preferences page.
//
// Copyright (c) 2009 Michael Imamura.
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

#include "StdAfx.h"

#include "../../engine/Util/Config.h"
#include "../../engine/Util/OS.h"
#include "../../engine/Util/Str.h"
#include "../../engine/VideoServices/VideoBuffer.h"
#include "../../engine/VideoServices/SoundServer.h"

#include "GameApp.h"

#include "resource.h"

#include "VideoAudioPrefsPage.h"

using boost::format;
using boost::str;

using namespace HoverRace::Client;
using namespace HoverRace::Util;
using namespace HoverRace::VideoServices;

VideoAudioPrefsPage::VideoAudioPrefsPage(MR_GameApp *app) :
	SUPER(_("Video and Audio"), IDD_DISPLAY_INTENSITY),
	app(app)
{
}

VideoAudioPrefsPage::~VideoAudioPrefsPage()
{
}

/**
 * Retrieve the selected monitor.
 * @param hwnd The parent dialog.
 * @param monitors The list of monitors.
 * @return The selected monitor from the list of monitors, or @c NULL if the
 *         default monitor is selected.
 */
const OS::Monitor *VideoAudioPrefsPage::GetSelectedMonitor(HWND hwnd, const OS::monitors_t *monitors)
{
	int idx = SendDlgItemMessage(hwnd, IDC_MONITOR, CB_GETCURSEL, 0, 0);
	return (idx == 0) ? NULL : &((*monitors)[idx - 1]);
}

/**
 * Retrieve the selected resolution.
 * @param hwnd The parent dialog.
 * @return The resolution or @c NULL if no resolution is selected.
 *         It is up to the caller to delete the object.
 */
OS::Resolution *VideoAudioPrefsPage::GetSelectedResolution(HWND hwnd)
{
	OS::Resolution *oldRes = NULL;
	int oldResIdx = SendDlgItemMessage(hwnd, IDC_RES, CB_GETCURSEL, 0, 0);
	if (oldResIdx != CB_ERR) {
		size_t sz = (size_t)SendDlgItemMessage(hwnd, IDC_RES, CB_GETLBTEXTLEN, oldResIdx, 0);
		char *buf = (char*)malloc(sz + 1);
		buf[sz] = '\0';
		SendDlgItemMessage(hwnd, IDC_RES, CB_GETLBTEXT, oldResIdx, (LPARAM)buf);
		oldRes = new OS::Resolution(buf);
		free(buf);
	}
	return oldRes;
}

/**
 * Populate the resolution selector based on the selected monitor.
 * @param hwnd The parent dialog.
 * @param monitors The list of monitors.
 */
void VideoAudioPrefsPage::UpdateResolutionList(HWND hwnd, OS::monitors_t *monitors)
{
	const OS::Monitor *sel = GetSelectedMonitor(hwnd, monitors);
	if (sel == NULL) {
		// Find the primary monitor.
		for (OS::monitors_t::const_iterator iter = monitors->begin();
			iter != monitors->end(); ++iter)
		{
			if (iter->primary) {
				sel = &(*iter);
				break;
			}
		}
		if (sel == NULL) {
			// No monitors are marked as primary (shouldn't happen).
			sel = &((*monitors)[1]);
		}
	}
	if (sel->resolutions.empty()) {
		ASSERT(FALSE);
		return;
	}

	// Retrieve the currently selected resolution so we can try
	// to find a matching one in the new list.
	const OS::Resolution *oldRes = GetSelectedResolution(hwnd);

	SendDlgItemMessage(hwnd, IDC_RES, CB_RESETCONTENT, 0, 0);

	int i = 0;
	int selIdx = -1;
	for (OS::resolutions_t::const_iterator iter = sel->resolutions.begin();
		iter != sel->resolutions.end(); ++iter, ++i)
	{
		SendDlgItemMessage(hwnd, IDC_RES, CB_ADDSTRING, 0, (LPARAM)iter->AsString().c_str());
		if (oldRes != NULL && selIdx == -1 && *oldRes < *iter) {
			selIdx = (i == 0) ? 0 : i - 1;
		}
	}
	if (selIdx == -1) selIdx = sel->resolutions.size() - 1;
	SendDlgItemMessage(hwnd, IDC_RES, CB_SETCURSEL, selIdx, 0);

	delete oldRes;
}

/**
 * Populate the monitor selector with the list of monitors.
 * @param hwnd The parent dialog.
 * @param monitors The list of monitors.
 */
void VideoAudioPrefsPage::InitMonitorList(HWND hwnd, OS::monitors_t *monitors) {
	Config *cfg = Config::GetInstance();

	SendDlgItemMessageW(hwnd, IDC_MONITOR, CB_ADDSTRING, 0,
		(LPARAM)(const wchar_t*)Str::UW(_("Default Monitor")));

	int i = 1;
	int sel = 0;
	const std::string &cfgSelMon = cfg->video.fullscreenMonitor;
	for (OS::monitors_t::const_iterator iter = monitors->begin();
		iter != monitors->end(); ++iter, ++i)
	{
		SendDlgItemMessage(hwnd, IDC_MONITOR, CB_ADDSTRING, 0, (LPARAM)iter->name.c_str());
		if (cfgSelMon == iter->id) sel = i;
	}
	SendDlgItemMessage(hwnd, IDC_MONITOR, CB_SETCURSEL, sel, 0);

	// Init the current resolution from config so that UpdateResolutionList()
	// will pick the nearest one.
	SendDlgItemMessage(hwnd, IDC_RES, CB_ADDSTRING, 0,
		(LPARAM)OS::Resolution(cfg->video.xResFullscreen, cfg->video.yResFullscreen).AsString().c_str());
	SendDlgItemMessage(hwnd, IDC_RES, CB_SETCURSEL, 0, 0);

	UpdateResolutionList(hwnd, monitors);
}

BOOL VideoAudioPrefsPage::DlgProc(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	//ASSERT(This->mVideoBuffer != NULL);
	Config *cfg = Config::GetInstance();

	BOOL lReturnValue = FALSE;

	static double lOriginalGamma;
	static double lOriginalContrast;
	static double lOriginalBrightness;
	static float lOriginalSfxVolume;
	static boost::shared_ptr<OS::monitors_t> monitors;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
			app->GetVideoBuffer()->GetPaletteAttrib(lOriginalGamma, lOriginalContrast, lOriginalBrightness);
			lOriginalSfxVolume = cfg->audio.sfxVolume;

			// i18n of buttons and stuff
			// what if the field is not wide enough?
			SetDlgItemTextW(pWindow, IDC_VIDEO_GROUP, Str::UW(_("Video")));
			SetDlgItemTextW(pWindow, IDC_GAMMA, Str::UW(_("Gamma")));
			SetDlgItemTextW(pWindow, IDC_CONTRAST, Str::UW(_("Contrast")));
			SetDlgItemTextW(pWindow, IDC_BRIGHTNESS, Str::UW(_("Brightness")));

			SetDlgItemTextW(pWindow, IDC_AUDIO_GROUP, Str::UW(_("Audio")));
			SetDlgItemTextW(pWindow, IDC_VOLUME, Str::UW(_("Volume")));
			UpdateAudioStatus(pWindow);

			SetDlgItemTextW(pWindow, IDC_FULLSCREEN_GROUP, Str::UW(_("Fullscreen Settings")));
			SetDlgItemTextW(pWindow, IDC_MONITOR_LBL, Str::UW(_("Fullscreen Monitor")));
			SetDlgItemTextW(pWindow, IDC_FS_RES, Str::UW(_("Fullscreen Resolution")));

			SendDlgItemMessage(pWindow, IDC_GAMMA_SLIDER, TBM_SETRANGE, 0, MAKELONG(0, 200));
			SendDlgItemMessage(pWindow, IDC_CONTRAST_SLIDER, TBM_SETRANGE, 0, MAKELONG(0, 100));
			SendDlgItemMessage(pWindow, IDC_BRIGHTNESS_SLIDER, TBM_SETRANGE, 0, MAKELONG(0, 100));
			SendDlgItemMessage(pWindow, IDC_SFX_VOLUME_SLIDER, TBM_SETRANGE, 0, MAKELONG(0, 100));

			SendDlgItemMessage(pWindow, IDC_GAMMA_SLIDER, TBM_SETPOS, TRUE, long (lOriginalGamma * 100));
			SendDlgItemMessage(pWindow, IDC_CONTRAST_SLIDER, TBM_SETPOS, TRUE, long (lOriginalContrast * 100));
			SendDlgItemMessage(pWindow, IDC_BRIGHTNESS_SLIDER, TBM_SETPOS, TRUE, long (lOriginalBrightness * 100));
			SendDlgItemMessage(pWindow, IDC_SFX_VOLUME_SLIDER, TBM_SETPOS, TRUE, long (lOriginalSfxVolume * 100));

			// Populate the monitors dropdown.
			monitors = OS::GetMonitors();
			InitMonitorList(pWindow, monitors.get());

			UpdateIntensityDialogLabels(pWindow);
			break;

		case WM_HSCROLL:
			cfg->video.gamma = SendDlgItemMessage(pWindow, IDC_GAMMA_SLIDER, TBM_GETPOS, 0, 0) / 100.0;
			cfg->video.contrast = SendDlgItemMessage(pWindow, IDC_CONTRAST_SLIDER, TBM_GETPOS, 0, 0) / 100.0;
			cfg->video.brightness = SendDlgItemMessage(pWindow, IDC_BRIGHTNESS_SLIDER, TBM_GETPOS, 0, 0) / 100.0;
			cfg->audio.sfxVolume = SendDlgItemMessage(pWindow, IDC_SFX_VOLUME_SLIDER, TBM_GETPOS, 0, 0) / 100.0f;

			UpdateIntensityDialogLabels(pWindow);

			app->GetVideoBuffer()->CreatePalette(cfg->video.gamma, cfg->video.contrast, cfg->video.brightness);
			app->AssignPalette();
			break;

		case TB_ENDTRACK:
			{
				double lGamma;
				double lContrast;
				double lBrightness;
	
				app->GetVideoBuffer()->GetPaletteAttrib(lGamma, lContrast, lBrightness);
	
				SendDlgItemMessage(pWindow, IDC_GAMMA_SLIDER, TBM_SETPOS, TRUE, long (lGamma * 100));
				SendDlgItemMessage(pWindow, IDC_CONTRAST_SLIDER, TBM_SETPOS, TRUE, long (lContrast * 100));
				SendDlgItemMessage(pWindow, IDC_BRIGHTNESS_SLIDER, TBM_SETPOS, TRUE, long (lBrightness * 100));
			}
			break;

		// Menu options
		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDC_MONITOR:
					if(HIWORD(pWParam) == CBN_SELCHANGE) {
						UpdateResolutionList(pWindow, monitors.get());
					}
					break;
			}
			break;

		case WM_NOTIFY:
			switch (((NMHDR FAR *) pLParam)->code) {
				case PSN_RESET:
					cfg->audio.sfxVolume = lOriginalSfxVolume;
					app->GetVideoBuffer()->CreatePalette(lOriginalGamma, lOriginalContrast, lOriginalBrightness);
					app->AssignPalette();
					monitors.reset();
					break;

				case PSN_APPLY:
					const OS::Monitor *mon = GetSelectedMonitor(pWindow, monitors.get());
					cfg->video.fullscreenMonitor = (mon == NULL) ? "" : mon->id;
					OS::Resolution *res = GetSelectedResolution(pWindow);
					if (res != NULL) {
						cfg->video.xResFullscreen = res->w;
						cfg->video.yResFullscreen = res->h;
						delete res;
					} else {
						cfg->video.xResFullscreen = GetSystemMetrics(SM_CXSCREEN);
						cfg->video.yResFullscreen = GetSystemMetrics(SM_CYSCREEN);

						std::string lErrorBuffer = str(format("%s %dx%d") %
							_("Invalid resolution; reverting to default") %
							cfg->video.xResFullscreen %
							cfg->video.yResFullscreen);
						MessageBoxW(pWindow, Str::UW(lErrorBuffer.c_str()), PACKAGE_NAME_L, MB_OK);
					}
					cfg->Save();
					monitors.reset();
					break;

			}
			break;

	}

	return lReturnValue;
}

/**
 * Update the labels for the intensity sliders.
 * @param pWindow Dialog handle.
 */
void VideoAudioPrefsPage::UpdateIntensityDialogLabels(HWND pWindow)
{
	static format lblFmt("%0.2f");

	char buf[5];
	buf[4] = '\0';

	long gamma = SendDlgItemMessage(pWindow, IDC_GAMMA_SLIDER, TBM_GETPOS, 0, 0);
	long contrast = SendDlgItemMessage(pWindow, IDC_CONTRAST_SLIDER, TBM_GETPOS, 0, 0);
	long brightness = SendDlgItemMessage(pWindow, IDC_BRIGHTNESS_SLIDER, TBM_GETPOS, 0, 0);
	long sfxVolume = SendDlgItemMessage(pWindow, IDC_SFX_VOLUME_SLIDER, TBM_GETPOS, 0, 0);

	SetDlgItemText(pWindow, IDC_GAMMA_TXT, str(lblFmt % (gamma / 100.0)).c_str());
	SetDlgItemText(pWindow, IDC_CONTRAST_TXT, str(lblFmt % (contrast / 100.0)).c_str());
	SetDlgItemText(pWindow, IDC_BRIGHTNESS_TXT, str(lblFmt % (brightness / 100.0)).c_str());
	SetDlgItemText(pWindow, IDC_SFX_VOLUME_TXT, str(lblFmt % (sfxVolume / 100.0)).c_str());
}

/**
 * Update the audio status label.
 * The status label is used to explain to the user why there is no sound,
 * so they don't keep trying to fiddle with the volume slider.
 * @param hwnd Dialog handle.
 */
void VideoAudioPrefsPage::UpdateAudioStatus(HWND hwnd)
{
	std::string status = SoundServer::GetInitError();
	if (status.length() == 0 && Config::GetInstance()->runtime.silent) {
		status = _("HoverRace is currently in silent mode.");
	}
	SetDlgItemTextW(hwnd, IDC_AUDIO_STATUS, Str::UW(status.c_str()));
}
