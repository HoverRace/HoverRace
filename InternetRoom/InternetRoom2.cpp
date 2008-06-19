// InternetRoom2.cpp

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
// Need some adjustements to run on Win32..specially if using IPC
//
// This is a CGI application
//
// Query form (version 1)
//   URL?=REFRESH USER_ID TIME_STAMP
//   URL?=ADD_CHAT USER_ID MESSAGE
//   URL?=ADD_USER MAJOR-MINORID VERSION KEY2 KEY3 ALIAS
//   URL?=ADD_GAME USER_ID GAME_NAME TRACK_NAME NBLAP WEAPON PORT   note IP-ADDR fetch from REMOTE_ADDR env
//   URL?=JOIN_GAME GAME_ID USER_ID
//   URL?=DEL_GAME GAME_ID USER_ID
//   URL?=LEAVE_GAME GAME_ID USER_ID
//   URL?=DEL_USER USER_ID
//   URL?=START_GAME GAME_ID USER_ID
//   URL?=MESSAGE USER_ID TIME_LEFT:MINUTE_LEFT Message
//
//   USER_ID have the form INT-UNSIGNED  (INDEX-UNIQUE_ID)
//   GAME_ID have the form INT-UNSIGNED  (INDEX-UNIQUE_ID)
//
// ReturnValue have the followinf format
//    SUCCESS\nDATA
//    or
//    ERROR ERROR_CODE\n
//
// ERROR_CODE
//   100 Unable to add user??
//   101 No more Shareware user allowed
//   102 No more user allowed
//   103 Incompatible version
//   104 Expired key
//   105 Already used key  (report for investigation)
//   106 Reserved Alias name (choose an other one)
//
//   200 Unable to send refresh info ??
//   201 Not on-line
//
//   300 Unable to add chat message ???
//   301 Not on-line
//
//   400 Unable to add game ???
//   401 Not on-line
//   402 No more entry avail
//
//   500 Unable to join Game
//   501 Not On-Line
//   502 Game no more availlable
//   503 Game Full
//
//   600 Unable to delete Game
//   601 Not on-line
//   602 Game do not exist
//   603 Not Owner
//
//   700 Unable to leave game ???
//   701 Not on-line
//   702 Game do not exist
//   703 Must join first
//
//   800 Unable to delete user
//   801 Not on-line
//
//   900 Unable to start Game ???
//   901 Not on-line
//   902 Game do not exist
//   903 Not Owner
//
//  1000 Unable to add message
//  1001 Not on-line
//  1002 Not authorized

#define _FAST_CGI_
#define _NO_IPC_
#define _DYN_CONFIG_							  //Read configuration from disk
// #define _EXPIRED_WARNING_
// #define _EXPIRED_
// #define _REG_ONLY_

#ifdef _FAST_CGI_
#include "/usr/local/fcgi/include/fcgi_stdio.h"
#else
// #include <stdio.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>

// Required section to compile on Windows
#ifdef WIN32

#include <direct.h>
#include <io.h>

#ifndef _NO_IPC_

struct sembuf
{
	unsigned short sem_num;
	short sem_op;
	short sem_flg;
};

union semun
{
	int val;
	// struct semid_ds *buf;
	unsigned short *array;
};

int semget(int, int, int);
int semop(int, struct sembuf *, int);
int semctl(int, int, int, ...);
int shmget(int, int, int);
char *shmat(int, char *, int);
int shmdt(char *);
int shmctl(int, int, void *);
int fork();
int sleep(int);

#define IPC_CREAT 1024
#define IPC_EXCL  2048
#define IPC_RMID  4096
#define SETVAL 16
#endif

#else

#ifndef _NO_IPC_

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#endif

#include <unistd.h>
#endif

#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE -1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// BETA ROOM
// #define IR_SEM_KEY         71327
// #define IR_SHMEM_KEY       71327

// Main Room
#define IR_SEM_KEY         71329
#define IR_SHMEM_KEY       71329

// VIP Room
// #define IR_SEM_KEY         71330
// #define IR_SHMEM_KEY       71330

// Ladder Room
// #define IR_SEM_KEY         71331
// #define IR_SHMEM_KEY       71331

// French Room
// #define IR_SEM_KEY         71332
// #define IR_SHMEM_KEY       71332

#define IR_USER_NAME_LEN     20
#define IR_USER_KEY_LEN      20
#define IR_GAME_NAME_LEN     32
#define IR_TRACK_NAME_LEN    32
#define IR_CHAT_MESSAGE_LEN 200
#define IR_MAX_PLAYER_GAME    8

#define IR_MAX_CLIENT        100
#define IR_MAX_GAME           20
#define IR_MAX_CHAT_MESSAGE   22
#define IR_MAX_BANNED         10

#define IR_EXPIRATION_DELAY   50
#define IR_CLEANUP_DELAY      10

#define IR_VERSION_MIN         1
#define IR_VERSION_MAX         1

#define IR_MAX_USER_IP        16
#define IR_MAX_USER_DOM       20

#define IR_BANNEDSLOT       1000

struct UserID
{
	int mMajor;
	int mMinor;
};

// Hardcoded banned ip
const char *gBannedISP_IP[] =
{
	// "204.244.221", KUJONES
	// "206.12.26",   DOLOMITES
	NULL
};

const struct UserID gPermBannedUser[] =
{
	{-1, -1}
};

# ifdef _DYN_CONFIG_

#define IR_MAX_MONITOR 500

struct UserID gSUList[IR_MAX_MONITOR] =
{
	{-1, -1}
};

#else
const struct UserID gSUList[] =
{
	// {  0,  0 },  // Root account
	{											  // MMC
		1, 1
	},
	{											  // Shredd
		1, 10
	},
	{											  //
		1, 11
	},
	{											  // ThaBone
		1, 20
	},
	{											  // Uncle Dave
		1, 22
	},
	{											  // Nick H
		1, 23
	},
	{											  // Crappy
		1, 25
	},
	{											  // Speedman
		1, 32
	},
	{											  // Maniac
		1, 33
	},
	{											  // DJ
		1, 34
	},
	{											  // Maxim
		1, 37
	},
	{											  // Vera
		1, 38
	},
	{											  // Dozier
		1, 39
	},
	{											  // Gunderson
		1, 53
	},
	{											  // NewSome
		1, 64
	},
	{											  // jenny
		1, 69
	},
	{											  // John F
		1, 74
	},
	{											  // Cary
		1, 85
	},
	{-1, -1}
};
#endif

// Structure definition
class IRState
{
	protected:
		class User
		{
			public:

				int mTimeStamp;
				time_t mExpiration;
				int mId;

				struct UserID mKeyID;
				char mName[IR_USER_NAME_LEN + 1];  // Alias

				// unsigned int  mKey2;
				// unsigned int  mKey3;

				// Loging data
				time_t mLogingTime;
				int mGameCode;
				int mNbRefresh;
				int mNbChat;
				char mIP[IR_MAX_USER_IP + 1];
				char mDomain[IR_MAX_USER_DOM + 1];

		};

		class Game
		{
			public:

				BOOL mUsed;
				BOOL mStarted;
				int mTimeStamp;
				int mId;

