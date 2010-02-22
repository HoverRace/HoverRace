
// IntroMovie.h
// The intro movie that plays at startup.
//
// Copyright (c) 2010 Michael Imamura.
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

#ifdef WITH_DIRECTSHOW
#	include <dshow.h>
#endif

namespace HoverRace {
namespace Client {

/**
 * Manages the intro movie that is played at startup.
 * @author Michael Imamura
 */
class IntroMovie
{
	public:
		IntroMovie(HWND hwnd=NULL, HINSTANCE hinst=NULL);
		~IntroMovie();

	private:
		void Clean();
#		ifdef WITH_DIRECTSHOW
			HRESULT InitDirectShow(const std::string &movieFilename);
#		endif

	public:
		void Play();

		void ResetSize();
		void Repaint(HDC hdc);
		void ResetPalette(bool background=false);

	private:
		HWND hwnd;
#		ifdef WITH_DIRECTSHOW
			IGraphBuilder *graph;
			IVMRWindowlessControl *winCtl;
			IMediaControl *mediaCtl;
#		else
			HWND movieWnd;
#		endif
};

}  // namespace Client
}  // namespace HoverRace
