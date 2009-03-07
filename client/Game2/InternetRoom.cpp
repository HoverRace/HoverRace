// InternetRoom.cpp
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

#include "InternetRoom.h"
#include "MatchReport.h"
#include "TrackDownloadDialog.h"
#include "resource.h"
#include "../../engine/Util/Config.h"
#include "../../engine/Util/Str.h"
#include "../../engine/Util/StrRes.h"

using namespace HoverRace::Util;

#define MRM_DNS_ANSWER        (WM_USER + 1)
#define MRM_NET_EVENT         (WM_USER + 7)
#define MRM_DLG_END_ADD       (WM_USER + 10)
#define MRM_DLG_END_JOIN      (WM_USER + 11)

#define MRM_BIN_BUFFER_SIZE    25000			  // 25 K this is BIG enough

#define REFRESH_DELAY        200
#define REFRESH_TIMEOUT    11000
#define OP_TIMEOUT         22000
#define FAST_OP_TIMEOUT     6000
#define CHAT_TIMEOUT       18000
#define SCORE_OP_TIMEOUT   12000

#define IMMEDIATE                 1
#define REFRESH_EVENT             1
#define REFRESH_TIMEOUT_EVENT     2
#define CHAT_TIMEOUT_EVENT        3
#define OP_TIMEOUT_EVENT          4

#define LOAD_BANNER_TIMEOUT_EVENT     8
#define ANIM_BANNER_TIMEOUT_EVENT     9

#define MR_IR_LIST_PORT 80

// #endif

#define MR_MAX_SERVER_ENTRIES  12
#define MR_MAX_BANNER_ENTRIES  10

#define MR_HTTP_SCORE_SERVER     0
#define MR_HTTP_ROOM_SERVER      1
#define MR_HTTP_LADDER_ROOM      2
#define MR_NREG_BANNER_SERVER    8
#define MR_REG_BANNER_SERVER     9

class MR_InternetServerEntry
{
	public:
		CString mName;
		int mType;
		unsigned long mAddress;
		unsigned mPort;
		CString mURL;
		unsigned long mLadderIP;
		unsigned mLadderPort;
		CString mLadderReportURL;
};

class MR_BannerServerEntry : public MR_InternetServerEntry
{
	public:
		int mDelay;								  //in sec
		CString mClickURL;
		BOOL mIndirectClick;					  // Load location first
		CString mLastCookie;
};

MR_InternetServerEntry gScoreServer;
MR_InternetServerEntry gServerList[MR_MAX_SERVER_ENTRIES];
MR_BannerServerEntry gBannerList[MR_MAX_SERVER_ENTRIES];

int gNbServerEntries = 0;
int gCurrentServerEntry = -1;
int gNbBannerEntries = 0;
int gCurrentBannerEntry = 0;

MR_InternetRoom *MR_InternetRoom::mThis = NULL;

static BOOL gAskPassword = TRUE;

static CString MR_Pad(const char *pSrc);
static CString GetLine(const char *pSrc);
static int GetLineLen(const char *pSrc);
static const char *GetNextLine(const char *pSrc);

static int FindFocusItem(HWND pWindow);

// DNS Cache
static CString gUserNameCache;
static CString gsServerAlias; // this appears to never actually be used
unsigned long gsServerIP; // this appears to never actually be used

// MR_InternetRequest

MR_InternetRequest::MR_InternetRequest()
{
	mSocket = INVALID_SOCKET;
	mBinMode = FALSE;
	mBinBuffer = NULL;
	mBinIndex = 0;
}

MR_InternetRequest::~MR_InternetRequest()
{
	Close();

	if(mBinBuffer != NULL) {
		delete[] mBinBuffer;
	}
}

void MR_InternetRequest::SetBin()
{
	mBinMode = TRUE;

	if(mBinBuffer == NULL) {
		mBinBuffer = new char[MRM_BIN_BUFFER_SIZE];
	}
}

void MR_InternetRequest::Close()
{
	if(mSocket != INVALID_SOCKET) {
		closesocket(mSocket);
		mSocket = INVALID_SOCKET;
	}
}

BOOL MR_InternetRequest::Working() const
{
	return (mSocket != INVALID_SOCKET);
}

void MR_InternetRequest::Clear()
{
	Close();
	mBuffer = "";
	mRequest = "";
	mBinIndex = 0;

}

BOOL MR_InternetRequest::Send(HWND pWindow, unsigned long pIP, unsigned pPort, const char *pURL, const char *pCookie)
{
	BOOL lReturnValue = FALSE;

	if(!Working()) {
		Clear();

		mStartTime = time(NULL);
		/*
		   mRequest.Format( "GET http://%s HTTP/1.0\n\r"
		   "Accept: text/plain\n\r"
		   "UserAgent:  HoverRace/0.1\n\r"
		   "\n\r",
		   pURL                             );
		 */

		const char *lURL = strchr(pURL, '/');
		char lReqBuffer[1024];

		if((lURL == NULL) || mBinMode) {
			lURL = pURL;
		}

		const std::string &ua = Config::GetInstance()->GetUserAgentId();
		if(pCookie == NULL) {
			sprintf(lReqBuffer, "GET %s HTTP/1.0\n\r"
			// "Connection: Keep-Alive\n\r"
				"Accept: */*\n\r"
				"User-Agent: %s\n\r"
			// "User-Agent: Mozilla/3.0 (Win95; I)\n\r"
			// "Host: 205.181.206.67:80\n\r"
			// "Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, */*\n\r"
				"\n\r" "\n\r",
				lURL, ua.c_str());
		}
		else {
			sprintf(lReqBuffer, "GET %s HTTP/1.0\n\r"
			// "User-Agent: Mozilla/3.0 (Win95; I)\n\r"
			// "Host: 205.181.206.67:80\n\r"
			// "Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, */*\n\r"
				"Accept: */*\n\r"
				"User-Agent: %s\n\r"
				"Cookie: %s\n\r"
				"\n\r" "\n\r",
				lURL, ua.c_str(), pCookie);
		}

		mRequest = lReqBuffer;

		mSocket = socket(PF_INET, SOCK_STREAM, 0);

		ASSERT(mSocket != INVALID_SOCKET);

		SOCKADDR_IN lAddr;

		lAddr.sin_family = AF_INET;
		lAddr.sin_addr.s_addr = pIP;
		lAddr.sin_port = htons((unsigned short) pPort);

		WSAAsyncSelect(mSocket, pWindow, MRM_NET_EVENT, FD_CONNECT | FD_READ | FD_CLOSE);

		connect(mSocket, (struct sockaddr *) &lAddr, sizeof(lAddr));

		lReturnValue = TRUE;
	}
	return lReturnValue;
}

BOOL MR_InternetRequest::ProcessEvent(WPARAM pWParam, LPARAM pLParam)
{
	// static variables required to patch E-On/ICE protocol

	BOOL lReturnValue = FALSE;

	if(Working() && (pWParam == mSocket)) {
		lReturnValue = TRUE;

		switch (WSAGETSELECTEVENT(pLParam)) {
			case FD_CONNECT:
				// We are now connected, send the request
					{
						send(mSocket, mRequest, mRequest.GetLength(), 0);
					}
					mRequest = "";
					break;

			case FD_READ:
			case FD_CLOSE:

				int lNbRead;

				if(mBinMode) {
					if(mBinIndex >= MRM_BIN_BUFFER_SIZE) {
						Close();
					}
					else {
						lNbRead = recv(mSocket, mBinBuffer + mBinIndex, MRM_BIN_BUFFER_SIZE - mBinIndex, 0);

						if(lNbRead >= 0) {
							mBinIndex += lNbRead;
						}
						else {
							Close();
						}
					}

				}
				else {
					char lReadBuffer[1025];

					lNbRead = recv(mSocket, lReadBuffer, 1024, 0);

					if(lNbRead > 0) {
						lReadBuffer[lNbRead] = 0;

						{
							mBuffer += lReadBuffer;
						}
					}
				}
	
				if(WSAGETSELECTEVENT(pLParam) == FD_CLOSE) {
					Close();
				}
				break;

		}
	}

	return lReturnValue;

}

const char *MR_InternetRequest::GetBuffer() const
{
	return mBuffer;
}

const char *MR_InternetRequest::GetBinBuffer(int &pSize) const
{
	pSize = mBinIndex;
	return mBinBuffer;
}

BOOL MR_InternetRequest::IsReady() const
{
	return (((mBinIndex != 0) || !mBuffer.IsEmpty()) && !Working());
}

// MR_InternetRoom

MR_InternetRoom::MR_InternetRoom(BOOL pAllowRegistred, int pMajorID, int pMinorID, unsigned pKey2, unsigned pKey3, const std::string &pMainServer) :
	mMainServer(pMainServer)
{
	int lCounter;

	mBannerRequest.SetBin();

	mAllowRegistred = pAllowRegistred;
	mMajorID = pMajorID;
	mMinorID = pMinorID;
	mKey2 = pKey2;
	mKey3 = pKey3;

	for(lCounter = 0; lCounter < eMaxClient; lCounter++) {
		mClientList[lCounter].mValid = FALSE;
	}

	for(lCounter = 0; lCounter < eMaxGame; lCounter++) {
		mGameList[lCounter].mValid = FALSE;
	}

	mCurrentLocateRequest = NULL;
	mModelessDlg = NULL;

	// Init WinSock
	WORD lVersionRequested = MAKEWORD(1, 1);
	WSADATA lWsaData;

	if(WSAStartup(lVersionRequested, &lWsaData)) {
		ASSERT(FALSE);
	}

	mNbSuccessiveRefreshTimeOut = 0;
	mCurrentBannerIndex = 0;

}

MR_InternetRoom::~MR_InternetRoom()
{
	// Close WinSock
	WSACleanup();

	ASSERT(mModelessDlg == NULL);
}

