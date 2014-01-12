
// SoundServer.cpp
//
// Copyright (c) 1995-1998 - Richard Langlois and Grokksoft Inc.
// Copyright (c) 2014 Michael Imamura.
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

#ifdef WITH_SDL_MIXER
#	include <SDL2/SDL_mixer.h>
#else
#	include <AL/alut.h>
#endif

#include "../Util/MR_Types.h"
#include "../Util/Config.h"
#include "../Util/Log.h"

#include "SoundServer.h"

using namespace HoverRace::Util;

namespace HoverRace {
namespace VideoServices {

#define MR_MAX_SOUND_COPY 6

namespace {
	bool soundDisabled = false;
	std::string initErrorStr;
}

#ifdef WITH_SDL_MIXER
	namespace {
		std::stack<int> freeChannels;

		/**
		 * Allocate a free channel for use.
		 * @return A free channel or -1 if no channels are available.
		 */
		int AllocChannel() {
			int retv = -1;

			SDL_LockAudio();

			if (!freeChannels.empty()) {
				retv = freeChannels.top();
				freeChannels.pop();
			}

			SDL_UnlockAudio();

			return retv;
		}

		/**
		 * Free a previously-allocated channel.
		 * This is called automatically by the mixer when a channel has
		 * finished playing.  It may also be called if the channel allocated
		 * by AllocChannel() is not used.
		 * @param channel The channel to deallocate.
		 */
		void FreeChannel(int channel) {
			if (channel >= 0) {
				SDL_LockAudio();
				freeChannels.push(channel);
				SDL_UnlockAudio();
			}
		}
	}
#endif

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

#		ifdef WITH_SDL_MIXER
			Mix_Chunk *chunk;
#		else
			int mNbCopy;
			ALuint mBuffer;
			ALuint mSoundBuffer[MR_MAX_SOUND_COPY];  // Actually the sources.
#		endif

		int mNormalFreq;

	public:
		SoundBuffer();
		virtual ~SoundBuffer();

		virtual BOOL Init(const char *pData, int pNbCopy);

		void SetParams(int pCopy, int pDB, double pSpeed, int pPan);

		/*
		int GetNbCopy() const;
		*/

		static void DeleteAll();

		virtual void ApplyCumCommand();

		static void ApplyCumCommandForAll();

};

class ShortSound : public SoundBuffer
{
	typedef SoundBuffer SUPER;
	protected:
		int mCurrentCopy;

	public:
		ShortSound();
		~ShortSound();

		void Play(int pDB, double pSpeed, int pPan);
};

class ContinuousSound : public SoundBuffer
{
	typedef SoundBuffer SUPER;
	protected:
#		ifdef WITH_SDL_MIXER
			int mNbCopy;
			int channels[MR_MAX_SOUND_COPY];
#		endif
		BOOL mOn[MR_MAX_SOUND_COPY];
		int mMaxDB[MR_MAX_SOUND_COPY];
		double mMaxSpeed[MR_MAX_SOUND_COPY];

		void ResetCumStat();

	public:
		ContinuousSound();
		~ContinuousSound();

		virtual BOOL Init(const char *pData, int pNbCopy);

		void Pause(int pCopy);
		void Restart(int pCopy);