				char mName[IR_GAME_NAME_LEN + 1];
				char mTrack[IR_TRACK_NAME_LEN + 1];
				int mNbLap;
				int mNbPlayer;
				int mPlayers[IR_MAX_PLAYER_GAME];
				char mAddr[16];
				unsigned mPort;
				int mWeapon;
		};

		class ChatMessage
		{
			public:
				int mTimeStamp;
				char mData[IR_CHAT_MESSAGE_LEN];
		};

		class BannedIP
		{
			public:
				time_t mExpiration;
				char mIP[IR_MAX_USER_IP + 1];
		};

		time_t mLastCleanUp;
		int mTimeStamp;
		unsigned int mNextUserId;
		unsigned int mNextGameId;

		User mUser[IR_MAX_CLIENT];
		Game mGame[IR_MAX_GAME];
		BannedIP mBannedIP[IR_MAX_BANNED];

		int mChatTail;
		ChatMessage mChatFifo[IR_MAX_CHAT_MESSAGE];

		void PrintState();
		void RefreshUser(int pUser);
		void RemoveUser(int pUser, BOOL pTimeOut);

		BOOL ValidUserId(int pUserIndex, int pUserId) const;
		BOOL ValidGameId(int pGameIndex, int pGameId) const;

	public:

		void Clear();
		void LoadDynConfig();

		void VerifyExpirations();				  // Return TRUE if a modification have been made

		// These actions print a return value

												  // Refresh
		void PrintStateChange(int pPlayerIndex, int pPlayerId, int pTimeStamp);
		void AddChatMessage(int pUserIndex, int pUserId, const char *pMessage);
		void AddMessage(const char *pMessage);
		void BanIP(const char *pIP, int pDuration /*in minutes */ );
		void AddUser(const char *pUserName, int pVersion, int pMajorID, int pMinorID, unsigned int pKey2, unsigned int pKey3);
		void AddGame(const char *pGameName, const char *pTrackName, int pNbLap, int pPlayerIndex, int pPlayerId, const char *pRemoteAddr, unsigned int pPort, int pWeapon);
		void JoinGame(int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId);
		void DeleteGame(int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId);
		void LeaveGame(int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId);
		void DeleteUser(int pUserIndex, int pUserId);
		void StartGame(int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId);

};

// Local variable
FILE *gLogFile = NULL;

#ifdef _NO_IPC_

IRState gGlobalState;
#endif

// Local functions
static int StrMaxCopy(char *pDest, const char *pSrc, int pDestSize);
static BOOL StrCompare(const char *pStr, const char *pToken);
static void Unpad(char *pStr);
static void UnpadQuery(char *pStr);

static BOOL InitLogFile();
static void CloseLogFile();
static const char *GetTime(time_t pTime);

static const char *chk(const char *pSrc);

static void ChatFilter(char *pSrc);

// Implementation

void IRState::Clear()
{
	int lCounter;

	mLastCleanUp = time(NULL);
	mTimeStamp = 1;

	for(lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++) {
		mUser[lCounter].mExpiration = 0;
		mUser[lCounter].mTimeStamp = 0;
	}

	for(lCounter = 0; lCounter < IR_MAX_GAME; lCounter++) {
		mGame[lCounter].mUsed = 0;
		mUser[lCounter].mTimeStamp = 0;
	}

	for(lCounter = 0; lCounter < IR_MAX_BANNED; lCounter++) {
		mBannedIP[lCounter].mExpiration = 0;
	}

	mChatTail = 0;
	mNextUserId = 1;
	mNextGameId = 1;

	for(lCounter = 0; lCounter < IR_MAX_CHAT_MESSAGE; lCounter++) {
		mChatFifo[lCounter].mTimeStamp = 0;
	}

	LoadDynConfig();
}

void IRState::LoadDynConfig()
{
#ifdef _DYN_CONFIG_

	FILE *lFile = fopen("monitors.txt", "r");

	if(lFile != NULL) {
		int lCounter;

		for(lCounter = 0; lCounter < (IR_MAX_MONITOR - 1);) {
			char lBuffer[40];
			if(!fgets(lBuffer, sizeof(lBuffer), lFile)) {
				break;
			}
			else {
				if(sscanf(lBuffer, "%d-%d", &gSUList[lCounter].mMajor, &gSUList[lCounter].mMinor) == 2) {
					lCounter++;
				}
			}
		}

		gSUList[lCounter].mMajor = -1;
		gSUList[lCounter].mMinor = -1;

		fclose(lFile);
	}
#endif
}

BOOL IRState::ValidUserId(int pUserIndex, int pUserId) const
{
	BOOL lReturnValue = FALSE;

	if((pUserIndex >= 0) && (pUserIndex < IR_MAX_CLIENT)) {
		if((mUser[pUserIndex].mExpiration != 0) && (mUser[pUserIndex].mId == pUserId)) {
			lReturnValue = TRUE;
		}
	}
	return lReturnValue;
}

BOOL IRState::ValidGameId(int pGameIndex, int pGameId) const
{
	BOOL lReturnValue = FALSE;

	if((pGameIndex >= 0) && (pGameIndex < IR_MAX_GAME)) {
		if((mGame[pGameIndex].mUsed) && (mGame[pGameIndex].mId == pGameId)) {
			lReturnValue = TRUE;
		}
	}
	return lReturnValue;
}

void IRState::VerifyExpirations()
{
	int lCounter;

	time_t lCurrentTime = time(NULL);

	if((mLastCleanUp + IR_CLEANUP_DELAY) <= lCurrentTime) {
		for(lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++) {
			if((mUser[lCounter].mExpiration != 0) && (mUser[lCounter].mExpiration < lCurrentTime)) {
				RemoveUser(lCounter, TRUE);
			}
		}

		for(lCounter = 0; lCounter < IR_MAX_BANNED; lCounter++) {
			if(mBannedIP[lCounter].mExpiration < lCurrentTime) {
				mBannedIP[lCounter].mExpiration = 0;
			}
		}

		mLastCleanUp = lCurrentTime;
	}
}

void IRState::BanIP(const char *pIP, int pDuration /*in minutes */ )
{
	int lEntry = -1;

	// Verify that the banned IP contains 2 dots
	const char *lDot = strchr(pIP, '.');

	if(lDot != NULL) {

		lDot = strchr(pIP, '.');

		if(lDot != NULL) {
			if(pDuration > 60) {
				pDuration = 60;
			}
			// Find an empty entry or verify if the current entry do not already exist
			for(int lCounter = 0; lCounter < IR_MAX_BANNED; lCounter++) {
				if(mBannedIP[lCounter].mExpiration != 0) {
					if(!strcmp(mBannedIP[lCounter].mIP, pIP)) {
						lEntry = lCounter;
					}
				}
				else if(lEntry == -1) {
					lEntry = lCounter;
				}
			}

			if(lEntry != -1) {
				if(pDuration <= 0) {
					mBannedIP[lEntry].mExpiration = 0;
				}
				else {
					StrMaxCopy(mBannedIP[lEntry].mIP, pIP, IR_MAX_USER_IP + 1);
					mBannedIP[lEntry].mExpiration = time(NULL) + pDuration * 60 - 15;
				}
			}
		}
	}
}

