// ScoreServer.cpp

// Copyright 1997 GrokkSoft inc.
// All rights reserved

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
//
// This is a CGI application
//
// Query form (version 1)
//      gScoreRequestStr.Format( "%s?=LAPTIME^%u^%s^%s[%d-%d] %u %d",
//   URL only >> print an HTML score table
//   URL?=LAPTIME TIME TrackName Alias[Major-Minor] TrackSum CraftModel
//   URL?=RESET        Reload the track from file
//
// This deamon make no log entries since the HTTPD take care of it
//
//

// #define _FAST_CGI_
// #define _NO_IPC_
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

// SCORE SERVER DATA
#define IR_SEM_KEY         71325
#define IR_SHMEM_KEY       71325

#define IR_USER_NAME_LEN     24
#define IR_TRACK_NAME_LEN    32
#define IR_MAX_TRACK         20

struct CraftDescription
{
	int mModel;
	char *mName;
};

struct CraftDescription CraftModels[] =
{
	{0, "Basic Craft"},
	{1, "Low Cx Craft"},
	{2, "Bi-Turbo Craft"}
};

const gNbCraftModels = sizeof(CraftModels) / sizeof(struct CraftDescription);

char *RecordPeriod[] =
{
	"This Week",
	"All Times",
};

const gNbRecordPeriods = sizeof(RecordPeriod) / sizeof(char *);

// Structure definition
class ISState
{
	protected:
		class Track
		{
			public:
				char mName[IR_TRACK_NAME_LEN + 1];
				unsigned int mSum;
		};

		class Record
		{
			public:
				time_t mDate;
				int mDuration;
				char mUser[IR_USER_NAME_LEN + 1];
		};

		Track mTrack[IR_MAX_TRACK];
		Record mRecord[IR_MAX_TRACK][gNbCraftModels][gNbRecordPeriods];

		int mNbTrackEntries;

	public:

		void Read();
		void Write();

		// These actions print a return value

		void AddRecord(const char *pTrackName, unsigned pTrackSum, const char *pAlias, int pDuration, int pModel);
		void Print();
};

#ifdef _NO_IPC_

ISState gGlobalState;
#endif

// Local functions
static BOOL GetString(FILE * pFile, char *pBuffer, int pBufferSize);
static int StrMaxCopy(char *pDest, const char *pSrc, int pDestSize);
static BOOL StrCompare(const char *pStr, const char *pToken);
static void Unpad(char *pStr);
static void UnpadQuery(char *pStr);

static const char *GetDate(time_t pTime);

// Implementation

void ISState::AddRecord(const char *pTrackName, unsigned pTrackSum, const char *pAlias, int pDuration, int pModel)
{
	if(pDuration > 0) {
		BOOL lSave = FALSE;
		int lCounter;
		int lTrack = -1;

		// Find the track having the record
		for(lCounter = 0; lCounter < mNbTrackEntries; lCounter++) {
			if(!strcmp(pTrackName, mTrack[lCounter].mName)) {
				lTrack = lCounter;
				break;
			}
		}

		if(lTrack != -1) {
			if(mTrack[lCounter].mSum == 0) {
				mTrack[lCounter].mSum = pTrackSum;
				lSave = TRUE;
			}
			else if(mTrack[lCounter].mSum != pTrackSum) {
				lTrack = -1;
			}
		}

		if(lTrack != -1) {
			// Find the HoverModel
			int lModel = -1;

			for(lCounter = 0; lCounter < gNbCraftModels; lCounter++) {
				if(CraftModels[lCounter].mModel == pModel) {
					lModel = lCounter;
					break;
				}
			}

			if(lModel != -1) {
				for(int lPeriod = 0; lPeriod < gNbRecordPeriods; lPeriod++) {
					if((mRecord[lTrack][lModel][lPeriod].mDuration <= 0)
					|| (pDuration < mRecord[lTrack][lModel][lPeriod].mDuration)) {

						mRecord[lTrack][lModel][lPeriod].mDuration = pDuration;
						mRecord[lTrack][lModel][lPeriod].mDate = time(NULL);
						StrMaxCopy(mRecord[lTrack][lModel][lPeriod].mUser, pAlias, IR_USER_NAME_LEN + 1);

						lSave = TRUE;
					}
				}
			}
		}

		if(lSave) {
			Write();
		}
	}
}

