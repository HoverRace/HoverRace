// InternetRoom.h
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

#pragma once

#include <WINSOCK.h>

#include <string>

#include "../../engine/Parcel/TrackBundle.h"

#include "TrackSelectDialog.h"
#include "NetworkSession.h"
#include "Banner.h"
#include "RoomList.h"

namespace HoverRace {
namespace Client {

class InternetRequest
{
	protected:
		time_t mStartTime;
		BOOL mBinMode;							  // use Bin buffer and Bin sending mode"referer request"

		SOCKET mSocket;
		std::string mRequest;
		std::string mBuffer;

		char *mBinBuffer;
		int mBinIndex;

		void Close();
		BOOL Working() const;

	public:
		InternetRequest();
		~InternetRequest();

		void SetBin();

		BOOL Send(HWND pWindow, unsigned long pIP, unsigned int pPort, const char *pURL, const char *pCoookie = NULL);
		void Clear();							  // Abort and clean
												  // return TRUE if some processing was done
		BOOL ProcessEvent(WPARAM pWParam, LPARAM pLParam);

		const char *GetBuffer() const;
		const char *GetBinBuffer(int &pSize) const;

		BOOL IsReady() const;
};

class InternetRoom
{
	public:
		enum {
			eMaxClient = 300,
			eMaxGame = 100,
			eMaxChat = 50,
			eMaxPlayerGame = 16
		};

		enum {
			eChatModified = 1,
			eUsersModified = 2,
			eGamesModified = 4
		};

	protected:

		class Client
		{
			public:
				BOOL mValid;
				std::string mName;
				int mGame;
				int mMajorID;
				int mMinorID;

		};

		class Game
		{
			public:

				BOOL mValid;
				int mId;
				MR_TrackAvail mAvailCode;
				std::string mName;
				std::string mTrack;
				int mNbLap;
				BOOL mAllowWeapons;
				int mNbClient;
				int mClientList[eMaxPlayerGame];
				std::string mIPAddr;
				unsigned mPort;

		};

		static InternetRoom *mThis;

		RoomListPtr roomList;

		std::string mMainServer;

		int mCurrentUserIndex;
		int mCurrentUserId;
		int mCurrentGameIndex;
		int mCurrentGameId;

		std::string mUser;

		Client mClientList[eMaxClient];
		Game mGameList[eMaxGame];

		std::string mChatBuffer;

		int mLastRefreshTimeStamp;

		HWND mModelessDlg;

		// Temporary data
		HANDLE mCurrentLocateRequest;
		char mHostEnt[MAXGETHOSTSTRUCT];
		std::string mNetOpString;
		std::string mNetOpRequest;
		NetworkSession *mSession;
		VideoServices::VideoBuffer *mVideoBuffer;
		int mNbSuccessiveRefreshTimeOut;

		InternetRequest mOpRequest;
		InternetRequest mChatRequest;
		InternetRequest mRefreshRequest;
		InternetRequest mBannerRequest;
		InternetRequest mClickRequest;

		WNDPROC oldBannerProc;
		GifDecoder mBanner;
		int mCurrentBannerIndex;

		bool checkUpdates;

		std::ofstream *chatLog;

		/*
		static BOOL CALLBACK AskParamsCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		*/
		static BOOL CALLBACK RoomCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK BannerCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK NetOpCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK FastNetOpCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK GetAddrCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);

		BOOL AskRoomParams(HWND pParentWindow, BOOL pShouldRecheckServer);
		/*
		BOOL LocateServers(HWND pWindow, BOOL pShouldRecheckServer);
		*/
		BOOL AddUserOp(HWND pParentWindow);
		BOOL DelUserOp(HWND pParentWindow, BOOL pFastMode = FALSE);
		BOOL AddGameOp(HWND pParentWindow, const char *pGameName, const char *pTrackName, int pNbLap, char pGameOpts, unsigned pPort);
		BOOL DelGameOp(HWND pParentWindow);
		BOOL JoinGameOp(HWND pParentWindow, int pGameIndex);
		BOOL LeaveGameOp(HWND pParentWindow);
		BOOL AddMessageOp(HWND pParentWindow, const char *pMessage, int pNbHours = 0, int lNbMinutes = 0);

		void SelectGameForUser(HWND pWindow);
		void RefreshGameList(HWND pWindow);
		void RefreshGameSelection(HWND pWindow);
		void RefreshUserList(HWND pWindow);
		void RefreshChatOut(HWND pWindow);

		static BOOL VerifyError(HWND pParentWindow, const char *pAnswer);

		BOOL ParseState(const char *pAnswer);

		void OpenChatLog();
		void AddChatLine(const char *pText, bool neverLog=false);

		int LoadBanner(HWND pWindow, const char *pBuffer, int pBufferLen);
		int RefreshBanner(HWND pWindow);		  // Return next refresh time

		static void TrackOpenFailMessageBox(HWND parent, const std::string &name, const std::string &details);

	public:
		InternetRoom(const std::string &pMainServer, bool mustCheckUpdates);
		~InternetRoom();

		RoomListPtr GetRoomList() const { return roomList; }

		BOOL DisplayChatRoom(HWND pParentWindow, NetworkSession *pSession, VideoServices::VideoBuffer *pVideoBuffer, BOOL pShouldRecheckServer);

		// Modelless.. do not use for the moment
		/*
		   BOOL DisplayModeless( HWND pParentWindow, MR_NetworkSession* pSession, VideoServices::VideoBuffer* pVideoBuffer );
		   BOOL IsDisplayed()const;
		 */

};

BOOL MR_SendRaceResult(HWND pParentWindow, const char *pTrack,
	int pBestLapTime, int pMajorID, int pMinorID, const char *pAlias,
	unsigned int pTrackSum, int pHoverModel, int pTotalTime,
	int pNbLap, int pNbPlayer, HoverRace::Client::RoomListPtr roomList);

/*
BOOL MR_SendLadderResult(HWND pParentWindow, const char *pWinAlias,
	int pWinMajorID, int pWinMinorID, const char *pLoseAlias,
	int pLoseMajorID, int pLoseMinorID, const char *pTrack, int pNbLap);
*/

}  // namespace Client
}  // namespace HoverRace