void IRState::RemoveUser(int pUser, BOOL pExpired)
{
	mUser[pUser].mExpiration = 0;
	mUser[pUser].mTimeStamp = mTimeStamp;

	// Log this user entry
	if(InitLogFile()) {
		fprintf(gLogFile, "%s %d %c %d %d %d ^%s^(%d-%d) %s %s\n", GetTime(mUser[pUser].mLogingTime), time(NULL) - mUser[pUser].mLogingTime, pExpired ? 'o' : 't', mUser[pUser].mGameCode, mUser[pUser].mNbRefresh, mUser[pUser].mNbChat, mUser[pUser].mName, mUser[pUser].mKeyID.mMajor, mUser[pUser].mKeyID.mMinor, mUser[pUser].mIP, mUser[pUser].mDomain);
	}

	// Verify the games that the user take parts to
	for(int lGame = 0; lGame < IR_MAX_GAME; lGame++) {
		if(mGame[lGame].mUsed) {
			if(mGame[lGame].mPlayers[0] == pUser) {
				mGame[lGame].mUsed = FALSE;
				mGame[lGame].mTimeStamp = mTimeStamp;
			}
			else {
				for(int lPlayer = 1; lPlayer < mGame[lGame].mNbPlayer; lPlayer++) {
					if(mGame[lGame].mPlayers[lPlayer] == pUser) {
						mGame[lGame].mNbPlayer--;
						mGame[lGame].mTimeStamp = mTimeStamp;

						int lNbLeft = lPlayer - mGame[lGame].mNbPlayer;

						if(lNbLeft > 0) {
							memmove(&(mGame[lGame].mPlayers[lPlayer]), &(mGame[lGame].mPlayers[lPlayer + 1]), sizeof((mGame[lGame].mPlayers[lPlayer])) * lNbLeft);
						}
					}
				}
			}
		}
	}

	mTimeStamp++;
}

void IRState::RefreshUser(int pUser)
{
	time_t lTime = time(NULL);

	mUser[pUser].mExpiration = lTime + IR_EXPIRATION_DELAY;
	mUser[pUser].mNbRefresh++;

}

void IRState::PrintState()
{
	int lCounter;

	// Print current time-stamp
	printf("TIME_STAMP %d\n", mTimeStamp);

	// Send the users list
	for(lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++) {
		if(mUser[lCounter].mExpiration != 0) {
			printf("USER %d NEW\n", lCounter);
			printf("%d-%d\n", mUser[lCounter].mKeyID.mMajor, mUser[lCounter].mKeyID.mMinor);
			if(mUser[lCounter].mKeyID.mMajor == -1) {
				printf("%s(nonreg.)\n", mUser[lCounter].mName, mUser[lCounter].mIP);
			}
			else {
				printf("%s\n", mUser[lCounter].mName);
			}
		}
	}

	// Send the game list
	for(lCounter = 0; lCounter < IR_MAX_GAME; lCounter++) {
		if(mGame[lCounter].mUsed) {
			printf("GAME %d NEW %u\n", lCounter, mGame[lCounter].mId);
			printf("%s\n", mGame[lCounter].mName);
			printf("%s\n", mGame[lCounter].mTrack);
			printf("%s\n", mGame[lCounter].mAddr);
			printf("%u %d %d %d\n", mGame[lCounter].mPort, mGame[lCounter].mNbLap, mGame[lCounter].mWeapon, mGame[lCounter].mNbPlayer);

			for(int lPlayer = 0; lPlayer < mGame[lCounter].mNbPlayer; lPlayer++) {
				printf("%d ", mGame[lCounter].mPlayers[lPlayer]);
			}
			printf("\n");
		}
	}

	// Add some custom chat message

#ifdef _EXPIRED_WARNING_
	printf("CHAT\n--\n");						  // just a line to make things more clear
	printf("CHAT\nThis HoverRace version is now expired\n");
	printf("CHAT\nPlease download the new version from www.grokksoft.com\n");
	printf("CHAT\n--\n");						  // just a line to make things more clear

#else

	/*
	   // Compute time since last game and time to next game
	   // work in gmt
	   time_t lCurrentTime = time( NULL );

	   struct tm* lTime = gmtime( &lCurrentTime );

	   int  lToNext;
	   int  lFromLast;

	   int  lThisWeek =   lTime->tm_wday*86400
	   +lTime->tm_hour*3600
	   +lTime->tm_min*60
	   +lTime->tm_sec;

	   for( lCounter = 7; lCounter >0; lCounter-- )
	   {
	   if( lThisWeek > lTestTime[ lCounter ] )
	   {
	   break;
	   }
	   }

	   lFromLast   = lThisWeek-lTestTime[ lCounter ];
	   lToNext     = lTestTime[ lCounter+1 ]-lThisWeek;

	   if( lFromLast < (120*60) )
	   {
	   printf( "CHAT\nLast testing session started %d minutes ago\n", lFromLast/60 );
	   }
	   else
	   {
	   printf( "CHAT\n--\n" ); // just a line to make things more clear
	   }

	   if( lFromLast > 2*60 )
	   {
	   printf( "CHAT\nNext beta testing is %s\n", lTestTimeStr[ lCounter+1] );
	   printf( "CHAT\n(please come back in %d HOURS and %d MINUTES)\n", lToNext/3600, (lToNext%3600)/60 );
	   }
	 */
#endif

	// Send the chat messages
	/* Old messages are not sended
	   for( lCounter = 0; lCounter < IR_MAX_CHAT_MESSAGE; lCounter++ )
	   {
	   int lIndex = (lCounter+mChatTail)%IR_MAX_CHAT_MESSAGE;

	   if( mChatFifo[lIndex].mTimeStamp != 0 )
	   {
	   printf( "CHAT\n%s\n", mChatFifo[ lIndex ].mData );
	   }
	   }
	 */

}

void IRState::PrintStateChange(int pPlayerIndex, int pPlayerId, int pTimeStamp)
{

	if(!ValidUserId(pPlayerIndex, pPlayerId)) {
		if(pPlayerIndex == IR_BANNEDSLOT) {
			// prevent the user from deloging..and loosing the important message
			printf("SUCCESS\n");
		}
		else {
			printf("ERROR 201\n");
		}
	}
	else {
		int lCounter;

		RefreshUser(pPlayerIndex);

		printf("SUCCESS\n");

		if(pTimeStamp == mTimeStamp) {
			// notting changed
			return;
		}
		// Print current time-stamp
		printf("TIME_STAMP %d\n", mTimeStamp);

		// Send the users that have been modified
		for(lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++) {
			if((mUser[lCounter].mTimeStamp != 0) && (mUser[lCounter].mTimeStamp >= pTimeStamp)) {
				if(mUser[lCounter].mExpiration == 0) {
					// Expired or deleted
					printf("USER %d DEL\n", lCounter);
				}
				else {
					printf("USER %d NEW\n", lCounter);
					printf("%d-%d\n", mUser[lCounter].mKeyID.mMajor, mUser[lCounter].mKeyID.mMinor);

					if(mUser[lCounter].mKeyID.mMajor == -1) {
						printf("%s(nonreg.)\n", mUser[lCounter].mName, mUser[lCounter].mIP);
					}
					else {
						printf("%s\n", mUser[lCounter].mName);
					}
				}
			}
		}

		// Send the games that have been modified
		for(lCounter = 0; lCounter < IR_MAX_GAME; lCounter++) {
			if((mGame[lCounter].mTimeStamp != 0) && (mGame[lCounter].mTimeStamp >= pTimeStamp)) {
				if(!mGame[lCounter].mUsed) {
					// Expired or deleted
					printf("GAME %d DEL\n", lCounter);
				}
				else {
					printf("GAME %d NEW %u\n", lCounter, mGame[lCounter].mId);
					printf("%s\n", mGame[lCounter].mName);
					printf("%s\n", mGame[lCounter].mTrack);
					printf("%s\n", mGame[lCounter].mAddr);
					printf("%u %d %d %d\n", mGame[lCounter].mPort, mGame[lCounter].mNbLap, mGame[lCounter].mWeapon, mGame[lCounter].mNbPlayer);

					for(int lPlayer = 0; lPlayer < mGame[lCounter].mNbPlayer; lPlayer++) {
						printf("%d ", mGame[lCounter].mPlayers[lPlayer]);
					}
					printf("\n");
				}
			}
		}

		// Send the new chat messages
		for(lCounter = 0; lCounter < IR_MAX_CHAT_MESSAGE; lCounter++) {
			int lIndex = (lCounter + mChatTail) % IR_MAX_CHAT_MESSAGE;

			if((mChatFifo[lIndex].mTimeStamp != 0) && (mChatFifo[lIndex].mTimeStamp >= pTimeStamp)) {
				printf("CHAT\n*%s\n", mChatFifo[lIndex].mData);
			}
		}
	}
}

