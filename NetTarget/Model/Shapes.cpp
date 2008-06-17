// Shapes.cpp
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


#include "Shapes.h"



MR_Int32 MR_CylinderShape::XPos()const
{
   return AxisX();
}

MR_Int32 MR_CylinderShape::YPos()const
{
   return AxisY();
}

MR_Int32 MR_CylinderShape::XMin()const
{
   return AxisX()-RayLen();
}

MR_Int32 MR_CylinderShape::XMax()const
{
   return AxisX()+RayLen();
}

MR_Int32 MR_CylinderShape::YMin()const
{
   return AxisY()-RayLen();
}

MR_Int32 MR_CylinderShape::YMax()const
{
   return AxisY()+RayLen();
}


MR_ShapeInterface::eShape MR_CylinderShape::ShapeType()const
{
   return eCylinder;
}

MR_Int32 MR_LineSegmentShape::XPos()const
{
   return X0();
}

MR_Int32 MR_LineSegmentShape::YPos()const
{
   return Y0();
}

MR_Int32 MR_LineSegmentShape::XMin()const
{
   int lX0 = X0();
   int lX1 = X1();

   return( (lX0<lX1) ? lX0 : lX1 );
}

MR_Int32 MR_LineSegmentShape::XMax()const
{
   int lX0 = X0();
   int lX1 = X1();

   return( (lX0>lX1) ? lX0 : lX1 );
}

MR_Int32 MR_LineSegmentShape::YMin()const
{
   int lY0 = Y0();
   int lY1 = Y1();

   return( (lY0<lY1) ? lY0 : lY1 );
}

MR_Int32 MR_LineSegmentShape::YMax()const
{
   int lY0 = Y0();
   int lY1 = Y1();

   return( (lY0>lY1) ? lY0 : lY1 );
}

MR_ShapeInterface::eShape MR_LineSegmentShape::ShapeType()const
{
   return eLineSegment;
}


MR_Int32 MR_PolygonShape::XPos()const
{
   return (XMin()+XMax())/2;
}

MR_Int32 MR_PolygonShape::YPos()const
{
   return (YMin()+YMax())/2;
}

MR_ShapeInterface::eShape MR_PolygonShape::ShapeType()const
{
   return ePolygon;
}





// BOOL MR_GetContact( const MR_ShapeInterface&  /*pShape0*/, 
//                    const MR_ShapeInterface&  /*pShape1*/,
//                    MR_ShapeContact&          /*pContact*/ )
//{
//   // TODO
//
//   return FALSE;
//}


/*
void MR_GetPolygonInclusion( const MR_PolygonShape&     pPolygon,
                             const MR_ShapeInterface&   pShape,
                             MR_PolygonInclusion&       pAnswer )
{

   pAnswer.mInclusion      = MR_PolygonInclusion::eInside;
   pAnswer.mNbIntersection = 0;

   const MR_BoundingBoxShape* lBoundingBox = dynamic_cast<const MR_BoundingBoxShape*>( &pShape );

   if( lBoundingBox != NULL )
   {
      // Bounding box optimization
      // This is not only an optimization, it also
      // discard far objects that can cause overflow during calculation

   }

   if( pAnswer.mInclusion != MR_PolygonInclusion::eOutside )
   {
      // Warning, to be revied because this technique do not take in account
      // that that pShap is derived form one of the basic abstract shapes

      if( typeid( pShape ) == typeid( MR_CylinderShape ) )
      {

      }
      else if( typeid( pShape ) == typeid( MR_PointShape ) )
      {

      }
      else if( typeid( pShape ) == typeid( MR_WallShape ) )
      {

      }
      else if( typeid( pShape ) == typeid( MR_LineSegmentShape ) )
      {

      }
      else if( typeid( pShape ) == typeid( MR_PolygonShape ) )
      {

      }
      else if( typeid( pShape ) == typeid( MR_AssemblyShape ) )
      {

      }
      else if( typeid( pShape ) == typeid( MR_BoundingBoxShape ) )
      {
         // Already calculated during optimization
      }
      else
      {
         ASSERT( FALSE );
      }
   }
}
*/

/*
void MR_GetPolygonInclusion( const MR_PolygonShape&     pPolygon,
                             const MR_PointShape&       pShape,
                             MR_PolygonInclusion&       pAnswer )
{
   MR_2DCoordinate lPosition;

   lPosition.mX = pShape.X();
   lPosition.mY = pShape.Y();

   pAnswer.mInclusion  = MR_GetPolygonInclusion( pPolygon, lPosition )
                         ? MR_PolygonInclusion::eInside
                         : MR_PolygonInclusion::eOutside;

   pAnswer.mNbIntersection = 0;
}
*/
 
