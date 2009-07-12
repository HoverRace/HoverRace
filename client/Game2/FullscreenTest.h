
// FullscreenTest.h
// Header for the fullscreen test.
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

#pragma once

namespace HoverRace {
	namespace VideoServices {
		struct Font;
		class MultipartText;
		class NumericGlyphs;
		class StaticText;
	}
}

class MR_2DViewPort;
class MR_VideoBuffer;

namespace HoverRace {
namespace Client {

class FullscreenTest {

	public:
		FullscreenTest(int oldX, int oldY, const std::string &oldMonitor);
		~FullscreenTest();

	public:
		bool ShouldActivateTest(HWND parent) const;
		bool TickTimer();

	private:
		static int ScaleFont(int i, int resY);
		void UpdateSubheading(int resY);
		void InitWidgets(int resX, int resY);
	public:
		void Render(MR_VideoBuffer *dest);

	private:
		int oldX, oldY;
		std::string oldMonitor;

		MR_2DViewPort *viewport;
		bool widgetsInitialized;
		VideoServices::StaticText *heading;
		VideoServices::StaticText *subheading;

		int timeRemaining;
};

}  // namespace Client
}  // namespace HoverRace
