// NetInterface.h
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

#pragma once

#include <WINSOCK.h>

#include "../../engine/Util/MR_Types.h"
#include "../../engine/Util/Config.h"

#define MR_ID_NOT_SET				255

#define MR_OUT_QUEUE_LEN			2048
#define MR_MAX_NET_MESSAGE_LEN		255

#define MR_NET_REQUIRED				1
#define MR_NET_TRY					2
#define MR_NOT_REQUIRED				0
#define MR_NET_DATAGRAM				-1

namespace HoverRace {
namespace Client {

/**
 * The MR_NetMessageBuffer class is a wrapper for data being transmitted over the network.
 */
class NetMessageBuffer
{
	public:
		// MR_UInt16  mSendingTime:10; // 4ms of precision on a +-2sec range (if a datagram take more than 2 sec to travel..it will be droped (UDP only)
		// Not used yet
		MR_UInt16 mDatagramNumber:8;			  /// used only for datagrams (UDP)
		MR_UInt16 mDatagramQueue:2;				  /// used only for datagrams (UDP)
		MR_UInt16 mMessageType:6;
		MR_UInt8 mClient;
		MR_UInt8 mDataLen;
		MR_UInt8 mData[MR_MAX_NET_MESSAGE_LEN];

		// int MessageLen()const { return mDataLen+5; }
};

#define MR_NET_HEADER_LEN  (sizeof(NetMessageBuffer) - MR_MAX_NET_MESSAGE_LEN)

/**
 * The MR_NetworkPort class is the backbone for a single connection to a peer.
 */
class NetworkPort
{
	private:
		// std::string mNetAddr;
		// int     mPort;
		SOCKET mSocket;

		// UDP Information
		SOCKET mUDPRecvSocket;				/// the UDP socket we listen on
		SOCKADDR_IN mUDPRemoteAddr;			/// address information for the peer

		MR_UInt8 mLastSendedDatagramNumber[4];
		MR_UInt8 mLastReceivedDatagramNumber[4];
		MR_UInt8 mLastClient[4];

		int mWatchdog;

		int mAvgLag;		/// for lag computation
		int mMinLag;		/// for lag computation
		int mNbLagTest;		/// counts the number of lag tests that have been done
		int mTotalLag;		/// for lag computation

		int mInputMessageBufferIndex;
		NetMessageBuffer mInputMessageBuffer;

		// Output queue
		// This queue is only used for messages that must absolutely be sent
		// or those that are partially sent
		MR_UInt8 mOutQueue[MR_OUT_QUEUE_LEN];
		int mOutQueueLen;
		int mOutQueueHead;
	public:
		NetworkPort();
		~NetworkPort();

		void Connect(SOCKET pSocket, SOCKET pUDPRecvSocket);
		void SetRemoteUDPPort(unsigned int pPort);
		void Disconnect();
		BOOL IsConnected() const;

		SOCKET GetSocket() const;
		SOCKET GetUDPSocket() const;

		const NetMessageBuffer *Poll(int pClientId, BOOL pCheckClientId); // parameters are hacks
		void Send(const NetMessageBuffer *pMessage, int pReqLevel);
		BOOL UDPSend(SOCKET pSocket, NetMessageBuffer *pMessage, unsigned pQueueId, BOOL pResendLast);

		// Time related stuff
		BOOL AddLagSample(int pLag);
		BOOL LagDone() const;

		int GetAvgLag() const;
		int GetMinLag() const;
		void SetLag(int pAvgLag, int pMinLag);

		BOOL mTriedBackupIP;	/// if the first connection attempt fails we must try another
};

/**
 * The MR_NetworkInterface is the top-level controller for connecting and communicating with other clients.  The tough part to understand about
 * this class is the connection initiations.
 *
 * The start of the connection process begins in MasterConnect() for the server, SlavePreConnect() for a client connecting not through the IMR, and
 * SlaveConnect() for a client connecting through the IMR.  SlaveConnect() is also called after SlavePreConnect() (if a pre-connect phase was needed).
 * 
 * The "guts" of the connection establishing process can be found in the ListCallBack() dialog callback function.
 */
class NetworkInterface
{
	public:
		enum {
			eMaxClient = 9
		};
	private:
		std::string mPlayer;			/// player name
		int mId;					/// player id
		BOOL mServerMode;
		SOCKET mRegistrySocket;
		int mServerPort;
		int mUDPRecvPort;
		int mTCPRecvPort;
		std::string mServerAddr;
		std::string mGameName;			/// just the track name

		// UDP port
		SOCKET mUDPOutShortPort;
		SOCKET mUDPOutLongPort;
		SOCKET mUDPRecvSocket;		/// for one-port UDP hack

		// Data
		NetworkPort mClient[eMaxClient];
		std::string mClientName[eMaxClient];
		BOOL mAllPreLoguedRecv;					  /// Used by client to know if all prelogued have been received
		BOOL mCanBePreLogued[eMaxClient];
		BOOL mPreLoguedClient[eMaxClient];
		BOOL mConnected[eMaxClient];			  /// Correctly connected state used by the server
		DWORD mClientAddr[eMaxClient];
		DWORD mClientBkAddr[eMaxClient];
		int mClientPort[eMaxClient];

		int mReturnMessage;						  /// Message to return to the parent window in modeless mode

		// Dialog functions
		static NetworkInterface *mActiveInterface;
		static BOOL CALLBACK ServerPortCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK ServerAddrCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK WaitGameNameCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK ListCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK WarnAloneCallback(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK WaitConnCallback(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);

		// Helper function
		void SendConnectionDoneIfNeeded();

	public:
		// Creation and destruction
		NetworkInterface();
		~NetworkInterface();

		void SetPlayerName(const char *pPlayerName);
		const char *GetPlayerName() const;

		BOOL MasterConnect(HWND pWindow, const char *pGameName,
			BOOL pPromptForPort = TRUE,
			unsigned pDefaultPort = HoverRace::Util::Config::GetInstance()->net.tcpServPort,
			HWND * pModalessDlg = NULL, int pReturnMessage = 0);
		BOOL SlavePreConnect(HWND pWindow, std::string &pGameName);
		BOOL SlaveConnect(HWND pWindow, const char *pServerIP = NULL,
			unsigned pPort = HoverRace::Util::Config::GetInstance()->net.tcpServPort,
			const char *pGameName = NULL, HWND * pModalessDlg = NULL,
			int pReturnMessage = 0);

		void Disconnect();

		int GetClientCount() const;
		int GetId() const;

		int GetLagFromServer() const;
		int GetAvgLag(int pClient) const;
		int GetMinLag(int pClient) const;

		// helper function
		BOOL CreateUDPRecvSocket(int pPort);

		// return TRUE if queue not full
		BOOL UDPSend(int pClient, NetMessageBuffer * pMessage, BOOL pLongPort, BOOL pResendLast = FALSE);
		BOOL BroadcastMessage(NetMessageBuffer * pMessage, int pReqLevel);
		// BOOL BroadcastMessage( DWORD  pTimeStamp, int  pMessageType, int pMessageLen, const MR_UInt8* pMessage );
		BOOL FetchMessage(DWORD & pTimeStamp, int &pMessageType, int &pMessageLen, const MR_UInt8 * &pMessage, int &pClientId);
		// pTimeStamp must be set to current time stamp before fetch

		const char *GetPlayerName(int pIndex) const;
		BOOL IsConnected(int pIndex) const;
};

}  // namespace Client
}  // namespace HoverRace
