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

#ifndef NET_INTERFACE_H
#define NET_INTERFACE_H

#include <WINSOCK.h>

#include "../Util/MR_Types.h"
#include "../Util/Config.h"

#define MR_ID_NOT_SET				255

#define MR_OUT_QUEUE_LEN			2048
#define MR_MAX_NET_MESSAGE_LEN		255

#define MR_NET_REQUIRED				1
#define MR_NET_TRY					2
#define MR_NOT_REQUIRED				0
#define MR_NET_DATAGRAM				-1

#define STM_IMR_CHANNEL				100

class ISteamUser;
class ISteamNetworking;

/**
 * The MR_NetMessageBuffer class is a wrapper for data being transmitted over the network.
 */
class MR_NetMessageBuffer
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

#define MR_NET_HEADER_LEN  (sizeof(MR_NetMessageBuffer) - MR_MAX_NET_MESSAGE_LEN)

/**
 * The MR_NetworkPort class is the backbone for a single connection to a peer.
 */
class MR_NetworkPort
{
	private:
		// CString mNetAddr;
		// int     mPort;
		SOCKET mSocket;
		CSteamID mSteamID;

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
		MR_NetMessageBuffer mInputMessageBuffer;

		// Output queue
		// This queue is only used for messages that must absolutely be sent
		// or those that are partially sent
		MR_UInt8 mOutQueue[MR_OUT_QUEUE_LEN];
		int mOutQueueLen;
		int mOutQueueHead;
	public:
		MR_NetworkPort();
		~MR_NetworkPort();

		void Connect(SOCKET pSocket, SOCKET pUDPRecvSocket, CSteamID pSteamID, BOOL pSteamOnly);
		void SetRemoteUDPPort(unsigned int pPort);
		void Disconnect();
		void DisconnectSteam(BOOL pDisconnectSteam = TRUE);
		BOOL IsConnected() const;

		SOCKET GetSocket() const;
		SOCKET GetUDPSocket() const;
		CSteamID GetSteamId() const;

		const MR_NetMessageBuffer *Poll(int pClientId, BOOL pCheckClientId); // parameters are hacks
		void Send(const MR_NetMessageBuffer *pMessage, int pReqLevel, int pClient = STM_IMR_CHANNEL);
		BOOL UDPSend(SOCKET pSocket, MR_NetMessageBuffer *pMessage, unsigned pQueueId, BOOL pResendLast, int pClient = STM_IMR_CHANNEL);

		// Time related stuff
		BOOL AddLagSample(int pLag);
		BOOL LagDone() const;

		int GetAvgLag() const;
		int GetMinLag() const;
		void SetLag(int pAvgLag, int pMinLag);

		void SetInputMessageBuffer(MR_NetMessageBuffer* pInputMessageBuffer);
		MR_NetMessageBuffer GetInputMessageBuffer();

		BOOL mTriedBackupIP;	/// if the first connection attempt fails we must try another

		BOOL mSocketConnected;  // Using Steam otherwise
		BOOL mSteamOnly;
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
class MR_NetworkInterface
{
	public:
		enum {
			eMaxClient = 9
		};
	private:
		CString mPlayer;			/// player name
		int mId;					/// player id
		BOOL mServerMode;
		SOCKET mRegistrySocket;
		int mServerPort;
		int mUDPRecvPort;
		int mTCPRecvPort;
		CString mServerAddr;
		CSteamID mSteamID;
		CString mGameName;			/// just the track name

		// UDP port
		SOCKET mUDPOutShortPort;
		SOCKET mUDPOutLongPort;
		SOCKET mUDPRecvSocket;		/// for one-port UDP hack

		// Static Buffer
		static MR_NetMessageBuffer *sBuffer;

		// Static CSteamID
		static CSteamID sSteamID;
		BOOL mSteamOnly;

		// Data
		MR_NetworkPort mClient[eMaxClient];
		CString mClientName[eMaxClient];
		BOOL mAllPreLoguedRecv;					  /// Used by client to know if all prelogued have been received
		BOOL mCanBePreLogued[eMaxClient];
		BOOL mPreLoguedClient[eMaxClient];
		BOOL mConnected[eMaxClient];			  /// Correctly connected state used by the server
		DWORD mClientAddr[eMaxClient];
		DWORD mClientBkAddr[eMaxClient];
		int mClientPort[eMaxClient];
		CSteamID mClientSteamID[eMaxClient];

		int mReturnMessage;						  /// Message to return to the parent window in modeless mode

		// Game Modal
		HWND mGameModal;
		HWND mConnectModal;

		// Dialog functions
		static MR_NetworkInterface *mActiveInterface;
		static BOOL CALLBACK ServerPortCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK ServerAddrCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK WaitGameNameCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);
		static BOOL CALLBACK ListCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam);

		// Helper function
		void SendConnectionDoneIfNeeded();

		STEAM_CALLBACK( MR_NetworkInterface, OnP2PSessionRequest, P2PSessionRequest_t ); // REQUIRED
		STEAM_CALLBACK( MR_NetworkInterface, OnP2PSessionFailed, P2PSessionConnectFail_t ); // REQUIRED

	public:
		// Creation and destruction
		MR_NetworkInterface();
		~MR_NetworkInterface();

		void SetId(const int id);
		void SetPlayerName(const char *pPlayerName);
		const char *GetPlayerName() const;

		BOOL MasterConnect(HWND pWindow, const char *pGameName, BOOL pPromptForPort = TRUE, unsigned pDefaultPort = MR_Config::GetInstance()->net.tcpServPort, HWND * pModalessDlg = NULL, int pReturnMessage = 0);
		BOOL SlavePreConnect(HWND pWindow, CString & pGameName);
		BOOL SlaveConnect(HWND pWindow, const char *pServerIP = NULL, unsigned pPort = MR_Config::GetInstance()->net.tcpServPort, uint64 pSteamID = 0, const char *pGameName = NULL, HWND * pModalessDlg = NULL, int pReturnMessage = 0);

		int Connect(SOCKET pS, const sockaddr *pName, int pNamelen, CSteamID pSteamID, UINT lMsg, long lEvent);
		SOCKET MR_NetworkInterface::Accept(SOCKET pS, sockaddr *pAddr, int *pAddrlen, CSteamID pSteamID, UINT lMsg, long lEvent, int lClient);

		void Disconnect(BOOL pDisconnectSteam = TRUE);

		int GetClientCount() const;
		int GetId() const;
		CSteamID GetSteamId() const;
		BOOL GetSteamOnly() const;

		int GetLagFromServer() const;
		int GetAvgLag(int pClient) const;
		int GetMinLag(int pClient) const;

		// helper function
		BOOL CreateUDPRecvSocket(int pPort);

		void CheckP2PAvailability() const;

		// return TRUE if queue not full
		BOOL UDPSend(int pClient, MR_NetMessageBuffer * pMessage, BOOL pLongPort, BOOL pResendLast = FALSE);
		BOOL BroadcastMessage(MR_NetMessageBuffer * pMessage, int pReqLevel);
		// BOOL BroadcastMessage( DWORD  pTimeStamp, int  pMessageType, int pMessageLen, const MR_UInt8* pMessage );
		BOOL FetchMessage(DWORD & pTimeStamp, int &pMessageType, int &pMessageLen, const MR_UInt8 * &pMessage, int &pClientId);
		// pTimeStamp must be set to current time stamp before fetch

		const char *GetPlayerName(int pIndex) const;
		BOOL IsConnected(int pIndex) const;
};
#endif