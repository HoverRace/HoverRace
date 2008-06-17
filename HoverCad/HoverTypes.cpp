// HoverTypes.cpp
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

#include "stdafx.h"

#include "resource.h"
#include "HoverTypes.h"


CHoverType WallTextureList[] = 
{
   { 0, 1, 50, IDS_NONE,"[none]" },
   { 19, 1, 72,IDS_GRAY_WALL, "Gray Wall" },
   { 20, 1, 73,IDS_WALL_TOP, "Wall Top" },
   { 1, 1, 52, IDS_RIGHT_GRAY_ARROW, "Right Gray Arrow" },
   { 2, 1, 53, IDS_LEFT_GRAY_ARROW, "Left Gray Arrow" },
   { 3, 1, 54, IDS_RIGHT_RED_ARROW, "Right Red Arrow" },
   { 4, 1, 55, IDS_LEFT_RED_ARROW, "Left Red Arrow" },
   { 5, 1, 56, IDS_RIGHT_GREEN_ARROW, "Right Green Arrow" },
   { 6, 1, 57, IDS_LEFT_GREEN_ARROW, "Left Green Arrow" },
   { 21, 1, 70,IDS_RIGHT_YELLOW_NEON,  "Right Yellow Neon" },
   { 22, 1, 71,IDS_LEFT_YELLOW_NEON,  "Left Yellow Neon" },
   { 7, 1, 58, IDS_STEP_SIGN, "Step sign" },
   { 8, 1, 59, IDS_PASS_RIGHT, "Pass Right" },
   { 9, 1, 60, IDS_PASS_LEFT, "Pass Left" },
   { 10, 1, 61,IDS_DONOT_ENTER_S,  "Do Not Enter (Small)" },
   { 11, 1, 62,IDS_DONOT_ENTER_L, "Do Not Enter (Large)" },
   { 12, 1, 63,IDS_WATER, "Water" },
   { 13, 1, 64,IDS_SPEED_ZONE, "Speed Zone" },
   { 14, 1, 65,IDS_FUEL_ZONE, "Fuel Zone" },
   { 15, 1, 66,IDS_YELLOW, "Yellow" },
   { 16, 1, 67,IDS_CHECKERS, "Checkers" },
   { 17, 1, 68,IDS_PIT_LABEL, "Pit Label" },
   { 18, 1, 69,IDS_FINISH_LABEL, "Finish Label" }

   // Last used index is 22
};

int        WallTextureListSize = sizeof(WallTextureList)/sizeof(WallTextureList[0]);


CHoverType FloorTextureList[] =
{
//   { 1, 50, "[none]" },
   { 0, 1, 51, IDS_METAL_PLATE, "MetalPlate" },
//   { 1, 52, "Right Gray Arrow" },
//   { 1, 53, "Left Gray Arrow" },
//   { 1, 54, "Right Red Arrow" },
//   { 1, 55, "Left Red Arrow" },
//   { 1, 56, "Right Green Arrow" },
//   { 1, 57, "Left Green Arrow" },
   { 1, 1, 58, IDS_STEP_SIGN, "Step sign" },
//   { 1, 59, "Pass Right" },
//   { 1, 60, "Pass Left" },
//   { 1, 61, "Do Not Enter (Small)" },
//   { 1, 62, "Do Not Enter (Large)" },
   { 2, 1, 63, IDS_WATER, "Water" },
   { 3, 1, 64, IDS_SPEED_ZONE, "Speed Zone" },
   { 4, 1, 65, IDS_FUEL_ZONE, "Fuel Zone" },
   { 5, 1, 66, IDS_YELLOW, "Yellow" },
   { 6, 1, 67, IDS_CHECKERS, "Checkers" }
//   { 1, 68, "Pit Label" },
//   { 1, 69, "Finish Label" }

   // Last used index is 6
};

int        FloorTextureListSize = sizeof(FloorTextureList)/sizeof(FloorTextureList[0]);

