
// SoundServer.cpp
// DirectSound / OpenAL encapsulation module
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

#include "StdAfx.h"

#include <math.h>

#include <AL/alut.h>

#include "../Util/MR_Types.h"
#include "../Util/Config.h"

#include "SoundServer.h"

using HoverRace::Util::Config;

namespace HoverRace {
namespace VideoServices {

#define MR_MAX_SOUND_COPY 6

static bool soundDisabled = false;
static ALenum initErrorCode = ALUT_ERROR_NO_ERROR;

#define DSBVOLUME_MIN -10000

/// Convert millibels to linear.
static float DirectXToLinear(int value)
{
	return (value == DSBVOLUME_MIN) ? 0.0f : 
		powf(10.0f, (float)value / 2000.0f);
} 

class SoundBuffer
{
	private:
		static SoundBuffer *mList;
		SoundBuffer *mNext;

	protected:

		int mNbCopy;
		ALuint mBuffer;
		ALuint mSoundBuffer[MR_MAX_SOUND_COPY];  // Actually the sources.

		int mNormalFreq;

	public:
		SoundBuffer();
		virtual ~ SoundBuffer();

		BOOL Init(const char *pData, int pNbCopy);

		void SetParams(int pCopy, int pDB, double pSpeed, int pPan);

		int GetNbCopy() const;

		static void DeleteAll();

		virtual void ApplyCumCommand();

		static void ApplyCumCommandForAll();

};

class ShortSound : public SoundBuffer
{

	protected:
		int mCurrentCopy;

	public:
		ShortSound();
		~ShortSound();

		void Play(int pDB, double pSpeed, int pPan);
};

class ContinuousSound : public SoundBuffer
{
	protected:
		BOOL mOn[MR_MAX_SOUND_COPY];
		int mMaxDB[MR_MAX_SOUND_COPY];
		double mMaxSpeed[MR_MAX_SOUND_COPY];

		void ResetCumStat();

	public:
		ContinuousSound();
		~ContinuousSound();

		void Pause(int pCopy);
		void Restart(int pCopy);

