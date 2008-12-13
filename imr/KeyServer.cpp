/* KeyServer.cpp

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

// #define _FAST_CGI_
// #define _NO_IPC_

*/

#ifdef _FAST_CGI_
#include "/usr/local/fcgi/include/fcgi_stdio.h"
#else
/* #include <stdio.h> */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>

#define _ENCRYPT_STUFF
#ifdef WIN32
#include "../nettarget/game2/security.h"
#else
#include "security.h"
#endif

/* Required section to compile on Windows */
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
	/* struct semid_ds *buf; */
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

/* KEY SERVER DATA */
#define IR_SEM_KEY         75426
#define IR_SHMEM_KEY       75426

#define ID_MAJOR 2
#define ID_BASE  1
#define ID_MAX   200
#define NB_KEY   ID_MAX-ID_BASE

#define STATE_FILENAME "log/keyserverstate"
#define LOG_FILENAME   "log/key.log"
#define IP_FIFO_SIZE  10

/* Structure definition */

class KSState
{
	protected:
		class KeyEntry
		{
			public:
				time_t mTime;
				char mOwner[60];
				char mEMail[80];
				char mOrderNo[40];
				char mIP[16];
				char mPurchaseDate[15];
				char mKey1[21];
				char mKey2[21];
		};

		KeyEntry mKey[NB_KEY];
		int mNbKey;

		class IPFifoEntry
		{
			public:
				char mIP[16];
				time_t mTime;
				int mNbRetry;

		};

		IPFifoEntry mIPFifo[10];
		int mFifoHead;

	public:

		void Clear();
		void Read();
		BOOL Write();

		int VerifyAlreadyGivenKey(const char *pIP, const char *pOwner, const char *pEMail, const char *pOrderNo);
		BOOL VerifyIPFrequence(const char *pIP);  //Verify the fifo to avoid attack
		BOOL VerifyEMail(const char *pEMail);	  // Verify that email is not already used
		BOOL VefifyOrderNo(const char *OrderNo);  // Hope that there is a checksum or semething else in that. Also verify that the ordo havent been used yet
												  // Return Entry No
		int AddEntry(const char *pIP, const char *pOwner, const char *pEMail, const char *pOrderNo, const char *pPurchaseDate);
		void SetKeyForEntry(int pEntry, const char *pKey1, const char *pKey2);
		void PrintEntryKey(int pEntry);
};

BOOL VerifyOwnerFormat(const char *pOwner);		  // Just verify name length
BOOL VerifyEMailFormat(const char *pEMail);		  // Verify that email is valid

class Query
{
	public:
		char mOwner[60];
		char mEMail[80];
		char mOrderNo[40];
		char mIP[16];
		char mPurchaseDate[15];

		void Init(const char *pCGIQuery);

	private:
		void ExtractField(const char *pVar, const char *pQuery, char *pDest, int pDestLen);
};

#ifdef _NO_IPC_

KSState gGlobalState;
#endif

/* Local functions */
static int StrMaxCopy(char *pDest, const char *pSrc, int pDestSize);
static const char *GetTime(time_t pTime);

/* Implementation */
void Query::ExtractField(const char *pVar, const char *pQuery, char *pDest, int pDestLen)
{
	pDest[0] = 0;

	const char *pField = strstr(pQuery, pVar);

	if(pField != NULL) {
		pField += strlen(pVar);

		const char *lEnd = strchr(pField, '&');
		int lLen;

		if(lEnd == NULL) {
			lLen = strlen(pField);
		}
		else {
			lLen = lEnd - pField;
		}

		// copy character per character and interpret special chareacters
		int lIndex = 0;

		while((lIndex < pDestLen) && (lLen > 0)) {
			if(*pField == '%') {
				if(lLen >= 3) {
					int lValue = ' ';

					sscanf(pField + 1, "%2x", &lValue);
					pDest[lIndex++] = lValue;

					lLen -= 2;
					pField += 2;
				}
			}
			else if(*pField == '+') {
				pDest[lIndex++] = ' ';
			}
			else {
				pDest[lIndex++] = *pField;
			}

			lLen--;
			pField++;
		}
		pDest[lIndex] = 0;
	}
}