/*
// For the future
void IRState::AddSpecialMessage( pMessage )
{
   mChatFifo[ mChatTail ].mTimeStamp = mTimeStamp;

   StrMaxCopy( mChatFifo[ mChatTail ].mData, pMessage, IR_CHAT_MESSAGE_LEN );

   mChatTail = (mChatTail+1)%IR_MAX_CHAT_MESSAGE;

   mTimeStamp++;
}
*/

void IRState::AddChatMessage(int pUserIndex, int pUserId, const char *pMessage)
{
	char lBuffer[200];

	if(!ValidUserId(pUserIndex, pUserId)) {
		printf("ERROR 301");
	}
	else {
		printf("SUCCESS\n");

		if(pMessage[0] == '/') {
			// verify if the user have super user rights
			if((mUser[pUserIndex].mExpiration != 0)
			&& (mUser[pUserIndex].mKeyID.mMajor != -1)) {
				BOOL lOK = FALSE;
				int lBanDuration = 2;			  // Defauls; ban for 2 minutes
				const char *lUser;

				BOOL lKick = FALSE;

				if(!strncmp(pMessage, "/kick ", 6)) {
					lKick = TRUE;
					lUser = pMessage + 6;
				}
				else if(sscanf(pMessage, "/lban %d", &lBanDuration) == 1) {
					lUser = strchr(pMessage + 6, ' ');

					if(lUser != NULL) {
						lUser = lUser + 1;
						lKick = TRUE;
					}
				}

				if(lKick) {
					if(mUser[pUserIndex].mKeyID.mMajor == 0) {
						lOK = TRUE;
					}
					else {
						int lSUIndex = 0;

						while(!lOK && (gSUList[lSUIndex].mMajor != -1)) {
							if((gSUList[lSUIndex].mMajor == mUser[pUserIndex].mKeyID.mMajor)
							&& (gSUList[lSUIndex].mMinor == mUser[pUserIndex].mKeyID.mMinor)) {
								lOK = TRUE;
							}
							lSUIndex++;
						}
					}

					if(lOK) {

						BOOL lUserFound = FALSE;

						int lToBanMajor;
						int lToBanMinor;

						if(sscanf(lUser, "%u-%u", &lToBanMajor, &lToBanMinor) != 2) {
							lToBanMajor = -2;
							lToBanMinor = -2;
						}
						// find the user to kick (start from the end this way there is more chance to catch the good one(not always true)
						for(int lCounter = IR_MAX_CLIENT - 1; !lUserFound && (lCounter >= 0); lCounter--) {
							if((mUser[lCounter].mTimeStamp != 0) && (mUser[lCounter].mExpiration != 0)) {
								if(!strcmp(mUser[lCounter].mIP, lUser) || !strcmp(mUser[lCounter].mName, lUser) || ((mUser[lCounter].mKeyID.mMajor == lToBanMajor) && (mUser[lCounter].mKeyID.mMinor == lToBanMinor))
								) {
									lUserFound = TRUE;

									// Kick and quit
									if(InitLogFile()) {
										fprintf(gLogFile, "KICK %s(%d-%d) by  %s(%d-%d) for %d min %s\n", mUser[lCounter].mName, mUser[lCounter].mKeyID.mMajor, mUser[lCounter].mKeyID.mMinor, mUser[pUserIndex].mName, mUser[pUserIndex].mKeyID.mMajor, mUser[pUserIndex].mKeyID.mMinor, lBanDuration, mUser[lCounter].mIP);
									}

									if(mUser[lCounter].mKeyID.mMajor == 0) {

									}
									else {
										sprintf(lBuffer, "%s have been kicked by %s(%d-%d)", mUser[lCounter].mName, mUser[pUserIndex].mName, mUser[pUserIndex].mKeyID.mMajor, mUser[pUserIndex].mKeyID.mMinor);

										AddMessage(lBuffer);

										// Ban the correspoiding IP
										BanIP(mUser[lCounter].mIP, lBanDuration);

										// Ban the user for a
										RemoveUser(lCounter, TRUE);
									}
									break;
								}
							}
						}

						if(!lUserFound) {
							// well maybe it is just an IP to BAN
							if(InitLogFile()) {
								fprintf(gLogFile, "KICK %s by  %s(%d-%d) for %d min\n", lUser, mUser[pUserIndex].mName, mUser[pUserIndex].mKeyID.mMajor, mUser[pUserIndex].mKeyID.mMinor, lBanDuration);
							}

							sprintf(lBuffer, "%s have been kicked by %s(%d-%d)", lUser, mUser[pUserIndex].mName, mUser[pUserIndex].mKeyID.mMajor, mUser[pUserIndex].mKeyID.mMinor);

							AddMessage(lBuffer);

							// Ban the correspoiding IP
							BanIP(lUser, lBanDuration);

						}
					}
				}
			}
		}
		else {

			int lLen = 0;

			mChatFifo[mChatTail].mTimeStamp = mTimeStamp;

			if(mUser[pUserIndex].mExpiration != 0) {
				lLen += StrMaxCopy(mChatFifo[mChatTail].mData, mUser[pUserIndex].mName, IR_CHAT_MESSAGE_LEN / 4);

				/*
				   mChatFifo[ mChatTail ].mData[ lLen++ ] = '(';

				   if( mUser[ pUserIndex ].mKeyID.mMajor == -1 )
				   {
				   lLen += StrMaxCopy( mChatFifo[ mChatTail ].mData+lLen, mUser[ pUserIndex ].mIP , IR_CHAT_MESSAGE_LEN/4 );
				   }
				   else
				   {
				   lLen += sprintf( mChatFifo[ mChatTail ].mData+lLen, "%d-%d", mUser[ pUserIndex ].mKeyID.mMajor, mUser[ pUserIndex ].mKeyID.mMinor );
				   }
				   mChatFifo[ mChatTail ].mData[ lLen++ ] = ')';
				 */

				if(mUser[pUserIndex].mKeyID.mMajor == -1) {
					mChatFifo[mChatTail].mData[lLen++] = '(';
					lLen += StrMaxCopy(mChatFifo[mChatTail].mData + lLen, mUser[pUserIndex].mIP, IR_CHAT_MESSAGE_LEN / 4);
					mChatFifo[mChatTail].mData[lLen++] = ')';
				}

				mChatFifo[mChatTail].mData[lLen++] = '>';
				mChatFifo[mChatTail].mData[lLen++] = ' ';
			}

			StrMaxCopy(mChatFifo[mChatTail].mData + lLen, pMessage, IR_CHAT_MESSAGE_LEN - lLen);

			mChatTail = (mChatTail + 1) % IR_MAX_CHAT_MESSAGE;

			mTimeStamp++;

			mUser[pUserIndex].mNbChat++;
		}
	}
}