int MR_InternetRoom::ParseState(const char *pAnswer)
{
	int lReturnValue = 0;
	const char *lLinePtr;

	lLinePtr = pAnswer;

	while(lLinePtr != NULL) {
		if(!strncmp(lLinePtr, "TIME_STAMP", 10)) {
			sscanf(lLinePtr, "TIME_STAMP %d", &mLastRefreshTimeStamp);
		}
		else if(!strncmp(lLinePtr, "USER", 4)) {
			int lEntry;
			char lOp[10];

			if(sscanf(lLinePtr, "USER %d %9s", &lEntry, lOp) == 2) {
				if((lEntry >= 0) && (lEntry < eMaxClient)) {
					if(!strcmp(lOp, "DEL")) {
						lReturnValue |= eUsersModified;
						mClientList[lEntry].mValid = FALSE;
					}
					else if(!strcmp(lOp, "NEW")) {
						lLinePtr = GetNextLine(lLinePtr);

						if(lLinePtr != NULL) {
							lReturnValue |= eUsersModified;

							mClientList[lEntry].mMajorID = -1;
							mClientList[lEntry].mMinorID = -1;

							sscanf(GetLine(lLinePtr), "%d-%d", &mClientList[lEntry].mMajorID, &mClientList[lEntry].mMinorID);

							mClientList[lEntry].mValid = TRUE;
							mClientList[lEntry].mGame = -1;

							lLinePtr = GetNextLine(lLinePtr);

							mClientList[lEntry].mName = GetLine(lLinePtr);
						}
					}
				}
			}
		}
		else if(!strncmp(lLinePtr, "GAME", 4)) {
			int lEntry;
			char lOp[10];
			int lId = -1;

			if(sscanf(lLinePtr, "GAME %d %9s %u", &lEntry, lOp, &lId) >= 2) {
				if((lEntry >= 0) && (lEntry < eMaxGame)) {
					if(!strcmp(lOp, "DEL")) {
						lReturnValue |= eGamesModified;
						mGameList[lEntry].mValid = FALSE;
					}
					else if(!strcmp(lOp, "NEW")) {
						lLinePtr = GetNextLine(lLinePtr);

						if(lLinePtr != NULL) {
							lReturnValue |= eGamesModified;

							mGameList[lEntry].mValid = TRUE;
							mGameList[lEntry].mId = lId;
							mGameList[lEntry].mNbClient = 0;
							mGameList[lEntry].mNbLap = 1;
							mGameList[lEntry].mAllowWeapons = FALSE;
							mGameList[lEntry].mPort = (unsigned) -1;
							mGameList[lEntry].mName = GetLine(lLinePtr);

							lLinePtr = GetNextLine(lLinePtr);
							mGameList[lEntry].mTrack = GetLine(lLinePtr);
							mGameList[lEntry].mAvailCode = MR_GetTrackAvail(mGameList[lEntry].mTrack);

							lLinePtr = GetNextLine(lLinePtr);
							mGameList[lEntry].mIPAddr = GetLine(lLinePtr);

							lLinePtr = GetNextLine(lLinePtr);

							int lNbClient;
							int lDummyBool;

							if(sscanf(lLinePtr, "%u %d %d %d", &mGameList[lEntry].mPort, &mGameList[lEntry].mNbLap, &lDummyBool, &lNbClient) == 4) {
								mGameList[lEntry].mAllowWeapons = lDummyBool;

								if(lNbClient > eMaxPlayerGame) {
									lNbClient = eMaxPlayerGame;
								}

								if(lNbClient != 0) {
									lLinePtr = GetNextLine(lLinePtr);

									if(lLinePtr != NULL) {
										const char *lPtr = lLinePtr;

										for(int lCounter = 0; lCounter < lNbClient; lCounter++) {
											if(lPtr != NULL) {
												int lUserIndex = atoi(lPtr);

												if((lUserIndex >= 0) && (lUserIndex < eMaxClient)) {
													mGameList[lEntry].mClientList[mGameList[lEntry].mNbClient++] = lUserIndex;

													mClientList[lUserIndex].mGame = lEntry;
												}
												lPtr = strchr(lPtr, ' ');

												if(lPtr != NULL) {
													lPtr++;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}

		}
		else if(!strncmp(lLinePtr, "CHAT", 4)) {
			// Next line is a chat message
			lLinePtr = GetNextLine(lLinePtr);

			if(lLinePtr != NULL) {
				lReturnValue |= eChatModified;
				AddChatLine(GetLine(lLinePtr));
			}
		}

		lLinePtr = GetNextLine(lLinePtr);
	}

	return lReturnValue;

}

BOOL MR_InternetRoom::LocateServers(HWND pParentWindow, BOOL pShouldRecheckServer)
{
	BOOL lReturnValue = FALSE;
	mThis = this;

	if((gNbServerEntries > 0) && !pShouldRecheckServer) {
		lReturnValue = TRUE;
	}
	else {
		/* make sure global server list values are reset */
		gScoreServer = MR_InternetServerEntry();
		for(int lCounter = 0; lCounter < gNbServerEntries; lCounter++)
			gServerList[lCounter] = MR_InternetServerEntry();
		for(int lCounter = 0; lCounter < gNbBannerEntries; lCounter++)
			gBannerList[lCounter] = MR_BannerServerEntry();

		gNbServerEntries = 0;
		gCurrentServerEntry = -1;
		gNbBannerEntries = 0;
		gCurrentBannerEntry = 0;

		if(DialogBoxW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_NET_PROGRESS), pParentWindow, GetAddrCallBack) == IDOK) {
			lReturnValue = TRUE;
		}
	}

	return lReturnValue;
}

BOOL MR_InternetRoom::AddUserOp(HWND pParentWindow)
{
	BOOL lReturnValue = FALSE;

	mThis = this;

	mNetOpString = _("Connecting to the Internet Meeting Room...");

	mNetOpRequest.Format("%s?=ADD_USER%%%%%d-%d%%%%1%%%%%u%%%%%u%%%%%s", (const char *) gServerList[gCurrentServerEntry].mURL, mMajorID, (mMinorID == -1) ? -2 : mMinorID, mKey2, mKey3, (const char *) MR_Pad(mUser));

	lReturnValue = DialogBoxW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_NET_PROGRESS), pParentWindow, NetOpCallBack) == IDOK;

	if(lReturnValue) {
		const char *lData = mOpRequest.GetBuffer();

		while((lData != NULL) && strncmp(lData, "SUCCESS", 7)) {
			lData = GetNextLine(lData);
		}
		if(lData == NULL) {
			ASSERT(FALSE);
			lReturnValue = FALSE;
		}
		else {
			lData = GetNextLine(lData);

			sscanf(lData, "USER_ID %d-%u", &mCurrentUserIndex, &mCurrentUserId);

			AddChatLine(_("You are connected"));
			AddChatLine(_("Welcome to the HoverRace Internet Meeting Room"));

			ParseState(lData);
		}
	}

	mOpRequest.Clear();

	return lReturnValue;
}

BOOL MR_InternetRoom::DelUserOp(HWND pParentWindow, BOOL pFastMode)
{
	BOOL lReturnValue = FALSE;

	mThis = this;

	mNetOpString = _("Disconnecting from the Internet Meeting Room...");

	mNetOpRequest.Format("%s?=DEL_USER%%%%%d-%u", (const char *) gServerList[gCurrentServerEntry].mURL, mCurrentUserIndex, mCurrentUserId);

	lReturnValue = DialogBoxW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_NET_PROGRESS), pParentWindow, pFastMode ? FastNetOpCallBack : NetOpCallBack) == IDOK;

	mOpRequest.Clear();

	return lReturnValue;
}

BOOL MR_InternetRoom::AddGameOp(HWND pParentWindow, const char *pGameName, const char *pTrackName, int pNbLap, BOOL pWeapons, unsigned pPort)
{
	BOOL lReturnValue = FALSE;

	mThis = this;

	mNetOpString = _("Registering game with the Internet Meeting Room...");

	mNetOpRequest.Format("%s?=ADD_GAME%%%%%d-%u%%%%%s%%%%%s%%%%%d%%%%%d%%%%%d", (const char *) gServerList[gCurrentServerEntry].mURL, mCurrentUserIndex, mCurrentUserId, (const char *) MR_Pad(pGameName), (const char *) MR_Pad(pTrackName), pNbLap, pWeapons ? 1 : 0, pPort);

	lReturnValue = DialogBoxW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_NET_PROGRESS), pParentWindow, NetOpCallBack) == IDOK;

	if(lReturnValue) {
		const char *lData = mOpRequest.GetBuffer();

		while((lData != NULL) && strncmp(lData, "SUCCESS", 7)) {
			lData = GetNextLine(lData);
		}
		if(lData == NULL) {
			ASSERT(FALSE);
			lReturnValue = FALSE;
		}
		else {
			lData = GetNextLine(lData);

			sscanf(lData, "GAME_ID %d-%u", &mCurrentGameIndex, &mCurrentGameId);
		}
	}

	mOpRequest.Clear();

	return lReturnValue;
}

BOOL MR_InternetRoom::DelGameOp(HWND pParentWindow)
{
	BOOL lReturnValue = FALSE;

	mThis = this;

	mNetOpString = _("Unregistering game from the Internet Meeting Room...");

	mNetOpRequest.Format("%s?=DEL_GAME%%%%%d-%u%%%%%d-%u", (const char *) gServerList[gCurrentServerEntry].mURL, mCurrentGameIndex, mCurrentGameId, mCurrentUserIndex, mCurrentUserId);

	lReturnValue = DialogBoxW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_NET_PROGRESS), pParentWindow, NetOpCallBack) == IDOK;

	mOpRequest.Clear();

	return lReturnValue;
}

BOOL MR_InternetRoom::JoinGameOp(HWND pParentWindow, int pGameIndex)
{
	BOOL lReturnValue = FALSE;

	mThis = this;

	mNetOpString = _("Registering with the Internet Meeting Room...");

	mCurrentGameIndex = pGameIndex;
	mCurrentGameId = mGameList[pGameIndex].mId;

	mNetOpRequest.Format("%s?=JOIN_GAME%%%%%d-%u%%%%%d-%u%%%%%u%%%%%u",
		(const char *) gServerList[gCurrentServerEntry].mURL,
		mCurrentGameIndex, mCurrentGameId, mCurrentUserIndex, mCurrentUserId,
		Config::GetInstance()->net.tcpRecvPort,
		Config::GetInstance()->net.udpRecvPort);

	lReturnValue = DialogBoxW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_NET_PROGRESS), pParentWindow, NetOpCallBack) == IDOK;

	mOpRequest.Clear();

	return lReturnValue;
}