void Query::Init(const char *pCGIQuery)
{
	// Init each field
	ExtractField("owner=", pCGIQuery, mOwner, sizeof(mOwner));
	ExtractField("email=", pCGIQuery, mEMail, sizeof(mEMail));
	ExtractField("orderno=", pCGIQuery, mOrderNo, sizeof(mOrderNo));
	StrMaxCopy(mIP, getenv("REMOTE_ADDR"), sizeof(mIP));
	ExtractField("pdate=", pCGIQuery, mPurchaseDate, sizeof(mPurchaseDate));
}

void KSState::Clear()
{
	mNbKey = 0;

	for(int lCounter = 0; lCounter < IP_FIFO_SIZE; lCounter++) {
		mIPFifo[lCounter].mIP[0] = 0;
		mIPFifo[lCounter].mTime = 0;
		mIPFifo[lCounter].mNbRetry = 0;
	}
	mFifoHead = 0;
}

void KSState::Read()
{
	Clear();

	FILE *lFile = fopen(STATE_FILENAME, "rb");

	if(lFile != NULL) {
		fread(this, sizeof(KSState), 1, lFile);
		fclose(lFile);
	}
}

BOOL KSState::Write()
{
	BOOL lReturnValue = FALSE;
	FILE *lFile;

	lFile = fopen(STATE_FILENAME, "wb");

	if(lFile != NULL) {
		if(fwrite(this, sizeof(KSState), 1, lFile) == 1) {
			lReturnValue = TRUE;
		}
		fclose(lFile);
	}
	return lReturnValue;
}

int KSState::VerifyAlreadyGivenKey(const char *pIP, const char *pOwner, const char *pEMail, const char *pOrderNo)
{
	int lReturnValue = -1;

	// Verify if OrderNo havent been already used
	for(int lCounter = 0; lCounter < mNbKey; lCounter++) {
		if(!strcmp(pOrderNo, mKey[lCounter].mOrderNo)) {
			// Found it. Now verify that email is the same
			if(!strcmp(pEMail, mKey[lCounter].mEMail)) {
				// Verify IP
				if(!strncmp(pIP, mKey[lCounter].mIP, 6)) {
					// Verify owner name
					if(!strcmp(pOwner, mKey[lCounter].mOwner)) {
						lReturnValue = lCounter;
					}
				}
			}
			break;
		}
	}
	return lReturnValue;
}

BOOL KSState::VerifyIPFrequence(const char *pIP)
{
	BOOL lReturnValue = TRUE;

	for(int lCounter = 0; lCounter < IP_FIFO_SIZE; lCounter++) {
		if(!strcmp(mIPFifo[lCounter].mIP, pIP)) {
			// ok verify if the IP is acceptable
			if(mIPFifo[lCounter].mNbRetry++ > 2) {
				if(time(NULL) < (mIPFifo[lCounter].mTime + (40 * 60))) {
					lReturnValue = FALSE;
				}
			}
			mIPFifo[lCounter].mNbRetry++;
			mIPFifo[lCounter].mTime = time(NULL);
			break;
		}
	}

	if(lCounter == IP_FIFO_SIZE) {
		// New IP.. just add it to the list
		mIPFifo[mFifoHead].mNbRetry = 0;
		mIPFifo[mFifoHead].mTime = time(NULL);
		strcpy(mIPFifo[mFifoHead].mIP, pIP);

		mFifoHead++;

		if(mFifoHead >= IP_FIFO_SIZE) {
			mFifoHead = 0;
		}
	}

	if(lReturnValue) {
		// Now verify if this IP havent recently order a key
		// Well verify that they dont order more than 2 at same IP

		int lNbOccurence = 0;

		for(int lCounter = 0; lCounter < mNbKey; lCounter++) {
			if(!strcmp(pIP, mKey[lCounter].mIP)) {
				lNbOccurence++;
			}
		}
		if(lNbOccurence >= 2) {
			lReturnValue = FALSE;
		}
	}
	return lReturnValue;
}

BOOL KSState::VerifyEMail(const char *pEMail)
{
	// Now verify if this IP havent recently order a key
	// Well verify that they dont order more than 2 at same IP

	BOOL lReturnValue = TRUE;
	int lNbOccurence = 0;

	for(int lCounter = 0; lCounter < mNbKey; lCounter++) {
		if(!strcmp(pEMail, mKey[lCounter].mEMail)) {
			lNbOccurence++;
		}
	}
	if(lNbOccurence >= 2) {
		lReturnValue = FALSE;
	}
	return lReturnValue;
}

