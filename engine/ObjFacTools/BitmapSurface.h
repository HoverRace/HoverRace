// BitmapSurface.h
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
// The class defined in this file can be used as a base class for
// walls or floor that are going to be simple bitmapped surfaces
//
// The bitmap must come from a resource
//

#pragma once

#include "ResourceLib.h"
#include "../Model/MazeElement.h"

#ifdef MR_ENGINE
#define MR_DllDeclare   __declspec( dllexport )
#else
#define MR_DllDeclare   __declspec( dllimport )
#endif

namespace HoverRace {
namespace ObjFac1 {

class MR_DllDeclare BitmapSurface : public MR_SurfaceElement
{
	protected:
		MR_ResBitmap * mBitmap;
		MR_ResBitmap *mBitmap2;
		int mRotationSpeed;						  // negative values mean left to right rotation
		int mRotationLen;

	public:
												  // old constructor..obsolete
		BitmapSurface(const MR_ObjectFromFactoryId & pId);
		BitmapSurface(const MR_ObjectFromFactoryId & pId, /*const */ MR_ResBitmap * pBitmap);
		BitmapSurface(const MR_ObjectFromFactoryId & pId, MR_ResBitmap * pBitmap1, MR_ResBitmap * pBitmap2, int pRotationSpeed, int pRotationLen);
		~BitmapSurface();

		// Rendering stuff
		void RenderWallSurface(MR_3DViewPort * pDest, const MR_3DCoordinate & pUpperLeft, const MR_3DCoordinate & pLowerRight, MR_Int32 pLen, MR_SimulationTime pTime);
		void RenderHorizontalSurface(MR_3DViewPort * pDest, int pNbVertex, const MR_2DCoordinate * pVertexList, MR_Int32 pLevel, BOOL pTop, MR_SimulationTime pTime);

		// Logic stuff
		const MR_ContactEffectList *GetEffectList();

};

class MR_DllDeclare VStretchBitmapSurface : public BitmapSurface
{
	private:
		int mMaxHeight;

	public:

		VStretchBitmapSurface(const MR_ObjectFromFactoryId & pId, /*const */ MR_ResBitmap * pBitmap, int pMaxHeight);
		VStretchBitmapSurface(const MR_ObjectFromFactoryId & pId, MR_ResBitmap * pBitmap1, MR_ResBitmap * pBitmap2, int pRotationSpeed, int pRotationLen, int pMaxHeight);
		void RenderWallSurface(MR_3DViewPort * pDest, const MR_3DCoordinate & pUpperLeft, const MR_3DCoordinate & pLowerRight, MR_Int32 pLen, MR_SimulationTime pTime);
};

}  // namespace Model
}  // namespace HoverRace

#undef MR_DllDeclare
