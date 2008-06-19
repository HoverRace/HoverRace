// MazeElement.h
//
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

//
// Elemetens is a class that is used to represent all the objects (Wall, floor, ceiling, Monster...)
// that can be found in a maze.
//
//

// They are two kings of elements, the surface elements and the free elements. The Surface
// elements have a geometry based of the maze geometry. That mean that a wall elements
// contains no geometry, it's geometry is defines by the surtace of the maze that it is attached to.
//
// free elements are monsters, furnitures... They contains there own geometry but they do not contains
// there position and orientation.
//
// Elements are formed from component. They are two kinds of components in an element, the Logic
// component and the rendering component. The logic component own the part of the object that
// is responsible of the logic simulation of the element. The rendering component contains the
// data needed for the rendering of the object(usually this component is empty)
//
// Data that is shared by the two possible component is store directly in the element body
//

#ifndef MAZE_ELEMENT_H
#define MAZE_ELEMENT_H

#include "Shapes.h"
#include "ContactEffect.h"
#include "../Util/DllObjectFactory.h"
#include "../VideoServices/3DViewPort.h"

#ifdef MR_MODEL
#define MR_DllDeclare   __declspec( dllexport )
#else
#define MR_DllDeclare   __declspec( dllimport )
#endif

// Class Declaration
class MR_Level;

// Renderer bases
class MR_SurfaceElementRenderer;				  // used and defines by the Observer module
class MR_FreeElementRenderer;					  // used and defines by the Observer module

class MR_FreeElementMovingInterface;

class MR_ElementNetState
{
	public:
		int mDataLen;
		const MR_UInt8 *mData;
};

class MR_DllDeclare MR_Element:public MR_ObjectFromFactory
{
	public:
		MR_Element(const MR_ObjectFromFactoryId & pId);

		virtual void AddRenderer();
												  // Initialisation string used during maze creation
		virtual BOOL InitString(const char *pInitStr = NULL);

												  // Serialize the logic component and the body of the component
		virtual void SerializeLogicState(CArchive & pArchive);

		// Contact effect interface
		virtual void ApplyEffect(const MR_ContactEffect * pEffect, MR_SimulationTime pTime, MR_SimulationTime pDuration, BOOL pValidDirection, MR_Angle pHorizontalDirection, MR_Int32 pZMin, MR_Int32 pZMax, MR_Level * pLevel);

		virtual const MR_ContactEffectList *GetEffectList();

		// Helper functions
		void ApplyEffects(const MR_ContactEffectList * pList, MR_SimulationTime pTime, MR_SimulationTime pDuration, BOOL pValidDirection, MR_Angle pHorizontalDirection, MR_Int32 pZMin, MR_Int32 pZMax, MR_Level * pLevel);

	private:
												  // Prevent overriding of the Serialize function
		virtual void Serialize(CArchive & pArchive);
};

class MR_DllDeclare MR_SurfaceElement:public MR_Element
{
	public:
		MR_SurfaceElement(const MR_ObjectFromFactoryId & pId);

		virtual void RenderWallSurface(MR_3DViewPort * pDest, const MR_3DCoordinate & pUpperLeft, const MR_3DCoordinate & pLowerRight, MR_Int32 pLen, MR_SimulationTime pTime);

		virtual void RenderHorizontalSurface(MR_3DViewPort * pDest, int pNbVertex, const MR_2DCoordinate * pVertexList, MR_Int32 pLevel, BOOL pTop, MR_SimulationTime pTime);
};

class MR_DllDeclare MR_FreeElement:public MR_Element
{
	public:
		MR_3DCoordinate mPosition;
		MR_Angle mOrientation;

	public:
		MR_FreeElement(const MR_ObjectFromFactoryId & pId);

		virtual void Render(MR_3DViewPort * pDest, MR_SimulationTime pTime);

		virtual void PlayInternalSounds();
		virtual void PlayExternalSounds(int pDB, int pPan);

		virtual MR_ElementNetState GetNetState() const;
		virtual void SetNetState(int pDataLen, const MR_UInt8 * pData);

		// Logic interface (For simulation)
												  // shoud return new room number
		virtual int Simulate(MR_SimulationTime pTimeSlice, MR_Level * pLevel, int pRoom);

												  // Shape that stop other elements movement
		virtual const MR_ShapeInterface *GetObstacleShape();
												  // Shape that give an effect when touched by a moving element
		virtual const MR_ShapeInterface *GetReceivingContactEffectShape();
												  // Shape that give an effect when touching while moving
		virtual const MR_ShapeInterface *GetGivingContactEffectShape();

		// Perm state hook
												  // object must return TRUE only if it accept the permanent hook number
		virtual BOOL AssignPermNumber(int pNumber);
		virtual void SetOwnerId(int pOwnerId);
};

#undef MR_DllDeclare
#endif
