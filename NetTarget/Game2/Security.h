// Security.h
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
// Set of macros used for security purpose
//

#ifndef _SECURITY_H
#define _SECURITY_H

#define PRODUCT_ID    1
#define HOVERRACE_ID  1

struct KeyStructure
{
   unsigned char      mKeySumK1;
   unsigned char      mKeySumK2;
   unsigned short int mProduct;

   unsigned int       mTimeStamp;   // May be usefull, also a good random addition
   
   unsigned short int mNameSum;
   unsigned short int mIDSum;
   
   unsigned int       mKeySumHard;   

   unsigned short int mKeySumHard2; // To be used only if first key is cracked
   unsigned short int mKeySumHard3; // To be used only if first key is cracked

};




// Total key size 20bytes , 40 hex char



static unsigned short TextToInt( const char* pText )
{
   unsigned short lReturnValue = 0;

   lReturnValue += (toupper( pText[0] )-'A')*(26*10*10);
   lReturnValue += (toupper( pText[1] )-'A')*(10*10);
   lReturnValue += (toupper( pText[2] )-'0')*10;
   lReturnValue += (toupper( pText[3] )-'0');

   return lReturnValue;
}

static void IntToText( unsigned short pValue, unsigned char* pDest )
{
   pDest[3]  = '0'+(pValue%10);
   pValue   /= 10;
   pDest[2]  = '0'+(pValue%10);
   pValue   /= 10;
   pDest[1]  = 'A'+(pValue%26);
   pValue   /= 26;
   pDest[0]  = 'A'+(pValue%26);
   pDest[4] = 0;
}

static void PasswordApply( const char* pPassword, KeyStructure* pStructure )
{
   int lCounter = 0;
   unsigned lPassword = 0;

   while( *pPassword != 0 )
   {
      lPassword += ((unsigned int)*pPassword)<<((lCounter*5)%28);
      pPassword++;
      lCounter++;
   }

   pStructure->mKeySumK1    ^= (unsigned char) (lPassword+343);
   pStructure->mKeySumK2    ^= (unsigned char) (lPassword+161);
   pStructure->mProduct     ^= (unsigned short)(lPassword+pStructure->mTimeStamp+112);
   pStructure->mNameSum     ^= (unsigned short)(lPassword+pStructure->mTimeStamp+834);
   pStructure->mIDSum       ^= (unsigned short)(lPassword+pStructure->mTimeStamp+534);
   pStructure->mKeySumHard  ^= lPassword+12127;
   pStructure->mKeySumHard2 ^= (unsigned short)(lPassword+22027);
   pStructure->mKeySumHard3 ^= (unsigned short)(lPassword-12027);
}

// static int lScrambleTable[]= { 5, 10, 2, 3,  9, 8, 6, 7, 11, 4, 0, 1,  13, 17, 15, 14, 16, 12, 18, 19 };
static int lScrambleTable[]= { 8,   1,  17, 5,
                               10, 18,  2,  7, 
                               0,   9,  12, 16,
                               14, 11,  3,  15, 
                               13,  4,  19, 6  };

static void Scramble( char* pDest, const char* pString )
{
  int lCounter;

  // Put string terminator
  pDest[20] = 0;

  for( lCounter = 0; lCounter < 20; lCounter++ )
  {
     pDest[ lCounter ] = pString[ lScrambleTable[lCounter] ];
  }
}

static void UnScramble( char* pDest, const char* pString )
{
  int lCounter;

  // Put string terminator
  pDest[20] = 0;

  for( lCounter = 0; lCounter < 20; lCounter++ )
  {
     pDest[ lScrambleTable[lCounter] ] = pString[ lCounter ];
  }
}

static void SetFromKeys( const char* pKey1, const char* pKey2, KeyStructure* pStructure )
{
   // Keys must be 20 char long before calling this function

   unsigned char lKey[41];

   UnScramble( (char*)lKey,    pKey1 );   
   UnScramble( (char*)lKey+20, pKey2 );   

   unsigned short* lKeyStruct = (unsigned short*)pStructure;

   for( int lCounter = 0; lCounter < sizeof( KeyStructure)/2; lCounter++ )
   {
      lKeyStruct[ lCounter ] = TextToInt( (const char*)lKey+lCounter*4 );
   }   
}

