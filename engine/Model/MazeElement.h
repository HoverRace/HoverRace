// MazeElement.h
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

#include "Shapes.h"
#include "ContactEffect.h"
#include "../Util/DllObjectFactory.h"
#include "../VideoServices/Viewport3D.h"

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {
	namespace Model {
		class Level;
	}
	namespace Parcel {
		class ObjStream;
	}
}

namespace HoverRace {
namespace Model {

struct ElementNetState
{
	int mDataLen;
	const MR_UInt8 *mData;
};

/**
 * Represents all objects (wall, floor, ceiling, etc.) that can be found
 * in a maze.
 *
 * There are two kinds of elements, the surface elements and the free elements.
 * Surface elements have a geometry based on the maze geometry. That means that
 * a wall element contains no geometry, its geometry is defined by the surface
 * of the maze that it is attached to.
 *
 * Free elements are monsters, furniture, etc.  They contain their own geometry
 * but they do not contain their position or orientation.
 *
 * Elements are formed from components. There are two kinds of components in an
 * element, the logic component and the rendering component.
 * The logic component owns the part of the object that is responsible of the
 * logic simulation of the element. The rendering component contains the data
 * needed for the rendering of the object (usually this component is empty).
 *
 * Data that is shared by the two possible components is stored directly in the
 * element body.
 */
class MR_DllDeclare Element : public Util::ObjectFromFactory
{
	using SUPER = Util::ObjectFromFactory;

public:
	Element(const Util::ObjectFromFactoryId &id) : SUPER(id) { }
	virtual ~Element() { }

	virtual void AddRenderer() { }

	// Contact effect interface
	virtual void ApplyEffect(const ContactEffect *pEffect,
		MR_SimulationTime pTime, MR_SimulationTime pDuration,
		BOOL pValidDirection, MR_Angle pHorizontalDirection,
		MR_Int32 pZMin, MR_Int32 pZMax,
		Level *pLevel)
	{
		HR_UNUSED(pEffect, pTime, pDuration,
			pValidDirection, pHorizontalDirection,
			pZMin, pZMax, pLevel);
	}

	virtual const ContactEffectList *GetEffectList() { return nullptr; }

	// Helper functions
	void ApplyEffects(const ContactEffectList *pList,
		MR_SimulationTime pTime, MR_SimulationTime pDuration,
		BOOL pValidDirection, MR_Angle pHorizontalDirection,
		MR_Int32 pZMin, MR_Int32 pZMax,
		Level *pLevel);

private:
	// Prevent overriding of the Serialize function.
	void Serialize(HoverRace::Parcel::ObjStream &pArchive) override
	{
		SUPER::Serialize(pArchive);
	}
};

class MR_DllDeclare SurfaceElement : public Element
{
	using SUPER = Element;

public:
	SurfaceElement(const Util::ObjectFromFactoryId &id) : SUPER(id) { }
	virtual ~SurfaceElement() { }

	virtual void RenderWallSurface(VideoServices::Viewport3D *pDest,
		const MR_3DCoordinate &pUpperLeft, const MR_3DCoordinate &pLowerRight,
		MR_Int32 pLen, MR_SimulationTime pTime)
	{
		HR_UNUSED(pDest, pUpperLeft, pLowerRight, pLen, pTime);
	}

	virtual void RenderHorizontalSurface(VideoServices::Viewport3D *pDest,
		int pNbVertex, const MR_2DCoordinate *pVertexList,
		MR_Int32 pLevel, BOOL pTop, MR_SimulationTime pTime)
	{
		HR_UNUSED(pDest, pNbVertex, pVertexList, pLevel, pTop, pTime);
	}
};

class MR_DllDeclare FreeElement : public Element
{
	using SUPER = Element;

public:
	FreeElement(const Util::ObjectFromFactoryId &id) : SUPER(id) { }
	virtual ~FreeElement() { }

	virtual void Render(VideoServices::Viewport3D *pDest,
		MR_SimulationTime pTime)
	{
		HR_UNUSED(pDest, pTime);
	}

	virtual void PlayInternalSounds() { }
	virtual void PlayExternalSounds(int pDB, int pPan) { HR_UNUSED(pDB, pPan); }

	virtual ElementNetState GetNetState() const;
	virtual void SetNetState(int pDataLen, const MR_UInt8 *pData);

	// Logic interface (For simulation)

	/**
	 * Advance the simulation.
	 * @param pTimeSlice The time slice to simulate over.
	 * @param [in,out] pLevel The level (may be @c nullptr).
	 * @param pRoom The room number.
	 * @return The new room number.
	 */
	virtual int Simulate(MR_SimulationTime pTimeSlice,
		Level *pLevel, int pRoom)
	{
		HR_UNUSED(pTimeSlice, pLevel);
		return pRoom;
	}

	/**
	 * Get the shape that stops other elements' movement.
	 * @return The shape, or @c nullptr if none.
	 */
	virtual const ShapeInterface *GetObstacleShape() { return nullptr; }

	/**
	 * Get the shape that gives an effect when touched by a moving element.
	 * @return The shape, or @c nullptr if none.
	 */
	virtual const ShapeInterface *GetReceivingContactEffectShape() { return nullptr; }

	/**
	 * Get the shape that gives an effect when touching while moving.
	 * @return The shape, or @c nullptr if none.
	 */
	virtual const ShapeInterface *GetGivingContactEffectShape() { return nullptr; }

	// Perm state hook

	/**
	 * Assign a permanent hook number.
	 * @param pNumber The hook number.
	 * @return @c true if accepted, @c false otherwise.
	 */
	virtual BOOL AssignPermNumber(int pNumber)
	{
		HR_UNUSED(pNumber);
		return FALSE;
	}

	virtual void SetOwnerId(int pOwnerId) { HR_UNUSED(pOwnerId); }

public:
	MR_3DCoordinate mPosition;
	MR_Angle mOrientation;
};

}  // namespace Model
}  // namespace HoverRace

#undef MR_DllDeclare
