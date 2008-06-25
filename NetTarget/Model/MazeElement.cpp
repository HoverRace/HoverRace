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

#include "stdafx.h"

#include "MazeElement.h"
// #include "Shapes.h"

// MR_Element default behavior

MR_Element::MR_Element(const MR_ObjectFromFactoryId & pId)
:MR_ObjectFromFactory(pId)
{
}

void MR_Element::AddRenderer()
{												  /* The default function do notting */
}

BOOL MR_Element::InitString(const char *pInitStr)
{
	BOOL lReturnValue = TRUE;

	// Return an error if this function have not been
	// overridden and if the parameters are not null
	if(pInitStr != NULL && strlen(pInitStr) > 0)
		lReturnValue = FALSE;

	return lReturnValue;
}

void MR_Element::SerializeLogicState(CArchive & /*pArchive */ )
{												  /* The default function do notting */
}

void MR_Element::Serialize(CArchive & pArchive)
{
	MR_ObjectFromFactory::Serialize(pArchive);
}

void MR_Element::ApplyEffect(const MR_ContactEffect * /*pEffect */ ,
MR_SimulationTime /*pTime */ ,
MR_SimulationTime /*pDuration */ ,
BOOL /*pValidRirection */ ,
MR_Angle /*pHorizontalDirection */ ,
MR_Int32 /*pZMin */ ,
MR_Int32 /*pZMax */ ,
MR_Level * /*pLevel */ )
{												  /* Do notting by default */
}

const MR_ContactEffectList *MR_Element::GetEffectList()
{
	return NULL;
}

void MR_Element::ApplyEffects(const MR_ContactEffectList * pList, MR_SimulationTime pTime, MR_SimulationTime pDuration, BOOL pValidDirection, MR_Angle pHorizontalDirection, MR_Int32 pZMin, MR_Int32 pZMax, MR_Level * pLevel)
{
	if(pList != NULL) {
		POSITION lPos = pList->GetHeadPosition();

		while(lPos != NULL) {
			const MR_ContactEffect *lEffect = pList->GetNext(lPos);
			ApplyEffect(lEffect, pTime, pDuration, pValidDirection, pHorizontalDirection, pZMin, pZMax, pLevel);
		}
	}
}

// MR_SurfaceElement default behavior
MR_SurfaceElement::MR_SurfaceElement(const MR_ObjectFromFactoryId & pId)
:MR_Element(pId)
{
}

void MR_SurfaceElement::RenderWallSurface(MR_3DViewPort * /*pDest */ ,
const MR_3DCoordinate & /*pUpperLeft */ ,
const MR_3DCoordinate & /*pLowerRight */ ,
MR_Int32 /*pLen */ ,
MR_SimulationTime /*pTime */ )
{												  /* Notting is rendered by default */
}

void MR_SurfaceElement::RenderHorizontalSurface(MR_3DViewPort * /*pDest */ ,
int /*pNbVertex */ ,
const MR_2DCoordinate * /*pVertexList */ ,
MR_Int32 /*pLevel */ ,
BOOL /*pTop */ ,
MR_SimulationTime /*pTime */ )
{												  /* Notting is rendered by default */
}

// MR_FreeElement default behavior

MR_FreeElement::MR_FreeElement(const MR_ObjectFromFactoryId & pId)
:MR_Element(pId)
{
}

MR_ElementNetState MR_FreeElement::GetNetState() const
{
	MR_ElementNetState lReturnValue = { 0, NULL };
	return lReturnValue;
}

void MR_FreeElement::SetNetState(int pDataLen, const MR_UInt8 * /*pData */ )
{
	ASSERT(pDataLen == 0);						  // Why a Get without a Set??
}

void MR_FreeElement::Render(MR_3DViewPort * /*pDest */ , MR_SimulationTime /*pTime */ )
{												  /* Notting is rendered by default */
}

int MR_FreeElement::Simulate(MR_SimulationTime, MR_Level *, int pRoom)
{												  /* Notting to do by default */
	return pRoom;
}

const MR_ShapeInterface *MR_FreeElement::GetObstacleShape()
{
	return NULL;
}

const MR_ShapeInterface *MR_FreeElement::GetReceivingContactEffectShape()
{
	return NULL;
}

const MR_ShapeInterface *MR_FreeElement::GetGivingContactEffectShape()
{
	return NULL;
}

void MR_FreeElement::PlayInternalSounds()
{
}

void MR_FreeElement::PlayExternalSounds(int /*pDB */ , int /*pPan */ )
{
}

BOOL MR_FreeElement::AssignPermNumber(int /*pNumber */ )
{
	return FALSE;
}

void MR_FreeElement::SetOwnerId(int /*pOwnerId */ )
{
}
