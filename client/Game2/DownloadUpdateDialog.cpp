// DownloadUpdateDialog.cpp
// Downloader for HoverRace updates with progress bar.
//
// Copyright (c) 2009 Ryan Curtin
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

#include "StdAfx.h"

#include "DownloadUpdateDialog.h"
#include "resource.h"

#include <curl/curl.h>

#include <LiteUnzip.h>

#include <io.h>

#include "../../engine/Util/Config.h"
#include "../../engine/Util/Str.h"

#include <boost/filesystem.hpp>

using boost::str;
using boost::format;

using HoverRace::Util::Config;

using namespace HoverRace::Util;

#define INIT_CAPACITY (5 * 1024 * 1024)
#define MAX_CAPACITY (20 * 1024 * 1024)

/**
 * Constructor.
 * 
 * @param baseUrl Base URL of update to download
 * @param filename Name of file to download
 * @param destDir The destination directory we will put the update into
 */
DownloadUpdateDialog::DownloadUpdateDialog(const string &baseUrl, const string &filename, const string &destDir) :
	baseUrl(baseUrl), filename(filename), destDir(destDir), dlgHwnd(NULL), state(ST_INITIALIZING),
	bufSize(0), bufCapacity(INIT_CAPACITY)
{
	updateDl = curl_easy_init();

	curlErrorBuf = new char[CURL_ERROR_SIZE];
	curlErrorBuf[0] = 0;

//	dlBuf = (dlBuf_t *) malloc(INIT_CAPACITY * sizeof(dlBuf_t));
}

/// Destructor.
DownloadUpdateDialog::~DownloadUpdateDialog()
{
//	free(dlBuf);
	delete[] curlErrorBuf;
	curl_easy_cleanup(updateDl);
}

/**
 * Display the modal dialog and start the download.
 *
 * This dialog is modal; it will block until the download is finished
 * or fails.
 *
 * @param hinst The app instance handle.
 * @param parent The parent window handle.
 * @return @c true If the download was successful
 *      or @c false if the download failed.
 */
bool DownloadUpdateDialog::ShowModal(HINSTANCE hinst, HWND parent)
{
	bufSize = 0;
	bufTotal = 0;
	cancel = false;

	boost::thread thread(boost::bind(&DownloadUpdateDialog::ThreadProc, this));

	DWORD dlgRetv = DialogBoxParamW(hinst, MAKEINTRESOURCEW(IDD_DOWNLOAD_PROGRESS),
		parent, DlgFunc, reinterpret_cast<LPARAM>(this));
	if (dlgRetv == IDCANCEL) cancel = true;

	thread.join();

	return !cancel;
}

const char **DownloadUpdateDialog::GetStateNames()
{
	static const char *STATE_NAMES[] = {
		_("Initializing"),
		_("Downloading"),
		_("Finished"),
		};
	return STATE_NAMES;
}

void DownloadUpdateDialog::SetState(state_t st)
{
	state = st;

	OutputDebugString("State is now: ");
	OutputDebugString(GetStateNames()[st]);
	OutputDebugString("\n");

	// Notify dialog of state change.
	if (dlgHwnd != NULL) {
		PostMessage(dlgHwnd, WM_APP, state, 0);
	}
}

/// Update the dialog with the current status.
void DownloadUpdateDialog::UpdateDialogProgress(HWND hwnd)
{
	state_t curState = state;
	size_t curTotal = bufTotal;
	size_t curSize = bufSize;

	// Update progress bar.
	int pos;
	switch (curState) {
		case ST_INITIALIZING:
			pos = 0;
			break;

		case ST_DOWNLOADING:
			if (curTotal <= 0 || curSize <= 0) {
				pos = 0;
			} else if (curSize >= curTotal) {
				pos = 100;
			} else {
				pos = (curSize * 100) / (curTotal);
			}
			break;

		case ST_FINISHED:
			pos = 100;
			break;
	}

	SendDlgItemMessage(hwnd, IDC_DLPROGRESS, PBM_SETPOS, (WPARAM) pos, 0);

	// Update status text.
	if (curState < 0 || curState >= ST_LAST) {
		SetDlgItemTextW(hwnd, IDC_STATE, Str::UW(_("Processing...")));
	} else {
		std::ostringstream stateStr;
		if (curState == ST_DOWNLOADING) {
			stateStr << _("Downloading");
			if (curTotal <= 0) {
				if (curSize > 0) {
					stateStr << " (" << (curSize / 1024) << " KB)";
				}
			} else if (curTotal > 0) {
				stateStr << " (" << pos << "% of " << (curTotal / 1024) << " KB)";
			}
		} else {
			stateStr << GetStateNames()[curState];
		}
		stateStr << "...";
		SetDlgItemTextW(hwnd, IDC_STATE, Str::UW(stateStr.str().c_str()));
	}
}

