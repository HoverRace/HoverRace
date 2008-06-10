// ObjFac1/main.cpp
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

#include "stdafx.h"

#include "DefaultSurface.h"
#include "WoodSurface.h"
#include "BallElement.h"
#include "BabeElement.h"
#include "TestElement.h"
#include "GenericRenderer.h"
#include "FuelSource.h"
#include "DoubleSpeedSource.h"
#include "FinishLine.h"
#include "HoverRender.h"
#include "Missile.h"
#include "BumperGate.h"
#include "PowerUp.h"
#include "Mine.h"
#include "ObjFac1Res.h"
#include "../ObjFacTools/ObjectFactoryData.h"
#include "../ObjFacTools/SpriteHandle.h"

#define new DEBUG_NEW

#define MR_DllDeclare   __declspec( dllexport )

// Functions declaration

extern "C"
{
   MR_DllDeclare void                  MR_InitModule( HMODULE pModule );
   MR_DllDeclare void                  MR_CleanModule();
   MR_DllDeclare MR_UInt16             MR_GetObjectTypeCount  ();
   MR_DllDeclare MR_UInt16             MR_GetObjectTypeCount  ();
   MR_DllDeclare CString               MR_GetObjectFamily     ( MR_UInt16 pClassId );
   MR_DllDeclare CString               MR_GetObjectDescription( MR_UInt16 pClassId );
   MR_DllDeclare MR_ObjectFromFactory* MR_GetObject           ( MR_UInt16 pClassId );
};

// Data declaration
MR_ObjectFactoryData* gObjectFactoryData = NULL;


// Functions implementations
void MR_InitModule( HMODULE pModule )
{
   delete gObjectFactoryData;
   gObjectFactoryData = new MR_ObjectFactoryData( pModule, "ObjFac1.dat" );
}

void MR_CleanModule()
{
   delete gObjectFactoryData;
   gObjectFactoryData = NULL;   
}


MR_UInt16 MR_GetObjectTypeCount()
{
   return 0;
}

CString MR_GetObjectFamily( MR_UInt16 /*pClassId*/ )
{
   return "n/a";
}

CString MR_GetObjectDescription( MR_UInt16 /*pClassId*/ )
{
   return "n/a";
}

