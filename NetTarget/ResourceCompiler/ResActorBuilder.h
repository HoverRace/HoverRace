// ResActorBuilder.h
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

#ifndef MR_RES_ACTOR_BUILDER_H
#define MR_RES_ACTOR_BUILDER_H

#include "../ObjFacTools/ResourceLib.h"


class MR_ResActorBuilder: public MR_ResActor
{
   protected:

      Patch* ReadPatch( FILE* pFile, MR_ResourceLib* pBitmapLib );

   public:
      MR_ResActorBuilder( int pResourceId );

      BOOL BuildFromFile( const char* pFile, MR_ResourceLib* pBitmapLib );

};

#endif