BOOL MR_InternetRoom::LeaveGameOp(HWND pParentWindow)
{
	BOOL lReturnValue = FALSE;

	mThis = this;

	mNetOpString = _("Unregistering from the Internet Meeting Room...");

	mNetOpRequest.Format("%s?=LEAVE_GAME%%%%%d-%u%%%%%d-%u", (const char *) gServerList[gCurrentServerEntry].mURL, mCurrentGameIndex, mCurrentGameId, mCurrentUserIndex, mCurrentUserId);

	lReturnValue = DialogBoxW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_NET_PROGRESS), pParentWindow, NetOpCallBack) == IDOK;

	mOpRequest.Clear();

	return lReturnValue;
}

BOOL MR_InternetRoom::AddMessageOp(HWND pParentWindow, const char *pMessage, int pHours, int pMinutes)
{
	BOOL lReturnValue = FALSE;

	mThis = this;

	mNetOpString = _("Sending message to the Internet Meeting Room...");

	mNetOpRequest.Format("%s?=MESSAGE%%%%%d-%u%%%%%d:%d%%%%%s", (const char *) gServerList[gCurrentServerEntry].mURL, mCurrentUserIndex, mCurrentUserId, pHours, pMinutes, (const char *) MR_Pad(pMessage));

	lReturnValue = DialogBoxW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_NET_PROGRESS), pParentWindow, NetOpCallBack) == IDOK;

	mOpRequest.Clear();

	return lReturnValue;
}

BOOL MR_InternetRoom::AskRoomParams(HWND pParentWindow, BOOL pShouldRecheckServer)
{
	BOOL lReturnValue = FALSE;
	mThis = this;

	lReturnValue = mThis->LocateServers(pParentWindow, pShouldRecheckServer);

	if(lReturnValue) {
		lReturnValue = DialogBoxW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_INTERNET_PARAMS), pParentWindow, AskParamsCallBack) == IDOK;
	}

	return lReturnValue;
}


/**
 * This function is called to initiate the connection to the IMR
 */
BOOL MR_InternetRoom::DisplayChatRoom(HWND pParentWindow, MR_NetworkSession *pSession, MR_VideoBuffer *pVideoBuffer, BOOL pShouldRecheckServer)
{
	mUser = pSession->GetPlayerName();

	BOOL lReturnValue = AskRoomParams(pParentWindow, pShouldRecheckServer);

	if(lReturnValue) {
		mThis = this;

		mSession = pSession;
		mVideoBuffer = pVideoBuffer;

		mSession->SetPlayerName(mUser);

		if(gNbBannerEntries == 0) {
			lReturnValue = DialogBoxW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_INTERNET_MEETING), pParentWindow, RoomCallBack) == IDOK;
		}
		else {
			lReturnValue = DialogBoxW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_INTERNET_MEETING_PUB), pParentWindow, RoomCallBack) == IDOK;
		}

	}
	return lReturnValue;
}

/*
BOOL MR_InternetRoom::DisplayModeless( HWND pParentWindow, MR_NetworkSession* pSession, MR_VideoBuffer* pVideoBuffer )
{
   BOOL lReturnValue = AskRoomParams( pParentWindow );

   if( lReturnValue )
   {
	  mThis = this;

	  mSession = pSession;
	  mVideoBuffer = pVideoBuffer;

	  mSession->SetPlayerName( mUser );

	  lReturnValue = DialogBox( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDD_INTERNET_MEETING ), pParentWindow, RoomCallBack )==IDOK;
   }
   return lReturnValue;
}

BOOL MR_InternetRoom::IsDisplayed( )const
{
   return mWindow != NULL;
}
*/

void MR_InternetRoom::AddChatLine(const char *pText)
{

	if(!mChatBuffer.IsEmpty()) {
		mChatBuffer += "\r\n";
	}
	mChatBuffer += pText;

	// Determine if we must cut some lines from the buffer
	while(mChatBuffer.GetLength() > (40 * 40)) {
		int lIndex = mChatBuffer.Find('\n');

		if(lIndex < 0) {
			break;
		}
		else {
			mChatBuffer = mChatBuffer.Mid(lIndex + 1);
		}
	}
}

void MR_InternetRoom::SelectGameForUser(HWND pWindow)
{
	int lFocus = FindFocusItem(GetDlgItem(pWindow, IDC_USER_LIST));

	if(lFocus != -1) {
		if((mClientList[lFocus].mValid) && (mClientList[lFocus].mGame != -1)) {
			HWND lList = GetDlgItem(pWindow, IDC_GAME_LIST);
			LV_FINDINFO lInfo;

			lInfo.flags = LVFI_PARAM;
			lInfo.lParam = mClientList[lFocus].mGame;

			int lGameIndex = ListView_FindItem(lList, -1, &lInfo);

			if(lGameIndex != -1) {
				ListView_SetItemState(lList, lGameIndex, LVIS_FOCUSED, LVIS_FOCUSED);
			}
		}
	}
}

void MR_InternetRoom::RefreshGameSelection(HWND pWindow)
{
	int lGameIndex = FindFocusItem(GetDlgItem(pWindow, IDC_GAME_LIST));

	if(lGameIndex == -1) {
		SetDlgItemTextW(pWindow, IDC_TRACK_NAME, Str::UW(_("[no selection]")));
		SetDlgItemText(pWindow, IDC_NB_LAP, "");
		SetDlgItemText(pWindow, IDC_WEAPONS, "");
		SetDlgItemText(pWindow, IDC_AVAIL_MESSAGE, "");
		SetDlgItemText(pWindow, IDC_PLAYER_LIST, "");

		SendMessage(GetDlgItem(pWindow, IDC_JOIN), WM_ENABLE, FALSE, 0);
	}
	else {
		CString lAvailString = "";
		CString lPlayerList;

		switch (mGameList[lGameIndex].mAvailCode) {
			case eTrackAvail:
				lAvailString = _("Available");
				break;

			case eTrackNotFound:
				lAvailString = _("Join game to download from hoverrace.com");
				break;
		}

		for(int lCounter = 0; lCounter < mGameList[lGameIndex].mNbClient; lCounter++) {
			int lClientIndex = mGameList[lGameIndex].mClientList[lCounter];

			if(mClientList[lClientIndex].mValid) {
				if(!lPlayerList.IsEmpty()) {
					lPlayerList += "\r\n";
				}
				lPlayerList += mClientList[lClientIndex].mName;
			}

		}

		SetDlgItemText(pWindow, IDC_TRACK_NAME, mGameList[lGameIndex].mTrack);
		SetDlgItemInt(pWindow, IDC_NB_LAP, mGameList[lGameIndex].mNbLap, FALSE);
		SetDlgItemText(pWindow, IDC_WEAPONS, mGameList[lGameIndex].mAllowWeapons ? "on" : "off");
		SetDlgItemText(pWindow, IDC_AVAIL_MESSAGE, lAvailString);
		SetDlgItemText(pWindow, IDC_PLAYER_LIST, lPlayerList);

		SendMessage(GetDlgItem(pWindow, IDC_JOIN), WM_ENABLE, mGameList[lGameIndex].mAvailCode == eTrackAvail, 0);
	}

}

void MR_InternetRoom::RefreshGameList(HWND pWindow)
{
	HWND lList = GetDlgItem(pWindow, IDC_GAME_LIST);

	if(lList != NULL) {
		// Get selection
		int lSelected = FindFocusItem(lList);

		// Clear the content of the list box
		ListView_DeleteAllItems(lList);

		// Refill
		int lIndex = 0;
		for(int lCounter = 0; lCounter < eMaxGame; lCounter++) {
			if(mGameList[lCounter].mValid) {
				LV_ITEM lItem;

				lItem.mask = LVIF_TEXT | LVIF_PARAM;
				lItem.iItem = lIndex++;
				lItem.iSubItem = 0;
				lItem.pszText = (char *) (const char *) mGameList[lCounter].mName;
				lItem.lParam = lCounter;

				if(lCounter == lSelected) {
					lItem.mask |= LVIF_STATE;
					lItem.state = LVIS_FOCUSED;
					lItem.stateMask = LVIS_FOCUSED;
				}

				ListView_InsertItem(lList, &lItem);
			}
		}
	}
	RefreshGameSelection(pWindow);
}

void MR_InternetRoom::RefreshUserList(HWND pWindow)
{
	HWND lList = GetDlgItem(pWindow, IDC_USER_LIST);

	if(lList != NULL) {
		// Get selection
		int lSelected = FindFocusItem(lList);

		// Clear the content of the list box
		ListView_DeleteAllItems(lList);

		// Refill
		int lIndex = 0;
		for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
			if(mClientList[lCounter].mValid) {
				CString lName = mClientList[lCounter].mName;

				if(mClientList[lCounter].mMajorID != -1) {
					CString lExtension;
					lExtension.Format("[%d-%d]", mClientList[lCounter].mMajorID, mClientList[lCounter].mMinorID);
					lName += lExtension;
				}
				LV_ITEM lItem;

				lItem.mask = LVIF_TEXT | LVIF_PARAM;
				lItem.iItem = lIndex++;
				lItem.iSubItem = 0;
				lItem.pszText = (char *) (const char *) lName;
				lItem.lParam = lCounter;

				if(lCounter == lSelected) {
					lItem.mask |= LVIF_STATE;
					lItem.state = LVIS_FOCUSED;
					lItem.stateMask = LVIS_FOCUSED;
				}

				int lCode = ListView_InsertItem(lList, &lItem);

				ASSERT(lCode != -1);

			}
		}

	}

}

/***
 * Refresh the chat buffer.
 * Keep in mind that mChatBuffer is internally UTF-8 (not wide), so Str::UW must be used before displaying.
 */
void MR_InternetRoom::RefreshChatOut(HWND pWindow)
{
	HWND pDest = GetDlgItem(pWindow, IDC_CHAT_OUT);

	SetWindowTextW(pDest, Str::UW(mChatBuffer));

	SendMessage(pDest, EM_LINESCROLL, 0, 1000);
	// SendMessage( pDest, WM_VSCROLL, SB_BOTTOM, 0 );
}

