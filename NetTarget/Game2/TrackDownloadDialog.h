
/* TrackDownloadDialog.h
	Header for the TrackDownloadDialog class. */

#pragma once

#include <string>

typedef void CURL;

/**
 * Track downloader with built-in UI.
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

		void ThreadProc();

		BOOL DlgProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
		static BOOL CALLBACK DlgFunc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

		size_t WriteProc(void *ptr, size_t size, size_t nmemb);
		static size_t WriteFunc(void *ptr, size_t size, size_t nmemb, void *stream);

		bool ExtractTrackFile();

	private:
		std::string name;
		std::string trackFilename;
		HWND dlgHwnd;
		CURL *trackDl;
		char *curlErrorBuf;
		state_t state;
		bool cancel;

		typedef unsigned char dlBuf_t;
		dlBuf_t *dlBuf;
		size_t bufSize;
		size_t bufCapacity;
};
