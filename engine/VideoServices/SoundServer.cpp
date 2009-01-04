// SoundServer.cpp
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

#include "StdAfx.h"

#include <math.h>

#ifdef WITH_OPENAL
#	include <AL/alut.h>
#	ifdef _MSC_VER
#		pragma comment(lib, "OpenAL32.lib")
#		pragma comment(lib, "alut.lib")
#	endif
#else
#	include <mmreg.h>
#	include <dsound.h>
#endif

#include "../Util/MR_Types.h"
#include "../Util/Config.h"

#include "SoundServer.h"

#ifdef max
#	undef max
#endif

using std::max;

#define MR_MAX_SOUND_COPY 6

#ifndef WITH_OPENAL
// Adapted from http://www.gamedev.net/community/forums/topic.asp?topic_id=337397
//   and http://en.wikipedia.org/wiki/Decibel

/// Convert linear (0.0 - 1.0) to millibels.
static int LinearToDirectX(float value)
{
	return (value <= 0.01f) ? DSBVOLUME_MIN :
		static_cast<int>(floorf(2000.0f * log10f(value) + 0.5f));
}
#else
#	define DSBVOLUME_MIN -10000
#endif
/// Convert millibels to linear.
static float DirectXToLinear(int value)
{
	return (value == DSBVOLUME_MIN) ? 0.0f : 
		powf(10.0f, (float)value / 2000.0f);
} 

class MR_SoundBuffer
{
	private:
		static MR_SoundBuffer *mList;
		MR_SoundBuffer *mNext;

	protected:

		int mNbCopy;
#		ifdef WITH_OPENAL
			ALuint mBuffer;
			ALuint mSoundBuffer[MR_MAX_SOUND_COPY];  // Actually the sources.
#		else
			IDirectSoundBuffer *mSoundBuffer[MR_MAX_SOUND_COPY];
#		endif

		int mNormalFreq;

	public:
		MR_SoundBuffer();
		virtual ~ MR_SoundBuffer();

		BOOL Init(const char *pData, int pNbCopy);

		void SetParams(int pCopy, int pDB, double pSpeed, int pPan);

		int GetNbCopy() const;

		static void DeleteAll();

		virtual void ApplyCumCommand();

		static void ApplyCumCommandForAll();

};

class MR_ShortSound:public MR_SoundBuffer
{

	protected:
		int mCurrentCopy;

	public:
		MR_ShortSound();
		~MR_ShortSound();

		void Play(int pDB, double pSpeed, int pPan);
};

class MR_ContinuousSound:public MR_SoundBuffer
{
	protected:
		BOOL mOn[MR_MAX_SOUND_COPY];
		int mMaxDB[MR_MAX_SOUND_COPY];
		double mMaxSpeed[MR_MAX_SOUND_COPY];

		void ResetCumStat();

	public:
		MR_ContinuousSound();
		~MR_ContinuousSound();

		void Pause(int pCopy);
		void Restart(int pCopy);

		void ApplyCumCommand();
		void CumPlay(int pCopy, int pDB, double pSpeed);

};

// Variables
MR_SoundBuffer *MR_SoundBuffer::mList = NULL;

#ifndef WITH_OPENAL
IDirectSound *gDirectSound = NULL;
#endif

#ifdef WITH_OPENAL
static const char* const waveHeader =
	"RIFF----WAVE"
	"fmt \022\0\0\0" "----" "----" "----" "----\0\0"
	"data----";
#endif

// Implementation
MR_SoundBuffer::MR_SoundBuffer()
{
	mNbCopy = 0;
#ifdef WITH_OPENAL
	mBuffer = 0;
#endif

	for(int lCounter = 0; lCounter < MR_MAX_SOUND_COPY; lCounter++) {
		mSoundBuffer[lCounter] = NULL;
	}

	// Add the new buffer to the list
	mNext = mList;
	mList = this;

}