BOOL MR_InternetRoom::VerifyError(HWND pParentWindow, const char *pAnswer)
{
	BOOL lReturnValue = FALSE;
	int lCode = -1;

	const char *lLinePtr = pAnswer;

	while(lLinePtr != NULL) {
		if(!strncmp(lLinePtr, "SUCCESS", 7)) {
			lReturnValue = TRUE;
			break;
		}
		else if(!strncmp(lLinePtr, "ERROR", 5)) {
			sscanf(lLinePtr, "ERROR %d", &lCode);
			lReturnValue = FALSE;
			break;
		}
		lLinePtr = GetNextLine(lLinePtr);
	}

	if(!lReturnValue && (pParentWindow != NULL)) {
		BOOL lPopDlg = TRUE;
		CString lMessage;

		if(lCode == -1) {
			ASSERT(FALSE);
			lMessage = _("Communication error");
		}

		while(lMessage.IsEmpty()) {
			switch (lCode) {
				case 100:
					lMessage = _("Unable to add user");
					break;

				case 101:
					lMessage = _("No more Shareware users allowed");
					break;

				case 102:
					lMessage = _("No more user allowed");
					break;

				case 103:
					lMessage = _("Incompatible version");
					break;

				case 104:
					lMessage = _("Expired key");
					break;

				case 105:
					lMessage = _("Already used key (report for investigation)");
					break;

				case 200:
					lMessage = _("Unable to send refresh info");
					break;

				case 201:
					lMessage = _("Not online");
					break;

				case 300:
					lMessage = _("Unable to add chat message");
					break;

				case 301:
					lMessage = _("Not online");
					break;

				case 400:
					lMessage = _("Unable to add game");
					break;

				case 401:
					lMessage = _("Not online");
					break;

				case 402:
					lMessage = _("Entry is no longer available");
					break;

				case 500:
					lMessage = _("Unable to add user");
					break;

				case 501:
					lMessage = _("Not online");
					break;

				case 502:
					lMessage = _("Game is no longer available");
					break;

				case 503:
					lMessage = _("Game is full");
					break;

				case 600:
					lMessage = _("Unable to delete game");
					break;

				case 601:
					lMessage = _("Not online");
					lPopDlg = FALSE;
					break;

				case 602:
					lMessage = _("Game is no longer available");
					break;

				case 603:
					lMessage = _("Not owner");
					break;

				case 700:
					lMessage = _("Unable to leave game");
					break;

				case 701:
					lMessage = _("Not online");
					lPopDlg = FALSE;
					break;

				case 702:
					lMessage = _("Game is no longer available");
					lPopDlg = FALSE;
					break;

				case 703:
					lMessage = _("Must join first");
					break;

				case 800:
					lMessage = _("Unable to delete user");
					break;

				case 801:
					lMessage = _("Not online");
					lPopDlg = FALSE;
					break;

				case 900:
					lMessage = _("Unable to start game");
					break;

				case 901:
					lMessage = _("Not online");
					break;

				case 902:
					lMessage = _("Game is no longer available");
					break;

				case 903:
					lMessage = _("Not owner");
					break;

				case 1000:
					lMessage = _("Unable to add message");
					break;

				case 1001:
					lMessage = _("Not online");
					break;

				case 1002:
					lMessage = _("Not authorized");
					break;

			}

			if(lMessage.IsEmpty()) {
				if((lCode % 100) == 0) {
					lMessage = boost::str(boost::format("%s %d") %
						_("Unknown error code") %
						lCode).c_str();
				}
				else {
					// Restart the sequence but only with the genic number
					lCode = lCode - (lCode % 100);
				}
			}
		}

		if(lPopDlg) {
			MessageBoxW(pParentWindow, Str::UW(lMessage), Str::UW(_("Internet Meeting Room")), MB_ICONSTOP | MB_OK | MB_APPLMODAL);
		}
	}
	return lReturnValue;
}

int MR_InternetRoom::LoadBanner(HWND pWindow, const char *pBuffer, int pBufferLen)
{
	ASSERT(pWindow != NULL);

	HWND lWindow = GetDlgItem(pWindow, IDC_PUB);

	if(lWindow == NULL) {
		return 0;								  // no more refresh
	}
	else {
		mBanner.Decode((unsigned char *) pBuffer, pBufferLen);

		HDC hdc = GetDC(lWindow);

		HPALETTE lOldPalette = SelectPalette(hdc, mBanner.GetGlobalPalette(), FALSE);

		int lNbColors = RealizePalette(hdc);

		if(lOldPalette != NULL) {
			SelectPalette(hdc, mBanner.GetGlobalPalette(), TRUE);
		}

		ReleaseDC(lWindow, hdc);

		//TRACE("Colors2 %d  %d\n", lNbColors, GetLastError());

		mCurrentBannerIndex = 0;
		SendMessage(lWindow, BM_SETIMAGE, IMAGE_BITMAP, (long) mBanner.GetImage(0));

		return mBanner.GetDelay(0);
	}
}

int MR_InternetRoom::RefreshBanner(HWND pWindow)
{
	ASSERT(pWindow != NULL);

	HWND lWindow = GetDlgItem(pWindow, IDC_PUB);

	if((lWindow == NULL) || (mBanner.GetImageCount() == 0)) {
		return 0;								  // no more refresh
	}
	else {
		mCurrentBannerIndex = (mCurrentBannerIndex + 1) % mBanner.GetImageCount();

		SendMessage(lWindow, BM_SETIMAGE, IMAGE_BITMAP, (long) mBanner.GetImage(mCurrentBannerIndex));

		return mBanner.GetDelay(mCurrentBannerIndex);

	}

}

/*
BOOL CALLBACK MR_InternetRoom::AskPasswordCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
			{
				lReturnValue = TRUE;
				SetDlgItemText(pWindow, IDC_ALIAS, mThis->mUser);
	
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					lReturnValue = TRUE;
					break;

				case IDOK:
					{
						char lBuffer[40];
						char lPassword[40];
	
						GetDlgItemText(pWindow, IDC_ALIAS, lBuffer, sizeof(lBuffer));
						GetDlgItemText(pWindow, IDC_PASSWD, lPassword, sizeof(lPassword));
	
						// Verify the validity of the password
						mThis->mNetOpString.LoadString(IDS_IMR_PASSVAL);
	
						mThis->mNetOpRequest.Format("%s,%s,hover", lBuffer, lPassword);
	
						int lCode = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_NET_PROGRESS), pWindow, NULL) == IDOK;
	
						mThis->mOpRequest.Clear();
	
						if(lCode) {
							EndDialog(pWindow, IDOK);
							mThis->mUser = lBuffer;
							gUserNameCache = lBuffer;
						}
	
						lReturnValue = TRUE;
					}
					break;
			}
			break;
	}

	return lReturnValue;
}
*/

BOOL CALLBACK MR_InternetRoom::AskParamsCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
			{
				lReturnValue = TRUE;
				SetWindowTextW(pWindow, Str::UW(_("Internet Meeting Room")));
				SetDlgItemTextW(pWindow, IDC_YOUR_ALIAS, Str::UW(_("Your alias:")));
				SetDlgItemTextW(pWindow, IDC_ROOM_C, Str::UW(_("Room:")));
				SetDlgItemTextW(pWindow, IDC_ALIAS, Str::UW(mThis->mUser));
	
				if(gCurrentServerEntry == -1) {
					gCurrentServerEntry = 0;
				}
	
				for(int lCounter = 0; lCounter < gNbServerEntries; lCounter++) {
					SendDlgItemMessage(pWindow, IDC_ROOMLIST, LB_ADDSTRING, 0, (long) (const char *) gServerList[lCounter].mName);
				}
	
				// Select the current room
				SendDlgItemMessage(pWindow, IDC_ROOMLIST, LB_SETCURSEL, gCurrentServerEntry, 0);
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					lReturnValue = TRUE;
					break;

				case IDOK:
					{
						char lBuffer[30];
						/*
						char lURLBuffer[80];
	
						GetDlgItemText(pWindow, IDC_URL, lURLBuffer, sizeof(lURLBuffer));
						*/
	
						gCurrentServerEntry = SendDlgItemMessage(pWindow, IDC_ROOMLIST, LB_GETCURSEL, 0, 0);
	
						if(GetDlgItemText(pWindow, IDC_ALIAS, lBuffer, sizeof(lBuffer)) <= 0) {
							MessageBoxW(pWindow, Str::UW(_("You must enter an alias")), 
								Str::UW(_("Internet Meeting Room")), MB_ICONINFORMATION | MB_OK | MB_APPLMODAL);
						}
						else if((gCurrentServerEntry < 0) || (gCurrentServerEntry >= gNbServerEntries)) {
							gCurrentServerEntry = -1;
							MessageBoxW(pWindow, Str::UW(_("You must select a room")), 
								Str::UW(_("Internet Meeting Room")), MB_ICONINFORMATION | MB_OK | MB_APPLMODAL);
						}
						else {
							EndDialog(pWindow, IDOK);
							mThis->mUser = lBuffer;
							gUserNameCache = lBuffer;
						}
	
						lReturnValue = TRUE;
					}
					break;
			}
			break;
	}

	return lReturnValue;

}

