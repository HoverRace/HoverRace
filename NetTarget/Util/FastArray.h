// FastArray.h
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

#ifndef FAST_ARRAY_H
#define FAST_ARRAY_H

#ifdef MR_UTIL
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif

template <class pType> class MR_FastArrayBase
{
   protected:
      int    mNbItem;
      int    mArraySize;
      pType* mArray;
    
   public:
      MR_FastArrayBase<pType>( int pSize, pType* pData ) { mNbItem = 0; mArraySize = pSize; mArray = pData; };

      void   Add( pType pData )    { ASSERT( mNbItem<mArraySize ); mArray[ mNbItem++ ]=pData; };
      void   Use( int pCount = 1 ) { mNbItem+=pCount; ASSERT( mNbItem <= mArraySize ); };
      void   Clean()               { mNbItem = 0; }
      int    Used()const           { return mNbItem; }
      int    TotalSize()const      { return mArraySize; }
      int    Full()const           { return mNbItem==mArraySize; }
      BOOL   CanAdd( int pCount=1 ){ return mNbItem+pCount<=mArraySize; }

      BOOL   Contains( pType pData )const { for( int lCounter = 0; lCounter<mNbItem; lCounter++ ){ if( mArray[ lCounter ]==pData )return TRUE; }return FALSE;};

      const pType  operator[]( int pIndex )const { return mArray[ pIndex ]; }
      const pType& operator[]( int pIndex )      { return mArray[ pIndex ]; }

};

template <class pType> class MR_FastArray : public MR_FastArrayBase<class pType>
{
   public:
	  MR_FastArray(int pSize)
	  {
	  
	  };
      ~MR_FastArray() { 
		delete []mArray; 
	  };
};


template <class pType, int pSize> class MR_FixedFastArray:public MR_FastArrayBase< pType >
{
   protected:
      pType mData[ pSize ];

   public:
      MR_FixedFastArray< pType, pSize>():MR_FastArrayBase<pType>( pSize, mData ){};

};

#undef MR_DllDeclare

#endif