MR_SoundBuffer::~MR_SoundBuffer()
{
	// Remove form list
	if(mList == this) {
		mList = mNext;
		mNext = NULL;
	}
	else {
		MR_SoundBuffer *mPrev = mList;

		while(mPrev->mNext != this) {
			ASSERT(mPrev != NULL);

			mPrev = mPrev->mNext;
		}

		mPrev->mNext = mNext;
		mNext = NULL;
	}

	// Delete the sound buffers
#ifdef WITH_OPENAL
	alDeleteSources(mNbCopy, mSoundBuffer);
	alDeleteBuffers(1, &mBuffer);
#else
	for(int lCounter = 0; lCounter < mNbCopy; lCounter++) {
		if(mSoundBuffer[lCounter] != NULL) {
			mSoundBuffer[lCounter]->Release();
			mSoundBuffer[lCounter] = NULL;
		}
	}
#endif
}

void MR_SoundBuffer::ApplyCumCommand()
{
	// Do notting by default
}

void MR_SoundBuffer::ApplyCumCommandForAll()
{
	MR_SoundBuffer *mCurrent = mList;

	while(mCurrent != NULL) {
		mCurrent->ApplyCumCommand();
		mCurrent = mCurrent->mNext;
	}
}

void MR_SoundBuffer::DeleteAll()
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
BOOL MR_SoundBuffer::Init(const char *pData, int pNbCopy)
{
	BOOL lReturnValue = TRUE;

	ASSERT(mSoundBuffer[0] == NULL);			  // Already initialized

	if(pNbCopy > MR_MAX_SOUND_COPY) {
		ASSERT(FALSE);
		pNbCopy = MR_MAX_SOUND_COPY;
	}

	mNbCopy = pNbCopy;

	// Parse pData
	MR_UInt32 lBufferLen = *(MR_UInt32 *) pData;
#ifdef WITH_OPENAL
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
#else
	DSBUFFERDESC lDesc;
	WAVEFORMATEX *lWaveFormat = (WAVEFORMATEX *) (pData + sizeof(MR_UInt32));
	const char *lSoundData = pData + sizeof(MR_UInt32) + sizeof(WAVEFORMATEX);

	lWaveFormat->cbSize = 0;

	lDesc.dwSize = sizeof(lDesc);
	lDesc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC;
	lDesc.dwReserved = 0;
	lDesc.dwBufferBytes = lBufferLen;
	lDesc.lpwfxFormat = lWaveFormat;

	mNormalFreq = lWaveFormat->nSamplesPerSec;

	// Initialize the first copy
	int lCode;
	if((lCode = gDirectSound->CreateSoundBuffer(&lDesc, &(mSoundBuffer[0]), NULL)) != DS_OK) {
		ASSERT(lCode != DSERR_ALLOCATED);
		ASSERT(lCode != DSERR_BADFORMAT);
		ASSERT(lCode != DSERR_INVALIDPARAM);
		ASSERT(lCode != DSERR_NOAGGREGATION);
		ASSERT(lCode != DSERR_OUTOFMEMORY);
		ASSERT(FALSE);
		lReturnValue = FALSE;
	}

	if(lReturnValue) {
		void *lSoundBuffer;
		unsigned long lSoundBufferLen;

		if(mSoundBuffer[0]->Lock(0, lBufferLen, &lSoundBuffer, &lSoundBufferLen, NULL, NULL, 0) == DS_OK) {
			ASSERT(lSoundBufferLen == lBufferLen);

			memcpy(lSoundBuffer, lSoundData, lSoundBufferLen);

			mSoundBuffer[0]->Unlock(lSoundBuffer, lSoundBufferLen, NULL, 0);
		}
		else {
			ASSERT(FALSE);
			lReturnValue = FALSE;
		}
	}
	// Init extra copy
	for(int lCounter = 1; lReturnValue && (lCounter < mNbCopy); lCounter++) {
		if(gDirectSound->DuplicateSoundBuffer(mSoundBuffer[0], &mSoundBuffer[lCounter]) != DS_OK) {
			lReturnValue = FALSE;
		}
	}
#endif // WITH_OPENAL

	return lReturnValue;
}