BOOL VerifyEMailFormat(const char *pEMail)
{
	BOOL lReturnValue = FALSE;

	const char *lEMail = pEMail;

	if(lEMail != NULL) {
		lEMail = strchr(lEMail + 1, '@');

		if(lEMail != NULL) {
			lEMail = strchr(lEMail + 1, '.');

			if(lEMail != NULL) {
				lReturnValue = TRUE;
			}
		}
	}
	return lReturnValue;
}

BOOL VerifyOwnerFormat(const char *pOwner)
{
	return (strlen(pOwner) > 5);
}

BOOL KSState::VefifyOrderNo(const char *pOrderNo)
{
	BOOL lReturnValue = FALSE;

	if(strlen(pOrderNo) >= 32) {
		lReturnValue = TRUE;

		// Verify if that orderno havent already been used
		for(int lCounter = 0; lCounter < mNbKey; lCounter++) {
			if(!strcmp(pOrderNo, mKey[lCounter].mOrderNo)) {
				lReturnValue = FALSE;
				break;
			}
		}
	}
	return lReturnValue;
}

int KSState::AddEntry(const char *pIP, const char *pOwner, const char *pEMail, const char *pOrderNo, const char *pPurchaseDate)
{
	int lReturnValue = -1;

	if(mNbKey < NB_KEY) {
		mKey[mNbKey].mTime = time(NULL);
		StrMaxCopy(mKey[mNbKey].mOwner, pOwner, sizeof(mKey[mNbKey].mOwner));
		StrMaxCopy(mKey[mNbKey].mEMail, pEMail, sizeof(mKey[mNbKey].mEMail));
		StrMaxCopy(mKey[mNbKey].mOrderNo, pOrderNo, sizeof(mKey[mNbKey].mOrderNo));
		StrMaxCopy(mKey[mNbKey].mIP, pIP, sizeof(mKey[mNbKey].mIP));
		StrMaxCopy(mKey[mNbKey].mPurchaseDate, pPurchaseDate, sizeof(mKey[mNbKey].mPurchaseDate));
		mKey[mNbKey].mKey1[0] = 0;
		mKey[mNbKey].mKey2[0] = 0;

		lReturnValue = mNbKey++;
	}
	return lReturnValue;
}

void KSState::SetKeyForEntry(int pEntry, const char *pKey1, const char *pKey2)
{
	StrMaxCopy(mKey[pEntry].mKey1, pKey1, sizeof(mKey[pEntry].mKey1));
	StrMaxCopy(mKey[pEntry].mKey2, pKey2, sizeof(mKey[pEntry].mKey2));
}

void KSState::PrintEntryKey(int pEntry)
{
	printf("In order to be able to play HoverRace you must fill the " "registration info window with the following information:<br><br>\n");
	printf("<table>");
	printf("<tr><td>Owner:</td>" "<td nowrap><i>%s</i></td></tr>", mKey[pEntry].mOwner);
	printf("<tr><td>User ID:</td>" "<td nowrap><i>%d - %d</i></td></tr>", ID_MAJOR, pEntry + ID_BASE);
	printf("<tr><td>Registration Key Part 1:</td>" "<td nowrap><i>%s</i></td></tr>", mKey[pEntry].mKey1);
	printf("<tr><td>Registration Key Part 2:</td>" "<td nowrap><i>%s</i></td></tr>", mKey[pEntry].mKey2);
	printf("</table>");

	printf("<br><br>The <i>Password</i> and <i>Company</i> fields are optionals." "It is recommended to fill the <i>Password</i> field only if your computer is located in a public environment" "<br><br>Please keep a printed copy of this message because the electronic information can be easily lost");

}

