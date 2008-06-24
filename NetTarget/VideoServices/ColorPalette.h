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

#ifndef COLOR_PALETTE_H
#define COLOR_PALETTE_H

// #include <ddraw.h>

#include "../Util/MR_Types.h"

#ifdef MR_VIDEO_SERVICES
#define MR_DllDeclare   __declspec( dllexport )
#else
#define MR_DllDeclare   __declspec( dllimport )
#endif

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
MR_DllDeclare extern MR_UInt8 MR_ColorTable[MR_NB_COLOR_INTENSITY][MR_NB_COLORS];
MR_DllDeclare extern MR_UInt8 MR_ColorAdditionTable[MR_NB_COLORS][MR_NB_COLORS];
extern double MR_BasicPalette[][3];
extern int MR_BasicPaletteSize;

												  // return a vectors of MR_NB_COLORS-MR_RESERVED_COLORS
MR_DllDeclare PALETTEENTRY *MR_GetColors(double pGamma, double pIntensity = 0.8, double pIntensityBase = 0.0);
MR_DllDeclare const PALETTEENTRY & MR_ConvertColor(MR_UInt8 pRed, MR_UInt8 pGreen, MR_UInt8 pBlue, double pGamma, double pIntensity = 0.8, double pIntensityBase = 0.0);

#undef MR_DllDeclare
#endif