BOOL CALLBACK MR_InternetRoom::RoomCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
			{
				// i18n
				// so we don't need to have things translated multiple times
				std::string title = boost::str(boost::format("%s %s") %
					_("HoverRace") %
					_("Internet Meeting Room"));
				
				SetWindowTextW(pWindow, Str::UW(title.c_str()));
				SetDlgItemTextW(pWindow, IDC_GAME_LIST_C, Str::UW(_("Game list")));
				SetDlgItemTextW(pWindow, IDC_USERS_LIST_C, Str::UW(_("User list")));
				SetDlgItemTextW(pWindow, IDC_GAME_DETAILS_C, Str::UW(_("Game details")));
				SetDlgItemTextW(pWindow, IDC_TRACK_NAME_C, Str::UW(_("Track name:")));
				SetDlgItemTextW(pWindow, IDC_LAPS_C, Str::UW(_("Laps:")));
				SetDlgItemTextW(pWindow, IDC_WEAPONS_C, Str::UW(_("Weapons:")));
				SetDlgItemTextW(pWindow, IDC_AVAILABILITY_C, Str::UW(_("Availability:")));
				SetDlgItemTextW(pWindow, IDC_PLAYERS_LIST_C, Str::UW(_("Player list:")));
				SetDlgItemTextW(pWindow, IDC_CHAT_SECTION_C, Str::UW(_("Chat section")));
				SetDlgItemTextW(pWindow, IDC_JOIN, Str::UW(_("Join Game...")));
				SetDlgItemTextW(pWindow, IDC_ADD, Str::UW(_("New Game...")));
				SetDlgItemTextW(pWindow, IDCANCEL, Str::UW(_("Quit")));

				RECT lRect;
				HWND lList;
				LV_COLUMN lSpec;
	
				// Adjust dlg items
				lList = GetDlgItem(pWindow, IDC_USER_LIST);
				GetClientRect(lList, &lRect);
	
				// Create list columns
				lSpec.mask = LVCF_SUBITEM | LVCF_WIDTH | LVCF_FMT;
	
				lSpec.fmt = LVCFMT_LEFT;
				lSpec.cx = lRect.right - 1 - GetSystemMetrics(SM_CXVSCROLL);
				lSpec.iSubItem = 0;
	
				ListView_InsertColumn(lList, 0, &lSpec);
	
				lList = GetDlgItem(pWindow, IDC_GAME_LIST);
				GetClientRect(lList, &lRect);
	
				lSpec.mask = LVCF_SUBITEM | LVCF_WIDTH | LVCF_FMT;
	
				lSpec.fmt = LVCFMT_LEFT;
				lSpec.cx = lRect.right - 1 - GetSystemMetrics(SM_CXVSCROLL);
				lSpec.iSubItem = 0;
	
				ListView_InsertColumn(lList, 0, &lSpec);
	
				// Connect to server
	
				{
					if(!mThis->AddUserOp(pWindow)) {
						// Clear IP cache
						gsServerAlias = "";
	
						EndDialog(pWindow, IDCANCEL);
					}
					else {
						// Start the automatic refresh sequence
						SetTimer(pWindow, REFRESH_EVENT, 2 * REFRESH_DELAY, NULL);
	
						// Init dialog lists
						mThis->RefreshGameList(pWindow);
						mThis->RefreshUserList(pWindow);
						mThis->RefreshChatOut(pWindow);
					}
				}

				// Subclass the banner button so we can control the behavior.
				mThis->oldBannerProc = (WNDPROC)SetWindowLong(
					GetDlgItem(pWindow, IDC_PUB),
					GWL_WNDPROC,
					(LONG)BannerCallBack);
	
			}
			lReturnValue = TRUE;
	
			// Initiate banners loading in 1 seconds
			if(gNbBannerEntries > 0) {
				SetTimer(pWindow, LOAD_BANNER_TIMEOUT_EVENT, 1000, NULL);
			}
	
			break;

		case WM_QUERYNEWPALETTE:
			if(mThis->mBanner.GetGlobalPalette() != NULL) {
				HWND lBitmapCtl = GetDlgItem(pWindow, IDC_PUB);

				if(lBitmapCtl != NULL) {
					TRACE("PALSET\n");

					HDC hdc = GetDC(lBitmapCtl);

					// UnrealizeObject( mThis->mBanner.GetGlobalPalette() );
					HPALETTE lOldPalette = SelectPalette(hdc, mThis->mBanner.GetGlobalPalette(), FALSE);

					if(RealizePalette(hdc) > 0) {
						lReturnValue = TRUE;
					}

					if(lOldPalette != NULL) {
						SelectPalette(hdc, lOldPalette, FALSE);
					}

					ReleaseDC(lBitmapCtl, hdc);

					// InvalidateRgn( pWindow, NULL, TRUE );
					InvalidateRgn(lBitmapCtl, NULL, TRUE);
					// UpdateWindow( pWindow );
					// UpdateWindow( lBitmapCtl );

					// lReturnValue = TRUE;
				}
			}
			break;

		case WM_PALETTECHANGED:
			if((mThis->mBanner.GetGlobalPalette() != NULL) && ((HWND) pWParam != pWindow)) {
				HWND lBitmapCtl = GetDlgItem(pWindow, IDC_PUB);

				if((pWParam != (int) lBitmapCtl) && (lBitmapCtl != NULL)) {
					TRACE("PALCHANGE\n");
					HDC hdc = GetDC(lBitmapCtl);

					// UnrealizeObject( mThis->mBanner.GetGlobalPalette() );
					HPALETTE lOldPalette = SelectPalette(hdc, mThis->mBanner.GetGlobalPalette(), TRUE);

					if(RealizePalette(hdc) > 0) {
						lReturnValue = TRUE;
					}

					if(lOldPalette != NULL) {
						SelectPalette(hdc, lOldPalette, FALSE);
					}

					ReleaseDC(lBitmapCtl, hdc);

					// InvalidateRgn( pWindow, NULL, TRUE );
					InvalidateRgn(lBitmapCtl, NULL, TRUE);
					// UpdateWindow( pWindow );
					// UpdateWindow( lBitmapCtl );

					// lReturnValue = TRUE;

				}
			}
			break;

		case WM_TIMER:
			lReturnValue = TRUE;

			KillTimer(pWindow, pWParam);

			switch (pWParam) {
				case REFRESH_TIMEOUT_EVENT:
					{
						// Cancel the pending call
						mThis->mRefreshRequest.Clear();
						mThis->mNbSuccessiveRefreshTimeOut++;
	
						// Warn the user
						if(mThis->mNbSuccessiveRefreshTimeOut >= 2) {
							mThis->AddChatLine(_("Warning: communication timeout"));
							mThis->RefreshChatOut(pWindow);
						}
						// Initiate a new refresh
						SetTimer(pWindow, REFRESH_EVENT, 1 /*REFRESH_DELAY */ , NULL);
	
					}
					break;

				case REFRESH_EVENT:
					{
						CString lRequest;
	
						lRequest.Format("%s?=REFRESH%%%%%d-%u%%%%%d", (const char *) gServerList[gCurrentServerEntry].mURL, mThis->mCurrentUserIndex, mThis->mCurrentUserId, mThis->mLastRefreshTimeStamp);
						mThis->mRefreshRequest.Send(pWindow, gServerList[gCurrentServerEntry].mAddress, gServerList[gCurrentServerEntry].mPort, lRequest);
	
						// Activate timeout
						SetTimer(pWindow, REFRESH_TIMEOUT_EVENT, REFRESH_TIMEOUT, NULL);
					}
					break;

				case CHAT_TIMEOUT_EVENT:
					{
						// Cancel the pending call
						mThis->mChatRequest.Clear();
	
						// Warn the user
						mThis->AddChatLine(_("Warning: communication timeout"));
						mThis->RefreshChatOut(pWindow);
					}
					break;

				case LOAD_BANNER_TIMEOUT_EVENT:
					{
						// Already start timer for next load
						if(gNbBannerEntries > 0) {
							// TRACE( "LoadBanner\n" );
	
							int lNextEntry = (gCurrentBannerEntry + 1) % gNbBannerEntries;
	
							SetTimer(pWindow, LOAD_BANNER_TIMEOUT_EVENT, gBannerList[lNextEntry].mDelay * 1000, NULL);
	
							// Initiate the loading of a new banner
	
							if(mThis->mBannerRequest.Send(pWindow, gBannerList[lNextEntry].mAddress, gBannerList[lNextEntry].mPort, gBannerList[lNextEntry].mURL)) {
								// No timeout on that one
							}
						}
	
					}
					break;

				case ANIM_BANNER_TIMEOUT_EVENT:
					{
						TRACE("RefreshBanner\n");
						int lNextRefresh = mThis->RefreshBanner(pWindow);
	
						if(lNextRefresh != 0) {
							SetTimer(pWindow, ANIM_BANNER_TIMEOUT_EVENT, lNextRefresh, NULL);
						}
					}
					break;

			}
			break;

		case MRM_NET_EVENT:

			if(mThis->mChatRequest.ProcessEvent(pWParam, pLParam)) {
				if(mThis->mChatRequest.IsReady()) {
					// Simply reset
					KillTimer(pWindow, CHAT_TIMEOUT_EVENT);
					mThis->mChatRequest.Clear();
				}
			}

			if(mThis->mRefreshRequest.ProcessEvent(pWParam, pLParam)) {
				if(mThis->mRefreshRequest.IsReady()) {
					KillTimer(pWindow, REFRESH_TIMEOUT_EVENT);

					const char *lAnswer = mThis->mRefreshRequest.GetBuffer();

					mThis->mNbSuccessiveRefreshTimeOut = 0;

					if(!mThis->VerifyError(pWindow, lAnswer)) {
						EndDialog(pWindow, IDCANCEL);
					}
					else {
						// We must now parse the answer
						int lToRefresh = mThis->ParseState(lAnswer);

						if(lToRefresh & eGamesModified) {
							mThis->RefreshGameList(pWindow);
						}

						if(lToRefresh & eUsersModified) {
							mThis->RefreshUserList(pWindow);
						}

						if(lToRefresh & eChatModified) {
							mThis->RefreshChatOut(pWindow);
						}
						// Schedule a new refresh

						SetTimer(pWindow, REFRESH_EVENT, REFRESH_DELAY, NULL);
					}
					mThis->mRefreshRequest.Clear();
				}
			}

			if(mThis->mBannerRequest.ProcessEvent(pWParam, pLParam)) {
				//TRACE("LoadBanner\n");

				if(mThis->mBannerRequest.IsReady()) {
					int lBufferSize;
					const char *lBuffer = mThis->mBannerRequest.GetBinBuffer(lBufferSize);

					if(lBuffer != NULL) {
						// Kill animation timer
						KillTimer(pWindow, ANIM_BANNER_TIMEOUT_EVENT);

						const char *lGifBuf = NULL;
						// Find the GIF8 string indicating start of buffer
						// I hope that they wont create a GIF97 format

						int lEntry = (gCurrentBannerEntry + 1) % gNbBannerEntries;

						gBannerList[lEntry].mLastCookie = "";

						for(int lCounter = 0; lCounter < min(lBufferSize - 30, 400); lCounter++) {
							if(lBuffer[lCounter] == 'S') {
								if(!strncmp(lBuffer + lCounter, "Set-Cookie:", 11)) {
									// of we found a cookie
									// skip spaces
									lCounter += 11;
									while(isspace(lBuffer[lCounter])) {
										lCounter++;
									}

									if(!gBannerList[lEntry].mLastCookie.IsEmpty()) {
										gBannerList[lEntry].mLastCookie += "; ";
									}

									while((lBuffer[lCounter] != '\n') && (lBuffer[lCounter] != '\r') && (lBuffer[lCounter] != ';')) {
										gBannerList[lEntry].mLastCookie += lBuffer[lCounter++];
									}
								}
							}

							if(lBuffer[lCounter] == 'G') {
								if(!strncmp(lBuffer + lCounter, "GIF8", 4)) {
									lBufferSize -= lCounter;
									lGifBuf = lBuffer + lCounter;
									break;
								}
							}
						}

						if(lGifBuf != NULL) {
							int lNextRefresh = mThis->LoadBanner(pWindow, lGifBuf, lBufferSize);

							gCurrentBannerEntry = lEntry;

							if(lNextRefresh != 0) {
								SetTimer(pWindow, ANIM_BANNER_TIMEOUT_EVENT, lNextRefresh, NULL);
							}
						}
					}
				}
			}

			if(mThis->mClickRequest.ProcessEvent(pWParam, pLParam)) {
				TRACE("ClickBannerReady\n");

				if(mThis->mClickRequest.IsReady()) {
					// Find the location URL and load it
					const char *lLocation = strstr(mThis->mClickRequest.GetBuffer(), "Location:");

					if(lLocation != NULL) {
						char lURLBuffer[300];

						lURLBuffer[0] = 0;
						sscanf(lLocation + 9, " %299s", lURLBuffer);
						lURLBuffer[299] = 0;

						if(strlen(lURLBuffer) > 0) {
							LoadURL(pWindow, lURLBuffer);
						}
					}
					mThis->mClickRequest.Clear();
				}
			}

			break;

		case WM_NOTIFY:
			{
				NMHDR *lNotMessage = (NMHDR *) pLParam;
	
				switch (lNotMessage->idFrom) {
					case IDC_GAME_LIST:
						if(lNotMessage->code == LVN_ITEMCHANGED) {
							lReturnValue = TRUE;
							mThis->RefreshGameSelection(pWindow);
						}
						break;

				case IDC_USER_LIST:
					if(lNotMessage->code == LVN_ITEMCHANGED) {
						lReturnValue = TRUE;

						// Select the game corresponding to the selected
						mThis->SelectGameForUser(pWindow);
					}
					break;
			}
		}
		break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDOK:
					if(GetFocus() == GetDlgItem(pWindow, IDC_CHAT_IN)) {
						wchar_t lBuffer[200];

						lReturnValue = TRUE;

						GetDlgItemTextW(pWindow, IDC_CHAT_IN, lBuffer, sizeof(lBuffer));

						CString lRequest;

						lRequest.Format("%s?=ADD_CHAT%%%%%d-%u%%%%%s",
							(const char *) gServerList[gCurrentServerEntry].mURL,
							mThis->mCurrentUserIndex, mThis->mCurrentUserId,
							(const char *) MR_Pad(Str::WU(lBuffer)));

						if(mThis->mChatRequest.Send(pWindow, gServerList[gCurrentServerEntry].mAddress, gServerList[gCurrentServerEntry].mPort, lRequest)) {
							SetDlgItemText(pWindow, IDC_CHAT_IN, "");

							// Activate timeout
							SetTimer(pWindow, CHAT_TIMEOUT_EVENT, CHAT_TIMEOUT, NULL);
						}
						lReturnValue = TRUE;
					}
					break;

				case IDCANCEL:
					mThis->DelUserOp(pWindow);
					EndDialog(pWindow, IDCANCEL);
					lReturnValue = TRUE;
					break;

				case IDC_JOIN:
					lReturnValue = TRUE;

					if(mThis->mModelessDlg == NULL) {
						// First verify if the selected track can be played
						int lFocus = FindFocusItem(GetDlgItem(pWindow, IDC_GAME_LIST));

						if(lFocus != -1) {
							BOOL lSuccess = FALSE;

							// Register to the InternetServer
							lSuccess = mThis->JoinGameOp(pWindow, lFocus);

							if(lSuccess) {
								// Try to load the track
								// Load the track
								std::string lCurrentTrack((const char*)mThis->mGameList[lFocus].mTrack);
								MR_RecordFile *lTrackFile = MR_TrackOpen(pWindow, lCurrentTrack.c_str());
								if (lTrackFile == NULL) {
									lSuccess = TrackDownloadDialog(lCurrentTrack).ShowModal(GetModuleHandle(NULL), pWindow);
									if (lSuccess) {
										lTrackFile = MR_TrackOpen(pWindow, lCurrentTrack.c_str());
										if (lTrackFile == NULL) {
											lSuccess = FALSE;
										}
									}
								}

								if (lSuccess) {
									lSuccess = mThis->mSession->LoadNew(mThis->mGameList[lFocus].mTrack, lTrackFile, mThis->mGameList[lFocus].mNbLap, mThis->mGameList[lFocus].mAllowWeapons, mThis->mVideoBuffer);
								}

								if(lSuccess) {
									// connect to the game server
									CString lCurrentTrack;

									lCurrentTrack.Format("%s  %d laps %s", (const char *) mThis->mGameList[lFocus].mTrack, mThis->mGameList[lFocus].mNbLap);

									lSuccess = mThis->mSession->ConnectToServer(pWindow, mThis->mGameList[lFocus].mIPAddr, mThis->mGameList[lFocus].mPort, lCurrentTrack, &mThis->mModelessDlg, MRM_DLG_END_JOIN);
								}

								if(!lSuccess) {
									// Unregister from Game
									mThis->LeaveGameOp(pWindow);
								}
							}
						}
					}

					break;

				case IDC_ADD:
					lReturnValue = TRUE;

					if(mThis->mModelessDlg == NULL) {
						bool lSuccess = false;

						// Ask the user to select a track
						std::string lCurrentTrack;
						int lNbLap;
						bool lAllowWeapons;

						lSuccess = MR_SelectTrack(pWindow, lCurrentTrack, lNbLap, lAllowWeapons);

						if(lSuccess) {
							// Load the track
							MR_RecordFile *lTrackFile = MR_TrackOpen(pWindow, lCurrentTrack.c_str());
							lSuccess = (mThis->mSession->LoadNew(lCurrentTrack.c_str(), lTrackFile, lNbLap, lAllowWeapons, mThis->mVideoBuffer) != FALSE);
						}

						if(lSuccess) {
							// Register to the InternetServer
							lSuccess = (mThis->AddGameOp(pWindow,
								lCurrentTrack.c_str(), lCurrentTrack.c_str(),
								lNbLap, lAllowWeapons,
								Config::GetInstance()->net.tcpServPort) != FALSE);

							if(lSuccess) {
								// Wait client registration
								CString lTrackName;

								lTrackName.Format("%s  %d laps %s", lCurrentTrack.c_str(), lNbLap, lAllowWeapons ? "with weapons" : "no weapons");

								lSuccess = (mThis->mSession->WaitConnections(pWindow,
									lTrackName, FALSE,
									Config::GetInstance()->net.tcpServPort,
									&mThis->mModelessDlg, MRM_DLG_END_ADD) != FALSE);

								if(!lSuccess) {
									// Unregister Game
									mThis->DelGameOp(pWindow);
								}
							}
						}
					}
					break;

				case IDC_PUB:
					{
						if(!gBannerList[gCurrentBannerEntry].mClickURL.IsEmpty()) {
							if(gBannerList[gCurrentBannerEntry].mIndirectClick) {
								mThis->mClickRequest.Clear();
								mThis->mClickRequest.Send(pWindow, gBannerList[gCurrentBannerEntry].mAddress, gBannerList[gCurrentBannerEntry].mPort, gBannerList[gCurrentBannerEntry].mClickURL, gBannerList[gCurrentBannerEntry].mLastCookie);
	
							}
							else {
								LoadURL(pWindow, gBannerList[gCurrentBannerEntry].mClickURL);
							}
						}
					}
					break;

			}
			break;

		case MRM_DLG_END_ADD:
		case MRM_DLG_END_JOIN:
			lReturnValue = TRUE;

			mThis->mModelessDlg = NULL;

			if(pWParam == IDOK) {
				// Unregister user and game
				mThis->DelUserOp(pWindow, TRUE);

				// Quit with a success
				EndDialog(pWindow, IDOK);

				// Blink for user notification
				if(GetForegroundWindow() != GetParent(pWindow)) {
					FLASHWINFO lFlash;
					lFlash.cbSize = sizeof(lFlash);
					lFlash.hwnd = GetParent(pWindow);
					lFlash.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
					lFlash.uCount = 5;
					lFlash.dwTimeout = 0;

					FlashWindowEx(&lFlash);
				}
			}
			else {
				// Unregister Game
				if(pMsgId == MRM_DLG_END_ADD) {
					mThis->DelGameOp(pWindow);
				}
				else {
					mThis->LeaveGameOp(pWindow);
				}
			}
			break;

		case WM_DESTROY:
			{
				if(mThis->mModelessDlg != NULL) {
					// DestroyWindow( mThis->mModelessDlg );
					mThis->mModelessDlg = NULL;
				}
				mThis->mOpRequest.Clear();
				mThis->mChatRequest.Clear();
				mThis->mRefreshRequest.Clear();
			}
			break;
	}

	return lReturnValue;
}

