// InternetRoom3.cpp

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

// _DEAMON_ version command line
//
// ir.exe <port> <logfile> [-r]
//
//  Please respect parameters order
//  port:    Listening port number
//  logfile: Filename where access are reported (non shared file)
//  -r:      Access restricted to Registered members only
//

#define _WWW_LOCK_

#define _DAEMON_
// #define _FAST_CGI_
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

#ifdef _DAEMON_

#include <winsock.h>
#endif											  // _DEAMON_

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
#endif											  // _!NO_IPC_

#else											  // WIN32 !WIN32

#ifndef _NO_IPC_

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#endif											  // !NO_IPC

#include <unistd.h>
#include <stdarg.h>

#ifdef _DAEMON_

// #include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

#define INVALID_SOCKET -1
#endif											  // _DEAMON_

#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE -1
#endif

#ifndef FALSE
#define FALSE 0
#endif
#endif											  // !WIN32

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

#define IR_MAX_CLIENT         50
#define IR_MAX_GAME           15
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

#ifdef _DYN_CONFIG_

#define IR_MAX_BANNED_IP 50
char gBannedISP_IP[IR_MAX_BANNED_IP + 1][16];

#else
// Hardcoded banned ip

const char *gBannedISP_IP[] =
{
	// "204.244.221", KUJONES
	// "206.12.26",   DOLOMITES
	NULL
};
#endif

const char *gBannedHackers[] =
{
	"193.237.137.184",							  // Danny young
	"158.152.207.56",							  // Danny young
	"194.217.242.154",							  // Paul spirity
	NULL
};

const char *gFreeTracks[] =
{
	"ClassicH",
	"Steeplechase",
	"The Alley2",
	NULL
};

#ifdef _DYN_CONFIG_

#define IR_MAX_ALLOWED_IP 20
char gAllowedISP_IP[IR_MAX_ALLOWED_IP + 1][16];

#else
// Hardcoded banned ip

const char *gAllowedISP_IP[] =
{
	NULL
};
#endif

#ifdef _DYN_CONFIG_

#define IR_MAX_BANNED_USERS 100

struct UserID gPermBannedUser[IR_MAX_BANNED_USERS + 1];

#else
const struct UserID gPermBannedUser[] =
{
	{1, 294},
	{1, 805},
	{-1, -1}
};
#endif

#ifdef _REG_ONLY_
BOOL gRegOnly = TRUE;
#else
BOOL gRegOnly = FALSE;
#endif

# ifdef _DYN_CONFIG_

#define IR_MAX_MONITOR 500

struct UserID gSUList[IR_MAX_MONITOR] =
{
	{-1, -1}
};

#define IR_MAX_INTRO    6

char gIntro[IR_MAX_INTRO][100];

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
				BOOL mTrusted;
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
				int mUserDest;					  // -1 mean any
				int mUserSrc;					  // -1 mean none
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

		void WWWMessageList();
		void WWWUserList();
		void WWWAddMessage(const char *pMessage, const char *pIP);
};

// Local variable
FILE *gLogFile = NULL;
BOOL gFlushFile = FALSE;

#ifdef _DAEMON_

char gLogFileName[100];

#else

const char *gLogFileName = "log/ir.log";
#endif

#ifdef _NO_IPC_

IRState gGlobalState;
#endif

// Local functions
static int StrMaxCopy(char *pDest, const char *pSrc, int pDestSize);
static BOOL StrCompare(const char *pStr, const char *pToken);
static void Unpad(char *pStr);
static void UnpadQuery(char *pStr);
static void WWWUnpad(char *pStr);

static BOOL InitLogFile();
static void CloseLogFile();
static void FlushLogFile();
static const char *GetTime(time_t pTime);

static const char *chk(const char *pSrc);

static void ChatFilter(char *pSrc);

static void Print(const char *pFormat, ...);
static const char *GetRemoteAddr();
static const char *GetRemoteHost();
static unsigned short ComputeIDSum(int pMajorID, int pMinorID);

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
			char lBuffer[80];
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
	else {
		gSUList[0].mMajor = -1;
		gSUList[0].mMinor = -1;
	}

	lFile = fopen("intro.txt", "r");

	// fprintf( stderr, "intro file %d %d %d\n", lFile, IR_MAX_INTRO, sizeof( gIntro[ 0 ] ) );

	if(lFile != NULL) {
		int lCounter = 0;;

		for(lCounter = 0; lCounter < (IR_MAX_INTRO - 1); lCounter++) {
			if(!fgets(gIntro[lCounter], sizeof(gIntro[lCounter]), lFile)) {
				break;
			}
			else {

				char *lEoL = gIntro[lCounter];

				gIntro[lCounter][sizeof(gIntro[lCounter]) - 1] = 0;

				// fprintf( stderr, "intro %s\n", gIntro[ lCounter ] );

				while(*lEoL != 0) {
					if((*lEoL == 10) || (*lEoL == 13)) {
						*lEoL = 0;
						break;
					}
					lEoL++;
				}
			}
		}

		gIntro[lCounter][0] = 0;
		fclose(lFile);
	}

	lFile = fopen("bannedip.txt", "r");

	if(lFile != NULL) {

		int lCounter;

		for(lCounter = 0; lCounter < (IR_MAX_BANNED_IP - 1);) {
			char lBuffer[80];
			if(!fgets(lBuffer, sizeof(lBuffer), lFile)) {
				break;
			}
			else {
				lBuffer[15] = 0;

				if(strlen(lBuffer) > 0) {
					if(sscanf(lBuffer, "%s", gBannedISP_IP[lCounter])) {
						lCounter++;
					}
				}
			}
		}
		gBannedISP_IP[lCounter][0] = 0;

		fclose(lFile);
	}
	else {
		gBannedISP_IP[0][0] = 0;
	}

	lFile = fopen("allowedip.txt", "r");

	if(lFile != NULL) {

		int lCounter;

		for(lCounter = 0; lCounter < (IR_MAX_ALLOWED_IP - 1);) {
			char lBuffer[80];
			if(!fgets(lBuffer, sizeof(lBuffer), lFile)) {
				break;
			}
			else {
				lBuffer[15] = 0;

				if(strlen(lBuffer) > 0) {
					if(sscanf(lBuffer, "%s", gAllowedISP_IP[lCounter])) {
						lCounter++;
					}
				}
			}
		}
		gAllowedISP_IP[lCounter][0] = 0;

		fclose(lFile);
	}
	else {
		gAllowedISP_IP[0][0] = 0;
	}

	lFile = fopen("bannedid.txt", "r");

	if(lFile != NULL) {

		int lCounter;

		for(lCounter = 0; lCounter < (IR_MAX_BANNED_USERS);) {
			char lBuffer[80];
			if(!fgets(lBuffer, sizeof(lBuffer), lFile)) {
				break;
			}
			else {
				if(sscanf(lBuffer, "%d-%d", &gPermBannedUser[lCounter].mMajor, &gPermBannedUser[lCounter].mMinor) == 2) {
					lCounter++;
				}
			}
		}

		gPermBannedUser[lCounter].mMajor = -1;
		gPermBannedUser[lCounter].mMinor = -1;

		fclose(lFile);
	}
	else {
		gPermBannedUser[0].mMajor = -1;
		gPermBannedUser[0].mMinor = -1;
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
			if(pDuration > 120) {
				pDuration = 120;
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
	Print("TIME_STAMP %d\n", mTimeStamp);

	// Send the users list
	for(lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++) {
		if(mUser[lCounter].mExpiration != 0) {
			Print("USER %d NEW\n", lCounter);
			Print("%d-%d\n", mUser[lCounter].mKeyID.mMajor, mUser[lCounter].mKeyID.mMinor);
			if(mUser[lCounter].mKeyID.mMajor == -1) {
				Print("%s(nonreg.)\n", mUser[lCounter].mName, mUser[lCounter].mIP);
				// Print( "%s\n", mUser[ lCounter ].mName,mUser[ lCounter ].mIP );
			}
			else if(!mUser[lCounter].mTrusted) {
				Print("%s(not verified)\n", mUser[lCounter].mName, mUser[lCounter].mIP);
			}
			else {
				Print("%s\n", mUser[lCounter].mName);
			}
		}
	}

	// Send the game list
	for(lCounter = 0; lCounter < IR_MAX_GAME; lCounter++) {
		if(mGame[lCounter].mUsed) {
			Print("GAME %d NEW %u\n", lCounter, mGame[lCounter].mId);
			Print("%s\n", mGame[lCounter].mName);
			Print("%s\n", mGame[lCounter].mTrack);
			Print("%s\n", mGame[lCounter].mAddr);
			Print("%u %d %d %d\n", mGame[lCounter].mPort, mGame[lCounter].mNbLap, mGame[lCounter].mWeapon, mGame[lCounter].mNbPlayer);

			for(int lPlayer = 0; lPlayer < mGame[lCounter].mNbPlayer; lPlayer++) {
				Print("%d ", mGame[lCounter].mPlayers[lPlayer]);
			}
			Print("\n");
		}
	}

	// Add some custom chat message

#ifdef _EXPIRED_WARNING_
	Print("CHAT\n--\n");						  // just a line to make things more clear
	Print("CHAT\nThis HoverRace version is now expired\n");
	Print("CHAT\nPlease download the new version from www.grokksoft.com\n");
	Print("CHAT\n--\n");						  // just a line to make things more clear

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
	   Print( "CHAT\nLast testing session started %d minutes ago\n", lFromLast/60 );
	   }
	   else
	   {
	   Print( "CHAT\n--\n" ); // just a line to make things more clear
	   }

	   if( lFromLast > 2*60 )
	   {
	   Print( "CHAT\nNext beta testing is %s\n", lTestTimeStr[ lCounter+1] );
	   Print( "CHAT\n(please come back in %d HOURS and %d MINUTES)\n", lToNext/3600, (lToNext%3600)/60 );
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
	   Print( "CHAT\n%s\n", mChatFifo[ lIndex ].mData );
	   }
	   }
	 */

}