void ISState::Print()
{
	// HTML output

	printf("<table border=2>\n");

	for(int lTrack = -2; lTrack < mNbTrackEntries; lTrack++) {

		printf("<tr>\n");
		if(lTrack >= 0) {
			printf("<td>%s</td>\n", mTrack[lTrack].mName);
		}
		else if(lTrack == -2) {
			printf("<td rowspan=2></td>\n");
		}

		for(int lPeriod = 0; lPeriod < gNbRecordPeriods; lPeriod++) {
			if(lTrack == -2) {
				printf("<td colspan=%d align=\"Center\">%s</td>\n", gNbCraftModels, RecordPeriod[lPeriod]);
			}
			else {
				int lModel;
				int lRecordModel = -1;

				if(lTrack != -1) {
					for(int lModel = 0; lModel < gNbCraftModels; lModel++) {
						if(mRecord[lTrack][lModel][lPeriod].mDuration > 0) {
							if(lRecordModel == -1) {
								lRecordModel = lModel;
							}
							else if(mRecord[lTrack][lModel][lPeriod].mDuration < mRecord[lTrack][lRecordModel][lPeriod].mDuration) {
								lRecordModel = lModel;
							}
						}
					}
				}

				for(lModel = 0; lModel < gNbCraftModels; lModel++) {
					printf("<td align=\"Center\">\n");
					if(lTrack == -1) {
						printf(CraftModels[lModel].mName);
					}
					else {
						if(mRecord[lTrack][lModel][lPeriod].mDuration > 0) {
							printf("%s%2d:%02d:%02d%s<br>%s<br>%s\n", (lModel == lRecordModel) ? "<b>" : "", (mRecord[lTrack][lModel][lPeriod].mDuration / 60000), (mRecord[lTrack][lModel][lPeriod].mDuration / 1000) % 60, (mRecord[lTrack][lModel][lPeriod].mDuration / 10) % 100, (lModel == lRecordModel) ? "</b>" : "", mRecord[lTrack][lModel][lPeriod].mUser, GetDate(mRecord[lTrack][lModel][lPeriod].mDate));
						}
						else {
							printf("<br>\n");
						}
					}
					printf("</td>\n");
				}
			}
		}
		printf("</tr>\n");
	}
	printf("</table>\n");
}

void ISState::Read()
{
	char lLineBuffer[256];

	FILE *lFile = fopen("scores.txt", "r");

	if(lFile != NULL) {
		GetString(lFile, lLineBuffer, sizeof(lLineBuffer));

		mNbTrackEntries = 0;

		sscanf(lLineBuffer, "%d", &mNbTrackEntries);

		for(int lTrack = 0; lTrack < mNbTrackEntries; lTrack++) {
			GetString(lFile, mTrack[lTrack].mName, sizeof(mTrack[lTrack].mName));
			GetString(lFile, lLineBuffer, sizeof(lLineBuffer));
			sscanf(lLineBuffer, "%u", &mTrack[lTrack].mSum);

			for(int lPeriod = 0; lPeriod < gNbRecordPeriods; lPeriod++) {
				for(int lModel = 0; lModel < gNbCraftModels; lModel++) {
					GetString(lFile, lLineBuffer, sizeof(lLineBuffer));

					sscanf(lLineBuffer, "%d %u", &mRecord[lTrack][lModel][lPeriod].mDuration, &mRecord[lTrack][lModel][lPeriod].mDate);
					GetString(lFile, mRecord[lTrack][lModel][lPeriod].mUser, sizeof(mRecord[lTrack][lModel][lPeriod].mUser));
				}
			}
		}
		fclose(lFile);
	}
}

void ISState::Write()
{
	FILE *lFile = fopen("scores.txt", "w");

	if(lFile != NULL) {
		fprintf(lFile, "%d\n", mNbTrackEntries);

		for(int lTrack = 0; lTrack < mNbTrackEntries; lTrack++) {
			fprintf(lFile, "%s\n%u\n", mTrack[lTrack].mName, mTrack[lTrack].mSum);

			for(int lPeriod = 0; lPeriod < gNbRecordPeriods; lPeriod++) {
				for(int lModel = 0; lModel < gNbCraftModels; lModel++) {
					fprintf(lFile, "%d %u\n", mRecord[lTrack][lModel][lPeriod].mDuration, mRecord[lTrack][lModel][lPeriod].mDate);
					fprintf(lFile, "%s\n", mRecord[lTrack][lModel][lPeriod].mUser);
				}
			}
		}
		fclose(lFile);
	}
}

