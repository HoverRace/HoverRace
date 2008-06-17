// RaceEffects.h
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



#ifndef RACE_EFFECTS_H
#define RACE_EFFECTS_H

#include "ContactEffect.h"

#ifdef MR_MODEL
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif



class MR_SpeedDoubler: public MR_ContactEffect
{
   public:
      // No parameter
};

class MR_FuelGain: public MR_ContactEffect
{
   public:
      double mFuelQty; // in seconds per millisecond at the pump
};


class MR_LostOfControl: public MR_ContactEffect
{
   public:
      enum mType { eMissile, eMine };
      int mType;
      int mElementId; // Used for mines only
      int mHoverId;   // Craft that created the effect
};


class MR_CheckPoint: public MR_ContactEffect
{
   public:
      enum CheckPointType   { eFinishLine, eCheck1, eCheck2 };
      CheckPointType mType;
};

class MR_PowerUpEffect: public MR_ContactEffect
{
   public:
      int mElementPermId;
};




#undef MR_DllDeclare
      
#endif