		void ApplyCumCommand();
		void CumPlay(int pCopy, int pDB, double pSpeed);

};

SoundBuffer *SoundBuffer::mList = NULL;

static const char* const waveHeader =
	"RIFF----WAVE"
	"fmt \022\0\0\0" "----" "----" "----" "----\0\0"
	"data----";

SoundBuffer::SoundBuffer()
{
#	ifdef WITH_SDL_MIXER
		chunk = nullptr;
#	else
		mNbCopy = 0;
		mBuffer = 0;

		for(int lCounter = 0; lCounter < MR_MAX_SOUND_COPY; lCounter++) {
			mSoundBuffer[lCounter] = 0;
		}
#	endif

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
#	ifdef WITH_SDL_MIXER
		Mix_FreeChunk(chunk);
#	else
		alDeleteSources(mNbCopy, mSoundBuffer);
		alDeleteBuffers(1, &mBuffer);
#	endif
}

void SoundBuffer::ApplyCumCommand()
{
	// Do nothing by default
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

#	ifndef WITH_SDL_MIXER
		ASSERT(!mSoundBuffer[0]);			  // Already initialized

		if(pNbCopy > MR_MAX_SOUND_COPY) {
			ASSERT(FALSE);
			pNbCopy = MR_MAX_SOUND_COPY;
		}

		mNbCopy = pNbCopy;
#	endif

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

#	ifdef WITH_SDL_MIXER
		chunk = Mix_LoadWAV_RW(SDL_RWFromMem(buf, bufSize), 1);
		if (!chunk) {
			ASSERT(FALSE);
			lReturnValue = FALSE;
		}
#	else
		mBuffer = alutCreateBufferFromFileImage(buf, bufSize);
		if (mBuffer == AL_NONE) {
			ASSERT(FALSE);
			lReturnValue = FALSE;
		} else {
			alGenSources(mNbCopy, mSoundBuffer);
			for (int i = 0; i < mNbCopy; ++i) {
				alSourcei(mSoundBuffer[i], AL_BUFFER, mBuffer);
			}
		}
#	endif

	free(buf);

	return lReturnValue;
}

void SoundBuffer::SetParams(int pCopy, int pDB, double pSpeed, int pPan)
{
	if (soundDisabled) return;

#	ifndef WITH_SDL_MIXER
		if(pCopy >= mNbCopy) {
			pCopy = mNbCopy - 1;
		}
#	endif

	// Global sound effect volume setting.
	float vol = Config::GetInstance()->audio.sfxVolume;

	float attenuatedVolume = vol * DirectXToLinear(pDB);

	// Clamp volume to accepted range.
	if (attenuatedVolume < 0.0f) attenuatedVolume = 0.0f;
	else if (attenuatedVolume > 1.0f) attenuatedVolume = 1.0f;

	if (pSpeed < 0.01f) pSpeed = 0.01f;

#	ifdef WITH_SDL_MIXER
		Mix_Volume(pCopy, static_cast<int>(attenuatedVolume * MIX_MAX_VOLUME));
		// Note: SDL_Mixer does not do pitch shifting.
		//TODO: Panning (currently unused anyway).
#	else
		ALuint src = mSoundBuffer[pCopy];
		if (src) {
			alSourcef(mSoundBuffer[pCopy], AL_GAIN, attenuatedVolume);
			alSourcef(mSoundBuffer[pCopy], AL_PITCH, static_cast<float>(pSpeed));
			//TODO: Simulate panning by changing position.
		}
#	endif
}

/*
int SoundBuffer::GetNbCopy() const
{
	return mNbCopy;
}
*/

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

#	ifdef WITH_SDL_MIXER
		int channel = AllocChannel();
		if (channel >= 0) {
			SetParams(channel, pDB, pSpeed, pPan);
			if (Mix_PlayChannel(channel, chunk, 0) < 0) {
				Log::Warn("Failed to play on channel %d: %s", channel,
					Mix_GetError());
				FreeChannel(channel);
			}
		}
#	else
		SetParams(mCurrentCopy, pDB, pSpeed, pPan);
		alSourcePlay(mSoundBuffer[mCurrentCopy]);

		mCurrentCopy++;
		if(mCurrentCopy >= mNbCopy) {
			mCurrentCopy = 0;
		}
#	endif
}

// class ContinuousSound
ContinuousSound::ContinuousSound()
{
#	ifdef WITH_SDL_MIXER
		for (int i = 0; i < MR_MAX_SOUND_COPY; i++) {
			channels[i] = -1;
		}
#	endif
	ResetCumStat();
}

ContinuousSound::~ContinuousSound()
{
#	ifdef WITH_SDL_MIXER
		for (int i = 0; i < MR_MAX_SOUND_COPY; i++) {
			if (channels[i] >= 0) {
				Mix_HaltChannel(channels[i]);
			}
		}
#	endif
}

BOOL ContinuousSound::Init(const char *pData, int pNbCopy)
{
	auto retv = SUPER::Init(pData, pNbCopy);

#	ifdef WITH_SDL_MIXER
	if (retv) {
		mNbCopy = pNbCopy;
		for (int i = 0; i < pNbCopy; i++) {
			channels[i] = AllocChannel();
		}
	}
#	endif

	return retv;
}

