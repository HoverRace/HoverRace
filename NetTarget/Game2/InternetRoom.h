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

#ifndef INTERNET_ROOM_H
#define INTERNET_ROOM_H

#include <WINSOCK.h>

#include "TrackSelect.h"
#include "NetworkSession.h"
#include "Banner.h"



class MR_InternetRequest
{
   protected:
      time_t  mStartTime;
      BOOL    mBinMode; // use Bin buffer and Bin sending mode"referer request"

      SOCKET  mSocket;
      CString mRequest;
      CString mBuffer;

      char*   mBinBuffer;
      int     mBinIndex;

      void Close();
      BOOL Working()const;

   public:
      MR_InternetRequest();
      ~MR_InternetRequest();

      void SetBin();

      BOOL Send( HWND pWindow, unsigned long pIP, unsigned int pPort, const char* pURL, const char* pCoookie = NULL );
      void Clear();  // Abort and clean
      BOOL ProcessEvent( WPARAM pWParam, LPARAM pLParam ); // return TRUE if some processing was done

      const char* GetBuffer()const;
      const char* GetBinBuffer( int& pSize )const;


      BOOL IsReady()const;      
};



class MR_InternetRoom
{
   public:
      enum
      {
         eMaxClient     = 300,
         eMaxGame       = 100,
         eMaxChat       =  50,
         eMaxPlayerGame =  16
      };

      enum
      {
         eChatModified  = 1,
         eUsersModified = 2,
         eGamesModified = 4
      };

   protected:

      class Client      
      {
      public:
         BOOL     mValid;
         CString  mName;
         int      mGame;
         int      mMajorID;
         int      mMinorID;
         
      };

      class Game
      {
      public:

         BOOL            mValid;
         int             mId;
         MR_TrackAvail   mAvailCode;
         CString         mName;
         CString         mTrack;
         int             mNbLap;
         BOOL            mAllowWeapons;
         int             mNbClient;
         int             mClientList[eMaxPlayerGame];
         CString         mIPAddr;
         unsigned        mPort;

      };

      static MR_InternetRoom* mThis;

      int           mCurrentUserIndex;
      int           mCurrentUserId;
      int           mCurrentGameIndex;
      int           mCurrentGameId;

      int           mMajorID;
      int           mMinorID;
      CString       mUser;
      unsigned      mKey2;
      unsigned      mKey3;
      // CString       mServerURL;
      // unsigned long mServerIP;
      // CString       mURLPath;

      Client        mClientList[ eMaxClient ];
      Game          mGameList[ eMaxGame ];

      CString       mChatBuffer;

      int           mLastRefreshTimeStamp;

      HWND          mModelessDlg;

      BOOL          mAllowRegistred;

      // Temporary data
      HANDLE             mCurrentLocateRequest;           
      char               mHostEnt[MAXGETHOSTSTRUCT];
      CString            mNetOpString;
      CString            mNetOpRequest;
      MR_NetworkSession* mSession;
      MR_VideoBuffer*    mVideoBuffer;
      int                mNbSuccessiveRefreshTimeOut;

      MR_InternetRequest mOpRequest;
      MR_InternetRequest mChatRequest;
      MR_InternetRequest mRefreshRequest;      
      MR_InternetRequest mBannerRequest;      
      MR_InternetRequest mClickRequest;      

      MR_GifDecoder      mBanner;
      int                mCurrentBannerIndex;

      static BOOL CALLBACK AskPasswordCallBack( HWND pWindow, UINT  pMsgId, WPARAM  pWParam, LPARAM  pLParam );
      static BOOL CALLBACK AskParamsCallBack(   HWND pWindow, UINT  pMsgId, WPARAM  pWParam, LPARAM  pLParam );
      static BOOL CALLBACK RoomCallBack(        HWND pWindow, UINT  pMsgId, WPARAM  pWParam, LPARAM  pLParam );
      static BOOL CALLBACK NetOpCallBack(       HWND pWindow, UINT  pMsgId, WPARAM  pWParam, LPARAM  pLParam );
      static BOOL CALLBACK FastNetOpCallBack(   HWND pWindow, UINT  pMsgId, WPARAM  pWParam, LPARAM  pLParam );
      static BOOL CALLBACK GetAddrCallBack(     HWND pWindow, UINT  pMsgId, WPARAM  pWParam, LPARAM  pLParam );
      
      BOOL AskRoomParams( HWND pParentWindow );
      BOOL LocateServers( HWND pWindow );
      BOOL AddUserOp( HWND pParentWindow );
      BOOL DelUserOp( HWND pParentWindow, BOOL pFastMode = FALSE );      
      BOOL AddGameOp( HWND pParentWindow, const char* pGameName, const char* pTrackName, int pNbLap, BOOL pAlllowWeapons, unsigned pPort );
      BOOL DelGameOp( HWND pParentWindow );
      BOOL JoinGameOp( HWND pParentWindow, int pGameIndex );
      BOOL LeaveGameOp( HWND pParentWindow );
      BOOL AddMessageOp( HWND pParentWindow, const char* pMessage, int pNbHours=0, int lNbMinutes=0 );

      void SelectGameForUser( HWND pWindow );
      void RefreshGameList( HWND pWindow );
      void RefreshGameSelection( HWND pWindow );
      void RefreshUserList( HWND pWindow );
      void RefreshChatOut( HWND pWindow );


      static BOOL VerifyError( HWND pParentWindow, const char* pAnswer );

      BOOL ParseState( const char* pAnswer );

      void AddChatLine( const char* pText );

      int LoadBanner( HWND pWindow, const char* pBuffer, int pBufferLen );
      int RefreshBanner( HWND pWindow ); // Return next refresh time

   public:
      MR_InternetRoom( BOOL pAllowRegistred, int pMajorID, int pMinorID, unsigned pKey2, unsigned pKey3 );
      ~MR_InternetRoom();

      BOOL DisplayChatRoom( HWND pParentWindow, MR_NetworkSession* pSession, MR_VideoBuffer* pVideoBuffer );


      // Modelless.. do not use for the moment
      /*
      BOOL DisplayModeless( HWND pParentWindow, MR_NetworkSession* pSession, MR_VideoBuffer* pVideoBuffer );
      BOOL IsDisplayed()const;
      */

};



BOOL MR_SendRaceResult( HWND pParentWindow, const char* pTrack, int pBestLapTime, int pMajorID, int pMinorID, const char* pAlias, unsigned int pTrackSum, int pHoverModel, int pTotalTime, int pNbLap, int pNbPlayer );
BOOL MR_SendLadderResult( HWND pParentWindow,
                          const char* pWinAlias,  int pWinMajorID,  int pWinMinorID,
                          const char* pLoseAlias, int pLoseMajorID, int pLoseMinorID,
                          const char* pTrack, int pNbLap
                        );




#endif

