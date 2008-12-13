// InternetRoom.cpp
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

// HoverRace Internet meeting room for Unix or Win32.
//
// This is a CGI application
//
// Query form
//   URL?=REFRESH USER_ID
//   URL?=ADD_CHAT USER_ID MESSAGE
//   URL?=ADD_USER NAME KEY
//   URL?=ADD_GAME USER_ID GAME_NAME TRACK_NAME NB_LAP
//   URL?=JOIN_GAME USER_ID GAME_ID
//   URL?=DEL_GAME GAME_ID

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE -1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define IR_USER_NAME_LEN     20
#define IR_GAME_NAME_LEN     20
#define IR_TRACK_NAME_LEN    20
#define IR_CHAT_MESSAGE_LEN 100
#define IR_MAX_PLAYER_GAME    8

#define IR_MAX_CLIENT        100
#define IR_MAX_GAME           20
#define IR_MAX_CHAT_MESSAGE   30

#define IR_EXPIRATION_DELAY   15

#define IR_STATE_FNAME        "data/state"
#define IR_STATE_FNAME_LOCK   "data/state.lock"

// Structure definition
class IRState
{
	protected:
		class User
		{
			public:

				int mTimeStamp;
				time_t mExpiration;

				char mName[IR_USER_NAME_LEN + 1];

				int mLastFetchTimeStamp;
		};

		class Game
		{
			public:

				int mTimeStamp;
				time_t mExpiration;

				char mName[IR_GAME_NAME_LEN + 1];
				char mTrack[IR_TRACK_NAME_LEN + 1];
				int mNbLap;
				int mNbPlayer;
				int mPlayers[IR_MAX_PLAYER_GAME];
		};

		class ChatMessage
		{
			public:

				int mTimeStamp;
				char mData[IR_CHAT_MESSAGE_LEN];
		};

		int mTimeStamp;

		User mUser[IR_MAX_CLIENT];
		Game mGame[IR_MAX_GAME];

		int mChatTail;
		ChatMessage mChatFifo[IR_MAX_CHAT_MESSAGE];

		void PrintState(int pUser);
		void RefreshUser(int pUser);

	public:

		void Clear();
		void Read(FILE * pFile);
		void Write(FILE * pFile);

		BOOL VerifyExpirations();				  // Return TRUE if a modification have been made

		void PrintStateChange(int pUser);

		void IncTimeStamp();
		void AddChatMessage(const char *pMessage);

		// These actions print a return value
		BOOL AddUser(const char *pUserName);
		BOOL AddGame(const char *pGameName, const char *pTrackName, int pNbLap, int pPlayer);
		BOOL JoinGame(int pGame, int pPlayer);

		void DeleteGame(int pGame);

};

// Local functions

static BOOL CreateGlobalStructure(const char *pVarName, const char *pLockName);
static BOOL CreateLock(const char *pLockName);
static BOOL PostSystemRebootTest(const char *pVarName, const char *pLockName);

// Implementation

void IRState::Clear()
{
	int lCounter;

	mTimeStamp = 1;

	for(lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++) {
		mUser[lCounter].mExpiration = 0;
		mUser[lCounter].mTimeStamp = 0;
	}

	for(lCounter = 0; lCounter < IR_MAX_GAME; lCounter++) {
		mGame[lCounter].mExpiration = 0;
		mGame[lCounter].mTimeStamp = 0;
	}

	mChatTail = 0;

	for(lCounter = 0; lCounter < IR_MAX_CHAT_MESSAGE; lCounter++) {
		mChatFifo[lCounter].mTimeStamp = 0;
	}
}

void IRState::Read(FILE * pFile)
{
	fseek(pFile, 0, SEEK_SET);
	fread(this, sizeof(IRState), 1, pFile);
}

void IRState::Write(FILE * pFile)
{
	fseek(pFile, 0, SEEK_SET);
	fwrite(this, sizeof(IRState), 1, pFile);
}

BOOL IRState::VerifyExpirations()
{
	BOOL lReturnValue = FALSE;
	int lCounter;

	time_t lCurrentTime = time(NULL);

	for(lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++) {
		if((mUser[lCounter].mExpiration != 0) && (mUser[lCounter].mExpiration < lCurrentTime)) {
			lReturnValue = TRUE;
			mUser[lCounter].mExpiration = 0;
			mUser[lCounter].mTimeStamp = mTimeStamp;
		}
	}

	for(lCounter = 0; lCounter < IR_MAX_GAME; lCounter++) {
		if((mGame[lCounter].mExpiration != 0) && (mGame[lCounter].mExpiration < lCurrentTime)) {
			lReturnValue = TRUE;
			mGame[lCounter].mExpiration = 0;
			mGame[lCounter].mTimeStamp = mTimeStamp;
		}
	}

	return lReturnValue;
}