BOOL CALLBACK MR_InternetRoom::BannerCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	switch (pMsgId) {
		case WM_SETCURSOR:
			if (LOWORD(pLParam) == HTCLIENT) {
				SetCursor(LoadCursor(NULL, IDC_HAND));
			}
			return TRUE;

		default:
			return CallWindowProc(mThis->oldBannerProc, pWindow, pMsgId, pWParam, pLParam);
	}
}

BOOL CALLBACK MR_InternetRoom::GetAddrCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	BOOL lReturnValue = FALSE;

	static unsigned long gServerIP;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
			{
				SetWindowTextW(pWindow, Str::UW(_("Network Transaction Progress")));

				gServerIP = 0;
	
				// Setup message
				SetDlgItemTextW(pWindow, IDC_TEXT, Str::UW(_("Locating the main server...")));
	
				// Try to find the IP addr of the Internet Room
				char lServer[40];
	
				int lLen = mThis->mMainServer.find('/', 0);
	
				if(lLen < 0) {
					lLen = mThis->mMainServer.length();
				}
				else if(lLen > (sizeof(lServer) - 1)) {
					lLen = sizeof(lServer) - 1;
				}
	
				memcpy(lServer, mThis->mMainServer.c_str(), lLen);
				lServer[lLen] = 0;
	
				mThis->mCurrentLocateRequest = WSAAsyncGetHostByName(pWindow, MRM_DNS_ANSWER, lServer, mThis->mHostEnt, MAXGETHOSTSTRUCT);
				gServerIP = 0;
	
				// start a timeout timer
				SetTimer(pWindow, 1, 20000 + OP_TIMEOUT, NULL);
			}
			break;

		case WM_TIMER:
			{
				KillTimer(pWindow, pWParam);
				lReturnValue = TRUE;
	
				switch (pWParam) {
					case 1:
						if(gServerIP == 0) {
							MessageBoxW(pWindow, 
								Str::UW(_("Unable to find the server.\n"
									"You must be connected to the Internet to use this function.\n"
									"If the problem persists, you can report it to the\n"
									"HoverRace forums, in the 'Help Desk' section.")),
								Str::UW(_("Internet Meeting Room")),
								MB_ICONSTOP | MB_OK | MB_APPLMODAL);
							
							EndDialog(pWindow, IDCANCEL);
	
							WSACancelAsyncRequest(mThis->mCurrentLocateRequest);
						}
						break;

					case 2:
						MessageBoxW(pWindow, Str::UW(_("Connection timeout")), 
							Str::UW(_("Internet Meeting Room")), MB_ICONSTOP | MB_OK | MB_APPLMODAL);

						EndDialog(pWindow, IDCANCEL);

						mThis->mOpRequest.Clear();
						break;

				}
				break;
			}
			break;

		case MRM_DNS_ANSWER:
			{
				const struct hostent *lData = (struct hostent *) &(mThis->mHostEnt);
	
				KillTimer(pWindow, 1);
	
				if((WSAGETASYNCERROR(pLParam) == 0) && (lData->h_addr_list[0] != NULL)) {
					gServerIP = *(unsigned long *) (lData->h_addr_list[0]);
	
					// Now fetch the server list file
	
					// Fill the request
					mThis->mNetOpRequest = mThis->mMainServer.c_str();
	
					// Initiate the request
					mThis->mOpRequest.Send(pWindow, gServerIP, MR_IR_LIST_PORT, mThis->mNetOpRequest);
	
					// start a timeout timer
					SetTimer(pWindow, 2, 10000 + OP_TIMEOUT, NULL);
	
					// Return a success
					//EndDialog( pWindow, IDOK );
				}
				else {
					lReturnValue = TRUE;
													  // Generate a time-out
					SetTimer(pWindow, 1, IMMEDIATE, NULL);
				}
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					WSACancelAsyncRequest(mThis->mCurrentLocateRequest);
					KillTimer(pWindow, 1);
					KillTimer(pWindow, 2);
					mThis->mOpRequest.Clear();

					lReturnValue = TRUE;
					break;
			}
			break;

		case MRM_NET_EVENT:
			mThis->mOpRequest.ProcessEvent(pWParam, pLParam);

			if(mThis->mOpRequest.IsReady()) {
				KillTimer(pWindow, 2);

				// Parse the result here
				const char *lData = mThis->mOpRequest.GetBuffer();

				while((lData != NULL) && strncmp(lData, "SERVER LIST", 11)) {
					lData = GetNextLine(lData);
				}

				if(lData == NULL) {
					// Server down or fine not found :(
					MessageBoxW(pWindow, Str::UW(_("Server error; please contact the server administrator")),
						Str::UW(_("Internet Meeting Room")), MB_ICONSTOP | MB_OK | MB_APPLMODAL);

					EndDialog(pWindow, IDCANCEL);
				}
				else {

					lData = GetNextLine(lData);

					while(lData != NULL) {
						char lNameBuffer[40];
						char lURLBuffer[120];
						// Ladder code was never actually used but is at least half-implemented
						// char lURLLadderReport[120];
						char lClickURLBuffer[220];
						unsigned lNibble[4];
						// unsigned lNibble2[4];
						int lServerType = -1;

						sscanf(lData, "%d", &lServerType);

						switch (lServerType) {
							case 0:
								{
									if(sscanf(lData, "%d %40s %u.%u.%u.%u %u %120s", &gScoreServer.mType, lNameBuffer, &lNibble[0], &lNibble[1], &lNibble[2], &lNibble[3], &gScoreServer.mPort, lURLBuffer) == 8) {
										gScoreServer.mName = lNameBuffer;
										gScoreServer.mURL = lURLBuffer;
										gScoreServer.mAddress = lNibble[0]
											+ (lNibble[1] << 8)
											+ (lNibble[2] << 16)
											+ (lNibble[3] << 24);
	
									}
								}
								break;

							case 1:
								if(gNbServerEntries < MR_MAX_SERVER_ENTRIES) {
									if(sscanf(lData, "%d %40s %u.%u.%u.%u %u %120s", &gServerList[gNbServerEntries].mType, lNameBuffer, &lNibble[0], &lNibble[1], &lNibble[2], &lNibble[3], &gServerList[gNbServerEntries].mPort, lURLBuffer) == 8) {
										gServerList[gNbServerEntries].mName = lNameBuffer;
										gServerList[gNbServerEntries].mURL = lURLBuffer;
										gServerList[gNbServerEntries].mAddress = lNibble[0]
											+ (lNibble[1] << 8)
											+ (lNibble[2] << 16)
											+ (lNibble[3] << 24);

										gNbServerEntries++;
									}
								}
								break;

							case 2:
								/*
								   if( gNbServerEntries < MR_MAX_SERVER_ENTRIES )
									   {
									   if( sscanf( lData, "%d %40s %u.%u.%u.%u %u %120s %u.%u.%u.%u %u %120s",
									   &gServerList[ gNbServerEntries ].mType,
									   lNameBuffer,
									   &lNibble[0],
									   &lNibble[1],
									   &lNibble[2],
									   &lNibble[3],
									   &gServerList[ gNbServerEntries ].mPort,
									   lURLBuffer,
									   &lNibble2[0],
									   &lNibble2[1],
									   &lNibble2[2],
									   &lNibble2[3],
									   &gServerList[ gNbServerEntries ].mLadderPort,
									   lURLLadderReport ) == 14                        )
									   {
									   gServerList[ gNbServerEntries ].mName            = lNameBuffer;
									   gServerList[ gNbServerEntries ].mURL             = lURLBuffer;
									   gServerList[ gNbServerEntries ].mLadderReportURL = lURLLadderReport;
									   gServerList[ gNbServerEntries ].mAddress =   lNibble[0]
									   +(lNibble[1]<<8)
									   +(lNibble[2]<<16)
									   +(lNibble[3]<<24);
	
									   gServerList[ gNbServerEntries ].mLadderIP =   lNibble2[0]
									   +(lNibble2[1]<<8)
									   +(lNibble2[2]<<16)
									   +(lNibble2[3]<<24);
	
									   gNbServerEntries++;
									   }
									   }
									 */
									break;

							case 8:
							case 9:
								if((lServerType == 8) && (mThis->mMajorID >= 0)) {
									break;
								}
								if((lServerType == 9) && (mThis->mMajorID < 0)) {
									break;
								}

								if(gNbBannerEntries < MR_MAX_BANNER_ENTRIES) {
									if(sscanf(lData, "%d %40s %u.%u.%u.%u %u %120s %d %220s", &gBannerList[gNbServerEntries].mType, lNameBuffer, &lNibble[0], &lNibble[1], &lNibble[2], &lNibble[3], &gBannerList[gNbBannerEntries].mPort, lURLBuffer, &gBannerList[gNbBannerEntries].mDelay, lClickURLBuffer) == 10) {
										gBannerList[gNbBannerEntries].mName = lNameBuffer;
										gBannerList[gNbBannerEntries].mURL = lURLBuffer;
										gBannerList[gNbBannerEntries].mClickURL = lClickURLBuffer;
										gBannerList[gNbBannerEntries].mAddress = lNibble[0]
											+ (lNibble[1] << 8)
											+ (lNibble[2] << 16)
											+ (lNibble[3] << 24);

										gBannerList[gNbBannerEntries].mLastCookie = "";
										gBannerList[gNbBannerEntries].mIndirectClick = (strstr(lClickURLBuffer, "//") == NULL);
										gNbBannerEntries++;
									}
								}

								break;
						}

						lData = GetNextLine(lData);
					}

					EndDialog(pWindow, IDOK);
				}
			}
			lReturnValue = TRUE;
			break;

	}
	return lReturnValue;
}