void IRState::AddMessage(const char *pMessage)
{
	mChatFifo[mChatTail].mTimeStamp = mTimeStamp;

	StrMaxCopy(mChatFifo[mChatTail].mData, pMessage, IR_CHAT_MESSAGE_LEN);

	mChatTail = (mChatTail + 1) % IR_MAX_CHAT_MESSAGE;
	mTimeStamp++;
}

void IRState::AddUser(const char *pUserName, int pVersion, int pMajorID, int pMinorID, unsigned int pKey2, unsigned int pKey3)
{
	char lUserName[60];

	// First verify the version
	if((pVersion < IR_VERSION_MIN) || (pVersion > IR_VERSION_MAX)) {
		printf("ERROR 103\n");
	}
	else {
		int lCounter;

		// Verify that the alias have not already been chosen
		for(int lAliasIndex = 0; lAliasIndex < 10; lAliasIndex++) {

			if(lAliasIndex == 0) {
				strncpy(lUserName, pUserName, sizeof(lUserName) - 1);
			}
			else {
				sprintf(lUserName, "%0.30s%d", pUserName, lAliasIndex);
			}
			lUserName[sizeof(lUserName) - 1] = 0;

			for(lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++) {
				if((mUser[lCounter].mTimeStamp != 0) && (mUser[lCounter].mExpiration != 0)) {
					if(!strcmp(lUserName, mUser[lCounter].mName)) {
						break;
					}
				}
			}

			if(lCounter == IR_MAX_CLIENT) {
				break;
			}
		}

		// find an empty entry for the new user
		for(lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++) {
			if(mUser[lCounter].mExpiration == 0) {
				break;
			}
		}

		if(lCounter == IR_MAX_CLIENT) {
			printf("ERROR 102\n");
		}
		else {
			int lBannedDuration;
			BOOL lTempBanned = FALSE;
			BOOL lBanned = FALSE;
			BOOL lExpired = FALSE;
			BOOL lDoubleLog = FALSE;
			BOOL lNotReg = FALSE;

			int lIPIndex = 0;

			// Verify that the ISP is not Banned
			if(pMajorID == -1) {
				while(!lBanned && (gBannedISP_IP[lIPIndex] != NULL)) {
					if(!strncmp(gBannedISP_IP[lIPIndex], chk(getenv("REMOTE_ADDR")), strlen(gBannedISP_IP[lIPIndex]))) {
						lBanned = TRUE;
					}
					lIPIndex++;
				}
			}
			// Verify that user is not temporary banned
			if(!lBanned) {
				for(int lCounter = 0; lCounter < IR_MAX_BANNED; lCounter++) {
					if(mBannedIP[lCounter].mExpiration > 0) {
						if(!strncmp(mBannedIP[lCounter].mIP, chk(getenv("REMOTE_ADDR")), strlen(mBannedIP[lCounter].mIP))) {
							lTempBanned = TRUE;
							lBannedDuration = ((mBannedIP[lCounter].mExpiration - time(NULL)) / 60) + 1;

							if(lBannedDuration < 1) {
								lBannedDuration = 1;
							}
						}
					}
				}
			}
#ifdef _REG_ONLY_
			if(pMajorID == -1) {
				lNotReg = TRUE;
			}
#endif

			// Verify if the user is not already logued
			if(pMajorID != -1) {
				// Go thru user list
				for(int lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++) {
					if((mUser[lCounter].mTimeStamp != 0) && (mUser[lCounter].mExpiration != 0)) {
						if((mUser[lCounter].mKeyID.mMajor == pMajorID) && (mUser[lCounter].mKeyID.mMinor == pMinorID)) {
							lDoubleLog = TRUE;
						}
					}
				}
			}

#ifdef _EXPIRED_
			lExpired = TRUE;
#endif

			if(!lTempBanned && !lBanned && !lExpired && !lDoubleLog && !lNotReg) {
				mUser[lCounter].mTimeStamp = mTimeStamp;
				mUser[lCounter].mExpiration = time(NULL) + IR_EXPIRATION_DELAY;
				mUser[lCounter].mId = mNextUserId++;
				mUser[lCounter].mKeyID.mMajor = pMajorID;
				mUser[lCounter].mKeyID.mMinor = pMinorID;
				// mUser[lCounter].mKey2                 = pKey2;
				// mUser[lCounter].mKey3                 = pKey3;

				StrMaxCopy(mUser[lCounter].mName, lUserName, IR_USER_NAME_LEN);
				mTimeStamp++;

				// Fill loging fields
				mUser[lCounter].mLogingTime = time(NULL);
				mUser[lCounter].mGameCode = 0;
				mUser[lCounter].mNbRefresh = 0;
				mUser[lCounter].mNbChat = 0;
				StrMaxCopy(mUser[lCounter].mIP, chk(getenv("REMOTE_ADDR")), IR_MAX_USER_IP);
				StrMaxCopy(mUser[lCounter].mDomain, chk(getenv("REMOTE_HOST")), IR_MAX_USER_DOM);

				printf("SUCCESS\n", lCounter);
				printf("USER_ID %d-%u\n", lCounter, mUser[lCounter].mId);

				PrintState();

				// Monitor Warning
				// Verify if a monitor
				if(pMajorID != -1) {
					int lSUIndex = 0;

					while(gSUList[lSUIndex].mMajor != -1) {
						if((gSUList[lSUIndex].mMajor == pMajorID)
						&& (gSUList[lSUIndex].mMinor == pMinorID)) {
							printf("CHAT\n--\n");
							printf("CHAT\nYou are a monitor\n");
							printf("CHAT\nBe careful with the kick feature because it now suspend user access for 2 minutes\n");
							printf("CHAT\nThose who abuse will lose there right to kick ppl out\n");
							printf("CHAT\n--\n");

							break;
						}
						lSUIndex++;
					}
				}

			}
			else {
				// We got one
				printf("SUCCESS\n", lCounter);
				printf("USER_ID %d-%u\n", IR_BANNEDSLOT, 0);

				if(lExpired) {
					printf("CHAT\n--\n");
					printf("CHAT\nThis version of HoverRace is expired\n");
					printf("CHAT\nPlease download the new version from www.grokksoft.com\n");
					printf("CHAT\n--\n");

					// Write this to the LOG
					if(InitLogFile()) {
						fprintf(gLogFile, "EXPIRED ^%s^ %d-%d %s\n", pUserName, pMajorID, pMinorID, chk(getenv("REMOTE_ADDR"))
							);
					}
				}
				else if(lTempBanned) {
					printf("CHAT\n--\n");
					printf("CHAT\nYou have been temporary banned by a monitor\n");
					printf("CHAT\nYou will be allowed to come back to the room in %d minute%s\n", lBannedDuration, (lBannedDuration > 1) ? "s" : "");
					printf("CHAT\nPlease report any monitor abuse to richard@grokksoft.com\n");

					// Write this to the LOG
					if(InitLogFile()) {
						fprintf(gLogFile, "TEMPBANNED ^%s^ %s\n", pUserName, chk(getenv("REMOTE_ADDR"))
							);
					}
				}

				else if(lBanned) {
					printf("CHAT\nYour ISP have been temporary banned from this gameroom because one of there customer was causing troubles\n");
					printf("CHAT\nWe have informed your ISP and we will reenable your access as soon as they will have corrected the situation\n");
					printf("CHAT\nYou can communicate with them to accelerate the process. Sorry for the inconvenients\n");
					printf("CHAT\nYou can also communicate with us support@grokksoft.com to get a special code that will let you access the game\n");
					printf("CHAT\nAlso, If you register the game you will be able to play since we can track registred users\n");

					// Write this to the LOG
					if(InitLogFile()) {
						fprintf(gLogFile, "BANNED ^%s^ %s\n", pUserName, chk(getenv("REMOTE_ADDR"))
							);
					}
				}
				else if(lDoubleLog) {
					printf("CHAT\n--\n");
					printf("CHAT\nYour can not login because our database indicates that you are already logued in\n");
					printf("CHAT\nThis error may due to an error network. Retry to connect in a minute.\n");
					printf("CHAT\nIf it still dont works please contact support@grokksoft.com\n");

					// Write this to the LOG
					if(InitLogFile()) {
						fprintf(gLogFile, "DOUBLELOG ^%s^ %d-%d %s\n", pUserName, pMajorID, pMinorID, chk(getenv("REMOTE_ADDR"))
							);
					}
				}
				else if(lNotReg) {

					printf("CHAT\n--\n");
					printf("CHAT\nSorry\n");
					printf("CHAT\nYou must register to have access to this room\n");
					printf("CHAT\n--\n");

					// Write this to the LOG
					if(InitLogFile()) {
						fprintf(gLogFile, "UNREG ^%s^ %s\n", pUserName, chk(getenv("REMOTE_ADDR"))
							);
					}
				}
			}
		}
	}
}