int main(int pArgc, const char **pArgs)
{
	char *lPathEnd;
	int lLock;									  /* Semaphore */
	int lMemId;									  /* Shared memory */
	struct sembuf lSemOp;
	union semun lArg;

#ifdef _NO_IPC_

	gGlobalState.Read();
#endif

#ifdef _FAST_CGI_
	while(FCGI_Accept() >= 0)
#endif
	{

		// Print HTML Heading
		printf("Content-type: text/html%c%c", 10, 10);
		printf("<HTML>\n");
		printf("<HEAD><TITLE>HoverRace Ragistration -- atOnce version</TITLE></HEAD>\n");
		printf("<BODY bgcolor=\"White\" text=\"Black\" link=\"Blue\" vlink=\"Purple\" alink=\"Purple\">\n");
		// printf( "<font face=\"Arial\" >\n" );
		printf("<h2>HoverRace Registration</h2><br>\n");

		Query lQuery;

		lQuery.Init(getenv("QUERY_STRING"));

#ifndef _FAST_CGI_
		/* Change the local directory */
		lPathEnd = strrchr(pArgs[0], '/');

		if(lPathEnd != NULL) {
			*lPathEnd = 0;
			chdir(pArgs[0]);
		}
#endif

		KSState *lState = NULL;

		if(!strcmp(lQuery.mOwner, "RESET")) {

#ifdef _NO_IPC_
			// gGlobalState.Clear();
			// sleep( 2 );
			// return 0;
			// break;
#else

			int lLock = semget(IR_SEM_KEY, 1, 0777);
			int lMemId = shmget(IR_SHMEM_KEY, 1, 0666);

			if(lLock != -1) {
				union semun lNULL;

				semctl(lLock, 0, IPC_RMID, lNULL);
			}

			if(lMemId != -1) {
				shmctl(lMemId, IPC_RMID, NULL);
			}
#endif
			printf("RESET OK\n");

		}
		else {

#ifdef _NO_IPC_
			lState = &gGlobalState;

#else

			lSemOp.sem_flg = 0;					  /*Avoid corruption but must not core-dump SEM_UNDO;  // Risky but prevents dead-lock */
			lSemOp.sem_num = 0;

			/* First try to create the structure for the first time */
			/* Lock the data struct */
			lLock = semget(IR_SEM_KEY, 1, 0777 | IPC_CREAT | IPC_EXCL);

			if(lLock != -1) {

				/* Initialize the newly created semaphore */
				lArg.val = 1;

				semctl(lLock, 0, SETVAL, lArg);
			}
			else {
				lLock = semget(IR_SEM_KEY, 1, 0777);
			}

			if(lLock == -1) {
				printf("Internal Error #90\n");
				// printf( "Unable to get semaphore\n" );
			}
			else {
				lSemOp.sem_op = -1;

				if(semop(lLock, &lSemOp, 1) == -1) {
					printf("Internal Error #91\n");
					// printf( "Unable to decrement semaphore\n" );
					lLock = -1;
				}
				else {

					/* From here we can work safely */

					lMemId = shmget(IR_SHMEM_KEY, sizeof(KSState), 0666 | IPC_CREAT | IPC_EXCL);

					if(lMemId != -1) {
						lState = (KSState *) shmat(lMemId, NULL, 0);

						if((int) lState == -1) {
							lState = NULL;
						}

						if(lState == NULL) {
							printf("Internal Error #92\n");
							// printf( "Unable to attach shmem\n" );
						}
						else {
							lState->Read();

						}
					}
					else {
						lMemId = shmget(IR_SHMEM_KEY, sizeof(KSState), 0666);

						if(lMemId == -1) {
							printf("Internal Error #93\n");
							// printf( "Unable to get shmem\n" );
						}
						else {
							lState = (KSState *) shmat(lMemId, NULL, 0);

							if((int) lState == -1) {
								lState = NULL;
							}

							if(lState == NULL) {
								printf("Internal Error #94\n");
								// printf( "Unable to attach shmem\n" );
							}
						}
					}
				}
			}
#endif

			if(lState != NULL) {
				// First verify that we can wtite to the log file
				FILE *lLogFile = fopen(LOG_FILENAME, "a");

				if(lLogFile == NULL) {
					printf("Internal Error #101\n");
				}
				else {
					// First verify if this key havent been already allocated
					int lKeyEntry = lState->VerifyAlreadyGivenKey(lQuery.mIP, lQuery.mOwner, lQuery.mEMail, lQuery.mOrderNo);

					fprintf(lLogFile, "REQUEST %s %s <%s> <%s> <%s> <%s>\n", GetTime(time(NULL)), lQuery.mIP, lQuery.mOwner, lQuery.mEMail, lQuery.mOrderNo, lQuery.mPurchaseDate);

					if(lKeyEntry != -1) {
						lState->PrintEntryKey(lKeyEntry);
						fprintf(lLogFile, "REPRINT %d %d-%d\n", lKeyEntry, ID_MAJOR, ID_BASE + lKeyEntry);
					}
					else {
						// Ok verify if we can accept that entry

						// Verify EmailFormat
						if(!VerifyEMailFormat(lQuery.mEMail)) {
							printf("ERROR: Invalid e-mail format\n");
							fprintf(lLogFile, "ERROR Invalid email format\n");
						}
						else {
							// Verify IP..this discourace fraud attempts
							if(!lState->VerifyIPFrequence(lQuery.mIP)) {
								printf("Internal Error #102\n");
								fprintf(lLogFile, "ERROR Failled IPFreq test\n");
							}
							else {
								// Verify owner name format
								if(!VerifyOwnerFormat(lQuery.mOwner)) {
									printf("ERROR: Invalid format for the Owner field\n");
									fprintf(lLogFile, "ERROR Invalid Owenr format\n");
								}
								else if(!lState->VerifyEMail(lQuery.mEMail)) {
									printf("Internal Error #103\n");
									fprintf(lLogFile, "ERROR Failled EMail freq test\n");
								}
								else if(!lState->VefifyOrderNo(lQuery.mOrderNo)) {
									printf("Internal Error #104\n");
									fprintf(lLogFile, "ERROR Failled OrderNo freq test\n");
								}
								else {
									lKeyEntry = lState->AddEntry(lQuery.mIP, lQuery.mOwner, lQuery.mEMail, lQuery.mOrderNo, lQuery.mPurchaseDate);

									if(lKeyEntry == -1) {
										printf("Internal Error #105\n");
										fprintf(lLogFile, "ERROR Table full\n");
									}
									else {
										// Seem that we have a good entry
										KeyStructure lKeyStruct;

										int lMajorID = ID_MAJOR;
										int lMinorID = lKeyEntry + ID_BASE;

										lKeyStruct.mProduct = PRODUCT_ID;
										lKeyStruct.mTimeStamp = time(NULL);

										lKeyStruct.mNameSum = ComputeUserNameSum(NormalizeUser(lQuery.mOwner));
										lKeyStruct.mIDSum = ComputeIDSum(lMajorID, lMinorID);

										lKeyStruct.mKeySumHard = ComputeHardKeySum(&lKeyStruct);
										lKeyStruct.mKeySumHard2 = ComputeHardKeySum2(&lKeyStruct);
										lKeyStruct.mKeySumHard3 = ComputeHardKeySum3(&lKeyStruct, NormalizeUser(lQuery.mOwner), lMinorID);

										lKeyStruct.mKeySumK2 = GetKey2Sum(&lKeyStruct);
										lKeyStruct.mKeySumK1 = GetKey1Sum(&lKeyStruct);

										// Print all info to output log
										char lKey1[21];
										char lKey2[21];

										GetToKeys(lKey1, lKey2, &lKeyStruct);

										if(fprintf(lLogFile, "[NEW KEY]\n%s\n%s\n%s\n%d %d\n%s\n%s\n", lQuery.mOrderNo, lQuery.mOwner, lQuery.mEMail, lMajorID, lMinorID, lKey1, lKey2) <= 0) {
											printf("Internal Error #106\n");
										}
										else if(fflush(lLogFile) != 0) {
											printf("Internal Error #107\n");
										}
										else {
											lState->SetKeyForEntry(lKeyEntry, lKey1, lKey2);
											if(!lState->Write()) {
												printf("Internal Error #108\n");
												fprintf(lLogFile, "[BUG] Unable to write STATE\n");
											}
											else {
												lState->PrintEntryKey(lKeyEntry);
												fprintf(lLogFile, "[Success]\n");
											}
										}
									}
								}
							}
						}
					}
					fclose(lLogFile);
				}
			}
#ifdef _NO_IPC_
			lState = NULL;
#else

			/* Release lock */
			if(lLock != -1) {
				lSemOp.sem_op = 1;

				semop(lLock, &lSemOp, 1);

				/* Release memory */
				if(lState != NULL) {
					shmdt((char *) lState);
				}
			}
#endif
		}
		// HTML Footing
		//printf( "</font>\n" );
		printf("<br><br>If you encounter any problem please contact us at <i><a href=\"mailto:support@grokksoft.com\">support@grokksoft.com</a></i> \n");
		printf("</BODY></HTML>\n\n");

	}

	return 0;
}

static int StrMaxCopy(char *pDest, const char *pSrc, int pDestSize)
{
	int lCounter = 0;

	if(pSrc != NULL) {
		for(; lCounter < pDestSize; lCounter++) {
			if((pDest[lCounter] = pSrc[lCounter]) == 0) {
				break;
			}
		}
	}
	if(pDestSize > 0) {
		pDest[pDestSize - 1] = 0;
	}

	return lCounter;
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
