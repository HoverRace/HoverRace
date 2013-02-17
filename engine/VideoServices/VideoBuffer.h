// VideoBuffer.h
// Header for rendering system.
//
// Copyright (c) 2013 Michael Imamura.
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

#pragma once

#include "../Util/MR_Types.h"
#include "../Util/OS.h"
#include "ColorPalette.h"

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

struct SDL_Surface;

namespace HoverRace {
namespace VideoServices {

class MR_DllDeclare VideoBuffer
{
	public:
		class Lock {
			VideoBuffer &videoBuffer;
			public:
				Lock(VideoBuffer &videoBuffer) : videoBuffer(videoBuffer) {
					videoBuffer.LockLegacySurface();
				}
				~Lock() {
					videoBuffer.UnlockLegacySurface();
				}
		};

	public:
		VideoBuffer();
		~VideoBuffer();

		// Signals from ClientApp that certain settings have changed.
		void OnDesktopModeChange(int width, int height);
		void OnWindowResChange();

		void AssignPalette();
		void CreatePalette();
		void SetBackgroundPalette(std::unique_ptr<MR_UInt8[]> &palette);

	private:
		void LockLegacySurface();
		void UnlockLegacySurface();
		void Flip();

	public:
		int GetWidth() const { return width; }
		int GetHeight() const { return height; }
		int GetPitch() const { return pitch; }
		int GetZPitch() const { return width; }

		int GetXRes() const { return width; }  ///< @deprecated Use GetWidth() instead.
		int GetYRes() const { return height; }  ///< @deprecated Use GetHeight() instead.
		int GetLineLen() const { return pitch; }  ///< @deprecated Use GetPitch() instead.
		int GetZLineLen() const { return width; }  ///< @deprecated Use GetZPitch() instead.

		MR_UInt8 *GetBuffer() const { return vbuf; }
		MR_UInt16 *GetZBuffer() const { return zbuf; }

		void Clear(MR_UInt8 color = 0);

		typedef std::pair<int,int> pixelMeter_t;
		pixelMeter_t GetPixelMeter() const;

		void DrawPoint(int pX, int pY, MR_UInt8 pColor);
		void DrawLine(int pX0, int pY0, int pX1, int pY1, MR_UInt8 pColor);
		void DrawHorizontalLine(int pY, int pX0, int pX1, MR_UInt8 pColor);

	private:
		int desktopWidth, desktopHeight;
		int width, height, pitch;
		bool fullscreen;

		SDL_Surface *legacySurface;
		MR_UInt8 *vbuf;
		MR_UInt16 *zbuf;

		std::unique_ptr<MR_UInt8[]> bgPalette;

		ColorPalette::paletteEntry_t palette[256];
};

}  // namespace VideoServices
}  // namespace HoverRace

#undef MR_DllDeclare