void MR_SoundBuffer::SetParams(int pCopy, int pDB, double pSpeed, int pPan)
{
	if(pCopy >= mNbCopy) {
		pCopy = mNbCopy - 1;
	}

	// Global sound effect volume setting.
	float vol = MR_Config::GetInstance()->audio.sfxVolume;

#ifdef WITH_OPENAL
	float attenuatedVolume = vol * DirectXToLinear(pDB);

	// Clamp volume to accepted range.
	if (attenuatedVolume < 0.0f) attenuatedVolume = 0.0f;
	else if (attenuatedVolume > 1.0f) attenuatedVolume = 1.0f;

	if (pSpeed < 0.01f) pSpeed = 0.01f;

	ALuint src = mSoundBuffer[pCopy];
	if (src != NULL) {
		alSourcef(mSoundBuffer[pCopy], AL_GAIN, attenuatedVolume);
		alSourcef(mSoundBuffer[pCopy], AL_PITCH, static_cast<float>(pSpeed));
		//TODO: Simulate panning by changing position.
	}
#else
	long attenuatedVolume;
	if (vol >= 0.99f) {
		attenuatedVolume = pDB;
	}
	else {
		attenuatedVolume = LinearToDirectX(vol * DirectXToLinear(pDB));
	}

	// Clamp volume to accepted range.
	if (attenuatedVolume > DSBVOLUME_MAX) attenuatedVolume = DSBVOLUME_MAX;
	else if (attenuatedVolume < DSBVOLUME_MIN) attenuatedVolume = DSBVOLUME_MIN;

	if(mSoundBuffer[pCopy] != NULL) {
		unsigned long lFreq = static_cast<unsigned long>(mNormalFreq * pSpeed);

		if(lFreq > 100000) {
			lFreq = 100000;
		}
		mSoundBuffer[pCopy]->SetVolume(attenuatedVolume);
		mSoundBuffer[pCopy]->SetFrequency(lFreq);
		mSoundBuffer[pCopy]->SetPan(pPan);
	}
#endif
}

int MR_SoundBuffer::GetNbCopy() const
{
	return mNbCopy;
}

// MR_ShortSound
MR_ShortSound::MR_ShortSound()
{
	mCurrentCopy = 0;
}

MR_ShortSound::~MR_ShortSound()
{
}

void MR_ShortSound::Play(int pDB, double pSpeed, int pPan)
{
#ifdef WITH_OPENAL
	SetParams(mCurrentCopy, pDB, pSpeed, pPan);
	alSourcePlay(mSoundBuffer[mCurrentCopy]);
#else
	mSoundBuffer[mCurrentCopy]->SetCurrentPosition(0);
	SetParams(mCurrentCopy, pDB, pSpeed, pPan);
	mSoundBuffer[mCurrentCopy]->Play(0, 0, 0);
#endif

	mCurrentCopy++;
	if(mCurrentCopy >= mNbCopy) {
		mCurrentCopy = 0;
	}
}

// class MR_ContinuousSound
MR_ContinuousSound::MR_ContinuousSound()
{
	ResetCumStat();
}

MR_ContinuousSound::~MR_ContinuousSound()
{
}

void MR_ContinuousSound::ResetCumStat()
{
	for(int lCounter = 0; lCounter < mNbCopy; lCounter++) {
		mOn[lCounter] = FALSE;
		mMaxSpeed[lCounter] = 0;
		mMaxDB[lCounter] = -10000;
	}
}

void MR_ContinuousSound::Pause(int pCopy)
{
	if(pCopy >= mNbCopy) {
		pCopy = mNbCopy - 1;
	}

#ifdef WITH_OPENAL
	alSourcePause(mSoundBuffer[pCopy]);
#else
	mSoundBuffer[pCopy]->Stop();
#endif

}

void MR_ContinuousSound::Restart(int pCopy)
{
	if(pCopy >= mNbCopy) {
		pCopy = mNbCopy - 1;
	}
#ifdef WITH_OPENAL
	alSourceRewind(mSoundBuffer[pCopy]);
	alSourcei(mSoundBuffer[pCopy], AL_LOOPING, AL_TRUE);
	alSourcePlay(mSoundBuffer[pCopy]);
#else
	mSoundBuffer[pCopy]->Play(0, 0, DSBPLAY_LOOPING);
#endif
}

void MR_ContinuousSound::ApplyCumCommand()
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

void MR_ContinuousSound::CumPlay(int pCopy, int pDB, double pSpeed)
{
	if(pCopy >= mNbCopy) {
		pCopy = mNbCopy - 1;
	}

	mOn[pCopy] = TRUE;
	mMaxDB[pCopy] = max(mMaxDB[pCopy], pDB);
	mMaxSpeed[pCopy] = max(mMaxSpeed[pCopy], pSpeed);

}