static void GetToKeys( char* pKey1, char* pKey2, KeyStructure* pStructure )
{
   unsigned char lKey[41];

   unsigned short* lKeyStruct = (unsigned short*)pStructure;

   for( int lCounter = 0; lCounter < sizeof( KeyStructure)/2; lCounter++ )
   {
      IntToText( lKeyStruct[ lCounter ], lKey+lCounter*4 );
   }   

   Scramble( pKey1, (const char*)lKey );   
   Scramble( pKey2, (const char*)lKey+20 );   
}

static unsigned char GetKey1Sum( KeyStructure* pStructure )
{
   unsigned short lSum = 10;

   unsigned char* lKey = (unsigned char*)pStructure;

   for( int lCounter = 1; lCounter < 10; lCounter++ )
   {
      lSum += lKey[lCounter];
   }
   return lSum;
}

static unsigned char GetKey2Sum( KeyStructure* pStructure )
{
   unsigned short lSum = 20;

   unsigned char* lKey = (unsigned char*)pStructure;

   for( int lCounter = 10; lCounter < 20; lCounter++ )
   {
      lSum += lKey[lCounter];
   }
   return lSum;
}


static const char* NormalizeUser( const char* pName )
{

   static char lReturnValue[41];
   int lIndex = 0;

   while( (*pName != 0)&&(lIndex<40 ))
   {
      char lChar = toupper( *pName );

      if( (lChar >= 'A')&&(lChar<='Z' ) )
      {
         lReturnValue[ lIndex++ ] = lChar;
      }
      pName++;
   }
   lReturnValue[ lIndex ] = 0;

   return lReturnValue;
}

static unsigned short ComputeUserNameSum( const char* pNormName )
{
   unsigned int lReturnValue = 0;

   while( *pNormName != 0 )
   {
      lReturnValue += *pNormName;
      lReturnValue <<= 1;
      lReturnValue += *pNormName;
      lReturnValue <<= 2;
      lReturnValue ^= lReturnValue >> 16;
      lReturnValue &= 0xFFFF;

      pNormName++;
   }   
   return (unsigned short)lReturnValue;
}

static unsigned short ComputeIDSum( int pMajorID, int pMinorID )
{
   unsigned int lReturnValue = 30;

   lReturnValue += pMajorID+(~pMinorID)+(pMajorID<<5)+(pMinorID<<3)+pMinorID*3;

   lReturnValue ^= lReturnValue >> 16;
   lReturnValue &= 0xFFFF;

   return (unsigned short)lReturnValue;
}



typedef void (*funcptr)();

// This one make the application to crash if there is an error in the key
// violent..but cool :)
#define HardVerification( pFunctionMinus112, pKeyStruct )\
{\
   unsigned int lOffset = 648;\
\
   unsigned char* lKey = (unsigned char*)pKeyStruct;\
\
   for( int lCounter = 2; lCounter<12; lCounter++ )\
   {\
      lOffset += lKey[lCounter] + lKey[lCounter]*7;\
      unsigned int lOverflow = lOffset >> 29;\
      lOffset <<= 3;\
      lOffset += lOverflow;\
   }\
   lOffset -= (pKeyStruct)->mKeySumHard;\
\
   TRACE( "Offset %d\n", lOffset ); \
   (((funcptr)(((unsigned int)(const char*)pFunctionMinus112)+lOffset)))(); \
\
}

#ifdef _ENCRYPT_STUFF

static unsigned int ComputeHardKeySum( KeyStructure* pKeyStruct )
{
}

static unsigned short ComputeHardKeySum2( KeyStructure* pKeyStruct )
{
}

static unsigned short ComputeHardKeySum3( KeyStructure* pKeyStruct, const char* pNormName, int pMinorID )
{
}


#endif

#endif
