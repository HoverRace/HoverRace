// ColorPalette.h
//
// The color palette is a module, not a class. It is the same
// colorpalette that is always used
//
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
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

#ifdef _WIN32
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
namespace VideoServices {

namespace ColorPalette {

#define MR_NB_COLORS                  256
#define MR_RESERVED_COLORS_BEGINNING   10
#define MR_RESERVED_COLORS_END         15
// THIS IS JUST A GUESS, IT IS PROBABLY WRONG
// FIX ME
// MR_RESERVED_COLORS added based on a guess
#define MR_RESERVED_COLORS              6
#define MR_BASIC_COLORS               100		  // Includes some extra space
#define MR_BACK_COLORS                128

#define MR_NB_COLOR_INTENSITY       256
#define MR_NORMAL_INTENSITY         128

// Color convertion tables
MR_DllDeclare extern MR_UInt8 colorTable[MR_NB_COLOR_INTENSITY][MR_NB_COLORS];
MR_DllDeclare extern MR_UInt8 colorAdditionTable[MR_NB_COLORS][MR_NB_COLORS];
extern double basicPalette[][3];
extern int basicPaletteSize;

												  // return a vectors of MR_NB_COLORS-MR_RESERVED_COLORS
MR_DllDeclare PALETTEENTRY *GetColors(double pGamma, double pIntensity = 0.8, double pIntensityBase = 0.0);
MR_DllDeclare const PALETTEENTRY &ConvertColor(MR_UInt8 pRed, MR_UInt8 pGreen, MR_UInt8 pBlue, double pGamma, double pIntensity = 0.8, double pIntensityBase = 0.0);

}  // namespace ColorPalette

}  // namespace VideoServices
}  // namespace HoverRace

#undef MR_DllDeclare
