// ResSoundBuilder.h
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



#ifndef MR_RES_SOUND_BUILDER_H
#define MR_RES_SOUND_BUILDER_H

#include "../ObjFacTools/ResSound.h"



class MR_ResShortSoundBuilder: public MR_ResShortSound
{
   public:
      MR_ResShortSoundBuilder( int pResourceId );

      BOOL BuildFromFile( const char* pFile, int pNbCopy );

};

class MR_ResContinuousSoundBuilder: public MR_ResContinuousSound
{
   public:
      MR_ResContinuousSoundBuilder( int pResourceId );

      BOOL BuildFromFile( const char* pFile, int pNbCopy );

};


#endif