void IRState::AddGame(const char *pGameName, const char *pTrackName, int pNbLap, int pPlayerIndex, int pPlayerId, const char *pRemoteAddr, unsigned int pPort, int pWeapon)
{

	// Verify the the user is valid
	if(!ValidUserId(pPlayerIndex, pPlayerId)) {
		printf("ERROR 401\n");
	}
	else {
		int lCounter;

		// find an empty entry for the new game
		for(lCounter = 0; lCounter < IR_MAX_GAME; lCounter++) {
			if(!mGame[lCounter].mUsed) {
				break;
			}
		}

		if(lCounter == IR_MAX_GAME) {
			printf("ERROR 402\n");
		}
		else {
			mGame[lCounter].mTimeStamp = mTimeStamp;
			mGame[lCounter].mUsed = TRUE;
			mGame[lCounter].mStarted = FALSE;
			mGame[lCounter].mId = mNextGameId++;
			mGame[lCounter].mNbLap = pNbLap;
			mGame[lCounter].mNbPlayer = 1;
			mGame[lCounter].mPlayers[0] = pPlayerIndex;
			mGame[lCounter].mWeapon = pWeapon;
			mGame[lCounter].mPort = pPort;

			StrMaxCopy(mGame[lCounter].mName, pGameName, IR_GAME_NAME_LEN);
			StrMaxCopy(mGame[lCounter].mTrack, pTrackName, IR_TRACK_NAME_LEN);
			StrMaxCopy(mGame[lCounter].mAddr, pRemoteAddr, sizeof(mGame[lCounter].mAddr));

			printf("SUCCESS\nGAME_ID %d-%u\n", lCounter, mGame[lCounter].mId);
			mTimeStamp++;

			mUser[pPlayerIndex].mGameCode = 1;

		}
	}
}

void IRState::JoinGame(int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId)
{

	if(!ValidUserId(pPlayerIndex, pPlayerId)) {
		printf("ERROR 501\n");
	}
	else if(!ValidGameId(pGameIndex, pGameId)) {
		printf("ERROR 502\n");
	}
	else if((mGame[pGameIndex].mUsed) && (mGame[pGameIndex].mNbPlayer >= IR_MAX_PLAYER_GAME)) {
		printf("ERROR 503\n");
	}
	else {
		mGame[pGameIndex].mTimeStamp = mTimeStamp;
		mGame[pGameIndex].mPlayers[mGame[pGameIndex].mNbPlayer++] = pPlayerIndex;

		printf("SUCCESS\n");
		mTimeStamp++;

		mUser[pPlayerIndex].mGameCode = 2;

	}
}

void IRState::DeleteGame(int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId)
{
	if(!ValidUserId(pPlayerIndex, pPlayerId)) {
		printf("ERROR 601\n");
	}
	else if(!ValidGameId(pGameIndex, pGameId)) {
		printf("ERROR 602\n");
	}
	else if(mGame[pGameIndex].mPlayers[0] != pPlayerIndex) {
		printf("ERROR 603\n");
	}
	else {
		printf("SUCCESS\n");

		mGame[pGameIndex].mTimeStamp = mTimeStamp;
		mGame[pGameIndex].mUsed = FALSE;
		mTimeStamp++;

		mUser[pPlayerIndex].mGameCode = 3;

	}
}

void IRState::LeaveGame(int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId)
{
	if(!ValidUserId(pPlayerIndex, pPlayerId)) {
		printf("ERROR 701\n");
	}
	else if(!ValidGameId(pGameIndex, pGameId)) {
		printf("ERROR 702\n");
	}
	else {
		int lCounter;

		// Find player index
		for(lCounter = 1; lCounter < mGame[pGameIndex].mNbPlayer; lCounter++) {
			if(mGame[pGameIndex].mPlayers[lCounter] == pPlayerIndex) {
				break;
			}
		}

		if(lCounter == mGame[pGameIndex].mNbPlayer) {
			printf("ERROR 703\n");
		}
		else {
			mGame[pGameIndex].mNbPlayer--;
			mGame[pGameIndex].mTimeStamp = mTimeStamp;

			int lNbLeft = lCounter - mGame[pGameIndex].mNbPlayer;

			if(lNbLeft > 0) {
				memmove(&(mGame[pGameIndex].mPlayers[lCounter]), &(mGame[pGameIndex].mPlayers[lCounter + 1]), sizeof((mGame[pGameIndex].mPlayers[lCounter])) * lNbLeft);
			}

			printf("SUCCESS\n");

			mTimeStamp++;

			mUser[pPlayerIndex].mGameCode = 4;
		}
	}
}

void IRState::DeleteUser(int pUserIndex, int pUserId)
{
	if(!ValidUserId(pUserIndex, pUserId)) {
		printf("ERROR 801\n");
	}
	else {
		printf("SUCCESS\n");
		RemoveUser(pUserIndex, FALSE);
	}
}