// namespace MR_SoundServer

bool MR_SoundServer::Init(
#	ifndef WITH_OPENAL
		HWND pWindow
#	endif
	)
{
	bool lReturnValue = true;

#ifdef WITH_OPENAL
	lReturnValue = (alutInit(NULL, NULL) == AL_TRUE);
#else
	if(gDirectSound == NULL) {
		if(DirectSoundCreate(NULL, &gDirectSound, NULL) == DS_OK) {

			if(gDirectSound->SetCooperativeLevel(pWindow, DSSCL_NORMAL) != DS_OK) {
				ASSERT(FALSE);
				lReturnValue = false;
			}
		}
		else {
			lReturnValue = false;
		}
	}
#endif

	return lReturnValue;
}

void MR_SoundServer::Close()
{
	MR_SoundBuffer::DeleteAll();

#ifdef WITH_OPENAL
	alutExit();
#else
	if(gDirectSound != NULL) {
		gDirectSound->Release();
		gDirectSound = NULL;
	}
#endif
}

MR_ShortSound *MR_SoundServer::CreateShortSound(const char *pData, int pNbCopy)
{
#ifndef WITH_OPENAL
	if(gDirectSound != NULL) {
#endif
		MR_ShortSound *lReturnValue = new MR_ShortSound;

		if(!lReturnValue->Init(pData, pNbCopy)) {
			lReturnValue = NULL;
		}
		return lReturnValue;
#ifndef WITH_OPENAL
	}
	else {
		return NULL;
	}
#endif
}

void MR_SoundServer::DeleteShortSound(MR_ShortSound * pSound)
{
	delete pSound;
}

void MR_SoundServer::Play(MR_ShortSound * pSound, int pDB, double pSpeed, int pPan)
{
	if(pSound != NULL) {
		pSound->Play(pDB, pSpeed, pPan);
	}
}

int MR_SoundServer::GetNbCopy(MR_ShortSound * pSound)
{
	if(pSound != NULL) {
		return pSound->GetNbCopy();
	}
	else {
		return 1;
	}
}

MR_ContinuousSound *MR_SoundServer::CreateContinuousSound(const char *pData, int pNbCopy)
{
#ifndef WITH_OPENAL
	if(gDirectSound != NULL) {
#endif
		MR_ContinuousSound *lReturnValue = new MR_ContinuousSound;

		if(!lReturnValue->Init(pData, pNbCopy)) {
			lReturnValue = NULL;
		}
		return lReturnValue;
#ifndef WITH_OPENAL
	}
	else {
		return NULL;
	}
#endif
}

void MR_SoundServer::DeleteContinuousSound(MR_ContinuousSound * pSound)
{
	delete pSound;
}

void MR_SoundServer::Play(MR_ContinuousSound * pSound, int pCopy, int pDB, double pSpeed, int /*pPan */ )
{
	if(pSound != NULL) {
		pSound->CumPlay(pCopy, pDB, pSpeed);
	}
}

void MR_SoundServer::ApplyContinuousPlay()
{
#ifndef WITH_OPENAL
	if(gDirectSound != NULL) {
#endif
		MR_SoundBuffer::ApplyCumCommandForAll();
#ifndef WITH_OPENAL
	}
#endif
}

/*
void MR_SoundServer::Pause(   MR_ContinuousSound* pSound, int pCopy )
{
   if( pSound != NULL )
   {
	  pSound->Pause( pCopy );
   }
}

void MR_SoundServer::Restart( MR_ContinuousSound* pSound, int pCopy )
{
   if( pSound != NULL )
   {
	  pSound->Restart( pCopy );
   }
}

void MR_SoundServer::SetParams( MR_ContinuousSound* pSound, int pCopy, int pDB, double pSpeed, int pPan )
{
   if( pSound != NULL )
   {
	  pSound->SetParams( pCopy, pDB, pSpeed, pPan );
   }
}
*/

int MR_SoundServer::GetNbCopy(MR_ContinuousSound * pSound)
{
	if(pSound != NULL) {
		return pSound->GetNbCopy();
	}
	else {
		return 1;
	}
}
