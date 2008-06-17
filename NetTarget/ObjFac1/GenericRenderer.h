// GenericRenderer.h
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
// This element is used to test and build new FreeElements
// 


#ifndef GENERIC_RENDERER_H
#define GENERIC_RENDERER_H

#include "../ObjFacTools/FreeElementBaseRenderer.h"


class MR_GenericRenderer: public MR_FreeElementBaseRenderer
{

   public:
      MR_GenericRenderer( const MR_ObjectFromFactoryId& pId, int pActorRes );
      ~MR_GenericRenderer();  
      
};

#endif