void IRState::StartGame(int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId)
{
	if(!ValidUserId(pPlayerIndex, pPlayerId)) {
		printf("ERROR 901\n");
	}
	else if(!ValidGameId(pGameIndex, pGameId)) {
		printf("ERROR 902\n");
	}
	else if(mGame[pGameIndex].mPlayers[0] != pPlayerIndex) {
		printf("ERROR 903\n");
	}
	else {
		printf("SUCCESS\n");

		mGame[pGameIndex].mStarted = TRUE;
		mTimeStamp++;
	}
}

int main(int pArgc, const char **pArgs)
{

#ifdef _NO_IPC_

	gGlobalState.Clear();
	if(InitLogFile()) {
		fprintf(gLogFile, "IMR Init version 0.13.13b %s\n", GetTime(time(NULL)));
	}
#endif

#ifdef _FAST_CGI_
	while(FCGI_Accept() >= 0)
#endif
	{
		BOOL lPrintTitle = TRUE;

		// Send the header required by the server
		printf("Content-type: text/plain%c%c", 10, 10);

		const char *lQueryPtr = getenv("QUERY_STRING");

		if(lQueryPtr == NULL) {
			printf("No parameters\n");
		}
		else {
			char lQuery[4096];
			char lOp[12];

			StrMaxCopy(lQuery, lQueryPtr, 4096);

			UnpadQuery(lQuery);

			if(sscanf(lQuery, "=%11s", lOp) == 1) {
#ifndef _FAST_CGI_
				// Change the local directory
				char *lPathEnd = strrchr(pArgs[0], '/');

				if(lPathEnd != NULL) {
					*lPathEnd = 0;
					chdir(pArgs[0]);
				}
#endif

				if(!strcmp(lOp, "RESET")) {
					printf("RESET OK\n\n");

#ifdef _NO_IPC_
					// gGlobalState.Clear();
					// sleep( 2 );
					// return 0;
					// break;
#else

					int lLock = semget(IR_SEM_KEY, 1, 0777);
					int lMemId = shmget(IR_SHMEM_KEY, sizeof(IRState), 0666);

					if(lLock != -1) {
						union semun lNULL;

						semctl(lLock, 0, IPC_RMID, lNULL);
					}

					if(lMemId != -1) {
						shmctl(lMemId, IPC_RMID, NULL);
					}
#endif

					if(InitLogFile()) {
						fprintf(gLogFile, "IMR Reset %s\n", GetTime(time(NULL)));
					}
#ifdef _FAST_CGI_
					break;
#endif

				}
				else if(!strcmp(lOp, "SET_MESSAGE")) {
				}

				/*
				   else if( !strcmp( lOp, "STICK" ) )
				   {
				   // Fork and leave a child running for 1 hour
				   // NOT USED not TESTED

				   int lCode = fork();
				   if( lCode == 0 )
				   {
				   if( InitLogFile() )
				   {
				   fprintf( gLogFile, "STICK Start %s\n", GetTime( time(NULL)) );
				   CloseLogFile();
				   }

				   close( 0 );
				   close( 1 );
				   close( 2 );
				   sleep( 3600 );

				   if( InitLogFile() )
				   {
				   fprintf( gLogFile, "STICK End %s\n", GetTime( time(NULL)) );
				   CloseLogFile();
				   }

				   }

				   else if( lCode == -1 )
				   {
				   printf( "ERROR\n" );
				   }
				   else
				   {
				   printf( "SUCCESS\n" );
				   }

				   }
				 */

				else {
					IRState *lState = NULL;

#ifdef _NO_IPC_
					lState = &gGlobalState;

#else

					int lLock;					  // Semaphore
					int lMemId;					  // Shared memory

					struct sembuf lSemOp;

					lSemOp.sem_flg = 0;			  //Avoid corruption but must not core-dump SEM_UNDO;  // Risky but prevents dead-lock
					lSemOp.sem_num = 0;

					// First try to create the structure for the first time
					// Lock the data struct
					lLock = semget(IR_SEM_KEY, 1, 0777 | IPC_CREAT | IPC_EXCL);

					if(lLock != -1) {
						union semun lArg;

						// Initialize the newly created semaphore
						lArg.val = 1;

						semctl(lLock, 0, SETVAL, lArg);
					}
					else {
						lLock = semget(IR_SEM_KEY, 1, 0777);
					}

					if(lLock == -1) {
						printf("Unable to get semaphore\n");
					}
					else {
						lSemOp.sem_op = -1;

						if(semop(lLock, &lSemOp, 1) == -1) {
							printf("Unable to decrement semaphore\n");
							lLock = -1;
						}
						else {

							// From here we can work safely

							lMemId = shmget(IR_SHMEM_KEY, sizeof(IRState), 0666 | IPC_CREAT | IPC_EXCL);

							if(lMemId != -1) {
								lState = (IRState *) shmat(lMemId, NULL, 0);

								if((int) lState == -1) {
									lState = NULL;
								}

								if(lState == NULL) {
									printf("Unable to attach shmem\n");
								}
								else {
									lState->Clear();

									if(InitLogFile()) {
										fprintf(gLogFile, "IMR Init %s\n", GetTime(time(NULL)));
									}

								}
							}
							else {
								lMemId = shmget(IR_SHMEM_KEY, sizeof(IRState), 0666);

								if(lMemId == -1) {
									printf("Unable to get shmem\n");
								}
								else {
									lState = (IRState *) shmat(lMemId, NULL, 0);

									if((int) lState == -1) {
										lState = NULL;
									}

									if(lState == NULL) {
										printf("Unable to attach shmem\n");
									}
								}
							}
						}
					}
#endif

					if(lState != NULL) {

						lPrintTitle = FALSE;

						lState->VerifyExpirations();

						if(!strcmp(lOp, "REFRESH")) {
							int lUserIndex;
							int lUserId;
							int lTimeStamp;

							if(sscanf(lQuery, "%*s %d-%u %d", &lUserIndex, &lUserId, &lTimeStamp) == 3) {
								lState->PrintStateChange(lUserIndex, lUserId, lTimeStamp);
							}
						}
						else if(!strcmp(lOp, "ADD_CHAT")) {
							int lUserIndex;
							int lUserId;
							char lChatMessage[200];

							if(sscanf(lQuery, "%*s %d-%u %200s", &lUserIndex, &lUserId, lChatMessage) == 3) {
								Unpad(lChatMessage);
								ChatFilter(lChatMessage);
								lState->AddChatMessage(lUserIndex, lUserId, lChatMessage);
							}
						}
						//   URL?=ADD_USER MAJOR-MINORID VERSION KEY2 KEY3 ALIAS
						else if(!strcmp(lOp, "ADD_USER")) {
							int lMajorID;
							int lMinorID;
							int lVersion;
							unsigned int lKey2;
							unsigned int lKey3;
							char lUserName[40];

#ifdef _EXPIRED_
							lState->AddUser("User", 1, -1, -1, 0, 0);
#else
							if(sscanf(lQuery, "%*s %d-%d %d %d %d %40s", &lMajorID, &lMinorID, &lVersion, &lKey2, &lKey3, lUserName) == 6) {
								Unpad(lUserName);
								ChatFilter(lUserName);
								lState->AddUser(lUserName, lVersion, lMajorID, lMinorID, lKey2, lKey3);
							}
#endif
						}
						// URL?=ADD_GAME USER_ID GAME_NAME TRACK_NAME NBLAP WEAPON PORT
						else if(!strcmp(lOp, "ADD_GAME")) {
							int lUserIndex;
							int lUserId;
							int lNbLap;
							char lGameName[40];
							char lTrackName[40];
							int lWeapon;
							unsigned lPort;

							if(sscanf(lQuery, "%*s %d-%u %40s %40s %d %d %u", &lUserIndex, &lUserId, lGameName, lTrackName, &lNbLap, &lWeapon, &lPort) == 7) {
								const char *lRemoteAddr = getenv("REMOTE_ADDR");

								if((lRemoteAddr != NULL) && (strlen(lRemoteAddr) != 0)) {
									Unpad(lTrackName);
									Unpad(lGameName);
									lState->AddGame(lGameName, lTrackName, lNbLap, lUserIndex, lUserId, lRemoteAddr, lPort, lWeapon);
								}
							}
						}
						else if(!strcmp(lOp, "JOIN_GAME")) {
							int lUserIndex;
							int lUserId;
							int lGameIndex;
							int lGameId;

							if(sscanf(lQuery, "%*s %d-%u %d-%u", &lGameIndex, &lGameId, &lUserIndex, &lUserId) == 4) {
								lState->JoinGame(lGameIndex, lGameId, lUserIndex, lUserId);
							}
						}
						else if(!strcmp(lOp, "DEL_GAME")) {
							int lUserIndex;
							int lUserId;
							int lGameIndex;
							int lGameId;

							if(sscanf(lQuery, "%*s %d-%u %d-%u", &lGameIndex, &lGameId, &lUserIndex, &lUserId) == 4) {
								lState->DeleteGame(lGameIndex, lGameId, lUserIndex, lUserId);
							}
						}
						else if(!strcmp(lOp, "LEAVE_GAME")) {
							int lUserIndex;
							int lUserId;
							int lGameIndex;
							int lGameId;

							if(sscanf(lQuery, "%*s %d-%u %d-%u", &lGameIndex, &lGameId, &lUserIndex, &lUserId) == 4) {
								lState->LeaveGame(lGameIndex, lGameId, lUserIndex, lUserId);
							}
						}
						else if(!strcmp(lOp, "DEL_USER")) {
							int lUserIndex;
							int lUserId;

							if(sscanf(lQuery, "%*s %d-%u", &lUserIndex, &lUserId) == 2) {
								lState->DeleteUser(lUserIndex, lUserId);
							}
						}
						else if(!strcmp(lOp, "START_GAME")) {
							int lUserIndex;
							int lUserId;
							int lGameIndex;
							int lGameId;

							if(sscanf(lQuery, "%*s %d-%u %d-%u", &lGameIndex, &lGameId, &lUserIndex, &lUserId) == 4) {
								lState->StartGame(lGameIndex, lGameId, lUserIndex, lUserId);
							}
						}
						else if(!strcmp(lOp, "LOAD_DYN")) {
							printf("LOAD DYNAMIC OK\n\n");
							lState->LoadDynConfig();
						}
						else {
							lPrintTitle = TRUE;
						}
					}
#ifdef _NO_IPC_
					lState = NULL;
#else
					// Release lock
					if(lLock != -1) {
						lSemOp.sem_op = 1;

						semop(lLock, &lSemOp, 1);

						// Release memory
						if(lState != NULL) {
							shmdt((char *) lState);
						}
					}
#endif

				}
			}
		}

		CloseLogFile();

		if(lPrintTitle) {
			printf("Internet Meeting Room (c)1996,97 GrokkSoft inc.\n");
		}
	}

	return 0;
}