CHoverType CeilingTextureList[] = 
{
   { 0, 1, 50, IDS_NONE, "[none]" },
   { 1, 1, 51, IDS_METAL_PLATE, "MetalPlate" },
//   { 1, 52, "Right Gray Arrow" },
//   { 1, 53, "Left Gray Arrow" },
//   { 1, 54, "Right Red Arrow" },
//   { 1, 55, "Left Red Arrow" },
//   { 1, 56, "Right Green Arrow" },
//   { 1, 57, "Left Green Arrow" },
   { 2, 1, 58, IDS_STEP_SIGN, "Step sign" },
//   { 1, 59, "Pass Right" },
//   { 1, 60, "Pass Left" },
//   { 1, 61, "Do Not Enter (Small)" },
//   { 1, 62, "Do Not Enter (Large)" },
   { 3, 1, 63, IDS_WATER, "Water" },
   { 4, 1, 64, IDS_SPEED_ZONE, "Speed Zone" },
   { 5, 1, 65, IDS_FUEL_ZONE, "Fuel Zone" },
   { 6, 1, 66, IDS_YELLOW, "Yellow" },
   { 7, 1, 67, IDS_CHECKERS, "Checkers" }
//   { 1, 68, "Pit Label" },
//   { 1, 69, "Finish Label" }

   // Last used index is 7
};

int        CeilingTextureListSize = sizeof(CeilingTextureList)/sizeof(CeilingTextureList[0]);

CHoverType ObjectList[] = 
{
   { 0, 0, 1, IDS_STARTPOS_1, "StartingPosition 1" },
   { 1, 0, 2, IDS_STARTPOS_2, "StartingPosition 2" },
   { 2, 0, 3, IDS_STARTPOS_3, "StartingPosition 3" },
   { 3, 0, 4, IDS_STARTPOS_4, "StartingPosition 4" },
   { 4, 0, 5, IDS_STARTPOS_5, "StartingPosition 5" },
   { 5, 0, 6, IDS_STARTPOS_6, "StartingPosition 6" },
   { 6, 0, 7, IDS_STARTPOS_7, "StartingPosition 7" },
   { 7, 0, 8, IDS_STARTPOS_8, "StartingPosition 8" },
   { 8, 0, 9, IDS_STARTPOS_9, "StartingPosition 9" },
   { 9, 0, 10, IDS_STARTPOS_10, "StartingPosition 10" },
//   { 0, 11, "StartingPosition 11" },
//   { 0, 12, "StartingPosition 12" },
//   { 0, 13, "StartingPosition 13" },
//   { 0, 14, "StartingPosition 14" },
//   { 0, 15, "StartingPosition 15" },
//   { 0, 16, "StartingPosition 16" },

   { 12, 1, 202, IDS_FINISH_LINE, "Finish Line" },
   { 16, 1, 203, IDS_CHKPNT_1, "CheckPoint 1" },
   { 17, 1, 204, IDS_CHKPNT_2, "CheckPoint 2" },
   { 10, 1, 200, IDS_FUEL_SOURCE, "Fuel Source" },
   { 11, 1, 201, IDS_SPEED_RAMP, "Speed Ramp" },
   { 13, 1, 170, IDS_INFL_COLUMN, "Inflating column" },
   { 14, 1, 151, IDS_MINE, "Mine" },
   { 15, 1, 152, IDS_BOOSTER, "Booster" }

   // LAst used index is 17
};

int        ObjectListSize = sizeof(ObjectList)/sizeof(ObjectList[0]);


int MR_GetTypeIndex( int pId, CHoverType* pList, int pListSize )
{
   int lReturnValue = 0; // avoid errors

   for( int lCounter = 0; lCounter < pListSize; lCounter ++ )
   {
      if( pList[ lCounter ].mSerialId == pId )
      {
         lReturnValue = lCounter;
      }
   }
   return lReturnValue;
}
