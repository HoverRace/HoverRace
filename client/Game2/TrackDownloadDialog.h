
// TrackDownloadDialog.h
// Header for the track downloader.
//
// Copyright (c) 2008, 2009 Michael Imamura.
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

#pragma once

#include <string>

typedef void CURL;

/**
 * Track downloader with built-in UI.
 * @author Michael Imamura
 */
class TrackDownloadDialog
{
	private:
		TrackDownloadDialog() { }
	public:
		TrackDownloadDialog(const std::string &name);
		~TrackDownloadDialog();

		bool ShowModal(HINSTANCE hinst, HWND parent);

	private:
		enum state_t {
			ST_INITIALIZING,
			ST_DOWNLOADING,
			ST_EXTRACTING,
			ST_FINISHED,
			ST_LAST
		};
		void SetState(state_t st);
		void UpdateDialogProgress(HWND hwnd);

		void ThreadProc();

		BOOL DlgProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
		static BOOL CALLBACK DlgFunc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

		size_t WriteProc(void *ptr, size_t size, size_t nmemb);
		static size_t WriteFunc(void *ptr, size_t size, size_t nmemb, void *stream);
		size_t ProgressProc(double dlTotal, double dlNow);
		static size_t ProgressFunc(void *clientp, double dlTotal, double dlNow, double, double);

		bool ExtractTrackFile();

	private:
		std::string name;
		std::string trackFilename;
		HWND dlgHwnd;
		CURL *trackDl;
		char *curlErrorBuf;
		volatile state_t state;
		volatile bool cancel;

		typedef unsigned char dlBuf_t;
		dlBuf_t *dlBuf;
		volatile size_t bufSize;
		size_t bufCapacity;
		volatile size_t bufTotal;
};
