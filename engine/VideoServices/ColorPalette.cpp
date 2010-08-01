// ColorPalette.cpp
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

#include "StdAfx.h"

#include <math.h>

#include "ColorPalette.h"

namespace HoverRace {
namespace VideoServices {

namespace ColorPalette {

namespace {
	void AssignPaletteEntry(paletteEntry_t &ent, double r, double g, double b)
	{
#		ifdef WITH_SDL
			ent.r = static_cast<MR_UInt8>(r);
			ent.g = static_cast<MR_UInt8>(g);
			ent.b = static_cast<MR_UInt8>(b);
#		else
			ent.peRed = static_cast<MR_UInt8>(r);
			ent.peGreen = static_cast<MR_UInt8>(g);
			ent.peBlue = static_cast<MR_UInt8>(b);
			ent.peFlags = 0;  // PC_EXPLICIT
#		endif
	}
}

/**
 * Generate the "basic" range of the color palette.
 * The "basic" range starts at index @c MR_RESERVED_COLORS_BEGINNING and
 * contains @c MR_BASIC_COLORS entries.  The "background" range follows
 * the "basic" range.
 * @param pGamma The gamma correction (the inverse of the configuration gamma).
 * @param pIntensity The color intensity (contrast * brightness).
 * @param pIntensityBase The base intensity (brightness - intensity).
 * @return An array of size @c MR_BASIC_COLORS defining the "basic" range.
 */
paletteEntry_t *GetColors(double pGamma, double pIntensity, double pIntensityBase)
{
	paletteEntry_t *lReturnValue = new paletteEntry_t[MR_BASIC_COLORS];

	int lColorIndex = 0;

	for(; lColorIndex < basicPaletteSize; lColorIndex++) {
		double lRed;
		double lGreen;
		double lBlue;

		// Compute the gamma correction
		lRed = pIntensityBase + pIntensity * pow(basicPalette[lColorIndex][0], pGamma);
		lGreen = pIntensityBase + pIntensity * pow(basicPalette[lColorIndex][1], pGamma);
		lBlue = pIntensityBase + pIntensity * pow(basicPalette[lColorIndex][2], pGamma);

		// Return in the int domain
		lRed *= 256;
		lGreen *= 256;
		lBlue *= 256;

		if(lRed >= 256) {
			lRed = 255;
		}

		if(lGreen >= 256) {
			lGreen = 255;
		}

		if(lBlue >= 256) {
			lBlue = 255;
		}

		AssignPaletteEntry(lReturnValue[lColorIndex], lRed, lGreen, lBlue);
	}

	for(; lColorIndex < MR_BASIC_COLORS; lColorIndex++) {

		AssignPaletteEntry(lReturnValue[lColorIndex], 255, 255, lColorIndex - 15);

		lColorIndex++;
	}

	return lReturnValue;
}

/**
 * Generate a palette entry from RGB components.
 * This is usually to apply color correction to the background palette.
 * @param pRed The red component.
 * @param pGreen The green component.
 * @param pBlue The blue component.
 * @param pGamma The gamma correction (the inverse of the configuration gamma).
 * @param pIntensity The color intensity (contrast * brightness).
 * @param pIntensityBase The base intensity (brightness - intensity).
 * @return The generated palette entry.
 */
const paletteEntry_t &ConvertColor(MR_UInt8 pRed, MR_UInt8 pGreen, MR_UInt8 pBlue, double pGamma, double pIntensity, double pIntensityBase)
{
	static paletteEntry_t lReturnValue;

	double lRed;
	double lGreen;
	double lBlue;

	// Compute the gamma correction
	lRed = pIntensityBase + pIntensity * pow(pRed / 256.0, pGamma);
	lGreen = pIntensityBase + pIntensity * pow(pGreen / 256.0, pGamma);
	lBlue = pIntensityBase + pIntensity * pow(pBlue / 256.0, pGamma);

	lRed *= 256;
	lGreen *= 256;
	lBlue *= 256;

	if(lRed >= 256) {
		lRed = 255;
	}

	if(lGreen >= 256) {
		lGreen = 255;
	}

	if(lBlue >= 256) {
		lBlue = 255;
	}

	AssignPaletteEntry(lReturnValue, lRed, lGreen, lBlue);

	return lReturnValue;
}

}  // namespace ColorPalette

}  // namespace VideoServices
}  // namespace HoverRace
