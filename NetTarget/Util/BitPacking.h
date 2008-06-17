// BitPacking.h


#ifndef BIT_PACKING_H
#define BIT_PACKING_H


#ifdef MR_UTIL
   #define MR_DllDeclare   __declspec( dllexport )
#else
   #define MR_DllDeclare   __declspec( dllimport )
#endif

#include "MR_Types.h"

class MR_BitPack
{

   protected:
      MR_UInt8 mData[1];

   public:
      void Clear( int pSize );
      void MR_BitPack::Set( int pOffset, int pLen, int pPrecision, MR_Int32 pValue );

      MR_Int32  Get( int pOffset, int pLen, int pPrecision )const;
      MR_UInt32 Getu( int pOffset, int pLen, int pPrecision )const;
};

// Inlined because good performances are needed
inline MR_Int32 MR_BitPack::Get( int pOffset, int pLen, int pPrecision )const
{
   union{ MR_Int32 s; MR_UInt32 u; } lReturnValue;
   
   lReturnValue.u = (*(MR_UInt32*)(mData+pOffset/8))>>(pOffset%8);

   if( (pLen+(pOffset%8))>32 )
   {
      lReturnValue.u |= (*(MR_UInt32*)(mData+(pOffset/8)+4))<<(32-(pOffset%8));
   }

   // Clear overhead bits and preserve sign
   lReturnValue.s = lReturnValue.s<<(32-pLen)>>(32-pLen);

   return lReturnValue.s<<pPrecision;
}
   

inline MR_UInt32 MR_BitPack::Getu( int pOffset, int pLen, int pPrecision )const
{
   MR_UInt32 lReturnValue;
   
   lReturnValue = (*(MR_UInt32*)(mData+pOffset/8))>>(pOffset%8);

   if( pLen+(pOffset%8)>32 )
   {
      lReturnValue |= (*(MR_UInt32*)(mData+(pOffset/8)+4))<<(32-(pOffset%8));
   }

   // Clear overhead bits
   lReturnValue = lReturnValue<<(32-pLen)>>(32-pLen);

   return lReturnValue<<pPrecision;
}

inline void MR_BitPack::Clear( int pSize )
{
   memset( mData, 0, pSize );
}

inline void MR_BitPack::Set( int pOffset, int pLen, int pPrecision, MR_Int32 pValue )
{

   MR_UInt32 lValue = pValue>>pPrecision;

   lValue = lValue<<(32-pLen)>>(32-pLen);

   (*(MR_UInt32*)(mData+pOffset/8)) |= lValue<<(pOffset%8);

   if( (pLen+(pOffset%8))>32 )
   {
      (*(MR_UInt32*)(mData+(pOffset/8)+4)) |= lValue>>(32-(pOffset%8));
   }
}



#undef MR_DllDeclare

#endif