void IRState::PrintStateChange(int pPlayerIndex, int pPlayerId, int pTimeStamp)
{

	if(!ValidUserId(pPlayerIndex, pPlayerId)) {
		if(pPlayerIndex == IR_BANNEDSLOT) {
			// prevent the user from deloging..and loosing the important message
			Print("SUCCESS\n");
		}
		else {
			Print("ERROR 201\n");
		}
	}
	else {
		int lCounter;

		RefreshUser(pPlayerIndex);

		Print("SUCCESS\n");

		if(pTimeStamp == mTimeStamp) {
			// notting changed
			return;
		}
		// Print current time-stamp
		Print("TIME_STAMP %d\n", mTimeStamp);

		// Send the users that have been modified
		for(lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++) {
			if((mUser[lCounter].mTimeStamp != 0) && (mUser[lCounter].mTimeStamp >= pTimeStamp)) {
				if(mUser[lCounter].mExpiration == 0) {
					// Expired or deleted
					Print("USER %d DEL\n", lCounter);
				}
				else {
					Print("USER %d NEW\n", lCounter);
					Print("%d-%d\n", mUser[lCounter].mKeyID.mMajor, mUser[lCounter].mKeyID.mMinor);

					if(mUser[lCounter].mKeyID.mMajor == -1) {

						Print("%s(nonreg.)\n", mUser[lCounter].mName, mUser[lCounter].mIP);
						/*
						   Print( "%s\n",
						   mUser[ lCounter ].mName,
						   mUser[ lCounter ].mIP );
						 */

					}
					else if(!mUser[lCounter].mTrusted) {
						Print("%s(not verified)\n", mUser[lCounter].mName, mUser[lCounter].mIP);
					}
					else {
						Print("%s\n", mUser[lCounter].mName);
					}
				}
			}
		}

		// Send the games that have been modified
		for(lCounter = 0; lCounter < IR_MAX_GAME; lCounter++) {
			if((mGame[lCounter].mTimeStamp != 0) && (mGame[lCounter].mTimeStamp >= pTimeStamp)) {
				if(!mGame[lCounter].mUsed) {
					// Expired or deleted
					Print("GAME %d DEL\n", lCounter);
				}
				else {
					Print("GAME %d NEW %u\n", lCounter, mGame[lCounter].mId);
					Print("%s\n", mGame[lCounter].mName);
					Print("%s\n", mGame[lCounter].mTrack);
					Print("%s\n", mGame[lCounter].mAddr);
					Print("%u %d %d %d\n", mGame[lCounter].mPort, mGame[lCounter].mNbLap, mGame[lCounter].mWeapon, mGame[lCounter].mNbPlayer);

					for(int lPlayer = 0; lPlayer < mGame[lCounter].mNbPlayer; lPlayer++) {
						Print("%d ", mGame[lCounter].mPlayers[lPlayer]);
					}
					Print("\n");
				}
			}
		}

		// Send the new chat messages
		for(lCounter = 0; lCounter < IR_MAX_CHAT_MESSAGE; lCounter++) {
			int lIndex = (lCounter + mChatTail) % IR_MAX_CHAT_MESSAGE;

			if((mChatFifo[lIndex].mTimeStamp >= pTimeStamp)
			&& (mChatFifo[lIndex].mTimeStamp != 0)) {
				if((mChatFifo[lIndex].mUserDest == -1) || (mChatFifo[lIndex].mUserDest == pPlayerId) || (mChatFifo[lIndex].mUserSrc == pPlayerId)) {
					Print("CHAT\n%s\n", mChatFifo[lIndex].mData);
				}
			}
		}
	}
}

void IRState::WWWUserList()
{
	int lCounter;
	int lPrinted = 0;

	for(lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++) {
		if((mUser[lCounter].mTimeStamp != 0)) {
			if(mUser[lCounter].mExpiration != 0) {
				lPrinted++;

				if(mUser[lCounter].mKeyID.mMajor == -1) {
					/*
					   Print( "%s(nonreg.)<br>\n",
					   mUser[ lCounter ].mName );
					 */
					Print("%s<br>\n", mUser[lCounter].mName);
				}
				else {
					Print("%s(%d-%d)<br>\n", mUser[lCounter].mName, mUser[lCounter].mKeyID.mMajor, mUser[lCounter].mKeyID.mMinor);
				}
			}
		}
	}
}

