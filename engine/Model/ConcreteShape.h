// ConcreteShape.h
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
// This file contains non-abstact class that can be used
// to create stand alone MR_ShapeInterface derived objects

#pragma once

#include "Shapes.h"

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
namespace Model {

class MR_DllDeclare Cylinder : public CylinderShape
{
	public:
		MR_2DCoordinate mAxis;
		MR_Int32 mRayLen;
		MR_Int32 mZMin;
		MR_Int32 mZMax;

		MR_Int32 ZMin() const;
		MR_Int32 ZMax() const;
		MR_Int32 AxisX() const;
		MR_Int32 AxisY() const;
		MR_Int32 RayLen() const;

};

}  // namespace Model
}  // namespace HoverRace

#undef MR_DllDeclare
