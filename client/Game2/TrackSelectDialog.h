
// TrackSelectDialog.h
// Track selector.
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

#define OPT_ALLOW_WEAPONS	0x40
#define OPT_ALLOW_MINES		0x20
#define OPT_ALLOW_CANS		0x10
#define OPT_ALLOW_BASIC		0x08
#define OPT_ALLOW_BI		0x02
#define OPT_ALLOW_CX		0x04
#define OPT_ALLOW_EON		0x01

namespace HoverRace {
namespace Client {

class Rulebook;
typedef boost::shared_ptr<Rulebook> RulebookPtr;

class TrackSelectDialog {
	public:
		TrackSelectDialog();
		~TrackSelectDialog();

		RulebookPtr ShowModal(HINSTANCE hinst, HWND parent);

		const std::string &GetTrackName() const { return trackName; }
		int GetLaps() const { return laps; }
		char GetOpts() const { return opts; }

	private:
		BOOL DlgProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
		static BOOL CALLBACK DlgFunc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
		static BOOL CALLBACK ListCallBack(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

	private:
		std::string trackName;
		int laps;
		char opts;  ///< bits: (unused)(weapons)(mines)(cans)(basic)(bi)(cx)(eon)
};

}  // namespace Client
}  // namespace HoverRace
