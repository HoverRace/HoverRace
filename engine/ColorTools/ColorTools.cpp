
// ColorTools.cpp
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

#include "ColorTools.h"

namespace {

// Constants
struct ColorStructure
{
	double mRed;
	double mGreen;
	double mBlue;
};

// Basic colors
#define I  /255.0

const ColorStructure eBlack = { 0.0, 0.0, 0.0 };
const ColorStructure eWhite = { 1.0, 1.0, 1.0 };

const ColorStructure eSkin = { 248 I, 165 I, 98 I };
const ColorStructure eRoadYellow = { 252 I, 215 I, 1 I };
const ColorStructure eRoadGreen = { 89 I, 205 I, 69 I };
const ColorStructure eRoadRed = { 253 I, 10 I, 10 I };

ColorStructure eCarColor[] =
{
	{											  /* Red    */
		227 I, 6 I, 62 I
	},
	{											  /* Blue   */
		0 I, 44 I, 193 I
	},
	{											  /* Purple */
		142 I, 70 I, 166 I
	},
	{											  /* Yellow */
		247 I, 247 I, 0 I
	},
	{											  /* Orange */
		255 I, 136 I, 17 I
	}
};

#undef I

#define MR_COLOR_COMP        64

// Local variables
const int eNbColors = 68;

ColorStructure gsColors[eNbColors];
MR_UInt8 gsBestMatch[MR_COLOR_COMP][MR_COLOR_COMP][MR_COLOR_COMP];

const ColorStructure &ColorMix(const ColorStructure &pColor0,
	const ColorStructure &pColor1, int pRatio0, int pRatio1)
{
	double lRatio0 = (double) pRatio0 / (double) (pRatio0 + pRatio1);
	double lRatio1 = 1.0 - lRatio0;

	static ColorStructure lReturnValue;

	lReturnValue.mRed = pColor0.mRed * lRatio0 + pColor1.mRed * lRatio1;
	lReturnValue.mGreen = pColor0.mGreen * lRatio0 + pColor1.mGreen * lRatio1;
	lReturnValue.mBlue = pColor0.mBlue * lRatio0 + pColor1.mBlue * lRatio1;

	return lReturnValue;
}

MR_UInt8 GetNearestColor(double pRed, double pGreen, double pBlue)
{
	MR_UInt8 lReturnValue = 255;
	double lMinimalDistance = 10000;

	double lSourceIntensity = sqrt(pRed * pRed + pGreen + pGreen + pBlue * pBlue);

	for(MR_UInt8 lCurrentIndex = 0; lCurrentIndex < eNbColors; lCurrentIndex++) {
		double lRed = gsColors[lCurrentIndex].mRed;
		double lGreen = gsColors[lCurrentIndex].mGreen;
		double lBlue = gsColors[lCurrentIndex].mBlue;

		double lColorIntensity = sqrt(lRed * lRed + lGreen + lGreen + lBlue * lBlue);

		if(lColorIntensity > 0.05) {
			double lMultiplier = ((lSourceIntensity / lColorIntensity) * 1.0 + 1.0) / 2.0;

			lRed *= lMultiplier;
			lGreen *= lMultiplier;
			lBlue *= lMultiplier;
		}

		double lGreenDist = 12 * pow(lGreen - pGreen, 2);

		if(lGreenDist < lMinimalDistance) {
			double lRedDist = 8 * pow(lRed - pRed, 2);

			if(lRedDist + lGreenDist < lMinimalDistance) {
				double lBlueDist = pow(lBlue - pBlue, 2);

				if(lRedDist + lGreenDist + lBlueDist < lMinimalDistance) {
					// Compute the distance
					double lDistance = lRedDist + lGreenDist + lBlueDist;

					if(lDistance < lMinimalDistance) {
						lReturnValue = lCurrentIndex;
						lMinimalDistance = lDistance;
					}
				}
			}
		}
	}

	ASSERT(lReturnValue != 255);

	return lReturnValue;
}

} // namespace