void IRState::RefreshUser(int pUser)
{
	if(mUser[pUser].mExpiration != 0) {
		time_t lTime = time(NULL);

		mUser[pUser].mExpiration = lTime + IR_EXPIRATION_DELAY;

		for(int lCounter = 0; lCounter < IR_MAX_GAME; lCounter++) {
			if((mGame[lCounter].mExpiration != 0) && (mGame[lCounter].mPlayers[0] == pUser)) {
				mGame[lCounter].mExpiration = lTime + IR_EXPIRATION_DELAY;
			}
		}
	}
}

void IRState::PrintStateChange(int pUser)
{
	int lCounter;

	RefreshUser(pUser);

	if(mUser[pUser].mLastFetchTimeStamp == mTimeStamp) {
		// notting changed
		return;
	}

	// Send the users that have been modified
	for(lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++) {
		if((mUser[lCounter].mTimeStamp != 0) && (mUser[lCounter].mTimeStamp >= mUser[pUser].mLastFetchTimeStamp)) {
			if(mUser[lCounter].mExpiration == 0) {
				// Expired or deleted
				printf("USER %d DEL\n", lCounter);
			}
			else {
				printf("USER %d NEW\n", lCounter);
				printf("%s\n", mUser[lCounter].mName);
			}
		}
	}

	// Send the games that have been modified
	for(lCounter = 0; lCounter < IR_MAX_GAME; lCounter++) {
		if((mGame[lCounter].mTimeStamp != 0) && (mGame[lCounter].mTimeStamp >= mUser[pUser].mLastFetchTimeStamp)) {
			if(mGame[lCounter].mExpiration == 0) {
				// Expired or deleted
				printf("GAME %d DEL\n", lCounter);
			}
			else {
				printf("GAME %d NEW\n", lCounter);
				printf("%s\n", mGame[lCounter].mName);
				printf("%s\n", mGame[lCounter].mTrack);
				printf("%d %d\n", mGame[lCounter].mNbLap, mGame[lCounter].mNbPlayer);

				for(int lPlayer = 0; lPlayer < mGame[lCounter].mNbPlayer; lPlayer++) {
					printf("%d ", mGame[lCounter].mPlayers[lPlayer]);
				}
				printf("\n");
			}
		}
	}

	// Send the new chat messages
	for(lCounter = 0; lCounter < IR_MAX_CHAT_MESSAGE; lCounter++) {
		int lIndex = (lCounter | +mChatTail) % IR_MAX_CHAT_MESSAGE;

		if((mChatFifo[lIndex].mTimeStamp != 0) && (mChatFifo[lIndex].mTimeStamp >= mUser[pUser].mLastFetchTimeStamp)) {
			printf("CHAT\n%s\n", mChatFifo[lIndex].mData);
		}
	}

	// Adjust fetch time
	mUser[pUser].mLastFetchTimeStamp = mTimeStamp;
}

void IRState::IncTimeStamp()
{
	mTimeStamp++;
}

void IRState::AddChatMessage(const char *pMessage)
{
	mChatFifo[mChatTail].mTimeStamp = mTimeStamp;
	strncpy(mChatFifo[mChatTail].mData, pMessage, IR_CHAT_MESSAGE_LEN);

	mChatTail = (mChatTail + 1) % IR_MAX_CHAT_MESSAGE;
}

// These actions print a return value
BOOL IRState::AddUser(const char *pUserName)
{
	BOOL lReturnValue = TRUE;
	int lCounter;

	// find an empty entry for the new user
	for(lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++) {
		if(mUser[lCounter].mExpiration == 0) {
			break;
		}
	}

	if(lCounter == IR_MAX_CLIENT) {
		lReturnValue = FALSE;
	}
	else {
		mUser[lCounter].mTimeStamp = mTimeStamp;
		mUser[lCounter].mExpiration = time(NULL) + IR_EXPIRATION_DELAY;
		strncpy(mUser[lCounter].mName, pUserName, IR_USER_NAME_LEN);
	}

	if(lReturnValue) {
		printf("SUCCESS\n");
	}
	else {
		printf("ERROR\n");
	}
	return lReturnValue;
}

BOOL IRState::AddGame(const char *pGameName, const char *pTrackName, int pNbLap, int pPlayer)
{
	BOOL lReturnValue = TRUE;
	int lCounter;

	// find an empty entry for the new game
	for(lCounter = 0; lCounter < IR_MAX_GAME; lCounter++) {
		if(mGame[lCounter].mExpiration == 0) {
			break;
		}
	}

	if(lCounter == IR_MAX_CLIENT) {
		lReturnValue = FALSE;
	}
	else {
		mGame[lCounter].mTimeStamp = mTimeStamp;
		mGame[lCounter].mExpiration = time(NULL) + IR_EXPIRATION_DELAY;
		mGame[lCounter].mNbLap = pNbLap;
		mGame[lCounter].mNbPlayer = 1;
		mGame[lCounter].mPlayers[0] = pPlayer;

		strncpy(mGame[lCounter].mName, pGameName, IR_GAME_NAME_LEN);
		strncpy(mGame[lCounter].mTrack, pTrackName, IR_TRACK_NAME_LEN);
	}

	if(lReturnValue) {
		printf("SUCCESS\n");
	}
	else {
		printf("ERROR\n");
	}
	return lReturnValue;
}

