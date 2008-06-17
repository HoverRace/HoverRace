// FastFifo.h
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

#ifndef FAST_FIFO_H
#define FAST_FIFO_H

#ifdef MR_UTIL
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif

template <class pType> class MR_FastFifoBase
{
   
   protected:
      int    mHead;
      int    mNbItem;
      int    mArraySize;
      pType* mArray;
    
   public:

      MR_FastFifoBase<pType>( int pSize, pType* pData ) { mNbItem = 0; mHead = 0; mArraySize = pSize; mArray = pData; };
     
      void   Add( pType pData )         { mArray[ (mHead+mNbItem)%mArraySize] = pData; if( mNbItem==mArraySize){mHead = (mHead+1)%mArraySize;}else{mNbItem++;} };
      void   Remove( int pCount=1 )     { if( pCount>mNbItem ){pCount=mNbItem;} mNbItem -= pCount; mHead = (mHead+pCount)%mArraySize; };
      pType  GetHead()                  { return mArray[ mHead ]; };
      void   Clean()                    { mNbItem = 0; }
      int    Used()const                { return mNbItem; }
      int    TotalSize()const           { return mArraySize; }
      int    Full()const                { return mNbItem==mArraySize; }
      BOOL   CanAdd( int pCount=1 )const{ return mNbItem+pCount<=mArraySize; }
      BOOL   IsEmpty()const             { return( mNbItem==0 ); }

      const pType operator[]( int pIndex )const { return mArray[ (mHead+pIndex)%mArraySize ]; };

      // Special functions to add at the head and remove at the tail
      void   AddHead( pType pData )    { mHead = (mHead+mArraySize-1)%mArraySize; mArray[ mHead ]=pData; if( mNbItem!=mArraySize){mNbItem++;} };
      void   RemoveTail( int pCount=1 ){ if( pCount>mNbItem ){pCount=mNbItem;} mNbItem -= pCount; };
      
};


template <class pType> class MR_FastFifo:public MR_FastFifoBase< pType >
{
   public:
      MR_FastFifo<pType>( int pSize ):MR_FastFifoBase<pType>( pSize, new pType[ pSize ] ){};
      ~MR_FastFifo<pType>()                                                              { delete []mArray; };
};


template <class pType, int pSize> class MR_FixedFastFifo:public MR_FastFifoBase< pType >
{
   protected:
      pType mData[ pSize ];

   public:
      MR_FixedFastFifo< pType, pSize>():MR_FastFifoBase<pType>( pSize, mData ){};

};

#undef MR_DllDeclare

#endif

