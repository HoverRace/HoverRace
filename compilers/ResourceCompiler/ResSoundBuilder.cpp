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

#include <SDL2/SDL.h>

#include "ResSoundBuilder.h"
#include "BitmapHelper.h"

namespace HoverRace {
namespace ResourceCompiler {

bool ParseSoundFile(const char *filename, const char *&destData, MR_UInt32 &destLen);

ResShortSoundBuilder::ResShortSoundBuilder(int pResourceId) :
	SUPER(pResourceId)
{

}

bool ResShortSoundBuilder::BuildFromFile(const char *pFile, int pNbCopy)
{
	mNbCopy = pNbCopy;
	return ParseSoundFile(pFile, (const char *&) mData, mDataLen);
}

ResContinuousSoundBuilder::ResContinuousSoundBuilder(int pResourceId) :
	SUPER(pResourceId)
{

}

bool ResContinuousSoundBuilder::BuildFromFile(const char *pFile, int pNbCopy)
{
	mNbCopy = pNbCopy;
	return ParseSoundFile(pFile, (const char *&) mData, mDataLen);
}

bool ParseSoundFile(const char *filename, const char *&destData, MR_UInt32 &destLen)
{
	SDL_AudioSpec spec;
	MR_UInt8 *audioBuf;
	MR_UInt32 bufLen;
	if (SDL_LoadWAV(filename, &spec, &audioBuf, &bufLen) == NULL) {
		std::cerr << _("ERROR") << ": " << _("invalid WAV file") << ": " <<
			filename << ": " << SDL_GetError();
		return false;
	}

	destLen =
		4 +  // sizeof(MR_UInt32)
		18 +  // sizeof(WAVEFORMATEX)
		bufLen;
	destData = new char[destLen]();
	unsigned int bitsPerSample = spec.format & 0xff;

	*(MR_UInt32*)(destData + 0) = bufLen;

	const char *hdr = destData + sizeof(MR_UInt32);
	*(MR_UInt16*)(hdr + 0) = 1;  // wFormatTag = WAVE_FORMAT_PCM
	*(MR_UInt16*)(hdr + 2) = spec.channels;  // nChannels
	*(MR_UInt32*)(hdr + 4) = static_cast<MR_UInt32>(spec.freq);  // nSamplesPerSec
	*(MR_UInt32*)(hdr + 8) = static_cast<MR_UInt32>(spec.freq * spec.channels) * (bitsPerSample / 8);  // nAvgBytesPerSec
	*(MR_UInt16*)(hdr + 12) =  // nBlockAlign
		static_cast<MR_UInt16>(spec.channels * (bitsPerSample / 8));
	*(MR_UInt16*)(hdr + 14) =  // wBitsPerSample
		static_cast<MR_UInt16>(bitsPerSample);

	memcpy((void*)(destData + sizeof(MR_UInt32) + 18), audioBuf, bufLen);

	SDL_FreeWAV(audioBuf);
	return true;
}

}  // namespace ResourceCompiler
}  // namespace HoverRace