int main(int pArgc, const char **pArgs)
{

#ifdef _NO_IPC_

	gGlobalState.Read();
#endif

#ifdef _FAST_CGI_
	while(FCGI_Accept() >= 0)
#endif
	{
		// Send the header required by the server
		printf("Content-type: text/plain%c%c", 10, 10);
		// printf("Content-type: text/html%c%c", 10, 10);

		const char *lQueryPtr = getenv("QUERY_STRING");

		char lQuery[4096];
		char lOp[12];

		lQuery[0] = 0;
		lOp[0] = 0;

		if(lQueryPtr != NULL) {
			StrMaxCopy(lQuery, lQueryPtr, 4096);
			UnpadQuery(lQuery);
		}

#ifndef _FAST_CGI_
		// Change the local directory
		char *lPathEnd = strrchr(pArgs[0], '/');

		if(lPathEnd != NULL) {
			*lPathEnd = 0;
			chdir(pArgs[0]);
		}
#endif

		sscanf(lQuery, "=%11s", lOp);

		if(!strcmp(lOp, "RESET")) {
			printf("RESET OK\n\n");

#ifdef _NO_IPC_
			// gGlobalState.Clear();
			// sleep( 2 );
			// return 0;
			// break;
#else

			int lLock = semget(IR_SEM_KEY, 1, 0777);
			int lMemId = shmget(IR_SHMEM_KEY, sizeof(ISState), 0666);

			if(lLock != -1) {
				union semun lNULL;

				semctl(lLock, 0, IPC_RMID, lNULL);
			}

			if(lMemId != -1) {
				shmctl(lMemId, IPC_RMID, NULL);
			}
#endif

#ifdef _FAST_CGI_
			break;
#endif

		}
		else {
			ISState *lState = NULL;

#ifdef _NO_IPC_
			lState = &gGlobalState;

#else

			int lLock;							  // Semaphore
			int lMemId;							  // Shared memory

			struct sembuf lSemOp;

			lSemOp.sem_flg = 0;					  //Avoid corruption but must not core-dump SEM_UNDO;  // Risky but prevents dead-lock
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

					lMemId = shmget(IR_SHMEM_KEY, sizeof(ISState), 0666 | IPC_CREAT | IPC_EXCL);

					if(lMemId != -1) {
						lState = (ISState *) shmat(lMemId, NULL, 0);

						if((int) lState == -1) {
							lState = NULL;
						}

						if(lState == NULL) {
							printf("Unable to attach shmem\n");
						}
						else {
							lState->Read();

						}
					}
					else {
						lMemId = shmget(IR_SHMEM_KEY, sizeof(ISState), 0666);

						if(lMemId == -1) {
							printf("Unable to get shmem\n");
						}
						else {
							lState = (ISState *) shmat(lMemId, NULL, 0);

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
				if(strlen(lOp) == 0) {
					// Return the score table
					lState->Print();
				}
				else if(!strcmp(lOp, "LAPTIME")) {
					int lDuration;
					char lTrackName[80];
					char lUser[80];
					unsigned int lChkSum;
					int lCraft;

					if(sscanf(lQuery, "%*s %d %s %s %u %d", &lDuration, lTrackName, lUser, &lChkSum, &lCraft) == 5) {
						Unpad(lTrackName);
						Unpad(lUser);

						lState->AddRecord(lTrackName, lChkSum, lUser, lDuration, lCraft);
					}
					printf("OK\n\n");
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

const char *GetDate(time_t pTime)
{
	static char lTimeBuffer[256];
	time_t lTimer = pTime;

	struct tm *lTime = localtime(&lTimer);

	lTimeBuffer[0] = 0;
	strftime(lTimeBuffer, sizeof(lTimeBuffer), "%m-%d-%y", lTime);

	return lTimeBuffer;
}

BOOL GetString(FILE * pFile, char *pBuffer, int pBufferSize)
{
	pBuffer[0] = 0;

	if(fgets(pBuffer, pBufferSize, pFile) == NULL) {
		return FALSE;
	}
	else {
		while(*pBuffer != 0) {
			if((*pBuffer == '\n') || (*pBuffer == '\r')) {
				*pBuffer = 0;
				break;
			}
			pBuffer++;
		}
		return TRUE;
	}

}
