// ResSoundBuilder.cpp
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

#include "stdafx.h"
#include "ResSoundBuilder.h"
#include "BitmapHelper.h"

#include <vfw.h>
#include <mmreg.h>


// Local prototypes
BOOL ParseFile( const char* pFile, const char*& pData, int& pDataLen );
BOOL DSParseWaveResource(void *pvRes, WAVEFORMATEX **ppWaveHeader, BYTE **ppbWaveData,DWORD *pcbWaveSize);




MR_ResShortSoundBuilder::MR_ResShortSoundBuilder( int pResourceId )
                        :MR_ResShortSound( pResourceId )
{

}

BOOL MR_ResShortSoundBuilder::BuildFromFile( const char* pFile, int pNbCopy )
{
   mNbCopy = pNbCopy;
   return ParseFile( pFile, mData, mDataLen );
}


MR_ResContinuousSoundBuilder::MR_ResContinuousSoundBuilder( int pResourceId )
                             :MR_ResContinuousSound( pResourceId )
{

}


BOOL MR_ResContinuousSoundBuilder::BuildFromFile( const char* pFile, int pNbCopy )
{
   mNbCopy = pNbCopy;
   return ParseFile( pFile, mData, mDataLen );
}


BOOL ParseFile( const char* pFile, const char*& pData, int& pDataLen )
{
   static char lFileBuffer[ 1000000 ];

   BOOL  lReturnValue = TRUE;
   FILE* lFile        = fopen( pFile, "rb" );


   pData = NULL;
   pDataLen = 0;

   if( lFile == NULL )
   {
      lReturnValue = FALSE;
   }
   else
   {
      if( fread( lFileBuffer, 1, sizeof( lFileBuffer ), lFile ) > 20 )
      {
         WAVEFORMATEX* lWaveFormat;
         BYTE*         lWaveData;
         DWORD         lWaveSize;

         lReturnValue = DSParseWaveResource( (void *)lFileBuffer, &lWaveFormat, &lWaveData, &lWaveSize );

         if( lReturnValue )
         {
            pDataLen = sizeof( MR_UInt32 )+sizeof( WAVEFORMATEX )+lWaveSize;
            pData    = new char[ pDataLen ];

            *(MR_UInt32*)(pData+0) = lWaveSize;
            *(WAVEFORMATEX*)(pData+sizeof(MR_UInt32)) = *lWaveFormat;
            memcpy( (void*)(pData+sizeof( MR_UInt32 ) + sizeof( WAVEFORMATEX )), lWaveData, lWaveSize );
         }
      }
      else
      {
         lReturnValue = FALSE;
      }
      fclose( lFile );
   }

   return lReturnValue;
}


// Ugly code Copied from a microsoft example
// Hoping that it works (remember.. if it works.. don't touch it)
BOOL DSParseWaveResource(void *pvRes, WAVEFORMATEX **ppWaveHeader, BYTE **ppbWaveData,DWORD *pcbWaveSize)
{
    DWORD *pdw;
    DWORD *pdwEnd;
    DWORD dwRiff;
    DWORD dwType;
    DWORD dwLength;

    if (ppWaveHeader)
        *ppWaveHeader = NULL;

    if (ppbWaveData)
        *ppbWaveData = NULL;

    if (pcbWaveSize)
        *pcbWaveSize = 0;

    pdw = (DWORD *)pvRes;
    dwRiff = *pdw++;
    dwLength = *pdw++;
    dwType = *pdw++;

    if (dwRiff != mmioFOURCC('R', 'I', 'F', 'F'))
        goto exit;      // not even RIFF

    if (dwType != mmioFOURCC('W', 'A', 'V', 'E'))
        goto exit;      // not a WAV

    pdwEnd = (DWORD *)((BYTE *)pdw + dwLength-4);

    while (pdw < pdwEnd)
    {
        dwType = *pdw++;
        dwLength = *pdw++;

        switch (dwType)
        {
        case mmioFOURCC('f', 'm', 't', ' '):
            if (ppWaveHeader && !*ppWaveHeader)
            {
                if (dwLength < sizeof(WAVEFORMAT))
                    goto exit;      // not a WAV

                *ppWaveHeader = (WAVEFORMATEX *)pdw;

                if ((!ppbWaveData || *ppbWaveData) &&
                    (!pcbWaveSize || *pcbWaveSize))
                {
                    return TRUE;
                }
            }
            break;

        case mmioFOURCC('d', 'a', 't', 'a'):
            if ((ppbWaveData && !*ppbWaveData) ||
                (pcbWaveSize && !*pcbWaveSize))
            {
                if (ppbWaveData)
                    *ppbWaveData = (LPBYTE)pdw;

                if (pcbWaveSize)
                    *pcbWaveSize = dwLength;

                if (!ppWaveHeader || *ppWaveHeader)
                    return TRUE;
            }
            break;
        }

        pdw = (DWORD *)((BYTE *)pdw + ((dwLength+1)&~1));
    }

exit:
    return FALSE;
}