void MR_ColorTools::Init()
{

	// init gsColors
	gsColors[0] = eWhite;
	gsColors[1] = eBlack;
	gsColors[2] = ColorMix(eWhite, eBlack, 15, 1);
	gsColors[3] = ColorMix(eWhite, eBlack, 14, 2);
	gsColors[4] = ColorMix(eWhite, eBlack, 13, 3);
	gsColors[5] = ColorMix(eWhite, eBlack, 12, 4);
	gsColors[6] = ColorMix(eWhite, eBlack, 11, 5);
	gsColors[7] = ColorMix(eWhite, eBlack, 10, 6);
	gsColors[8] = ColorMix(eWhite, eBlack, 9, 7);
	gsColors[9] = ColorMix(eWhite, eBlack, 8, 8);
	gsColors[10] = ColorMix(eWhite, eBlack, 7, 9);
	gsColors[11] = ColorMix(eWhite, eBlack, 6, 10);
	gsColors[12] = ColorMix(eWhite, eBlack, 5, 11);
	gsColors[13] = ColorMix(eWhite, eBlack, 4, 12);
	gsColors[14] = ColorMix(eWhite, eBlack, 3, 13);
	gsColors[15] = ColorMix(eWhite, eBlack, 2, 14);
	gsColors[16] = ColorMix(eWhite, eBlack, 1, 15);

	gsColors[17] = eSkin;
	gsColors[18] = ColorMix(eSkin, eBlack, 9, 1);
	gsColors[19] = ColorMix(eSkin, eBlack, 7, 3);
	gsColors[20] = ColorMix(eSkin, eWhite, 3, 1);

	gsColors[21] = eRoadGreen;
	gsColors[22] = ColorMix(eRoadGreen, eBlack, 3, 1);
	gsColors[23] = ColorMix(eRoadGreen, eBlack, 2, 2);
	gsColors[24] = ColorMix(eRoadGreen, eWhite, 1, 1);

	gsColors[25] = eRoadRed;
	gsColors[26] = ColorMix(eRoadRed, eBlack, 3, 1);
	gsColors[27] = ColorMix(eRoadRed, eBlack, 2, 2);
	gsColors[28] = ColorMix(eRoadRed, eWhite, 1, 1);

	gsColors[29] = ColorMix(eRoadYellow, eBlack, 4, 0);
	gsColors[30] = ColorMix(eRoadYellow, eBlack, 3, 1);
	gsColors[31] = ColorMix(eRoadYellow, eBlack, 2, 2);
	gsColors[32] = ColorMix(eRoadYellow, eBlack, 1, 3);

	gsColors[33] = ColorMix(eCarColor[0], eBlack, 10, 0);
	gsColors[34] = ColorMix(eCarColor[0], eBlack, 9, 1);
	gsColors[35] = ColorMix(eCarColor[0], eBlack, 8, 2);
	gsColors[36] = ColorMix(eCarColor[0], eBlack, 7, 3);
	gsColors[37] = ColorMix(eCarColor[0], eWhite, 3, 1);
	gsColors[38] = ColorMix(eCarColor[0], eWhite, 2, 2);
	gsColors[39] = ColorMix(eCarColor[0], eWhite, 1, 3);

	gsColors[40] = ColorMix(eCarColor[1], eBlack, 10, 0);
	gsColors[41] = ColorMix(eCarColor[1], eBlack, 9, 1);
	gsColors[42] = ColorMix(eCarColor[1], eBlack, 8, 2);
	gsColors[43] = ColorMix(eCarColor[1], eBlack, 7, 3);
	gsColors[44] = ColorMix(eCarColor[1], eWhite, 3, 1);
	gsColors[45] = ColorMix(eCarColor[1], eWhite, 2, 2);
	gsColors[46] = ColorMix(eCarColor[1], eWhite, 1, 3);

	gsColors[47] = ColorMix(eCarColor[2], eBlack, 10, 0);
	gsColors[48] = ColorMix(eCarColor[2], eBlack, 9, 1);
	gsColors[49] = ColorMix(eCarColor[2], eBlack, 8, 2);
	gsColors[50] = ColorMix(eCarColor[2], eBlack, 7, 3);
	gsColors[51] = ColorMix(eCarColor[2], eWhite, 3, 1);
	gsColors[52] = ColorMix(eCarColor[2], eWhite, 2, 2);
	gsColors[53] = ColorMix(eCarColor[2], eWhite, 1, 3);

	gsColors[54] = ColorMix(eCarColor[3], eBlack, 12, 0);
	gsColors[55] = ColorMix(eCarColor[3], eBlack, 11, 1);
	gsColors[56] = ColorMix(eCarColor[3], eBlack, 10, 2);
	gsColors[57] = ColorMix(eCarColor[3], eBlack, 9, 3);
	gsColors[58] = ColorMix(eCarColor[3], eWhite, 3, 1);
	gsColors[59] = ColorMix(eCarColor[3], eWhite, 2, 2);
	gsColors[60] = ColorMix(eCarColor[3], eWhite, 1, 3);

	gsColors[61] = ColorMix(eCarColor[4], eBlack, 12, 0);
	gsColors[62] = ColorMix(eCarColor[4], eBlack, 11, 1);
	gsColors[63] = ColorMix(eCarColor[4], eBlack, 10, 2);
	gsColors[64] = ColorMix(eCarColor[4], eBlack, 9, 3);
	gsColors[65] = ColorMix(eCarColor[4], eWhite, 3, 1);
	gsColors[66] = ColorMix(eCarColor[4], eWhite, 2, 2);
	gsColors[67] = ColorMix(eCarColor[4], eWhite, 1, 3);

	// Init gsBestMatch
	for(int lRed = 0; lRed < MR_COLOR_COMP; lRed++) {
		for(int lGreen = 0; lGreen < MR_COLOR_COMP; lGreen++) {
			for(int lBlue = 0; lBlue < MR_COLOR_COMP; lBlue++) {
				double lRedComp = (lRed + 0.5) / double (MR_COLOR_COMP);
				double lGreenComp = (lGreen + 0.5) / double (MR_COLOR_COMP);
				double lBlueComp = (lBlue + 0.5) / double (MR_COLOR_COMP);

				gsBestMatch[lRed][lGreen][lBlue] = GetNearestColor(lRedComp, lGreenComp, lBlueComp);
			}
		}
	}
}