BOOL IRState::JoinGame(int pGame, int pPlayer)
{
	BOOL lReturnValue = FALSE;

	if((pGame >= 0) && (pGame < IR_MAX_GAME)) {
		if((mGame[pGame].mExpiration != 0) && (mGame[pGame].mNbPlayer < IR_MAX_PLAYER_GAME)) {
			mGame[pGame].mTimeStamp = mTimeStamp;
			mGame[pGame].mPlayers[mGame[pGame].mNbPlayer++] = pPlayer;
			lReturnValue = TRUE;
		}
	}

	if(lReturnValue) {
		printf("SUCCESS\n");
	}
	else {
		printf("ERROR\n");
	}
	return lReturnValue;
}

void IRState::DeleteGame(int pGame)
{
	if((pGame >= 0) && (pGame < IR_MAX_GAME)) {
		if(mGame[pGame].mExpiration != 0) {
			mGame[pGame].mTimeStamp = mTimeStamp;
			mGame[pGame].mExpiration = 0;
		}
	}
}

int main(int pArgc, const char **pArgs)
{
	char lPath[256];
	char lVarName[256];
	char lLockName[256];

	const char *lPathEnd = strrchr(pArgs[0], '/');

	if(lPathEnd == NULL) {
		lPathEnd = strrchr(pArgs[0], '\\');
	}

	if(lPathEnd != NULL) {
		int lLen = lPathEnd - pArgs[0] + 1;
		memcpy(lPath, pArgs[0], lLen);
		lPath[lLen] = 0;
	}
	else {
		strcpy(lPath, "./");
	}

	strcpy(lVarName, lPath);
	strcat(lVarName, IR_STATE_FNAME);

	strcpy(lLockName, lPath);
	strcat(lLockName, IR_STATE_FNAME_LOCK);

	// Send the header required by the server
	printf("Content-type: text/plain%c%c", 10, 10);

	if(pArgc > 1) {
		if(!strcmp(pArgs[1], "CREATE")) {
			// Create the variable global structure on disk
			if(CreateGlobalStructure(lVarName, lLockName)) {
				printf("SUCCESS\n");
			}
			else {
				printf("ERROR\n");
			}
		}
	}
	else {

		const char *lQuery = getenv("QUERY_STRING");

		if(lQuery != NULL) {
			char lOp[12];

			if(sscanf(lQuery, "=%11s", lOp) == 1) {

				// Open the data structure

				if() {
					IRState lState;

					if(!strcmp(lOp, "REFRESH")) {

					}
					else if(!strcmp(lOp, "ADD_CHAT")) {

					}
					else if(!strcmp(lOp, "ADD_USER")) {

					}
					else if(!strcmp(lOp, "ADD_GAME")) {

					}
					else if(!strcmp(lOp, "JOIN_GAME")) {

					}
					else if(!strcmp(lOp, "DEL_GAME")) {

					}
					// Close the variable
				}
			}
		}
	}
	return 0;
}

BOOL CreateGlobalStructure(const char *pVarName, const char *pLockName)
{
	BOOL lReturnValue = FALSE;

	unlink(pVarName);
	unlink(pLockName);

	int lFile = creat(pLockName, _S_IREAD);

	if(lFile != -1) {
		lReturnValue = TRUE;
		close(lFile);
	}

	if(CreateLock(pLockName)) {
		FILE lFile = fopen(pVarName, "wb");

		if(lFile != NULL) {
			IRState lState;

			lState.Clean();
			lState.Write(lFile);
			fclose(lFile);

			unlink(pLockName);

			lReturnValue = TRUE;
		}
	}
	return lReturnValue;
}

BOOL CreateLock(const char *pLockName)
{
	BOOL lReturnValue = FALSE;

	time_t lEndTime = time(NULL) + 5;

	while(!lReturnValue && (time(NULL) < lEndTime)) {
		int lFile = creat(pLockName, _S_IREAD);

		if(lFile != -1) {
			lReturnValue = TRUE;
			close(lFile);
		}
	}
	return lReturnValue;
}

static void PostSystemRebootTest(const char *pVarName, const char *pLockName)
{
	time_t lCurrentTime = time(NULL);
	struct stat lStat;

	if(stat(pVarName, lStat) == 0) {
		if((lStat.st_mtime + 90) < lCurrentTime) {
			int lFile = creat(pLockName, _S_IREAD);

			if(lFile != -1) {
				// Ok everything is clean, we can clean the existing structure

			}
			else {
				// A lock exist, verify if this lock is enough old
				if(stat(pLockName, lStat) == -1) {

				}
			}

			if(stat(pVarName, lStat) == -1) {

			}
			CreateGlobalStructure(pVarName, pLockName);
		}
	}
	else {
		// Structure must be manually created

	}
}
