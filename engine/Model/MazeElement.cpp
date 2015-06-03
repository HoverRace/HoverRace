// Element.cpp
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

#include "../Parcel/ObjStream.h"

#include "MazeElement.h"

using HoverRace::Parcel::ObjStream;

namespace HoverRace {
namespace Model {

// Element default behavior

BOOL Element::InitString(const char *pInitStr)
{
	BOOL lReturnValue = TRUE;

	// Return an error if this function have not been
	// overridden and if the parameters are not null
	if (pInitStr && strlen(pInitStr) > 0)
		lReturnValue = FALSE;

	return lReturnValue;
}

void Element::ApplyEffects(const ContactEffectList *pList,
	MR_SimulationTime pTime, MR_SimulationTime pDuration,
	BOOL pValidDirection, MR_Angle pHorizontalDirection,
	MR_Int32 pZMin, MR_Int32 pZMax,
	Level *pLevel)
{
	if (pList) {
		for (auto lEffect : *pList) {
			ApplyEffect(lEffect, pTime, pDuration,
				pValidDirection, pHorizontalDirection,
				pZMin, pZMax, pLevel);
		}
	}
}

// FreeElement default behavior

ElementNetState FreeElement::GetNetState() const
{
	ElementNetState lReturnValue = { 0, nullptr };
	return lReturnValue;
}

void FreeElement::SetNetState(int pDataLen, const MR_UInt8*)
{
	ASSERT(pDataLen == 0);
}

}  // namespace Model
}  // namespace HoverRace
