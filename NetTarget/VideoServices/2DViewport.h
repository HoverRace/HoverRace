// 2DViewport.h
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


#ifndef _2D_VIEWPORT_H
#define _2D_VIEWPORT_H

#include "VideoBuffer.h"


#ifdef MR_VIDEO_SERVICES
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif




class MR_2DViewPort
{   
   protected:

      enum { eNone    = 0,
             eBuffer  = 1,
             eXSize   = 2,
             eYSize   = 4,
             eXPitch  = 8, 
             eYPitch  = 16
           };


      // VideobufferRelativeStock
      MR_VideoBuffer* mVideoBuffer;
      MR_UInt8*       mBuffer;          // With computed offset
      int             mLineLen;

      int             mXPitch;
      int             mYPitch;


      // ViewportStock
      int mXRes;
      int mYRes;

   
      MR_DllDeclare virtual void OnMetricsChange( int pMetrics );

   public:

      MR_DllDeclare MR_2DViewPort();
      MR_DllDeclare ~MR_2DViewPort();

      MR_DllDeclare void Setup( MR_VideoBuffer* pBuffer, int pX0, int pY0, int pSizeX, int pSizeY, int pMetrics = eNone );
      MR_DllDeclare void Clear( MR_UInt8 pColor = 0 );

      MR_DllDeclare int GetXRes()const;
      MR_DllDeclare int GetYRes()const;


      // 2DDrawing primitives, defined in VideoBuffer2DDraw.cpp
      MR_DllDeclare void DrawPoint( int pX, int pY, MR_UInt8 pColor );
      MR_DllDeclare void DrawLine( int pX0, int pY0, int pX1, int pY1, MR_UInt8 pColor );
      MR_DllDeclare void DrawHorizontalLine( int pY, int pX0, int pX1, MR_UInt8 pColor );

      // HighLevel 2D functionalities
      MR_DllDeclare void DrawHorizontalMeter( int pX0, int pXLen, int pY0, int pYHeight, int pXM, MR_UInt8 pColor, MR_UInt8 pBackColor );

      // Buffer access functions
      MR_UInt8*    GetBuffer();
      int          GetLineLen()const;


};



#undef MR_DllDeclare

#endif