MR_ObjectFromFactory* MR_GetObject( MR_UInt16 pClassId )
{
   MR_ObjectFromFactory*  lReturnValue = NULL;
   MR_ObjectFromFactoryId lId = { 1, pClassId };

   switch( pClassId )
   {
      case 1:
         lReturnValue = new MR_DefaultSurface( lId );
         break;

      case 2:
         lReturnValue = new MR_WoodSurface( lId );
         break;

      case 3:
         lReturnValue = new MR_BitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_FIRE ) );
         break;

      case 4:
         lReturnValue = new MR_BitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_BIG_BRICK ) );
         break;

      case 5:
         lReturnValue = new MR_BitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_RED_BRICK ) );
         break;

      case 10:
         lReturnValue = new MR_TestElement( lId, MR_DEMO_FIGHTER );
         break;

      case 11:
         lReturnValue = new MR_BabeElement( lId );
         break;

      case 12:
         lReturnValue = new MR_BallElement( lId );
         break;

      case 13:
         lReturnValue = new MR_TestElement( lId, MR_ELECTRO_CAR );
         break;

      case 50:
         lReturnValue = new MR_BitmapSurface( lId, NULL );
         break;

      case 51:
         lReturnValue = new MR_BitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_STD_FLOOR ) );
         break;

      case 52:
         lReturnValue = new MR_VStretchBitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_STD_RIGHT_WALL ), 4000 );
         break;

      case 53:
         lReturnValue = new MR_VStretchBitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_STD_LEFT_WALL ), 4000 );
         break;

      case 54:
         lReturnValue = new MR_VStretchBitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_RED_RIGHT_WALL_OFF ),gObjectFactoryData->mResourceLib.GetBitmap( MR_RED_RIGHT_WALL ),200, 4, 4000 );
         break;

      case 55:
         lReturnValue = new MR_VStretchBitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_RED_LEFT_WALL_OFF ), gObjectFactoryData->mResourceLib.GetBitmap( MR_RED_LEFT_WALL ), -200, 4, 4000 );
         break;

      case 56:
         lReturnValue = new MR_VStretchBitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_GREEN_RIGHT_WALL_OFF ),gObjectFactoryData->mResourceLib.GetBitmap( MR_GREEN_RIGHT_WALL ), 200, 4, 4000 );
         break;

      case 57:
         lReturnValue = new MR_VStretchBitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_GREEN_LEFT_WALL_OFF ), gObjectFactoryData->mResourceLib.GetBitmap( MR_GREEN_LEFT_WALL ), -200, 4, 4000 );
         break;

      case 58:
         lReturnValue = new MR_BitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_STEP_WALL ) );
         break;

      case 59:
         lReturnValue = new MR_BitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_PASS_RIGHT_WALL ) );
         break;

      case 60:
         lReturnValue = new MR_BitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_PASS_LEFT_WALL ) );
         break;

      case 61:
         lReturnValue = new MR_BitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_DO_NOT_ENTER_WALL1 ) );
         break;

      case 62:
         lReturnValue = new MR_BitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_DO_NOT_ENTER_WALL2 ) );
         break;

      case 63:
         lReturnValue = new MR_BitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_BLUE_BUBBLE_FLOOR ) );
         break;

      case 64:
         lReturnValue = new MR_BitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_SPEED_ZONE ) );
         break;

      case 65:
         lReturnValue = new MR_BitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_FUEL_ZONE ) );
         break;

      case 66:
         lReturnValue = new MR_BitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_YELLOW_STEP ) );
         break;

      case 67:
         lReturnValue = new MR_BitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_CHECKER ) );
         break;

      case 68:
         lReturnValue = new MR_BitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_PIT_WORD ) );
         break;

      case 69:
         lReturnValue = new MR_BitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_FINISH_WORD ) );
         break;

      case 70:
         lReturnValue = new MR_BitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_YELLOW_NEON ), gObjectFactoryData->mResourceLib.GetBitmap( MR_YELLOW_NEON_FLASH ), 50, 20 );
         break;

      case 71:
         lReturnValue = new MR_BitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_YELLOW_NEON ), gObjectFactoryData->mResourceLib.GetBitmap( MR_YELLOW_NEON_FLASH ), -50, 20 );
         break;

      case 72:
         lReturnValue = new MR_VStretchBitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_STD_WALL ), 4000 );
         break;

      case 73:
         lReturnValue = new MR_VStretchBitmapSurface( lId, gObjectFactoryData->mResourceLib.GetBitmap( MR_STD_WALL_TOP ), 6000 );
         break;

      case 100:
         lReturnValue = new MR_HoverRender( lId );
         break;
      
      case 150:
         lReturnValue = new MR_Missile( lId );
         break;

      case 151:
         lReturnValue = new MR_Mine( lId );
         break;

      case 152:
         lReturnValue = new MR_PowerUp( lId );
         break;


      case 170:
         lReturnValue = new MR_BumperGate( lId );
         break;

      case 200:
         lReturnValue = new MR_FuelSource( lId );
         break;

      case 201:
         lReturnValue = new MR_DoubleSpeedSource( lId );
         break;

      case 202:
         lReturnValue = new MR_FinishLine( lId, MR_CheckPoint::eFinishLine );
         break;

      case 203:
         lReturnValue = new MR_FinishLine( lId, MR_CheckPoint::eCheck1 );
         break;

      case 204:
         lReturnValue = new MR_FinishLine( lId, MR_CheckPoint::eCheck2 );
         break;

      case 1000:
         lReturnValue = new MR_SpriteHandle( lId, gObjectFactoryData->mResourceLib.GetSprite( MR_FONT1 ) );
         break;

      case 1100:
         lReturnValue = new MR_SpriteHandle( lId, gObjectFactoryData->mResourceLib.GetSprite( MR_MISSILE_STAT ) );
         break;

      case 1101:
         lReturnValue = new MR_SpriteHandle( lId, gObjectFactoryData->mResourceLib.GetSprite( MR_HOVER_ICONS ) );
         break;

      case 1102:
         lReturnValue = new MR_SpriteHandle( lId, gObjectFactoryData->mResourceLib.GetSprite( MR_MINE_STAT ) );
         break;

      case 1103:
         lReturnValue = new MR_SpriteHandle( lId, gObjectFactoryData->mResourceLib.GetSprite( MR_PWRUP_STAT ) );
         break;

   }

   return lReturnValue;
}
