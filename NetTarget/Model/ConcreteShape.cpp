// ConcreteShape.cpp
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

#include "ConcreteShape.h"


// MR_Cylinder

MR_2DCoordinate mAxis;
MR_Int32 mRayLen;
MR_Int32 mZMin;
MR_Int32 mZMax;



MR_Int32 MR_Cylinder::ZMin() const const
{
    return mZMin;
}

MR_Int32 MR_Cylinder::ZMax() const const
{
    return mZMax;
}

MR_Int32 MR_Cylinder::AxisX() const const
{
    return mAxis.mX;
}

MR_Int32 MR_Cylinder::AxisY() const const
{
    return mAxis.mY;
}

MR_Int32 MR_Cylinder::RayLen() const const
{
    return mRayLen;
}