void ContinuousSound::ResetCumStat()
{
	for(int lCounter = 0; lCounter < MR_MAX_SOUND_COPY; lCounter++) {
		mOn[lCounter] = FALSE;
		mMaxSpeed[lCounter] = 0;
		mMaxDB[lCounter] = -10000;
	}
}

void ContinuousSound::Pause(int pCopy)
{
	if (soundDisabled) return;

#	ifdef WITH_SDL_MIXER
		Mix_Pause(pCopy);
#	else
		if(pCopy >= mNbCopy) {
			pCopy = mNbCopy - 1;
		}

		alSourcePause(mSoundBuffer[pCopy]);
#	endif
}

void ContinuousSound::Restart(int pCopy)
{
	if (soundDisabled) return;

#	ifdef WITH_SDL_MIXER
		int channel = channels[pCopy];
		if (Mix_Paused(channel)) {
			Mix_Resume(channel);
		} else if (!Mix_Playing(channel)) {
			if (Mix_PlayChannel(channel, chunk, -1) < 0) {
				Log::Warn("Failed to play on channel %d: %s", channel,
					Mix_GetError());
			}
		}
#	else
		if(pCopy >= mNbCopy) {
			pCopy = mNbCopy - 1;
		}

		alSourcei(mSoundBuffer[pCopy], AL_LOOPING, AL_TRUE);
		ALint state;
		alGetSourcei(mSoundBuffer[pCopy], AL_SOURCE_STATE, &state);
		if (state != AL_PLAYING) {
			alSourcePlay(mSoundBuffer[pCopy]);
		}
#	endif
}

void ContinuousSound::ApplyCumCommand()
{
	for(int lCounter = 0; lCounter < mNbCopy; lCounter++) {
#		ifdef WITH_SDL_MIXER
			int source = channels[lCounter];
			if (source < 0) continue;
#		else
			int source = lCounter;
#		endif
		if(mOn[lCounter]) {
			SetParams(source, mMaxDB[lCounter], mMaxSpeed[lCounter], 0);
			Restart(source);
		}
		else {
			Pause(source);
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
	initErrorStr.clear();

	auto &runtimeCfg = Config::GetInstance()->runtime;

	soundDisabled = runtimeCfg.silent;
	if (soundDisabled) return true;

#	ifdef WITH_SDL_MIXER
		int reqFmts = MIX_INIT_OGG;
		int actualFmts = Mix_Init(reqFmts);
		if ((actualFmts & reqFmts) != reqFmts) {
			initErrorStr = Mix_GetError();
			soundDisabled = true;
		}
		else {
			if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 512) < 0) {
				initErrorStr = Mix_GetError();
				soundDisabled = true;
			}
		}

		if (!soundDisabled) {
			int numChannels = Mix_AllocateChannels(-1);
			for (int i = 0; i < numChannels; i++) {
				freeChannels.push(i);
			}
		}
		Mix_ChannelFinished(FreeChannel);
#	else
		if (alutInit(NULL, NULL) != AL_TRUE) {
			ALenum code = alutGetError();
			if (code == ALUT_ERROR_INVALID_OPERATION) {
				// OpenAL already initialized.
			}
			else {
				initErrorStr = alutGetErrorString(code);
				soundDisabled = true;
			}
		}
#	endif

	runtimeCfg.silent = soundDisabled;

	return !soundDisabled;
}

void SoundServer::Close()
{
	SoundBuffer::DeleteAll();

	if (soundDisabled) return;

#	ifdef WITH_SDL_MIXER
		Mix_HaltChannel(-1);
		Mix_CloseAudio();
		Mix_Quit();
#	else
		alutExit();
#	endif
}

/**
 * Retrieve the error message from the initialization.
 * Use this function to get details if SoundServer::Init() failed.
 * @return The error message (will be empty if there was no error).
 */
const std::string &SoundServer::GetInitError()
{
	return initErrorStr;
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

/*
int SoundServer::GetNbCopy(ShortSound * pSound)
{
	if(pSound != NULL) {
		return pSound->GetNbCopy();
	}
	else {
		return 1;
	}
}
*/

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
int SoundServer::GetNbCopy(ContinuousSound * pSound)
{
	if(pSound != NULL) {
		return pSound->GetNbCopy();
	}
	else {
		return 1;
	}
}
*/

}  // namespace VideoServices
}  // namespace HoverRace