MR_UInt8 MR_ColorTools::GetNearest(double pRed, double pGreen, double pBlue)
{

	int lRed = int (pRed * double (MR_COLOR_COMP));
	int lGreen = int (pGreen * double (MR_COLOR_COMP));
	int lBlue = int (pBlue * double (MR_COLOR_COMP));

	if(lRed < 0) {
		lRed = 0;
	}
	else if(lRed >= MR_COLOR_COMP) {
		lRed = MR_COLOR_COMP - 1;
	}

	if(lGreen < 0) {
		lGreen = 0;
	}
	else if(lGreen >= MR_COLOR_COMP) {
		lGreen = MR_COLOR_COMP - 1;
	}

	if(lBlue < 0) {
		lBlue = 0;
	}
	else if(lBlue >= MR_COLOR_COMP) {
		lBlue = MR_COLOR_COMP - 1;
	}

	return gsBestMatch[lRed][lGreen][lBlue];

}

void MR_ColorTools::GetComponents(MR_UInt8 pColor, double &pRed, double &pGreen, double &pBlue)
{
	pRed = gsColors[pColor].mRed;
	pGreen = gsColors[pColor].mGreen;
	pBlue = gsColors[pColor].mBlue;
}

void MR_ColorTools::GetIntComponents(MR_UInt8 pColor, int &pRed, int &pGreen, int &pBlue)
{
	pRed = int (gsColors[pColor].mRed * 256.0);
	pGreen = int (gsColors[pColor].mGreen * 256.0);
	pBlue = int (gsColors[pColor].mBlue * 256.0);

	if(pRed < 0) {
		pRed = 0;
	}
	else if(pRed >= 256) {
		pRed = 255;
	}

	if(pGreen < 0) {
		pGreen = 0;
	}
	else if(pGreen >= 256) {
		pGreen = 255;
	}

	if(pBlue < 0) {
		pBlue = 0;
	}
	else if(pBlue >= 256) {
		pBlue = 255;
	}
}

int MR_ColorTools::GetNbColors()
{
	return eNbColors;
}