static int StrMaxCopy(char *pDest, const char *pSrc, int pDestSize)
{
	int lCounter;

	for(lCounter = 0; lCounter < pDestSize; lCounter++) {
		if((pDest[lCounter] = pSrc[lCounter]) == 0) {
			break;
		}
	}

	if(pDestSize > 0) {
		pDest[pDestSize - 1] = 0;
	}

	return lCounter;
}

BOOL StrCompare(const char *pStr, const char *pToken)
{
	BOOL lReturnValue = FALSE;

	while((*pToken != 0) && (*pStr != 0) && (toupper(*pStr) == *pToken)) {
		pStr++;
		pToken++;
	}

	if((*pToken == 0)) {
		lReturnValue = TRUE;
	}

	if((*pToken == ' ') && (*pStr == 0)) {
		if(pToken[1] == 0) {
			lReturnValue = TRUE;
		}
	}

	return lReturnValue;
}

void Unpad(char *pStr)
{
	while(*pStr != 0) {
		if(*pStr == '*') {
			*pStr = ' ';
		}
		pStr++;
	}
}

void UnpadQuery(char *pStr)
{
	while(*pStr != 0) {
		if(*pStr == '^') {
			*pStr = ' ';
		}
		pStr++;
	}
}

BOOL InitLogFile()
{
	if(gLogFile == NULL) {
		gLogFile = fopen("log/ir.log", "a");
	}
	return (gLogFile != NULL);
}

void CloseLogFile()
{
	if(gLogFile != NULL) {
		fclose(gLogFile);
		gLogFile = NULL;
	}
}

const char *GetTime(time_t pTime)
{
	static char lTimeBuffer[256];
	time_t lTimer = pTime;

	struct tm *lTime = localtime(&lTimer);

	strcpy(lTimeBuffer, "error");
	strftime(lTimeBuffer, sizeof(lTimeBuffer), "%y-%m-%d_%H:%M:%S", lTime);

	return lTimeBuffer;
}

const char *chk(const char *pSrc)
{
	if(pSrc == NULL) {
		return "null";
	}
	else {
		return pSrc;
	}
}

static const char *gBannedWord[] =
{
	"ASS ",
	"A$$",
	"BALLS",
	"BEATCH",
	"BITCH",
	"BREAST",
	"CYBERSEX",
	"COCK",
	"DICK ",
	"FAG",
	"FUCK",
	"GAY",
	"HAVE SEX",
	"HO ",
	"HOE ",
	"KILL YOU",
	"KILL MY",
	"LICK MY",
	"NIGER",
	"PRICK",
	"PUSSY",
	"RAPE",
	"QUIER",
	"SHIT",
	"SLUT",
	"SUCKER",
	"SUCK MY",
	"SUICIDE",
	"TIT ",
	"TITS",
	"TITEY",
	"TO HELL ",
	NULL
};

void ChatFilter(char *pSrc)
{
	BOOL pPrevIsSpace = TRUE;

	while(*pSrc != 0) {
		if(!isalpha(*pSrc)) {
			pPrevIsSpace = TRUE;
		}
		else if(pPrevIsSpace) {
			int lCounter = 0;

			while(gBannedWord[lCounter] != NULL) {
				if(StrCompare(pSrc, gBannedWord[lCounter])) {
					int lChar;
					int lLenToRemove = strlen(gBannedWord[lCounter]) - 1;

					for(lChar = 0; lChar < lLenToRemove; lChar++) {
						*pSrc = '-';
						pSrc++;
					}
					if(gBannedWord[lCounter][lChar] != ' ') {
						*pSrc = '-';
					}
					else {
						pSrc--;
					}
					break;
				}
				lCounter++;
			}
			pPrevIsSpace = FALSE;
		}
		else {
			pPrevIsSpace = FALSE;
		}
		pSrc++;
	}
}