void IRState::WWWMessageList()
{
	int lCounter;

	for(lCounter = (IR_MAX_CHAT_MESSAGE - 12); lCounter < IR_MAX_CHAT_MESSAGE; lCounter++) {
		int lIndex = (lCounter + mChatTail) % IR_MAX_CHAT_MESSAGE;

		if(mChatFifo[lIndex].mTimeStamp != 0) {
			if((mChatFifo[lIndex].mUserDest == -1)) {
				Print("%s<br>\n", mChatFifo[lIndex].mData);
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
		Print("ERROR 301");
	}
	else {
		Print("SUCCESS\n");

		if(pMessage[0] == '/') {
			if(!strncmp(pMessage, "/msg ", 5)) {
				int lDestUserNameLen = 0;

				const char *lNextSpace = strchr(pMessage + 5, ' ');

				if(lNextSpace != NULL) {
					lDestUserNameLen = lNextSpace - (pMessage + 5);
				}

				if(lDestUserNameLen > 0) {

					mChatFifo[mChatTail].mTimeStamp = mTimeStamp;
					mChatFifo[mChatTail].mUserSrc = pUserId;
					mChatFifo[mChatTail].mUserDest = pUserId;

					for(int lCounter = IR_MAX_CLIENT - 1; (lCounter >= 0); lCounter--) {
						if((mUser[lCounter].mTimeStamp != 0) && (mUser[lCounter].mExpiration != 0)) {
							if(!strncmp(mUser[lCounter].mName, pMessage + 5, lDestUserNameLen)) {
								mChatFifo[mChatTail].mUserDest = mUser[lCounter].mId;
								break;
							}
						}
					}

					int lLen = 0;

					lLen += StrMaxCopy(mChatFifo[mChatTail].mData, mUser[pUserIndex].mName, IR_CHAT_MESSAGE_LEN / 4);

					if(mUser[pUserIndex].mKeyID.mMajor == -1) {
						mChatFifo[mChatTail].mData[lLen++] = '(';
						lLen += StrMaxCopy(mChatFifo[mChatTail].mData + lLen, mUser[pUserIndex].mIP, IR_CHAT_MESSAGE_LEN / 4);
						mChatFifo[mChatTail].mData[lLen++] = ')';
					}
					else if(!mUser[pUserIndex].mTrusted) {
						// mChatFifo[ mChatTail ].mData[ lLen++ ] = '(';
						lLen += StrMaxCopy(mChatFifo[mChatTail].mData + lLen, "(not verified)", IR_CHAT_MESSAGE_LEN / 4);
						// mChatFifo[ mChatTail ].mData[ lLen++ ] = ')';
					}

					lLen += StrMaxCopy(mChatFifo[mChatTail].mData + lLen, "<private>", IR_CHAT_MESSAGE_LEN / 2);

					mChatFifo[mChatTail].mData[lLen++] = '»';
					mChatFifo[mChatTail].mData[lLen++] = ' ';

					StrMaxCopy(mChatFifo[mChatTail].mData + lLen, pMessage + 6 + lDestUserNameLen, IR_CHAT_MESSAGE_LEN - lLen);

					mUser[pUserIndex].mNbChat++;

					mChatTail = (mChatTail + 1) % IR_MAX_CHAT_MESSAGE;
					mTimeStamp++;
				}
			}
			// verify if the user have super user rights
			else if((mUser[pUserIndex].mExpiration != 0)
			&& (mUser[pUserIndex].mKeyID.mMajor != -1)) {
				BOOL lOK = FALSE;
				int lBanDuration = 2;			  // Defauls; ban for 2 minutes
				const char *lUser;

				BOOL lKick = FALSE;

				if(!strncmp(pMessage, "/kick ", 6)) {
					lKick = TRUE;
					lUser = pMessage + 6;
				}
				else if(sscanf(pMessage, "/ban %d", &lBanDuration) == 1) {
					if(mUser[pUserIndex].mKeyID.mMajor == 0) {
						lUser = strchr(pMessage + 6, ' ');

						if(lUser != NULL) {
							lUser = lUser + 1;
							lKick = TRUE;

							if(mUser[pUserIndex].mKeyID.mMinor >= 10) {
								if(lBanDuration > 20) {
									lBanDuration = 20;
								}
							}
						}
					}
				}

				if(lKick) {
					if(mUser[pUserIndex].mKeyID.mMajor == 0) {
						lOK = TRUE;
					}
					else if(mUser[pUserIndex].mTrusted) {
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

									if((mUser[lCounter].mKeyID.mMajor == 0) && ((mUser[lCounter].mKeyID.mMinor < 10))) {

									}
									else if((mUser[lCounter].mKeyID.mMajor == 0) && ((mUser[pUserIndex].mKeyID.mMajor != 0))) {

									}
									else {
										sprintf(lBuffer, "%s has been kicked by %s(%d-%d) for %d min", mUser[lCounter].mName, mUser[pUserIndex].mName, mUser[pUserIndex].mKeyID.mMajor, mUser[pUserIndex].mKeyID.mMinor, lBanDuration);

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

							// AddMessage( lBuffer );

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
			mChatFifo[mChatTail].mUserSrc = pUserId;
			mChatFifo[mChatTail].mUserDest = -1;

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

			mChatFifo[mChatTail].mData[lLen++] = '»';
			mChatFifo[mChatTail].mData[lLen++] = ' ';

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
	mChatFifo[mChatTail].mUserSrc = -1;
	mChatFifo[mChatTail].mUserDest = -1;

	mChatFifo[mChatTail].mData[0] = '»';
	mChatFifo[mChatTail].mData[1] = '»';
	StrMaxCopy(mChatFifo[mChatTail].mData + 2, pMessage, IR_CHAT_MESSAGE_LEN - 2);

	mChatTail = (mChatTail + 1) % IR_MAX_CHAT_MESSAGE;
	mTimeStamp++;
}

void IRState::WWWAddMessage(const char *pMessage, const char *pIP)
{
	BOOL lBanned = FALSE;

	// Verify that this IP is not banned
	for(int lCounter = 0; lCounter < IR_MAX_BANNED; lCounter++) {
		if(mBannedIP[lCounter].mExpiration > 0) {
			if(!strncmp(mBannedIP[lCounter].mIP, pIP, strlen(mBannedIP[lCounter].mIP))) {
				lBanned = TRUE;
				break;
			}
		}
	}

	// Verify for permanently banned IP
	int lIPIndex = 0;

	while(!lBanned && (gBannedISP_IP[lIPIndex] != NULL) && (gBannedISP_IP[lIPIndex][0] != 0)) {
		if(!strncmp(gBannedISP_IP[lIPIndex], pIP, strlen(gBannedISP_IP[lIPIndex]))) {
			lBanned = TRUE;
		}
		lIPIndex++;
	}

	// Verify for permanently banned IP
	lIPIndex = 0;

	while(!lBanned && (gBannedHackers[lIPIndex] != NULL) && (gBannedHackers[lIPIndex][0] != 0)) {
		if(!strncmp(gBannedHackers[lIPIndex], pIP, strlen(gBannedHackers[lIPIndex]))) {
			lBanned = TRUE;
		}
		lIPIndex++;
	}

	if(!lBanned) {
		mChatFifo[mChatTail].mTimeStamp = mTimeStamp;
		mChatFifo[mChatTail].mUserSrc = -1;
		mChatFifo[mChatTail].mUserDest = -1;

		int lLen = 0;

		lLen += StrMaxCopy(mChatFifo[mChatTail].mData, "WWW", IR_CHAT_MESSAGE_LEN / 4);

		mChatFifo[mChatTail].mData[lLen++] = '(';
		lLen += StrMaxCopy(mChatFifo[mChatTail].mData + lLen, pIP, IR_CHAT_MESSAGE_LEN / 4);
		mChatFifo[mChatTail].mData[lLen++] = ')';

		mChatFifo[mChatTail].mData[lLen++] = '»';
		mChatFifo[mChatTail].mData[lLen++] = ' ';

		lLen += StrMaxCopy(mChatFifo[mChatTail].mData + lLen, pMessage, IR_CHAT_MESSAGE_LEN - lLen);

		mChatTail = (mChatTail + 1) % IR_MAX_CHAT_MESSAGE;
		mTimeStamp++;
	}
}

void IRState::AddUser(const char *pUserName, int pVersion, int pMajorID, int pMinorID, unsigned int pIDSum, unsigned int pKey3)
{
	char lUserName[60];

	// fprintf( stderr, "Adding User\n" );

	// First verify the version
	if((pVersion < IR_VERSION_MIN) || (pVersion > IR_VERSION_MAX)) {
		Print("ERROR 103\n");
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
			Print("ERROR 102\n");
		}
		else {
			int lBannedDuration;
			BOOL lTempBanned = FALSE;
			BOOL lBanned = FALSE;
			BOOL lExpired = FALSE;
			BOOL lDoubleLog = FALSE;
			BOOL lNotReg = FALSE;
			BOOL lImposter = FALSE;
			BOOL lNotAllowed = FALSE;
			BOOL lNotVerified = FALSE;

			// Verify if the user can be trusted
			/*
			   if( pMajorID == 2)
			   {
			   mUser[ lCounter ].mTrusted = TRUE;
			   }
			   else
			 */
			if(pMajorID == -1) {
				mUser[lCounter].mTrusted = FALSE;

				if((pIDSum != 0) || (pKey3 != 0)) {
					lImposter = TRUE;
				}
			}
			else {
				if(ComputeIDSum(pMajorID, pMinorID) == pIDSum) {
					mUser[lCounter].mTrusted = TRUE;

					if(pMinorID > 1999) {
						mUser[lCounter].mTrusted = FALSE;
						lImposter = TRUE;
					}
					// Try to catch that bummy guy
				}
				else {
					mUser[lCounter].mTrusted = FALSE;

					if(pMajorID == 0) {
						lImposter = TRUE;
					}
					else {
						lNotVerified = TRUE;
					}
				}
			}

			// Verify if the IP is allowed in
			if((gAllowedISP_IP[0] != NULL) && (gAllowedISP_IP[0][0] != 0)) {
				int lIPIndex = 0;

				lNotAllowed = TRUE;

				while(lNotAllowed && (gAllowedISP_IP[lIPIndex] != NULL) && (gAllowedISP_IP[lIPIndex][0] != 0)) {
					if(!strncmp(gAllowedISP_IP[lIPIndex], GetRemoteAddr(), strlen(gAllowedISP_IP[lIPIndex]))) {
						lNotAllowed = FALSE;
					}
					lIPIndex++;
				}
			}
			// Verify that the ISP is not Banned
			// Work only for non reg ppl
			if(pMajorID == -1) {
				int lIPIndex = 0;

				while(!lBanned && (gBannedISP_IP[lIPIndex] != NULL) && (gBannedISP_IP[lIPIndex][0] != 0)) {
					if(!strncmp(gBannedISP_IP[lIPIndex], GetRemoteAddr(), strlen(gBannedISP_IP[lIPIndex]))) {
						lBanned = TRUE;
					}
					lIPIndex++;
				}
			}
			else if(pMajorID == 2) {
				lBanned = TRUE;
			}
			else {
				// Verify if the user is not on the banned list
				int lCounter = 0;

				while(gPermBannedUser[lCounter].mMajor != -1) {
					if((gPermBannedUser[lCounter].mMajor == pMajorID)
					&& (gPermBannedUser[lCounter].mMinor == pMinorID)) {
						lBanned = TRUE;
						break;
					}
					lCounter++;
				}
			}

			if(!lBanned) {
				int lIPIndex = 0;

				while(!lBanned && (gBannedHackers[lIPIndex] != NULL) && (gBannedHackers[lIPIndex][0] != 0)) {
					if(!strncmp(gBannedHackers[lIPIndex], GetRemoteAddr(), strlen(gBannedHackers[lIPIndex]))) {
						lBanned = TRUE;
					}
					lIPIndex++;
				}
			}

			// Verify root access
			/*
			   if( (pMajorID == 0)&&(pMinorID==0) )
			   {
			   if( strncmp( "205.236.249", GetRemoteAddr(), 9 ) )
			   {
			   lBanned = TRUE;
			   }
			   }
			 */

			// Verify that user is not temporary banned
			if(!lBanned) {
				for(int lCounter = 0; lCounter < IR_MAX_BANNED; lCounter++) {
					if(mBannedIP[lCounter].mExpiration > 0) {
						if(!strncmp(mBannedIP[lCounter].mIP, GetRemoteAddr(), strlen(mBannedIP[lCounter].mIP))) {
							lTempBanned = TRUE;
							lBannedDuration = ((mBannedIP[lCounter].mExpiration - time(NULL)) / 60) + 1;

							if(lBannedDuration < 1) {
								lBannedDuration = 1;
							}
						}
					}
				}
			}

			if(gRegOnly && (pMajorID == -1)) {
				lNotReg = TRUE;
			}
			// Verify if the user is not already logued
			if(pMajorID != -1) {
				// Go thru user list
				for(int lCounter = 0; lCounter < IR_MAX_CLIENT; lCounter++) {
					if((mUser[lCounter].mTimeStamp != 0) && (mUser[lCounter].mExpiration != 0)) {
						if((mUser[lCounter].mKeyID.mMajor == pMajorID) && (mUser[lCounter].mKeyID.mMinor == pMinorID)) {
							if(InitLogFile()) {
								fprintf(gLogFile, "DOUBLELOG ^%s^ %d-%d %s\n", pUserName, pMajorID, pMinorID, GetRemoteAddr()
									);
							}
							//                     lDoubleLog = TRUE;
						}
					}
				}
			}

#ifdef _EXPIRED_
			lExpired = TRUE;
#endif

			if(!lNotAllowed && !lTempBanned && !lBanned && !lExpired && !lDoubleLog && !lNotReg && !lImposter && !lNotVerified) {
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
				StrMaxCopy(mUser[lCounter].mIP, GetRemoteAddr(), IR_MAX_USER_IP);
				StrMaxCopy(mUser[lCounter].mDomain, GetRemoteHost(), IR_MAX_USER_DOM);

				Print("SUCCESS\n", lCounter);
				Print("USER_ID %d-%u\n", lCounter, mUser[lCounter].mId);

				PrintState();

				// Login message
#ifdef _DYN_CONFIG_

				if(gIntro[0][0] != 0) {
					int lCounter = 0;
					Print("CHAT\n--\n");

					while(gIntro[lCounter][0] != 0) {
						Print("CHAT\n%s\n", gIntro[lCounter]);
						lCounter++;
					}

					Print("CHAT\n--\n");
				}
#else

				Print("CHAT\n--\n");
				Print("CHAT\nIf it is your first visit here please consult the IMR RULES at http://www.grokksoft.com/rules.html\n"
				// Print( "CHAT\n Main Rules: No foul language, No insulting and no scrolling.\n" )
					Print("CHAT\n--\n");
	#endif
				// Monitor Warning
				// Verify if a monitor
				if(pMajorID != -1) {
					int lSUIndex = 0; while(gSUList[lSUIndex].mMajor != -1) {
						if(mUser[lCounter].mTrusted && (gSUList[lSUIndex].mMajor == pMajorID)
						&& (gSUList[lSUIndex].mMinor == pMinorID)) {

							Print("CHAT\n--\n"); Print("CHAT\nYou are a monitor\n"); Print("CHAT\nPlease consult the new monitor procedure at http://www.grokksoft.com/mrules.html\n"); Print("CHAT\n--\n"); break;
						}
						lSUIndex++;
					}
				}
			}
			else {
				// We got one
				Print("SUCCESS\n", lCounter); Print("USER_ID %d-%u\n", IR_BANNEDSLOT, 0); if(lExpired) {
					Print("CHAT\n--\n"); Print("CHAT\nThis version of HoverRace is expired\n"); Print("CHAT\nPlease download the new version from www.grokksoft.com\n"); Print("CHAT\n--\n");
					// Write this to the LOG
					if(InitLogFile()) {
						fprintf(gLogFile, "EXPIRED ^%s^ %d-%d %s\n", pUserName, pMajorID, pMinorID, GetRemoteAddr()
							);
					}
				}
				else
				if(lTempBanned) {
					Print("CHAT\n--\n"); Print("CHAT\nYou have been temporary banned by a monitor\n"); Print("CHAT\nYou will be allowed to come back to the room in %d minute%s\n", lBannedDuration, (lBannedDuration > 1) ? "s" : ""); Print("CHAT\nPlease report any monitor abuse to richard@grokksoft.com\n");
					// Write this to the LOG
					if(InitLogFile()) {
						fprintf(gLogFile, "TEMPBANNED ^%s^ %s\n", pUserName, GetRemoteAddr()
							);
					}
				}

				else
				if(lBanned) {
					if(pMajorID == -1) {
						Print("CHAT\nYour ISP have been temporary banned from this gameroom because one of there customer was causing troubles\n"); Print("CHAT\nWe have informed your ISP and we will reenable your access as soon as they will have corrected the situation\n"); Print("CHAT\nYou can communicate with them to accelerate the process. Sorry for the inconvenients\n"); Print("CHAT\nYou can also communicate with us support@grokksoft.com to get a special code that will let you access the game\n"); Print("CHAT\nAlso, If you register the game you will be able to play since we can track registred users\n");
					}
					else {
						Print("CHAT\nYour key have been suspended because we suspect that it have been stolen\n"); Print("CHAT\nor used by an other person than its owner.\n"); Print("CHAT\nPlease contact support@grokksoft.com to solve the problem\n");
					}

					// Write this to the LOG
					if(InitLogFile()) {
						fprintf(gLogFile, "BANNED ^%s^(%d-%d) %s\n", pUserName, pMajorID, pMinorID, GetRemoteAddr()
							);
					}
				}
				else
				if(lDoubleLog) {
					Print("CHAT\n--\n"); Print("CHAT\nYour can not login because our database indicates that you are already logued in\n"); Print("CHAT\nThis error may due to an error network. Retry to connect in a minute.\n"); Print("CHAT\nIf it still dont works please contact support@grokksoft.com\n");
					// Write this to the LOG
					if(InitLogFile()) {
						fprintf(gLogFile, "DOUBLELOG ^%s^ %d-%d %s\n", pUserName, pMajorID, pMinorID, GetRemoteAddr()
							);
					}
				}
				else
				if(lNotReg) {

					Print("CHAT\n--\n"); Print("CHAT\nSorry\n"); Print("CHAT\nYou must register to have access to this room\n"); Print("CHAT\n--\n");
					// Write this to the LOG
					if(InitLogFile()) {
						fprintf(gLogFile, "UNREG ^%s^ %s\n", pUserName, GetRemoteAddr()
							);
					}
				}
				else
				if(lNotAllowed) {

					Print("CHAT\n--\n"); Print("CHAT\nSorry\n"); Print("CHAT\n--\n");
					// Write this to the LOG
					if(InitLogFile()) {
						fprintf(gLogFile, "EXTERN ^%s^ %s\n", pUserName, GetRemoteAddr()
							);
					}
				}
				else
				if(lImposter) {

					Print("CHAT\n--\n"); Print("CHAT\nSorry\n"); Print("CHAT\nYour key seem invalid please contact GrokkSoft support\n"); Print("CHAT\n--\n");
					// Write this to the LOG
					if(InitLogFile()) {
						fprintf(gLogFile, "IMPOSTER ^%s^ %d-%d %s\n", pUserName, pMajorID, pMinorID, GetRemoteAddr()
							);
					}
				}

				else
				if(lNotVerified) {
					Print("CHAT\n--\n"); Print("CHAT\nSorry\n"); Print("CHAT\nYour need to install the patch to be able to connect\n"); Print("CHAT\nThe patch is as www.w3u.com/grokksoft/hr1-01p1.exe\n"); Print("CHAT\nIf you need additional help write to support@grokksoft.com\n"); Print("CHAT\n--\n");
					// Write this to the LOG
					if(InitLogFile()) {
						fprintf(gLogFile, "NOT VERIFIED ^%s^ %d-%d %s\n", pUserName, pMajorID, pMinorID, GetRemoteAddr()
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
		Print("ERROR 401\n");
	}
	else {
		int lCounter; if(mUser[pPlayerIndex].mKeyID.mMajor == -1) {

			BOOL lOk = FALSE; lCounter = 0;
			// Verify that the user dont try to add an illegal track
			while(!lOk && (gFreeTracks[lCounter] != NULL)) {
				if(!strncmp(gFreeTracks[lCounter], pTrackName, strlen(gFreeTracks[lCounter]))) {
					lOk = TRUE;
				}
				lCounter++;
			}

			if(!lOk) {

				Print("BEEP\n");
				// Ban the correspoiding IP
					BanIP(mUser[pPlayerIndex].mIP, 20);
				// Ban the user for a
					RemoveUser(pPlayerIndex, TRUE); return;
			}
		}

		// find an empty entry for the new game
		for(lCounter = 0; lCounter < IR_MAX_GAME; lCounter++) {
			if(!mGame[lCounter].mUsed) {
				break;
			}
		}

		if(lCounter == IR_MAX_GAME) {
			Print("ERROR 402\n");
		}
		else {
			mGame[lCounter].mTimeStamp = mTimeStamp; mGame[lCounter].mUsed = TRUE; mGame[lCounter].mStarted = FALSE; mGame[lCounter].mId = mNextGameId++; mGame[lCounter].mNbLap = pNbLap; mGame[lCounter].mNbPlayer = 1; mGame[lCounter].mPlayers[0] = pPlayerIndex; mGame[lCounter].mWeapon = pWeapon; mGame[lCounter].mPort = pPort; StrMaxCopy(mGame[lCounter].mName, pGameName, IR_GAME_NAME_LEN); StrMaxCopy(mGame[lCounter].mTrack, pTrackName, IR_TRACK_NAME_LEN); StrMaxCopy(mGame[lCounter].mAddr, pRemoteAddr, sizeof(mGame[lCounter].mAddr)); Print("SUCCESS\nGAME_ID %d-%u\n", lCounter, mGame[lCounter].mId); mTimeStamp++; mUser[pPlayerIndex].mGameCode = 1;
		}
	}
}

void IRState::JoinGame(int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId)
{

	if(!ValidUserId(pPlayerIndex, pPlayerId)) {
		Print("ERROR 501\n");
	}
	else
	if(!ValidGameId(pGameIndex, pGameId)) {
		Print("ERROR 502\n");
	}
	else
	if((mGame[pGameIndex].mUsed) && (mGame[pGameIndex].mNbPlayer >= IR_MAX_PLAYER_GAME)) {
		Print("ERROR 503\n");
	}
	else {

		if(mUser[pPlayerIndex].mKeyID.mMajor == -1) {

			BOOL lOk = FALSE; int lCounter = 0;
			// Verify that the user dont try to add an illegal track
			while(!lOk && (gFreeTracks[lCounter] != NULL)) {
				if(!strncmp(gFreeTracks[lCounter], mGame[pGameIndex].mTrack, strlen(gFreeTracks[lCounter]))) {
					lOk = TRUE;
				}
				lCounter++;
			}

			if(!lOk) {
				Print("BEEP\n");
				// Ban the correspoiding IP
					BanIP(mUser[pPlayerIndex].mIP, 20);
				// Ban the user for a
					RemoveUser(pPlayerIndex, TRUE); return;
			}
		}

		mGame[pGameIndex].mTimeStamp = mTimeStamp; mGame[pGameIndex].mPlayers[mGame[pGameIndex].mNbPlayer++] = pPlayerIndex; Print("SUCCESS\n"); mTimeStamp++; mUser[pPlayerIndex].mGameCode = 2;
	}
}

void IRState::DeleteGame(int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId)
{
	if(!ValidUserId(pPlayerIndex, pPlayerId)) {
		Print("ERROR 601\n");
	}
	else
	if(!ValidGameId(pGameIndex, pGameId)) {
		Print("ERROR 602\n");
	}
	else
	if(mGame[pGameIndex].mPlayers[0] != pPlayerIndex) {
		Print("ERROR 603\n");
	}
	else {
		Print("SUCCESS\n"); mGame[pGameIndex].mTimeStamp = mTimeStamp; mGame[pGameIndex].mUsed = FALSE; mTimeStamp++; mUser[pPlayerIndex].mGameCode = 3;
	}
}

void IRState::LeaveGame(int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId)
{
	if(!ValidUserId(pPlayerIndex, pPlayerId)) {
		Print("ERROR 701\n");
	}
	else
	if(!ValidGameId(pGameIndex, pGameId)) {
		Print("ERROR 702\n");
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
			Print("ERROR 703\n");
		}
		else {
			mGame[pGameIndex].mNbPlayer--; mGame[pGameIndex].mTimeStamp = mTimeStamp; int lNbLeft = lCounter - mGame[pGameIndex].mNbPlayer; if(lNbLeft > 0) {
				memmove(&(mGame[pGameIndex].mPlayers[lCounter]), &(mGame[pGameIndex].mPlayers[lCounter + 1]), sizeof((mGame[pGameIndex].mPlayers[lCounter])) * lNbLeft);
			}

			Print("SUCCESS\n"); mTimeStamp++; mUser[pPlayerIndex].mGameCode = 4;
		}
	}
}

void IRState::DeleteUser(int pUserIndex, int pUserId)
{
	if(!ValidUserId(pUserIndex, pUserId)) {
		Print("ERROR 801\n");
	}
	else {
		Print("SUCCESS\n"); RemoveUser(pUserIndex, FALSE);
	}
}

void IRState::StartGame(int pGameIndex, int pGameId, int pPlayerIndex, int pPlayerId)
{
	if(!ValidUserId(pPlayerIndex, pPlayerId)) {
		Print("ERROR 901\n");
	}
	else
	if(!ValidGameId(pGameIndex, pGameId)) {
		Print("ERROR 902\n");
	}
	else
	if(mGame[pGameIndex].mPlayers[0] != pPlayerIndex) {
		Print("ERROR 903\n");
	}
	else {
		Print("SUCCESS\n"); mGame[pGameIndex].mStarted = TRUE; mTimeStamp++;
	}
}

#ifdef _DAEMON_

#define MAX_CONNECT 40
#define PORT_BUFFER_SIZE 1000

class IR_Port
{
	public:
		SOCKET mPort; char mBuffer[PORT_BUFFER_SIZE]; int mBufferIndex; char mIP[16]; time_t mConnectionTime; public:
		IR_Port(); ~IR_Port(); BOOL IsValid(); void Accept(SOCKET pParent); void Close(); void Read(); const char *GetCmd();
};
IR_Port::IR_Port()
{
	mPort = INVALID_SOCKET; Close();
}

IR_Port::~IR_Port()
{
	Close();
}

BOOL IR_Port::IsValid()
{
	return mPort != INVALID_SOCKET;
}

void IR_Port::Accept(SOCKET pParent)
{
	SOCKADDR_IN lAddr; int lAddrSize = sizeof(lAddr); lAddr.sin_family = AF_INET; lAddr.sin_addr.s_addr = INADDR_ANY; lAddr.sin_port = 0; mPort = accept(pParent, (struct sockaddr *) &lAddr, &lAddrSize); if(mPort != INVALID_SOCKET) {
		unsigned lIP = ntohl(lAddr.sin_addr.s_addr); sprintf(mIP, "%u.%u.%u.%u", (lIP >> 24) & 255, (lIP >> 16) & 255, (lIP >> 8) & 255, (lIP) & 255); struct linger lLinger; lLinger.l_onoff = FALSE; lLinger.l_linger = 10;
		// setsockopt( mPort, SOL_SOCKET, SO_LINGER, (const char*)&lLinger, sizeof( lLinger ) );
		// BOOL lBoolValue = TRUE;
		// setsockopt( mPort, SOL_SOCKET, SO_DONTLINGER, (const char*)&lBoolValue, sizeof( lBoolValue ) );
			int lValue = 8192; setsockopt(mPort, SOL_SOCKET, SO_SNDBUF, (const char *) &lValue, sizeof(lValue)); mConnectionTime = time(NULL);
	}
}

void IR_Port::Close()
{
	if(mPort != INVALID_SOCKET) {
#ifdef WIN32
		closesocket(mPort);
	#else
			close(mPort);
	#endif
			mPort = INVALID_SOCKET;
	}
	mBuffer[0] = 0; mBufferIndex = 0; mIP[0] = 0;
}

void IR_Port::Read()
{
	if(mPort != INVALID_SOCKET) {
		int lToRead = sizeof(mBuffer) - 1 - mBufferIndex; if(lToRead <= 0) {
			Close();
		}
		else {
			int lRead = recv(mPort, mBuffer + mBufferIndex, lToRead, 0); if(lRead <= 0) {
				Close();
			}
			else {
				mBufferIndex += lRead; mBuffer[mBufferIndex] = 0;
				// printf( "DATA --%s--\n", mBuffer );
			}
		}
	}
}

const char *IR_Port::GetCmd()
{

	if(mPort != INVALID_SOCKET) {
		char *lReturnValue = strchr(mBuffer, '?'); if(lReturnValue != NULL) {
			lReturnValue++; char *lEnd = lReturnValue; while(*lEnd != 0) {
				if((*lEnd == ' ') || (*lEnd == 10) || (*lEnd == 13)) {
					*lEnd = 0; return lReturnValue;
				}
				lEnd++;
			}
		}
	}
	return NULL;
}

IR_Port * gCurrentPort = NULL; SOCKET lMasterSocket; IR_Port lConnection[MAX_CONNECT]; void AcceptSockets()
{
	int lCount = 0; int lSocket = 0; time_t lExpiredTime = time(NULL) - 30; while(1) {

		// Find an empty entry
		for(; lSocket < MAX_CONNECT; lSocket++) {
			if(!lConnection[lSocket].IsValid()) {
				break;
			}
			else
			if(lConnection[lSocket].mConnectionTime < lExpiredTime) {
				lConnection[lSocket].Close(); if(InitLogFile()) {
					fprintf(gLogFile, "TimeOut close\n");
				}
				break;
			}
		}

		// fprintf( stderr, "connection %d\n", lSocket );

		if(lSocket == MAX_CONNECT) {
			// we have a problem.. no free socket entry
			// fprintf( stderr, "No socket entry left\n" );
			if(InitLogFile()) {
				fprintf(gLogFile, "IMR No free socket\n");
	#ifdef WIN32
					Sleep(2000);
	#else
					sleep(2);					  // reduce flow
	#endif
			}

			break;
		}
		else {
			lConnection[lSocket].Accept(lMasterSocket); if(!lConnection[lSocket].IsValid()) {
				break;
			}
			else {
				lCount++;
			}
		}
	}

	// fprintf( stderr, "Accept %d\n", lCount );
}
#endif

int main(int pArgc, const char **pArgs)
{

#ifdef _DAEMON_

#ifdef WIN32
	// Init Win sock
	WORD lVersionRequested = MAKEWORD(1, 1); WSADATA lWsaData; if(WSAStartup(lVersionRequested, &lWsaData)) {
		fprintf(stderr, "ERROR Unable to init WinSockets\n"); return 255;
	}
#endif

	// Deamon mode DATA
	int lMasterPort;
	// Init the master socket to wait connections
	lMasterPort = -1; if((pArgc == 3) || (pArgc == 4)) {
		sscanf(pArgs[1], "%d", &lMasterPort); sscanf(pArgs[2], "%s", gLogFileName); if(pArgc == 4) {
			if(!strcmp(pArgs[3], "-r")) {
				gRegOnly = TRUE;
			}
		}
	}

	if(lMasterPort == -1) {
		fprintf(stderr, "Bad command line option\n"); fprintf(stderr, "Usage: ir.exe <port> <logfile> [-r]\n");
	#ifdef WIN32
		// Close Win sockets
			WSACleanup();
	#endif
			return 255;
	}

	lMasterSocket = socket(PF_INET, SOCK_STREAM, 0); SOCKADDR_IN lAddr; lAddr.sin_family = AF_INET; lAddr.sin_addr.s_addr = INADDR_ANY; lAddr.sin_port = htons(lMasterPort); if(bind(lMasterSocket, (SOCKADDR *) & lAddr, sizeof(lAddr)) != 0) {
		fprintf(stderr, "Unable to init port %d\n", lMasterPort);
	#ifdef WIN32
		// Close Win sockets
			WSACleanup();
	#endif
			return 255;
	}

	unsigned long lValue = 1;
	#ifdef WIN32
		ioctlsocket(lMasterSocket, FIONBIO, &lValue);
	#else
		ioctl(lMasterSocket, FIONBIO, &lValue);
	#endif
		listen(lMasterSocket, 60);
	#endif
	#ifdef _NO_IPC_
	gGlobalState.Clear(); if(InitLogFile()) {
		fprintf(gLogFile, "IMR Init version 1.7.24 %s\n", GetTime(time(NULL)));
	}
#endif

#ifdef _DAEMON_

	// Ok from here we do not print anything
	// fclose( stdin );
	// fclose( stdout );
	// fclose( stderr );

#ifndef WIN32
	// daemon( 1, 0 );
#endif

	while(1)
	#endif
	{

#ifdef _DAEMON_

		int lSocket; fd_set lReadSet; FD_ZERO(&lReadSet); FD_SET(lMasterSocket, &lReadSet); for(lSocket = 0; lSocket < MAX_CONNECT; lSocket++) {
			if(lConnection[lSocket].IsValid()) {
				FD_SET(lConnection[lSocket].mPort, &lReadSet);
				// fprintf( stderr, "%d ", lSocket );
			}
		}

		// fprintf( stderr, "select\n" );

		if(select(sizeof(lReadSet) * 8, &lReadSet, NULL, NULL, NULL) == INVALID_SOCKET) {
			// fprintf( stderr, "select error\n" );

			// Stop for no reason
			continue;							  //very ugly code.. but this is only the server
		}
		// fprintf( stderr, "end select\n" );

		// Verify if we have no new connection
		if(FD_ISSET(lMasterSocket, &lReadSet)) {
			AcceptSockets();
		}

		for(lSocket = 0; lSocket < MAX_CONNECT; lSocket++)
	#endif
		{

#ifdef _DAEMON_

			const char *lQueryPtr = NULL; if((lConnection[lSocket].IsValid())
			&& (FD_ISSET(lConnection[lSocket].mPort, &lReadSet))) {
				// fprintf( stderr, "read %d\n", lSocket );

				lConnection[lSocket].Read();
				// fprintf( stderr, "end read %d\n", lSocket );
					lQueryPtr = lConnection[lSocket].GetCmd();
				// fprintf( stderr, "end getcmd %s\n", chk( lQueryPtr ) );
					gCurrentPort = &(lConnection[lSocket]);
			}

			if(lQueryPtr != NULL)
	#endif
	#ifdef _FAST_CGI_
				while(FCGI_Accept() >= 0)
	#endif
			{
				// fprintf( stderr, "query %s\n", lQueryPtr );

				BOOL lPrintTitle = TRUE;
				// Send the header required by the server
	#ifdef _DAEMON_
					Print("HTTP/1.0 200 OK%d", 10);
	#endif
					Print("Content-type: text/html%c%c", 10, 10);
	#ifndef _DAEMON_
					const char *lQueryPtr = getenv("QUERY_STRING");
	#endif
				if(lQueryPtr == NULL) {
					Print("No parameters\n");
				}
				else {
					char lQuery[4096]; char lOp[12]; StrMaxCopy(lQuery, lQueryPtr, 4096); UnpadQuery(lQuery); if(sscanf(lQuery, "=%11s", lOp) == 1) {
#ifndef _DAEMON_
#ifndef _FAST_CGI_
						// Change the local directory
						char *lPathEnd = strrchr(pArgs[0], '/'); if(lPathEnd != NULL) {
							*lPathEnd = 0; chdir(pArgs[0]);
						}
#endif
#endif

						if(!strcmp(lOp, "RESET")) {
							Print("RESET OK\n\n");
	#ifdef _NO_IPC_
							// gGlobalState.Clear();
							// sleep( 2 );
							// return 0;
							// break;
	#else
							int lLock = semget(IR_SEM_KEY, 1, 0777); int lMemId = shmget(IR_SHMEM_KEY, sizeof(IRState), 0666); if(lLock != -1) {
								union semun lNULL; semctl(lLock, 0, IPC_RMID, lNULL);
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
						else
						if(!strcmp(lOp, "SET_MESSAGE")) {
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
						   Print( "ERROR\n" );
						   }
						   else
						   {
						   Print( "SUCCESS\n" );
						   }

						   }
						 */

						else {
							IRState * lState = NULL;
	#ifdef _NO_IPC_
								lState = &gGlobalState;
	#else
								int lLock;		  // Semaphore
								int lMemId;		  // Shared memory
												  //Avoid corruption but must not core-dump SEM_UNDO;  // Risky but prevents dead-lock
								struct sembuf lSemOp; lSemOp.sem_flg = 0;
								lSemOp.sem_num = 0;
							// First try to create the structure for the first time
							// Lock the data struct
							lLock = semget(IR_SEM_KEY, 1, 0777 | IPC_CREAT | IPC_EXCL); if(lLock != -1) {
								union semun lArg;
								// Initialize the newly created semaphore
									lArg.val = 1; semctl(lLock, 0, SETVAL, lArg);
							}
							else {
								lLock = semget(IR_SEM_KEY, 1, 0777);
							}

							if(lLock == -1) {
								Print("Unable to get semaphore\n");
							}
							else {
								lSemOp.sem_op = -1; if(semop(lLock, &lSemOp, 1) == -1) {
									Print("Unable to decrement semaphore\n"); lLock = -1;
								}
								else {

									// From here we can work safely

									lMemId = shmget(IR_SHMEM_KEY, sizeof(IRState), 0666 | IPC_CREAT | IPC_EXCL); if(lMemId != -1) {
										lState = (IRState *) shmat(lMemId, NULL, 0); if((int) lState == -1) {
											lState = NULL;
										}

										if(lState == NULL) {
											Print("Unable to attach shmem\n");
										}
										else {
											lState->Clear(); if(InitLogFile()) {
												fprintf(gLogFile, "IMR Init %s\n", GetTime(time(NULL)));
											}

										}
									}
									else {
										lMemId = shmget(IR_SHMEM_KEY, sizeof(IRState), 0666); if(lMemId == -1) {
											Print("Unable to get shmem\n");
										}
										else {
											lState = (IRState *) shmat(lMemId, NULL, 0); if((int) lState == -1) {
												lState = NULL;
											}

											if(lState == NULL) {
												Print("Unable to attach shmem\n");
											}
										}
									}
								}
							}
#endif

							if(lState != NULL) {

								lPrintTitle = FALSE; lState->VerifyExpirations(); if(!strcmp(lOp, "REFRESH")) {
									int lUserIndex; int lUserId; int lTimeStamp; if(sscanf(lQuery, "%*s %d-%u %d", &lUserIndex, &lUserId, &lTimeStamp) == 3) {
										lState->PrintStateChange(lUserIndex, lUserId, lTimeStamp);
									}
								}
								else
								if(!strcmp(lOp, "ADD_CHAT")) {
									int lUserIndex; int lUserId; char lChatMessage[200]; if(sscanf(lQuery, "%*s %d-%u %200s", &lUserIndex, &lUserId, lChatMessage) == 3) {
										Unpad(lChatMessage); ChatFilter(lChatMessage); lState->AddChatMessage(lUserIndex, lUserId, lChatMessage);
									}
								}
								//   URL?=ADD_USER MAJOR-MINORID VERSION KEY2 KEY3 ALIAS
								else
								if(!strcmp(lOp, "ADD_USER")) {
									int lMajorID; int lMinorID; int lVersion; unsigned int lKey2; unsigned int lKey3; char lUserName[40];
	#ifdef _EXPIRED_
										lState->AddUser("User", 1, -1, -1, 0, 0);
	#else
									if(sscanf(lQuery, "%*s %d-%d %d %d %d %40s", &lMajorID, &lMinorID, &lVersion, &lKey2, &lKey3, lUserName) == 6) {
										Unpad(lUserName); ChatFilter(lUserName); lState->AddUser(lUserName, lVersion, lMajorID, lMinorID, lKey2, lKey3);
									}
#endif
								}
								// URL?=ADD_GAME USER_ID GAME_NAME TRACK_NAME NBLAP WEAPON PORT
								else
								if(!strcmp(lOp, "ADD_GAME")) {
									int lUserIndex; int lUserId; int lNbLap; char lGameName[40]; char lTrackName[40]; int lWeapon; unsigned lPort; if(sscanf(lQuery, "%*s %d-%u %40s %40s %d %d %u", &lUserIndex, &lUserId, lGameName, lTrackName, &lNbLap, &lWeapon, &lPort) == 7) {
										const char *lRemoteAddr = GetRemoteAddr(); if((lRemoteAddr != NULL) && (strlen(lRemoteAddr) != 0)) {
											Unpad(lTrackName); Unpad(lGameName); lState->AddGame(lGameName, lTrackName, lNbLap, lUserIndex, lUserId, lRemoteAddr, lPort, lWeapon);
										}
									}
								}
								else
								if(!strcmp(lOp, "JOIN_GAME")) {
									int lUserIndex; int lUserId; int lGameIndex; int lGameId; if(sscanf(lQuery, "%*s %d-%u %d-%u", &lGameIndex, &lGameId, &lUserIndex, &lUserId) == 4) {
										lState->JoinGame(lGameIndex, lGameId, lUserIndex, lUserId);
									}
								}
								else
								if(!strcmp(lOp, "DEL_GAME")) {
									int lUserIndex; int lUserId; int lGameIndex; int lGameId; if(sscanf(lQuery, "%*s %d-%u %d-%u", &lGameIndex, &lGameId, &lUserIndex, &lUserId) == 4) {
										lState->DeleteGame(lGameIndex, lGameId, lUserIndex, lUserId);
									}
								}
								else
								if(!strcmp(lOp, "LEAVE_GAME")) {
									int lUserIndex; int lUserId; int lGameIndex; int lGameId; if(sscanf(lQuery, "%*s %d-%u %d-%u", &lGameIndex, &lGameId, &lUserIndex, &lUserId) == 4) {
										lState->LeaveGame(lGameIndex, lGameId, lUserIndex, lUserId);
									}
								}
								else
								if(!strcmp(lOp, "DEL_USER")) {
									int lUserIndex; int lUserId; if(sscanf(lQuery, "%*s %d-%u", &lUserIndex, &lUserId) == 2) {
										lState->DeleteUser(lUserIndex, lUserId);
									}
								}
								else
								if(!strcmp(lOp, "START_GAME")) {
									int lUserIndex; int lUserId; int lGameIndex; int lGameId; if(sscanf(lQuery, "%*s %d-%u %d-%u", &lGameIndex, &lGameId, &lUserIndex, &lUserId) == 4) {
										lState->StartGame(lGameIndex, lGameId, lUserIndex, lUserId);
									}
								}
								else
								if(!strcmp(lOp, "LOAD_DYN")) {
									Print("LOAD DYNAMIC OK\n\n"); lState->LoadDynConfig();
								}
								else
								if(!strcmp(lOp, "WWW_ULIST")) {
									Print("<HTML>\n"); Print("<meta http-equiv=\"Refresh\" Content=8 >\n"); Print("<BODY BGCOLOR=\"000000\" TEXT=\"88ff00\">\n"); Print("<font face=\"Arial\" >\n"); lState->WWWUserList(); Print("<font>\n"); Print("</BODY></HTML>\n\n");
								}
								else
								if(!strcmp(lOp, "WWW_MLIST")) {
									Print("<HTML>\n"); Print("<meta http-equiv=\"Refresh\" Content=3 >\n"); Print("<BODY BGCOLOR=\"000000\" TEXT=\"ff9900\">\n"); Print("<font face=\"Arial\" >\n"); lState->WWWMessageList(); Print("<font>\n"); Print("<A NAME=\"bottom\">.</A>\n"); Print("</BODY></HTML>\n\n");
								}
								else
								if(!strcmp(lOp, "WWW_MADD")) {

									int lWidth = 140;
	#ifndef _WWW_LOCK_
									char lMessage[200]; lMessage[0] = 0; if(sscanf(lQuery, "%*s width=%d mes=%200s", &lWidth, lMessage) >= 1) {
										WWWUnpad(lMessage); ChatFilter(lMessage); if(strlen(lMessage) > 0) {
											lState->WWWAddMessage(lMessage, GetRemoteAddr());
										}
									}
#endif

									Print("<HTML><BODY bgcolor=\"000000\" >\n"); Print("<form method=\"GET\" action=\"/\">\n"); Print("<input type=\"hidden\" name=\"\" value=\"WWW_MADD\">\n"); Print("<input type=\"hidden\" name=\"width\" value=\"%d\">\n", lWidth); Print("<input type=\"text\" name=\"mes\" value=\"\" size=%d  maxlength=120>\n", lWidth); Print("</form></BODY></HTML>\n\n");
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
								lSemOp.sem_op = 1; semop(lLock, &lSemOp, 1);
								// Release memory
								if(lState != NULL) {
									shmdt((char *) lState);
								}
							}
#endif

						}
					}
				}

#ifdef _DAEMON_
				FlushLogFile();
	#else
					CloseLogFile();
	#endif
				if(lPrintTitle) {
					Print("Internet Meeting Room (c)1996,97 GrokkSoft inc.\n");
				}

				// fprintf( stderr, "end query\n" );

#ifdef _DAEMON_
				lConnection[lSocket].Close(); AcceptSockets();
	#endif
			}
		}
	}

#ifdef WIN32
	// Close Win sockets
	WSACleanup();
	#endif
		return 0;
}

static int StrMaxCopy(char *pDest, const char *pSrc, int pDestSize)
{
	int lCounter; for(lCounter = 0; lCounter < pDestSize; lCounter++) {
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
	BOOL lReturnValue = FALSE; while((*pToken != 0) && (*pStr != 0) && (toupper(*pStr) == *pToken)) {
		pStr++; pToken++;
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

/*
void Unpad( char* pStr )
{
   while( *pStr != 0 )
   {
	  if( *pStr == '*' )
	  {
		 *pStr = ' ';
	  }
	  pStr++;
   }
}

void UnpadQuery( char* pStr )
{
   while( *pStr != 0 )
   {
	  if( *pStr == '^' )
	  {
		 *pStr = ' ';
	  }
	  pStr++;
   }
}
*/

static BOOL gNewEscape; void WWWUnpad(char *pStr)
{
	gNewEscape = TRUE; char *lPtr = pStr; while(*lPtr) {
		if(*lPtr == '+') {
			*lPtr = ' ';
		}
		lPtr++;
	}

	Unpad(pStr);
}

void Unpad(char *pStr)
{
	char *pSrc; char *pDest; char lBuffer[3]; int lHexValue; if(gNewEscape) {
		pSrc = pStr; pDest = pStr; while(*pSrc != 0) {
			if((*pSrc == '%')) {
				lBuffer[0] = pSrc[1]; lBuffer[1] = pSrc[2]; lBuffer[2] = 0; if(sscanf(lBuffer, "%x", &lHexValue) == 1) {
					pSrc += 2; *pDest = (char) (unsigned char) lHexValue;
				}
				else {
					*pDest = '%';
				}
			}
			else {
				*pDest = *pSrc;
			}
			pSrc++; pDest++;
		}
		*pDest = 0;
	}
	else {
		while(*pStr != 0) {
			if(*pStr == '*') {
				*pStr = ' ';
			}
			pStr++;
		}
	}
}

void UnpadQuery(char *pStr)
{
	char *lSrc; char *lDest; gNewEscape = FALSE; if(strchr(pStr, '^') == NULL) {
		int lNbSpace = 0; gNewEscape = TRUE; lSrc = pStr; lDest = pStr; while(*lSrc != 0) {
			if((*lSrc == '%') && (lSrc[1] == '%')) {
				*lDest = ' '; lSrc++; lNbSpace++;
			}
			else {
				*lDest = *lSrc;
			}
			lSrc++; lDest++;
		}
		*lDest = 0; if(lNbSpace == 0) {
			// Replace & by spaces
			lSrc = pStr; while(*lSrc) {
				if(*lSrc == '&') {
					*lSrc = ' ';
				}
				lSrc++;
			}
		}
	}
	else {
		while(*pStr != 0) {
			if(*pStr == '^') {
				*pStr = ' ';
			}
			pStr++;
		}
	}
}

BOOL InitLogFile()
{
	gFlushFile = TRUE; if(gLogFile == NULL) {
		gLogFile = fopen(gLogFileName, "a");
	}
	return (gLogFile != NULL);
}

void CloseLogFile()
{
	if(gLogFile != NULL) {
		fclose(gLogFile); gLogFile = NULL;
	}
	gFlushFile = FALSE;
}

void FlushLogFile()
{
	if((gLogFile != NULL) && gFlushFile) {
		fflush(gLogFile); gFlushFile = FALSE;
	}
}

const char *GetTime(time_t pTime)
{
	static char lTimeBuffer[256]; time_t lTimer = pTime; struct tm *lTime = localtime(&lTimer); strcpy(lTimeBuffer, "error"); strftime(lTimeBuffer, sizeof(lTimeBuffer), "%y-%m-%d_%H:%M:%S", lTime); return lTimeBuffer;
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

const char *gBannedWord[] =
{
	"ASS ", "A$$", "BEATCH", "BITCH", "BREAST", "CHINK", "CYBERSEX", "COCK", "CUNT", "DICK ", "FAG", "FUCK", "GAY", "HACK", "HAVE SEX", "HO ", "HOE ", "HONKEY", "KILL YOU", "KILL MY", "LICK MY", "NIGGER", "PRICK", "PUSSY", "RAPE", "QUIER", "SHIT", "SLUT", "SPIC ", "SUCKER", "SUCK MY", "SUICIDE", "TIT ", "TITS", "TITEY", "TO HELL ", "TWAT", NULL
};
void ChatFilter(char *pSrc)
{

	BOOL pPrevIsSpace = TRUE; while(*pSrc != 0) {

		if(*pSrc == '»') {
			*pSrc = ' ';
		}

		if(!isalpha(*pSrc)) {
			pPrevIsSpace = TRUE;
		}
		else
		if(pPrevIsSpace) {
			int lCounter = 0; while(gBannedWord[lCounter] != NULL) {

				if(StrCompare(pSrc, gBannedWord[lCounter])) {
					int lChar; int lLenToRemove = strlen(gBannedWord[lCounter]) - 1; for(lChar = 0; lChar < lLenToRemove; lChar++) {
						*pSrc = '-'; pSrc++;
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

static void Print(const char *pFormat, ...)
{
	va_list lArgPtr; int lLen; char lBuffer[4096]; va_start(lArgPtr, pFormat); lLen = vsprintf(lBuffer, pFormat, lArgPtr); va_end(lArgPtr);
	#ifdef _DAEMON_
		send(gCurrentPort->mPort, lBuffer, lLen, 0);
	#else
		printf("%s", lBuffer);
	#endif
}

static const char *GetRemoteAddr()
{
#ifdef _DAEMON_
	return gCurrentPort->mIP;
	#else
		return chk(getenv("REMOTE_ADDR"));
	#endif
}

static const char *GetRemoteHost()
{
#ifdef _DAEMON_
	return "n/a";
	#else
		return chk(getenv("REMOTE_HOST"));
	#endif
}

static unsigned short ComputeIDSum(int pMajorID, int pMinorID)
{
	unsigned int lReturnValue = 30; lReturnValue += pMajorID + (~pMinorID) + (pMajorID << 5) + (pMinorID << 3) + pMinorID * 3; lReturnValue ^= lReturnValue >> 16; lReturnValue &= 0xFFFF; return (unsigned short) lReturnValue;
}
