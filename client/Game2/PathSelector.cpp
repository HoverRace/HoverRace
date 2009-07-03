
// PathSelector.cpp
// The directory selection dialog.
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

#include <shlobj.h>

#include "../../engine/Util/Str.h"

#include "PathSelector.h"

using namespace HoverRace::Client;
using namespace HoverRace::Util;

static bool SHGetTargetFolderPath(LPCITEMIDLIST pidlFolder, LPWSTR pszPath);

/**
 * Constructor.
 * @param title The description of what kind of folder to select (UTF-8).
 */
PathSelector::PathSelector(const std::string &title) :
	title(Str::UW(title.c_str()))
{
}

PathSelector::~PathSelector()
{
}

/**
 * Display the path selector.
 * @param parent The parent window.
 * @param[out] path The selected path.
 * @return @c true if the user selected a valid path,
 *         @c false if the user canceled the dialog or the path was invalid.
 */
bool PathSelector::ShowModal(HWND parent, std::string &path)
{
	wchar_t browsePath[MAX_PATH];
	BROWSEINFOW browseInfo;
	memset(&browseInfo, 0, sizeof(browseInfo));
	browseInfo.hwndOwner = parent;
	browseInfo.pidlRoot = NULL;
	browseInfo.pszDisplayName = browsePath;
	browseInfo.lpszTitle = title.c_str();
	browseInfo.ulFlags = BIF_NEWDIALOGSTYLE;
	browseInfo.lpfn = NULL;
	browseInfo.lParam = NULL;
	browseInfo.iImage = 0;
	LPITEMIDLIST pidl = SHBrowseForFolderW(&browseInfo);
	if (pidl == NULL) {
		return false;
	}
	else {
		bool retv = SHGetTargetFolderPath(pidl, browsePath);
		if (retv) {
			path = (const char*)Str::WU(browsePath);
		}
		CoTaskMemFree(pidl);
		return retv;
	}
}

// PIDL-to-path conversion utilities (from MSDN).
// See http://msdn.microsoft.com/en-us/library/bb762115(VS.85).aspx for details
// about why we need to jump through all of these hoops.

// Retrieves the UIObject interface for the specified full PIDL
static HRESULT SHGetUIObjectFromFullPIDL(LPCITEMIDLIST pidl, HWND hwnd, REFIID riid, void **ppv)
{
	LPCITEMIDLIST pidlChild;
	IShellFolder* psf;
	*ppv = NULL;
	HRESULT hr = SHBindToParent(pidl, IID_IShellFolder, (LPVOID*)&psf, &pidlChild);
	if (SUCCEEDED(hr))
	{
		hr = psf->GetUIObjectOf(hwnd, 1, &pidlChild, riid, NULL, ppv);
		psf->Release();
	}
	return hr;
}

// ILSkip and ILNext may already be defined.
#ifndef ILSkip
#	define ILSkip(pidl, cb)       ((LPITEMIDLIST)(((BYTE*)(pidl))+cb))
#endif
#ifndef ILNext
#	define ILNext(pidl)           ILSkip(pidl, (pidl)->mkid.cb)
#endif

static HRESULT SHILClone(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidl)
{
	DWORD cbTotal = 0;
	if (pidl)
	{
		LPCITEMIDLIST pidl_temp = pidl;
		cbTotal += sizeof(pidl_temp->mkid.cb);
		while (pidl_temp->mkid.cb) 
		{
			cbTotal += pidl_temp->mkid.cb;
			pidl_temp = ILNext(pidl_temp);
		}
	}

	*ppidl = (LPITEMIDLIST)CoTaskMemAlloc(cbTotal);

	if (*ppidl)
		CopyMemory(*ppidl, pidl, cbTotal);

	return  *ppidl ? S_OK: E_OUTOFMEMORY;
}
 
// Get the target PIDL for a folder PIDL. This also deals with cases of a folder  
// shortcut or an alias to a real folder.
static HRESULT SHGetTargetFolderIDList(LPCITEMIDLIST pidlFolder, LPITEMIDLIST *ppidl)
{
	IShellLink *psl;

	*ppidl = NULL;

	HRESULT hr = SHGetUIObjectFromFullPIDL(pidlFolder, NULL, IID_IShellLink, (LPVOID*)&psl);

	if (SUCCEEDED(hr))
	{
		hr = psl->GetIDList(ppidl);
		psl->Release();
	}

	// It's not a folder shortcut so get the PIDL normally.
	if (FAILED(hr))
		hr = SHILClone(pidlFolder, ppidl);

	return hr;
}

// Get the target folder for a folder PIDL. This deals with cases where a folder
// is an alias to a real folder, folder shortcuts, the My Documents folder, etc.
static bool SHGetTargetFolderPath(LPCITEMIDLIST pidlFolder, LPWSTR pszPath)
{
	LPITEMIDLIST pidlTarget;

	*pszPath = 0;
	HRESULT hr = SHGetTargetFolderIDList(pidlFolder, &pidlTarget);

	if (SUCCEEDED(hr))
	{
		SHGetPathFromIDListW(pidlTarget, pszPath);   // Make sure it is a path
		CoTaskMemFree(pidlTarget);
	}

	return *pszPath != 0;
}
