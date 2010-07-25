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

#include "StdAfx.h"

#include "../Parcel/ObjStream.h"

#include "MazeElement.h"

using HoverRace::Parcel::ObjStream;

namespace HoverRace {
namespace Model {

// Element default behavior

Element::Element(const Util::ObjectFromFactoryId & pId) :
	Util::ObjectFromFactory(pId)
{
}

void Element::AddRenderer()
{												  /* The default function do notting */
}

BOOL Element::InitString(const char *pInitStr)
{
	BOOL lReturnValue = TRUE;

	// Return an error if this function have not been
	// overridden and if the parameters are not null
	if(pInitStr != NULL && strlen(pInitStr) > 0)
		lReturnValue = FALSE;

	return lReturnValue;
}

void Element::SerializeLogicState(ObjStream & /*pArchive */ )
{												  /* The default function do notting */
}

void Element::Serialize(ObjStream & pArchive)
{
	Util::ObjectFromFactory::Serialize(pArchive);
}

void Element::ApplyEffect(const MR_ContactEffect * /*pEffect */ ,
MR_SimulationTime /*pTime */ ,
MR_SimulationTime /*pDuration */ ,
BOOL /*pValidRirection */ ,
MR_Angle /*pHorizontalDirection */ ,
MR_Int32 /*pZMin */ ,
MR_Int32 /*pZMax */ ,
Level * /*pLevel */ )
{												  /* Do notting by default */
}

const MR_ContactEffectList *Element::GetEffectList()
{
	return NULL;
}

void Element::ApplyEffects(const MR_ContactEffectList * pList, MR_SimulationTime pTime, MR_SimulationTime pDuration, BOOL pValidDirection, MR_Angle pHorizontalDirection, MR_Int32 pZMin, MR_Int32 pZMax, Level * pLevel)
{
	if(pList != NULL) {
		for (MR_ContactEffectList::const_iterator iter = pList->begin();
			iter != pList->end(); ++iter)
		{
			const MR_ContactEffect *lEffect = *iter;
			ApplyEffect(lEffect, pTime, pDuration, pValidDirection, pHorizontalDirection, pZMin, pZMax, pLevel);
		}
	}
}

// SurfaceElement default behavior
SurfaceElement::SurfaceElement(const Util::ObjectFromFactoryId & pId) :
	Element(pId)
{
}

void SurfaceElement::RenderWallSurface(VideoServices::Viewport3D * /*pDest */ ,
const MR_3DCoordinate & /*pUpperLeft */ ,
const MR_3DCoordinate & /*pLowerRight */ ,
MR_Int32 /*pLen */ ,
MR_SimulationTime /*pTime */ )
{												  /* Notting is rendered by default */
}

void SurfaceElement::RenderHorizontalSurface(VideoServices::Viewport3D * /*pDest */ ,
int /*pNbVertex */ ,
const MR_2DCoordinate * /*pVertexList */ ,
MR_Int32 /*pLevel */ ,
BOOL /*pTop */ ,
MR_SimulationTime /*pTime */ )
{												  /* Notting is rendered by default */
}

// FreeElement default behavior

FreeElement::FreeElement(const Util::ObjectFromFactoryId & pId) :
	Element(pId)
{
}

ElementNetState FreeElement::GetNetState() const
{
	ElementNetState lReturnValue = { 0, NULL };
	return lReturnValue;
}

void FreeElement::SetNetState(int pDataLen, const MR_UInt8 * /*pData */ )
{
	ASSERT(pDataLen == 0);						  // Why a Get without a Set??
}

void FreeElement::Render(VideoServices::Viewport3D * /*pDest */ , MR_SimulationTime /*pTime */ )
{												  /* Notting is rendered by default */
}

int FreeElement::Simulate(MR_SimulationTime, Level *, int pRoom)
{												  /* Notting to do by default */
	return pRoom;
}

const ShapeInterface *FreeElement::GetObstacleShape()
{
	return NULL;
}

const ShapeInterface *FreeElement::GetReceivingContactEffectShape()
{
	return NULL;
}

const ShapeInterface *FreeElement::GetGivingContactEffectShape()
{
	return NULL;
}

void FreeElement::PlayInternalSounds()
{
}

void FreeElement::PlayExternalSounds(int /*pDB */ , int /*pPan */ )
{
}

BOOL FreeElement::AssignPermNumber(int /*pNumber */ )
{
	return FALSE;
}

void FreeElement::SetOwnerId(int /*pOwnerId */ )
{
}

}  // namespace Model
}  // namespace HoverRace
