// Banner.h
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


#ifndef BANNER_H
#define BANNER_H

#define MR_MAX_IMAGES  32

class MR_GifDecoder
{
   private:
      int mNbImages;
      // int mXRes;
      // int mYRes;

      HPALETTE mGlobalPalette;
      HBITMAP  mBitmap[MR_MAX_IMAGES];
      int      mDelay[MR_MAX_IMAGES];

      void Clean();

   public:
      MR_GifDecoder();
      ~MR_GifDecoder();

      BOOL Decode( const unsigned char* pGifStream, int pStreamLen );

      HPALETTE GetGlobalPalette()const;
      int      GetImageCount()const;
      HBITMAP  GetImage(   int pImage )const;
      int      GetDelay(   int pImage )const;

};


BOOL LoadURL( HWND pWindow, const char* pURL );
BOOL LoadURLShortcut( HWND pWindow, const char* pShortcut );


#endif