// Thread function.
void DownloadUpdateDialog::ThreadProc()
{
	Config *cfg = Config::GetInstance();

	boost::filesystem::create_directories(destDir);

	string destFile = destDir + "/" + filename;

	outFile = fopen(destFile.c_str(), "wb");

	if(outFile == NULL) {
		// opening file failed
		string errMsg = boost::str(boost::format(_("Cannot open file %s for writing!"))	% destFile);

		MessageBoxW(NULL, Str::UW(errMsg.c_str()), PACKAGE_NAME_L, MB_ICONWARNING | MB_OK);

		EndDialog(dlgHwnd, IDCANCEL);

	} else {
		// opening file was a success, continue to download and write to it
		SetState(ST_INITIALIZING);

		// Error buffer.
		curl_easy_setopt(updateDl, CURLOPT_ERRORBUFFER, curlErrorBuf);

		// Callback functions.
		curl_easy_setopt(updateDl, CURLOPT_WRITEFUNCTION, WriteFunc);
		curl_easy_setopt(updateDl, CURLOPT_WRITEDATA, outFile);
		curl_easy_setopt(updateDl, CURLOPT_PROGRESSFUNCTION, ProgressFunc);
		curl_easy_setopt(updateDl, CURLOPT_PROGRESSDATA, this);
		curl_easy_setopt(updateDl, CURLOPT_NOPROGRESS, 0);

		// Misc HTTP options.
		curl_easy_setopt(updateDl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(updateDl, CURLOPT_FAILONERROR, 1);
	
		/* Enabling content encoding makes the Content-Length not work currently.
		curl_easy_setopt(trackDl, CURLOPT_ENCODING, "");  // Enable all encodings.
		curl_easy_setopt(trackDl, CURLOPT_HTTP_CONTENT_DECODING, 1);
		*/
		
		curl_easy_setopt(updateDl, CURLOPT_USERAGENT, cfg->GetUserAgentId().c_str());
		curl_easy_setopt(updateDl, CURLOPT_URL, (baseUrl + "/" + filename).c_str());

		if(!cancel) {
			SetState(ST_DOWNLOADING);
			int curlRetv = curl_easy_perform(updateDl);
			if(curlRetv != 0 && curlRetv != CURLE_ABORTED_BY_CALLBACK) {
				MessageBoxW(dlgHwnd, Str::UW(curlErrorBuf), PACKAGE_NAME_L, MB_ICONWARNING | MB_OK);
				cancel = true;
			}
		}

		if(!cancel) {
			if(bufSize <= 128) {
				std::string message = boost::str(boost::format(
					_("Cannot update!  File %s not found.  Contact a site administrator."))
					% (baseUrl + "/" + filename));

				MessageBoxW(dlgHwnd, Str::UW(message.c_str()), PACKAGE_NAME_L, MB_ICONWARNING | MB_OK);
				cancel = true;
			}
		}

		fclose(outFile);

		SetState(ST_FINISHED);

		if(dlgHwnd != NULL) {
			EndDialog(dlgHwnd, IDOK);
		}
	}
}

// Dialog callback.
BOOL DownloadUpdateDialog::DlgProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	BOOL retv = FALSE;

	switch (message) {

		case WM_INITDIALOG:
			dlgHwnd = hwnd;
			SetWindowTextW(hwnd, Str::UW(_("Update Download")));
			SetDlgItemTextW(hwnd, IDC_DLITEM, Str::UW(_("Downloading Update:")));
			SetDlgItemTextW(hwnd, IDC_ITEM_NAME, Str::UW(filename.c_str()));
			UpdateDialogProgress(hwnd);
			retv = TRUE;
			break;

		case WM_DESTROY:
			dlgHwnd = NULL;
			break;

		case WM_COMMAND:
			switch (LOWORD(wparam)) {
				case IDCANCEL:
					cancel = true;
					EndDialog(hwnd, IDCANCEL);
					retv = TRUE;
					break;
			}
			break;

		case WM_APP: // State changed.
			UpdateDialogProgress(hwnd);
			break;
	}

	return retv;
}

/// Global dialog callback dispatcher.
BOOL CALLBACK DownloadUpdateDialog::DlgFunc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	// Determine which instance to route the message to.
	DownloadUpdateDialog *dlg;
	if (message == WM_INITDIALOG) {
		dlg = reinterpret_cast<DownloadUpdateDialog *>(lparam);
		SetWindowLong(hwnd, GWL_USERDATA, lparam);
	} else {
		dlg = reinterpret_cast<DownloadUpdateDialog *>(GetWindowLong(hwnd, GWL_USERDATA));
		if (message == WM_DESTROY) {
			SetWindowLong(hwnd, GWL_USERDATA, 0);
		}
	}

	return (dlg == NULL) ? FALSE : dlg->DlgProc(hwnd, message, wparam, lparam);
}

/// Global write callback dispatcher.
size_t DownloadUpdateDialog::WriteFunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
	return fwrite(ptr, size, nmemb, (FILE *) stream); 
}

/// Progress callback for libcurl.
size_t DownloadUpdateDialog::ProgressProc(double dlTotal, double dlNow)
{
	bufTotal = static_cast<size_t>(dlTotal);
	bufSize = static_cast<size_t>(dlNow);

	// Notify dialog of progress change.
	if (dlgHwnd != NULL) {
		PostMessage(dlgHwnd, WM_APP, state, 0);
	}

	return cancel ? 1 : 0;
}

/// Global progress callback dispatcher.
size_t DownloadUpdateDialog::ProgressFunc(void *clientp, double dlTotal, double dlNow, double, double)
{
	return reinterpret_cast<DownloadUpdateDialog *>(clientp)->ProgressProc(dlTotal, dlNow);
}
