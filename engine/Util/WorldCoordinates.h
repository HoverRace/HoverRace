// WorldCoordinates.h
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

#ifndef WORLD_COORDINATES_H
#define WORLD_COORDINATES_H

#include "MR_Types.h"

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
	namespace Parcel {
		class ObjStream;
	}
}

// All geometrical coordinates are in millimeters.

class MR_DllDeclare MR_2DCoordinate
{
public:
	MR_2DCoordinate() { }
	MR_2DCoordinate(MR_Int32 pX, MR_Int32 pY) : mX(pX), mY(pY) { }

	void Serialize(HoverRace::Parcel::ObjStream &pArchive);

	// Comparison operators
	bool operator==(const MR_2DCoordinate &pCoordinate) const
	{
		return (mX == pCoordinate.mX) && (mY == pCoordinate.mY);
	}

	bool operator!=(const MR_2DCoordinate &pCoordinate) const
	{
		return (mX != pCoordinate.mX) || (mY != pCoordinate.mY);
	}

	MR_Int32 mX;
	MR_Int32 mY;
};

class MR_DllDeclare MR_3DCoordinate : public MR_2DCoordinate
{
	using SUPER = MR_2DCoordinate;

public:
	MR_3DCoordinate() { }
	MR_3DCoordinate(MR_Int32 pX, MR_Int32 pY, MR_Int32 pZ) :
		SUPER(pX, pY), mZ(pZ) { }

	void Serialize(HoverRace::Parcel::ObjStream &pArchive);

	MR_Int32 mZ;
};

// Angles
using MR_Angle = MR_Int16;

#define MR_PI             2048
#define MR_2PI            4096

#define MR_NORMALIZE_ANGLE( pAngle ) ( (MR_Angle) (( 2*MR_2PI+(pAngle) )%MR_2PI) )

#define RAD_2_MR_ANGLE( pAngle ) ((MR_Angle)( (static_cast<unsigned int>(pAngle*static_cast<double>(MR_2PI)*0.5/3.1415926536)+MR_2PI)%static_cast<unsigned int>(MR_2PI) ))

// Temporal units
using MR_SimulationTime = MR_Int32;  // In 1/1000th of seconds
// Do not stay in a maze for more than 49 days
// or the time will wrap-up

// Sin and Cos tables
MR_DllDeclare extern MR_Int16 MR_Sin[MR_2PI];
MR_DllDeclare extern MR_Int16 MR_Cos[MR_2PI];

#define MR_TRIGO_FRACT   16384					  // unity of the trigo tables
#define MR_TRIGO_SHIFT      14

// Function prototypes
MR_DllDeclare void MR_InitTrigoTables();

#undef MR_DllDeclare
#endif