BOOL CALLBACK MR_InternetRoom::NetOpCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
			{
				// Setup message
				SetWindowTextW(pWindow, Str::UW(_("Network Transaction Progress")));
				SetDlgItemTextW(pWindow, IDC_TEXT, Str::UW(mThis->mNetOpString));
	
				// Initiate the request
				mThis->mOpRequest.Send(pWindow, gServerList[gCurrentServerEntry].mAddress, gServerList[gCurrentServerEntry].mPort, mThis->mNetOpRequest);
	
				// start a timeout timer
				SetTimer(pWindow, OP_TIMEOUT_EVENT, OP_TIMEOUT, NULL);
			}
			break;

		case WM_TIMER:
			{
				KillTimer(pWindow, pWParam);
				lReturnValue = TRUE;
	
				MessageBoxW(pWindow, Str::UW(_("Connection timeout")), 
					Str::UW(_("Internet Meeting Room")), MB_ICONSTOP | MB_OK | MB_APPLMODAL);
	
				EndDialog(pWindow, IDCANCEL);
	
				mThis->mOpRequest.Clear();
	
			}
			break;

		case MRM_NET_EVENT:
			mThis->mOpRequest.ProcessEvent(pWParam, pLParam);

			if(mThis->mOpRequest.IsReady()) {
				KillTimer(pWindow, OP_TIMEOUT_EVENT);

				BOOL lError = mThis->VerifyError(pWindow, mThis->mOpRequest.GetBuffer());

				EndDialog(pWindow, lError ? IDOK : IDCANCEL);
			}
			lReturnValue = TRUE;
			break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					KillTimer(pWindow, OP_TIMEOUT_EVENT);
					mThis->mOpRequest.Clear();

					lReturnValue = TRUE;
					break;
			}
			break;
	}
	return lReturnValue;

}