		void ApplyCumCommand();
		void CumPlay(int pCopy, int pDB, double pSpeed);

};

// Variables
SoundBuffer *SoundBuffer::mList = NULL;

static const char* const waveHeader =
	"RIFF----WAVE"
	"fmt \022\0\0\0" "----" "----" "----" "----\0\0"
	"data----";

// Implementation
SoundBuffer::SoundBuffer()
{
	mNbCopy = 0;
	mBuffer = 0;

	for(int lCounter = 0; lCounter < MR_MAX_SOUND_COPY; lCounter++) {
		mSoundBuffer[lCounter] = 0;
	}

	// Add the new buffer to the list
	mNext = mList;
	mList = this;

}

SoundBuffer::~SoundBuffer()
{
	// Remove form list
	if(mList == this) {
		mList = mNext;
		mNext = NULL;
	}
	else {
		SoundBuffer *mPrev = mList;

		while(mPrev->mNext != this) {
			ASSERT(mPrev != NULL);

			mPrev = mPrev->mNext;
		}

		mPrev->mNext = mNext;
		mNext = NULL;
	}

	// Delete the sound buffers
	alDeleteSources(mNbCopy, mSoundBuffer);
	alDeleteBuffers(1, &mBuffer);
}

void SoundBuffer::ApplyCumCommand()
{
	// Do notting by default
}

void SoundBuffer::ApplyCumCommandForAll()
{
	SoundBuffer *mCurrent = mList;

	while(mCurrent != NULL) {
		mCurrent->ApplyCumCommand();
		mCurrent = mCurrent->mNext;
	}
}

void SoundBuffer::DeleteAll()
{
	while(mList != NULL) {
		delete mList;
	}
}

/**
 * Fill the buffer with sound data.
 * @param pData Data buffer.  First 32 bits are the data length, followed by
 *              a WAVEFORMATEX describing the data, followed by the data itself.
 * @param pNbCopy The number of copies to make.
 * @return @c TRUE if successful.
 */
BOOL SoundBuffer::Init(const char *pData, int pNbCopy)
{
	if (soundDisabled) return TRUE;

	BOOL lReturnValue = TRUE;

	ASSERT(!mSoundBuffer[0]);			  // Already initialized

	if(pNbCopy > MR_MAX_SOUND_COPY) {
		ASSERT(FALSE);
		pNbCopy = MR_MAX_SOUND_COPY;
	}

	mNbCopy = pNbCopy;

	// Parse pData
	MR_UInt32 lBufferLen = *(MR_UInt32 *) pData;
	const char *lSoundData = pData + sizeof(MR_UInt32);
	
	// Temporary WAV format buffer to pass to ALUT.
	int bufSize = 12 + (8 + 18) + (8 + lBufferLen);
	MR_UInt32 chunkSize = bufSize - 8;
	char *buf = (char*)malloc(bufSize);
	memcpy(buf, waveHeader, 12 + (8 + 18) + 8);
	memcpy(buf + 0x04, &chunkSize, 4);
	memcpy(buf + 0x14, lSoundData, 16);
	memcpy(buf + 0x2a, &lBufferLen, 4);
	lSoundData += 18;  // sizeof(WAVEFORMATEX)
	memcpy(buf + 0x2e, lSoundData, lBufferLen);

	mBuffer = alutCreateBufferFromFileImage(buf, bufSize);
	//mBuffer = alutCreateBufferHelloWorld();
	if (mBuffer == AL_NONE) {
		//MessageBox(NULL, alutGetErrorString(alutGetError()), "AIEEE", MB_OK);
		ASSERT(FALSE);
		lReturnValue = FALSE;
	} else {
		alGenSources(mNbCopy, mSoundBuffer);
		for (int i = 0; i < mNbCopy; ++i) {
			alSourcei(mSoundBuffer[i], AL_BUFFER, mBuffer);
		}
	}

	free(buf);

	return lReturnValue;
}

void SoundBuffer::SetParams(int pCopy, int pDB, double pSpeed, int pPan)
{
	if (soundDisabled) return;

	if(pCopy >= mNbCopy) {
		pCopy = mNbCopy - 1;
	}

	// Global sound effect volume setting.
	float vol = Config::GetInstance()->audio.sfxVolume;

	float attenuatedVolume = vol * DirectXToLinear(pDB);

	// Clamp volume to accepted range.
	if (attenuatedVolume < 0.0f) attenuatedVolume = 0.0f;
	else if (attenuatedVolume > 1.0f) attenuatedVolume = 1.0f;

	if (pSpeed < 0.01f) pSpeed = 0.01f;

	ALuint src = mSoundBuffer[pCopy];
	if (src) {
		alSourcef(mSoundBuffer[pCopy], AL_GAIN, attenuatedVolume);
		alSourcef(mSoundBuffer[pCopy], AL_PITCH, static_cast<float>(pSpeed));
		//TODO: Simulate panning by changing position.
	}
}

int SoundBuffer::GetNbCopy() const
{
	return mNbCopy;
}

// ShortSound
ShortSound::ShortSound()
{
	mCurrentCopy = 0;
}

ShortSound::~ShortSound()
{
}

void ShortSound::Play(int pDB, double pSpeed, int pPan)
{
	if (soundDisabled) return;

	SetParams(mCurrentCopy, pDB, pSpeed, pPan);
	alSourcePlay(mSoundBuffer[mCurrentCopy]);

	mCurrentCopy++;
	if(mCurrentCopy >= mNbCopy) {
		mCurrentCopy = 0;
	}
}

// class ContinuousSound
ContinuousSound::ContinuousSound()
{
	ResetCumStat();
}

ContinuousSound::~ContinuousSound()
{
}

void ContinuousSound::ResetCumStat()
{
	for(int lCounter = 0; lCounter < mNbCopy; lCounter++) {
		mOn[lCounter] = FALSE;
		mMaxSpeed[lCounter] = 0;
		mMaxDB[lCounter] = -10000;
	}
}

void ContinuousSound::Pause(int pCopy)
{
	if (soundDisabled) return;

	if(pCopy >= mNbCopy) {
		pCopy = mNbCopy - 1;
	}

	alSourcePause(mSoundBuffer[pCopy]);
}

void ContinuousSound::Restart(int pCopy)
{
	if (soundDisabled) return;

	if(pCopy >= mNbCopy) {
		pCopy = mNbCopy - 1;
	}

	alSourcei(mSoundBuffer[pCopy], AL_LOOPING, AL_TRUE);
	ALint state;
	alGetSourcei(mSoundBuffer[pCopy], AL_SOURCE_STATE, &state);
	if (state != AL_PLAYING) {
		alSourcePlay(mSoundBuffer[pCopy]);
	}
}

void ContinuousSound::ApplyCumCommand()
{

	for(int lCounter = 0; lCounter < mNbCopy; lCounter++) {
		if(mOn[lCounter]) {
			SetParams(lCounter, mMaxDB[lCounter], mMaxSpeed[lCounter], 0);
			Restart(lCounter);
		}
		else {
			Pause(lCounter);
		}
	}
	ResetCumStat();

}

void ContinuousSound::CumPlay(int pCopy, int pDB, double pSpeed)
{
	if(pCopy >= mNbCopy) {
		pCopy = mNbCopy - 1;
	}

	mOn[pCopy] = TRUE;
	mMaxDB[pCopy] = max(mMaxDB[pCopy], pDB);
	mMaxSpeed[pCopy] = max(mMaxSpeed[pCopy], pSpeed);

}

// namespace SoundServer

bool SoundServer::Init()
{
	soundDisabled = Config::GetInstance()->runtime.silent;
	if (soundDisabled) return true;

	bool lReturnValue = true;

	lReturnValue = (alutInit(NULL, NULL) == AL_TRUE);
	if (!lReturnValue) {
		ALenum code = alutGetError();
		if (code == ALUT_ERROR_INVALID_OPERATION) {
			// OpenAL already initialized.
			lReturnValue = true;
		}
		else {
			initErrorCode = code;
			Config::GetInstance()->runtime.silent = soundDisabled = true;
		}
	}

	return lReturnValue;
}

void SoundServer::Close()
{
	SoundBuffer::DeleteAll();

	if (soundDisabled) return;

	alutExit();
}

/**
 * Retrieve the error message from the initialization.
 * Use this function to get details if SoundServer::Init() failed.
 * @return The error message (will be empty if there was no error).
 */
std::string SoundServer::GetInitError()
{
	return (initErrorCode == ALUT_ERROR_NO_ERROR) ? "" :
		alutGetErrorString(initErrorCode);
}

ShortSound *SoundServer::CreateShortSound(const char *pData, int pNbCopy)
{
	ShortSound *lReturnValue = new ShortSound;

	if(!lReturnValue->Init(pData, pNbCopy)) {
		lReturnValue = NULL;
	}
	return lReturnValue;
}

void SoundServer::DeleteShortSound(ShortSound * pSound)
{
	delete pSound;
}

void SoundServer::Play(ShortSound * pSound, int pDB, double pSpeed, int pPan)
{
	if(pSound != NULL) {
		pSound->Play(pDB, pSpeed, pPan);
	}
}

int SoundServer::GetNbCopy(ShortSound * pSound)
{
	if(pSound != NULL) {
		return pSound->GetNbCopy();
	}
	else {
		return 1;
	}
}

ContinuousSound *SoundServer::CreateContinuousSound(const char *pData, int pNbCopy)
{
	ContinuousSound *lReturnValue = new ContinuousSound;

	if(!lReturnValue->Init(pData, pNbCopy)) {
		lReturnValue = NULL;
	}
	return lReturnValue;
}

void SoundServer::DeleteContinuousSound(ContinuousSound * pSound)
{
	delete pSound;
}

void SoundServer::Play(ContinuousSound * pSound, int pCopy, int pDB, double pSpeed, int /*pPan */ )
{
	if(pSound != NULL) {
		pSound->CumPlay(pCopy, pDB, pSpeed);
	}
}

void SoundServer::ApplyContinuousPlay()
{
	SoundBuffer::ApplyCumCommandForAll();
}

/*
void SoundServer::Pause(   ContinuousSound* pSound, int pCopy )
{
   if( pSound != NULL )
   {
	  pSound->Pause( pCopy );
   }
}

void SoundServer::Restart( ContinuousSound* pSound, int pCopy )
{
   if( pSound != NULL )
   {
	  pSound->Restart( pCopy );
   }
}

void SoundServer::SetParams( ContinuousSound* pSound, int pCopy, int pDB, double pSpeed, int pPan )
{
   if( pSound != NULL )
   {
	  pSound->SetParams( pCopy, pDB, pSpeed, pPan );
   }
}
*/

int SoundServer::GetNbCopy(ContinuousSound * pSound)
{
	if(pSound != NULL) {
		return pSound->GetNbCopy();
	}
	else {
		return 1;
	}
}

}  // namespace VideoServices
}  // namespace HoverRace
