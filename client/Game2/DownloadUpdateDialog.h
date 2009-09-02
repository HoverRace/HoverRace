// DownloadUpdateDialog.h
// Dialog to download an update from the server and notify user of progress.
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

#ifndef DOWNLOAD_UPDATE_DIALOG_H
#define DOWNLOAD_UPDATE_DIALOG_H

#include <string>

typedef void CURL;

using namespace std;

/**
 * Update downloader with progress bar to notify user.
 * Modeled closely on TrackDownloadDialog.
 *
 * @author Ryan Curtin
 */
class DownloadUpdateDialog
{
	public:
		DownloadUpdateDialog(const string &baseUrl, const string &filename, const string &destDir);
		~DownloadUpdateDialog();

		bool ShowModal(HINSTANCE hinst, HWND parent);

	private:
		enum state_t {
			ST_INITIALIZING,
			ST_DOWNLOADING,
			ST_FINISHED,
			ST_LAST
		};

		const char **GetStateNames();
		void SetState(state_t st);
		void UpdateDialogProgress(HWND hwnd);

		void ThreadProc();

		BOOL DlgProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
		static BOOL CALLBACK DlgFunc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

		static size_t WriteFunc(void *ptr, size_t size, size_t nmemb, void *stream);
		size_t ProgressProc(double dlTotal, double dlNow);
		static size_t ProgressFunc(void *clientp, double dlTotal, double dlNow, double, double);

	private:
		string baseUrl;
		string filename;
		string destDir;

		HWND dlgHwnd;

		CURL *updateDl;
		char *curlErrorBuf;

		volatile state_t state;
		volatile bool cancel;

		FILE *outFile;

		volatile size_t bufSize;
		size_t bufCapacity;
		volatile size_t bufTotal;
};

#endif