BOOL CALLBACK MR_InternetRoom::FastNetOpCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
			{
				// Setup message
				SetWindowTextW(pWindow, Str::UW(_("Network Transaction Progress")));
				SetDlgItemTextW(pWindow, IDC_TEXT, Str::UW(mThis->mNetOpString));
	
				// Initiate the request
				mThis->mOpRequest.Send(pWindow, gServerList[gCurrentServerEntry].mAddress, gServerList[gCurrentServerEntry].mPort, mThis->mNetOpRequest);
	
				// start a timeout timer
				SetTimer(pWindow, OP_TIMEOUT_EVENT, FAST_OP_TIMEOUT, NULL);
			}
			break;

		case WM_TIMER:
			{
				KillTimer(pWindow, pWParam);
				lReturnValue = TRUE;
				EndDialog(pWindow, IDCANCEL);
			}
			break;

		case MRM_NET_EVENT:
			mThis->mOpRequest.ProcessEvent(pWParam, pLParam);

			if(mThis->mOpRequest.IsReady()) {
				KillTimer(pWindow, OP_TIMEOUT_EVENT);

				// BOOL lError = mThis->VerifyError( pWindow, mThis->mOpRequest.GetBuffer() );

				EndDialog(pWindow, IDOK);		  // humm always return IDOK
			}
			lReturnValue = TRUE;
			break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					KillTimer(pWindow, OP_TIMEOUT_EVENT);
					mThis->mOpRequest.Clear();

					lReturnValue = TRUE;
					break;
			}
			break;
	}
	return lReturnValue;
}

CString gScoreRequestStr;
MR_InternetRequest gScoreRequest;

BOOL CALLBACK UpdateScoresCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG:
			{
				// Setup message
				SetWindowTextW(pWindow, Str::UW(_("Network Transaction Progress")));
				SetDlgItemTextW(pWindow, IDC_TEXT, Str::UW(_("Registering your best lap time to the server")));
	
				// Initiate the request
				gScoreRequest.Send(pWindow, gScoreServer.mAddress, gScoreServer.mPort, gScoreRequestStr);
	
				// start a timeout timer
				SetTimer(pWindow, OP_TIMEOUT_EVENT, SCORE_OP_TIMEOUT, NULL);
			}
			break;

		case WM_TIMER:
			{
				// Timeout
				gScoreRequest.Clear();
				KillTimer(pWindow, pWParam);
	
				// Ask the user if he want to retry
				if(MessageBoxW(pWindow, Str::UW(_("Connection timeout")), Str::UW(_("HoverRace")), 
					MB_ICONSTOP | MB_RETRYCANCEL | MB_APPLMODAL) == IDRETRY) {
					// Initiate the request
					gScoreRequest.Send(pWindow, gScoreServer.mAddress, gScoreServer.mPort, gScoreRequestStr);
					// start a timeout timer
					SetTimer(pWindow, OP_TIMEOUT_EVENT, SCORE_OP_TIMEOUT + 3000, NULL);
				}
				else {
					EndDialog(pWindow, IDCANCEL);
				}
				lReturnValue = TRUE;
			}
			break;

		case MRM_NET_EVENT:
			gScoreRequest.ProcessEvent(pWParam, pLParam);

			if(gScoreRequest.IsReady()) {
				KillTimer(pWindow, OP_TIMEOUT_EVENT);

				EndDialog(pWindow, IDOK);		  // humm always return IDOK
			}
			lReturnValue = TRUE;
			break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					KillTimer(pWindow, OP_TIMEOUT_EVENT);
					gScoreRequest.Clear();

					lReturnValue = TRUE;
					break;
			}
			break;
	}
	return lReturnValue;
}

BOOL MR_SendLadderResult(HWND pParentWindow, const char *pWinAlias, int pWinMajorID, int pWinMinorID, const char *pLoseAlias, int pLoseMajorID, int pLoseMinorID, const char *pTrack, int pNbLap)
{
	BOOL lReturnValue = FALSE;

	// First verify that both user are regs and the room is a ladder room
	if((gCurrentServerEntry != -1) && (gServerList[gCurrentServerEntry].mType == 2) && (pWinMajorID != -1) && (pLoseMajorID != -1)) {
		CString lWinID;
		CString lLoseID;
		CString lLaps;

		lWinID.Format("%d-%d", pWinMajorID, pWinMinorID);
		lLoseID.Format("%d-%d", pLoseMajorID, pLoseMinorID);
		lLaps.Format("%d", pNbLap);

		MReport_AddVariable("winalias", pWinAlias, "Winner");
		MReport_AddVariable("winid", lWinID, "Winner ID");
		MReport_AddVariable("losealias", pLoseAlias, "Loser");
		MReport_AddVariable("loseid", lLoseID, "Loser ID");
		MReport_AddVariable("track", pTrack);
		MReport_AddVariable("laps", lLaps, "Laps");

		lReturnValue = MReport_Process(pParentWindow, gServerList[gCurrentServerEntry].mLadderIP, gServerList[gCurrentServerEntry].mLadderPort, gServerList[gCurrentServerEntry].mLadderReportURL);

		MReport_Clear(TRUE);
	}
	return lReturnValue;
}

BOOL MR_SendRaceResult(HWND pParentWindow, const char *pTrack, int pBestLapTime, int pMajorID, int pMinorID, const char *pAlias, unsigned int pTrackSum, int pHoverModel, int pTotalTime, int pNbLap, int pNbPlayer)
{
	BOOL lReturnValue = FALSE;

	if(gScoreServer.mURL.GetLength() > 0) {
		// Create the RequestStrign
		/*if( pMajorID != -1 )
		   {
		   gScoreRequestStr.Format( "%s?=RESULT%%%%%u%%%%%s%%%%%s[%d-%d]%%%%%u%%%%%d%%%%%d%%%%%d%%%%%d",
		   (const char*)gScoreServer.mURL,
		   pBestLapTime,
		   (const char*)MR_Pad( pTrack ),
		   (const char*)MR_Pad( pAlias ),
		   pMajorID,
		   pMinorID,
		   pTrackSum,
		   pHoverModel,
		   pTotalTime,
		   pNbLap,
		   pNbPlayer
		   );
		   }
		   else
		   {
		 */
		gScoreRequestStr.Format("%s?=RESULT%%%%%u%%%%%s%%%%%s%%%%%u%%%%%d%%%%%d%%%%%d%%%%%d", (const char *) gScoreServer.mURL, pBestLapTime, (const char *) MR_Pad(pTrack), (const char *) MR_Pad(pAlias), pTrackSum, pHoverModel, pTotalTime, pNbLap, pNbPlayer);
		//}

		lReturnValue = DialogBoxW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDD_NET_PROGRESS), pParentWindow, UpdateScoresCallBack) == IDOK;

	}
	return lReturnValue;
}

CString MR_Pad(const char *pStr)
{
	CString lReturnValue;

	while(*pStr != 0) {
		/*
		if(isalnum(*(const unsigned char *) pStr)) {
			lReturnValue += *pStr;
		}
		*/
		/*
		else if((*(const unsigned char *) pStr) <= 32) {
			lReturnValue += "%20";
		}
		*/
		/*
		else {
		*/
			/*
			switch (*(const unsigned char *) pStr) {
				case '$':
				case '-':
				case '_':
				case '.':
				case '+':
				case '!':
				case '*':
				case '\'':
				case '(':
				case ')':
				case ',':
				case ':':
				case '@':
				case '&':
				case '=':
				case 187: // don't strip the prompt character anymore

					lReturnValue += *pStr;
					break;

				default:
				{
					*/
					CString lNumber;

					unsigned char c = *pStr;
					lNumber.Format("%%%02x", c);

					lReturnValue += lNumber;
					/*
				}
			}
			*/
		/*
		}
		*/
		pStr++;
	}

	return lReturnValue;

}

CString GetLine(const char *pSrc)
{
	return CString(pSrc, GetLineLen(pSrc));
}

int GetLineLen(const char *pSrc)
{
	int lReturnValue = 0;

	if(pSrc != NULL) {
		const char *lEoL = strchr(pSrc, '\n');

		if(lEoL != NULL) {
			lReturnValue = lEoL - pSrc;
		}
		else {
			lReturnValue = strlen(pSrc);
		}
	}
	return lReturnValue;
}

const char *GetNextLine(const char *pSrc)
{
	const char *lReturnValue = NULL;

	if(pSrc != NULL) {
		lReturnValue = pSrc + GetLineLen(pSrc);

		if(*lReturnValue == 0) {
			lReturnValue = NULL;
		}
		else {
			lReturnValue++;
		}
	}
	return lReturnValue;
}

int FindFocusItem(HWND pWindow)
{
	int lReturnValue = -1;

	int lCount = ListView_GetItemCount(pWindow);

	for(int lCounter = 0; (lReturnValue == -1) && (lCounter < lCount); lCounter++) {
		if(ListView_GetItemState(pWindow, lCounter, LVIS_FOCUSED) == LVIS_FOCUSED) {
			LV_ITEM lItemData;

			lItemData.mask = LVIF_PARAM;
			lItemData.iItem = lCounter;
			lItemData.iSubItem = 0;

			if(ListView_GetItem(pWindow, &lItemData)) {
				lReturnValue = lItemData.lParam;
			}
		}
	}

	return lReturnValue;
}