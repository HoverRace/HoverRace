// NetInterface.cpp
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

#include <Mmsystem.h>

#include "NetInterface.h"
#include "resource.h"
#include "../../engine/Util/StrRes.h"
#include "../../engine/Util/OS.h"
#include "../../engine/Util/Str.h"

using HoverRace::Util::Config;

// Private window messages
#define MRM_SERVER_CONNECT (WM_USER + 1)
#define MRM_NEW_CLIENT     (WM_USER + 2)
#define MRM_CLIENT         (WM_USER + 10)		  // 64 next reserved

#define MRNM_GET_GAME_NAME		40
#define MRNM_REMOVE_ENTRY		41
#define MRNM_GAME_NAME			42
#define MRNM_CONN_NAME_GET_SET	43
#define MRNM_CONN_NAME_SET		44
#define MRNM_CLIENT_ADDR_REQ	45
#define MRNM_CLIENT_ADDR		46
#define MRNM_LAG_TEST			47
#define MRNM_LAG_ANSWER			48
#define MRNM_LAG_INFO			49
#define MRNM_CONNECTION_DONE	50
#define MRNM_READY				51
#define MRNM_CANCEL_GAME		52
#define MRNM_SET_PLAYER_ID		53

#define MR_CONNECTION_TIMEOUT   21000			  // 21 sec

#define MR_PING_RETRY_TIME		750
#define MR_CLIENT_RETRY_TIME	5000

// Local prototypes
static CString GetLocalAddrStr();
static MR_UInt32 GetAddrFromStr(const char *pName);

using namespace HoverRace::Util;

MR_NetworkInterface *MR_NetworkInterface::mActiveInterface = NULL;

/**
 * Set up the MR_NetworkInterface.  Set up the client list and information, create and initialize both UDPOut ports, set them as non-blocking, and
 * set the player name (mPlayer) correctly.
 */
MR_NetworkInterface::MR_NetworkInterface()
{
	ASSERT(MR_NET_HEADER_LEN == 5);

	WORD lVersionRequested = MAKEWORD(1, 1);
	WSADATA lWsaData;

	if(WSAStartup(lVersionRequested, &lWsaData))
		ASSERT(FALSE);

	char lNameBuffer[80];
	DWORD lNameLen = sizeof(lNameBuffer);

	if(GetUserName(lNameBuffer, &lNameLen))
		mPlayer = lNameBuffer;

	mId = MR_ID_NOT_SET; // this denotes that it has not been set yet
	mServerMode = FALSE;
	mRegistrySocket = INVALID_SOCKET;
	mServerPort = 0;

	mAllPreLoguedRecv = FALSE;

	for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
		mPreLoguedClient[lCounter] = FALSE;
		mConnected[lCounter] = FALSE;
		mCanBePreLogued[lCounter] = FALSE;
	}

	// Init the UDP Output ports
	mUDPOutShortPort = socket(PF_INET, SOCK_DGRAM, 0);
	ASSERT(mUDPOutShortPort != INVALID_SOCKET);

	mUDPOutLongPort = socket(PF_INET, SOCK_DGRAM, 0);
	ASSERT(mUDPOutLongPort != INVALID_SOCKET);

	// Set these port as non-blocking
	int lCode;
	unsigned long lNonBlock = TRUE;
	lCode = ioctlsocket(mUDPOutShortPort, FIONBIO, &lNonBlock);
	ASSERT(lCode != SOCKET_ERROR);

	lNonBlock = TRUE;
	lCode = ioctlsocket(mUDPOutLongPort, FIONBIO, &lNonBlock);
	ASSERT(lCode != SOCKET_ERROR);

	// reduce buffer size to avoid data accumulation
	int lQueueSize = 120;
	lCode = setsockopt(mUDPOutShortPort, SOL_SOCKET, SO_SNDBUF, (char *) &lQueueSize, sizeof(int));
	ASSERT(lCode != SOCKET_ERROR);

	lQueueSize = 800;
	lCode = setsockopt(mUDPOutLongPort, SOL_SOCKET, SO_SNDBUF, (char *) &lQueueSize, sizeof(int));
	ASSERT(lCode != SOCKET_ERROR);

	// Bind to any avail addr
	SOCKADDR_IN lAddr;

	lAddr.sin_family = AF_INET;
	lAddr.sin_addr.s_addr = INADDR_ANY;
	lAddr.sin_port = 0;

	lCode = bind(mUDPOutShortPort, (LPSOCKADDR) & lAddr, sizeof(lAddr));
	ASSERT(lCode != SOCKET_ERROR);

	mUDPRecvPort = Config::GetInstance()->net.udpRecvPort;
	mTCPRecvPort = Config::GetInstance()->net.tcpRecvPort;
}

/**
 * Destroy the MR_NetworkInterface.  Close the UDP out sockets, disconnect, and call WSACleanup() to clean up WSA-related cruft.
 */
MR_NetworkInterface::~MR_NetworkInterface()
{
	if(mUDPOutShortPort != INVALID_SOCKET)
		closesocket(mUDPOutShortPort);

	if(mUDPOutLongPort != INVALID_SOCKET)
		closesocket(mUDPOutLongPort);

	Disconnect();
	WSACleanup();
}

/**
 * Get the player name of the specified index (pIndex).
 *
 * @param pIndex The index of the player whose name we are seeking
 */
const char *MR_NetworkInterface::GetPlayerName(int pIndex) const
{
	const char *lReturnValue = NULL;

	if(pIndex < 0)
		lReturnValue = mPlayer;
	else {
		if(pIndex < eMaxClient)
			lReturnValue = mClientName[pIndex];
	}
	return lReturnValue;
}

/**
 * Check if we are connected to the client with the specified index (pIndex).
 *
 * @param pIndex The index of the player whose connection status we are seeking
 */
BOOL MR_NetworkInterface::IsConnected(int pIndex) const
{
	ASSERT(pIndex < eMaxClient);

	if(pIndex == -1)
		return TRUE;
	else
		return mClient[pIndex].IsConnected();
}

/**
 * Close the registry socket and reset all variables to their "disconnected" state.
 */
void MR_NetworkInterface::Disconnect()
{
	if(mRegistrySocket != INVALID_SOCKET) {
		closesocket(mRegistrySocket);
		mRegistrySocket = INVALID_SOCKET;
	}
	mServerMode = FALSE;
	mId = MR_ID_NOT_SET;
	mServerPort = 0;
	mServerAddr = "";
	mGameName = "";

	mAllPreLoguedRecv = FALSE;

	for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
		mClient[lCounter].Disconnect();
		mPreLoguedClient[lCounter] = FALSE;
		mConnected[lCounter] = FALSE;
		mCanBePreLogued[lCounter] = FALSE;
	}

	// disconnect UDP recv socket
	if(mUDPRecvSocket != INVALID_SOCKET) {
		closesocket(mUDPRecvSocket);
		mUDPRecvSocket = INVALID_SOCKET;
	}
}

/**
 * Returns the number of connected clients (checks with IsConnected()).
 */
int MR_NetworkInterface::GetClientCount() const
{
	int lReturnValue = 0;

	for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
		if(mClient[lCounter].IsConnected())
			lReturnValue++;
	}
	return lReturnValue;
}

/**
 * Returns the player id of this interface ("what player are we?").
 */
int MR_NetworkInterface::GetId() const
{
	ASSERT((mId != 0) || mServerMode);
	return mId;
}

/**
 * Returns the calculated lag to the server.
 */
int MR_NetworkInterface::GetLagFromServer() const
{
	if(mServerMode)
		return 0;
	else
		return mClient[0].GetMinLag();
}

/**
 * Create the UDP receive port, set the port to be non-blocking, and bind it.  Returns TRUE on success (and FALSE on error).
 *
 * @param pPort The port to use
 */
BOOL MR_NetworkInterface::CreateUDPRecvSocket(int pPort)
{
	// this assert can be removed; early debugging artifact
	ASSERT(mUDPRecvSocket == INVALID_SOCKET);

	mUDPRecvSocket = socket(PF_INET, SOCK_DGRAM, 0);
	ASSERT(mUDPRecvSocket != INVALID_SOCKET);

	// make it non-blocking
	int lCode;
	unsigned long lNonBlock = TRUE;
	lCode = ioctlsocket(mUDPRecvSocket, FIONBIO, &lNonBlock);
	ASSERT(lCode != SOCKET_ERROR);

	// Bind the socket to the default port
	SOCKADDR_IN lLocalAddr;
	lLocalAddr.sin_family = AF_INET;
	lLocalAddr.sin_addr.s_addr = INADDR_ANY;
	lLocalAddr.sin_port = pPort;
	lCode = bind(mUDPRecvSocket, (LPSOCKADDR) &lLocalAddr, sizeof(lLocalAddr));
	ASSERT(lCode != SOCKET_ERROR);

	// bind() success code is 0
	return (lCode == 0);
}

/**
 * Return the smallest lag sample to the given client.
 *
 * @param pClient Index of the client
 */
int MR_NetworkInterface::GetMinLag(int pClient) const
{
	ASSERT((pClient >= 0) && (pClient < eMaxClient));
	return mClient[pClient].GetMinLag();
}

/**
 * Unlike its name says, this actually does the exact same thing as GetMinLag().  It returns the minimum, not the average like its name implies.  Hooray
 * for lying!
 *
 * @param pClient Index of the client
 */
int MR_NetworkInterface::GetAvgLag(int pClient) const
{
	ASSERT((pClient >= 0) && (pClient < eMaxClient));
	return mClient[pClient].GetMinLag();
}

/**
 * Send a message to the given client.
 *
 * @param pClient Index of the client
 * @param pMessage MR_NetMessageBuffer structure containing the message to be sent
 * @param pLongPort TRUE uses the UDP out long port, FALSE uses the UDP out short port
 * @param pResendLast Should be set to TRUE if we are resending pMessage
 */
BOOL MR_NetworkInterface::UDPSend(int pClient, MR_NetMessageBuffer *pMessage, BOOL pLongPort, BOOL pResendLast)
{
	ASSERT((pClient >= 0) && (pClient < eMaxClient));
	pMessage->mClient = mId;
	return mClient[pClient].UDPSend(pLongPort ? mUDPOutLongPort : mUDPOutShortPort, pMessage, pLongPort ? 0 : 1, pResendLast);
}

/**
 * Send a message to all clients.  Assumes long port for UDP out and that the message is not being resent.
 *
 * @param pMessage MR_NetMessageBuffer structure containing the message to be sent
 * @param pReqLevel If set to MR_NET_DATAGRAM UDP will be used; otherwise, TCP will be used
 */
BOOL MR_NetworkInterface::BroadcastMessage(MR_NetMessageBuffer *pMessage, int pReqLevel)
{
	pMessage->mClient = mId; // must ensure this
	for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
		if(pReqLevel == MR_NET_DATAGRAM)
			mClient[lCounter].UDPSend(mUDPOutLongPort, pMessage, 0, FALSE);
		else
			mClient[lCounter].Send(pMessage, pReqLevel);
	}
	return TRUE;
}

/*
BOOL MR_NetworkInterface::BroadcastMessage( DWORD  pTimeStamp, int  pMessageType, int pMessageLen, const MR_UInt8* pMessage )
{
	MR_NetMessageBuffer lMessage;

	lMessage.mSendingTime = pTimeStamp;
	lMessage.mMessageType = pMessageType;
	lMessage.mDataLen     = pMessageLen;

	if( pMessageLen > 0 )
	{
		memcpy( lMessage.mData, pMessage, pMessageLen );
	}

	return BroadcastMessage( &lMessage );

}
*/

/**
 * Fetches a message out of the queue.  Writes the message's data into the references passed as parameters.
 *
 * @param pTimeStamp Timestamp of the message
 * @param pMessageType ID type of the message
 * @param pMessageLen Length of the message
 * @param pMessage MR_UInt8 buffer containing the message
 * @param pClientId ID of the client
 */
BOOL MR_NetworkInterface::FetchMessage(DWORD &pTimeStamp, int &pMessageType, int &pMessageLen, const MR_UInt8 *&pMessage, int &pClientId)
{
	BOOL lReturnValue = FALSE;
	static int sLastClient = 0;

	for(int lCounter = 0; !lReturnValue && (lCounter < eMaxClient); lCounter++) {
		int lClient = (lCounter + sLastClient + 1) % eMaxClient;

		//TRACE("Polling client %d\n", lClient);
		const MR_NetMessageBuffer *lMessage = mClient[lClient].Poll((lClient >= mId) ? lClient + 1 : lClient, TRUE);

		if(lMessage != NULL) {
			lReturnValue = TRUE;
			sLastClient = lMessage->mClient;

			if(&pMessage != NULL) {
				pMessage = lMessage->mData;
			}
			pMessageLen = lMessage->mDataLen;
			pMessageType = lMessage->mMessageType;

			/*
			   DWORD lOtherSideEval = pTimeStamp-mClient[ lClient ].GetLag();

			   DWORD lOtherSideTime = lOtherSideEval&~4095 + lMessage->mSendingTime<<2;

			   int lDiff = lOtherSideTime-lOtherSideEval;

			   if( lDiff > 2048 )
			   {
			   lOtherSideTime -= 4096;
			   }
			   else if(lDiff < -2048 )
			   {
			   lOtherSideTime += 4096;
			   }

			   pTimeStamp = lOtherSideTime;
			 */
			pTimeStamp = 0;

			pClientId = lMessage->mClient;

			// We need to modify pClientId.  If we are player #1, player #2 should be client 0, player #3 should be client 1,
			// and so on.  If we are player #2, player #1 is client 0, player #3 is client 1, and so on.
			// What this boils down to is that if pClientId is greater than our own ID, we have to decrement it
			if(pClientId > mId)
				pClientId--;
		}
	}
	return lReturnValue;
}

/**
 * Set the player name.
 *
 * @param pPlayerName The player name
 */
void MR_NetworkInterface::SetPlayerName(const char *pPlayerName)
{
	mPlayer = pPlayerName;
}

/**
 * Get the player name.
 */
const char *MR_NetworkInterface::GetPlayerName() const
{
	return mPlayer;
} 

/**
 * Set up the network interface if we are the server.  Clear existing connections, set up the server socket, and set up the interface to wait
 * for incoming connections.
 *
 * @param pWindow Parent window
 * @param pGameName Track name
 * @param pPromptForPort Should we use the default port or choose our own?
 * @param pDefaultPort The default port
 * @param pModalessDlg If this is NULL, the "TCP Connections" dialog is modal
 */
BOOL MR_NetworkInterface::MasterConnect(HWND pWindow, const char *pGameName, BOOL pPromptForPort, unsigned pDefaultPort, HWND *pModalessDlg, int pReturnMessage)
{
	BOOL lReturnValue = FALSE;
	mActiveInterface = this;

	Disconnect();

	mGameName = pGameName;
	mServerMode = TRUE;
	mId = 0; // we are client 0 (the host)

	// Create the server Socket
	mRegistrySocket = socket(PF_INET, SOCK_STREAM, 0);

	if(mRegistrySocket == INVALID_SOCKET) {
		MessageBoxW(pWindow, Str::UW(_("Unable to create socket")), Str::UW(_("TCP Server")), MB_ICONERROR | MB_OK | MB_APPLMODAL);
	}
	else {
		// Ask server port number
		HMODULE lModuleHandle = GetModuleHandle(NULL /*"util.dll" */ );

		mServerPort = pDefaultPort;

		if(pPromptForPort) {
			lReturnValue = (DialogBoxW(lModuleHandle, MAKEINTRESOURCEW(IDD_SERVER_PORT), pWindow, ServerPortCallBack) == IDOK);
		}
		else {
			SOCKADDR_IN lAddr;

			lAddr.sin_family = AF_INET;
			lAddr.sin_addr.s_addr = INADDR_ANY;
			lAddr.sin_port = htons(mServerPort);

			if(bind(mRegistrySocket, (LPSOCKADDR) &lAddr, sizeof(lAddr)) != 0) {
				lReturnValue = FALSE;
				MessageBoxW(pWindow, Str::UW(_("Unable to use the port, try to close some WINSOCK applications")),
                 Str::UW(_("TCP Server")), MB_ICONERROR | MB_OK | MB_APPLMODAL);
			}
			else {
				lReturnValue = TRUE;
			}
		}

		// set up UDP receive port
		lReturnValue = CreateUDPRecvSocket(htons(mUDPRecvPort));

		if(!lReturnValue) {
			MessageBoxW(pWindow, Str::UW(_("Unable to use UDP recv port; choose a different port or close some Winsock applications.")),
              Str::UW(_("TCP Server")), MB_ICONERROR | MB_OK | MB_APPLMODAL);
		}

		if(lReturnValue) {
			// Determine server addr
			mServerAddr = GetLocalAddrStr();

			// Pop the Bit dialog
			if(pModalessDlg == NULL) {
				mReturnMessage = 0;

				// Normal mode (Modal)
				if(DialogBoxW(lModuleHandle, MAKEINTRESOURCEW(IDD_TCP_SERVER), pWindow, ListCallBack) != IDOK) {
					lReturnValue = FALSE;
				}
			}
			else {
				ASSERT(pReturnMessage != 0);

				mReturnMessage = pReturnMessage;

				// Modaless mode
				*pModalessDlg = CreateDialogW(lModuleHandle, MAKEINTRESOURCEW(IDD_TCP_SERVER), pWindow, ListCallBack);

				if(*pModalessDlg == NULL) {
					lReturnValue = FALSE;
				}
			}
		}
	}

	if(!lReturnValue) {
		Disconnect();
	}
	return lReturnValue;
}

/**
 * This function is called to get parameters about a server.  It is called as a result of the "Connect to TCP Server..." menu option being
 * selected.  When the dialog box is filled out, MR_NetworkInterface::ServerAddrCallBack is called.
 *
 * @param pWindow Parent window
 * @param pGameName CString to store the name of the game into
 */
BOOL MR_NetworkInterface::SlavePreConnect(HWND pWindow, CString &pGameName)
{
	BOOL lReturnValue = FALSE;
	mActiveInterface = this;

	Disconnect();

	pGameName = "";
	mGameName = "";

	// Create the registry Socket
	mRegistrySocket = socket(PF_INET, SOCK_STREAM, 0);

	if(mRegistrySocket == INVALID_SOCKET) {
		MessageBoxW(pWindow, Str::UW(_("Unable to create socket")), Str::UW(_("TCP Server")), MB_ICONERROR | MB_OK | MB_APPLMODAL);
	}
	else {
		// set up UDP receive socket
		lReturnValue = CreateUDPRecvSocket(htons(mUDPRecvPort));

		if(lReturnValue) {
			// Ask server port addr and number
			HMODULE lModuleHandle = GetModuleHandle(NULL /*"util.dll" */ );

			if(DialogBoxW(lModuleHandle, MAKEINTRESOURCEW(IDD_SERVER_ADDR), pWindow, ServerAddrCallBack) == IDOK) {
				lReturnValue = TRUE;

				pGameName = mGameName;
			} else {
				lReturnValue = FALSE;
			}
		}
		else {
			MessageBoxW(pWindow, Str::UW(_("Unable to use UDP recv port; choose a different port or close some Winsock applications.")),
              Str::UW(_("TCP Server")), MB_ICONERROR | MB_OK | MB_APPLMODAL);
		}
	}

	if(!lReturnValue) {
		Disconnect();
	}

	return lReturnValue;
}

/**
 * This function is called when a user connects to an existing game from the IMR, or, after the SlavePreConnect phase completes (which is called
 * when a user connects to a came from the "Connect to TCP Server..." option.  This phase sets up a dialog to get server information (if the PreConnect
 * phase never happened) which calls MR_NetworkInterface::ServerAddrCallBack() and later a dialog (the "TCP Connections" waiting screen) that calls
 * MR_NetworkInterface::ListCallBack().
 *
 * @param pWindow Parent window
 * @param pServerIP IP of the server
 * @param pDefaultPort Default port for the server (MR_DEFAULT_NET_PORT)
 * @param pGameName Name of the game (title of the track)
 * @param pModalessDlg If this is NULL, the "TCP Connections" dialog is modal
 */
BOOL MR_NetworkInterface::SlaveConnect(HWND pWindow, const char *pServerIP, unsigned pDefaultPort, const char *pGameName, HWND *pModalessDlg, int pReturnMessage)
{
	ASSERT(!mServerMode);

	BOOL lReturnValue = TRUE;

	HMODULE lModuleHandle = GetModuleHandle(NULL /*"util.dll" */ );

	if(pGameName != NULL) {
		mGameName = pGameName;
	}

	if(pServerIP != NULL) {
		Disconnect();
		ASSERT(!mServerMode);

		mRegistrySocket = socket(PF_INET, SOCK_STREAM, 0);

		if(mRegistrySocket == INVALID_SOCKET) {
			MessageBoxW(pWindow, Str::UW(_("Unable to create socket")), Str::UW(_("TCP Server")), MB_ICONERROR | MB_OK | MB_APPLMODAL);
			lReturnValue = FALSE;
		}
		else {
			// There was no pre-connect phase, do the preconnection now
			mServerAddr = pServerIP;
			mServerPort = pDefaultPort;
			mActiveInterface = this;

			lReturnValue = CreateUDPRecvSocket(htons(mUDPRecvPort));

			if(!lReturnValue) {
				MessageBoxW(pWindow, Str::UW(_("Unable to use UDP recv port; choose a different port or close some Winsock applications.")),
                 Str::UW(_("TCP Server")), MB_ICONERROR | MB_OK | MB_APPLMODAL);
			}

			// figure out the game information
			lReturnValue = (DialogBoxW(lModuleHandle, MAKEINTRESOURCEW(IDD_NET_PROGRESS), pWindow, WaitGameNameCallBack) == IDOK);
		}
	}

	ASSERT(mRegistrySocket != INVALID_SOCKET);
	ASSERT(mActiveInterface == this);

	// set up a dialog that will call ListCallBack
	if(lReturnValue) {
		if(pModalessDlg == NULL) {
			mReturnMessage = 0;

			// Normal mode (Modal)
			if(DialogBoxW(lModuleHandle, MAKEINTRESOURCEW(IDD_TCP_CLIENT), pWindow, ListCallBack) != IDOK) {
				lReturnValue = FALSE;
			}
		}
		else {
			ASSERT(pReturnMessage != 0);

			mReturnMessage = pReturnMessage;

			// Modaless mode (Modal)
			*pModalessDlg = CreateDialogW(lModuleHandle, MAKEINTRESOURCEW(IDD_TCP_CLIENT), pWindow, ListCallBack);

			if(*pModalessDlg == NULL) {
				lReturnValue = FALSE;
			}
		}
	}

	if(!lReturnValue) {
		Disconnect();
	}

	return lReturnValue;
}

/**
 * This callback is used by the "Port selection" dialog (IDD_SERVER_PORT).  This is where the user sets their player name and the port to be
 * used by the server.  Once the user selects a port the socket is initialized.
 *
 * @param pWindow Parent window
 * @param pMsgId ID of the message sent by the dialog
 * @param pWParam ID denoting a selected option (for pMsgId WM_COMMAND) 
 * @param pLParam not used but required for MFC callback
 */
BOOL CALLBACK MR_NetworkInterface::ServerPortCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG: // set up the dialog
			SetDlgItemText(pWindow, IDC_PLAYER_NAME, mActiveInterface->mPlayer);

			lReturnValue = TRUE;
			break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					lReturnValue = TRUE;
					break;

				case IDOK:
					// retrieve player name
					char lNameBuffer[80];
					GetDlgItemText(pWindow, IDC_PLAYER_NAME, lNameBuffer, sizeof(lNameBuffer));
					mActiveInterface->mPlayer = lNameBuffer;

					// Try to bind the socket to the addr
					mActiveInterface->mServerPort = Config::GetInstance()->net.tcpServPort;

					if(mActiveInterface->mServerPort <= 0) { // luser check
						MessageBoxW(pWindow, Str::UW(_("Port value must be greater than 0")), Str::UW(_("TCP Server")), MB_ICONERROR | MB_OK | MB_APPLMODAL);
					}
					else {
						SOCKADDR_IN lAddr;

						lAddr.sin_family = AF_INET;
						lAddr.sin_addr.s_addr = INADDR_ANY;
						lAddr.sin_port = htons(mActiveInterface->mServerPort);

						if(bind(mActiveInterface->mRegistrySocket, (LPSOCKADDR) &lAddr, sizeof(lAddr)) != 0) {
							MessageBoxW(pWindow, Str::UW(_("Unable to use the port, try to close some WINSOCK applications")), Str::UW(_("TCP Server")), MB_ICONERROR | MB_OK | MB_APPLMODAL);
						}
						else {
							EndDialog(pWindow, IDOK);
						}
					}
					lReturnValue = TRUE;
					break;
			}
	}

	return lReturnValue;
}

/**
 * Callback for the "Server selection" dialog box (IDD_SERVER_ADDR).  If the user hit OK, the entered player name, server address, and
 * port are saved.  Then, a new modal dialog is created (IDD_NET_PROGRESS) which calls WaitGameNameCallBack().
 *
 * @param pWindow Parent window
 * @param pMsgId ID of the message sent by the dialog
 * @param pWParam ID denoting a selected option (for pMsgId WM_COMMAND) 
 * @param pLParam not used but required for MFC callback
 */
BOOL CALLBACK MR_NetworkInterface::ServerAddrCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	BOOL lReturnValue = FALSE;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG: // set up the dialog
			SetDlgItemText(pWindow, IDC_PLAYER_NAME, mActiveInterface->mPlayer);
			SetDlgItemInt(pWindow, IDC_SERVER_PORT, 9530, FALSE); // 9530 is the default port
			lReturnValue = TRUE;
			break;

		case WM_COMMAND: // command was given
			switch (LOWORD(pWParam)) {
				case IDC_DEFAULT: // set the default port
					SetDlgItemInt(pWindow, IDC_SERVER_PORT, 9530, FALSE);
					lReturnValue = TRUE;
					break;

				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					lReturnValue = TRUE;
					break;

				case IDOK:
					{
						// retrieve player name
						char lNameBuffer[80];
						GetDlgItemText(pWindow, IDC_PLAYER_NAME, lNameBuffer, sizeof(lNameBuffer));
						mActiveInterface->mPlayer = lNameBuffer;
	
						char lBuffer[80];
						HMODULE lModuleHandle = GetModuleHandle(NULL /*"util.dll" */ );
	
						lBuffer[0] = 0;
						GetDlgItemText(pWindow, IDC_SERVER_ADDR, lBuffer, sizeof(lBuffer));
	
						mActiveInterface->mServerAddr = lBuffer;
						mActiveInterface->mServerPort = GetDlgItemInt(pWindow, IDC_SERVER_PORT, NULL, FALSE);
	
						if(DialogBoxW(lModuleHandle, MAKEINTRESOURCEW(IDD_NET_PROGRESS), pWindow, WaitGameNameCallBack) == IDOK) {
							EndDialog(pWindow, IDOK);
						}
						lReturnValue = TRUE;
					}
					break;
			}
			break;
	}

	return lReturnValue;
}

/**
 * This is the callback for the "Connecting to server" dialog box (IDD_NET_PROGRESS).  It actually gets called three times:
 *
 *   -#  When the dialog is initialized.  Set up the new socket to connect to the server.
 *   -#  When the new socket is connected to the server.  Ask the server for the game name.
 *   -#  When the server responds.  Save the game name, 
 *
 * @param pWindow Parent window
 * @param pMsgId ID of the message sent by the dialog
 * @param pWParam ID denoting a selected option (for pMsgId WM_COMMAND) 
 * @param pLParam not used but required for MFC callback
 */
BOOL CALLBACK MR_NetworkInterface::WaitGameNameCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	BOOL lReturnValue = FALSE;

	// static socket is to be setup by various callbacks
	static SOCKET sNewSocket;

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG: // set up the dialog
			{	
				SetWindowTextW(pWindow, Str::UW(_("Network Transaction Progress")));

				// set up the static socket used to connect to the server
				sNewSocket = socket(PF_INET, SOCK_STREAM, 0);
	
				if(sNewSocket == INVALID_SOCKET) { // socket creation failed
					SetDlgItemTextW(pWindow, IDC_TEXT, Str::UW(_("Unable to create socket")));
				}
				else {
					// "Connecting to server..."
					SetDlgItemTextW(pWindow, IDC_TEXT, Str::UW(_("Connecting to server...")));

					SOCKADDR_IN lAddr;
	
					lAddr.sin_family = AF_INET;
					lAddr.sin_addr.s_addr = GetAddrFromStr(mActiveInterface->mServerAddr);
					lAddr.sin_port = htons(mActiveInterface->mServerPort);

					mActiveInterface->mClientAddr[0] = GetAddrFromStr(mActiveInterface->mServerAddr);
					mActiveInterface->mClientBkAddr[0] = GetAddrFromStr(mActiveInterface->mServerAddr);
					mActiveInterface->mClientPort[0] = htons(mActiveInterface->mServerPort);

					// call this callback again when the socket successfully connects
					WSAAsyncSelect(sNewSocket, pWindow, MRM_SERVER_CONNECT, FD_CONNECT);
	
					// Disable Nagle's algorithm: we don't want to accumulate packets and send as larger ones,
					// but instead send packets as quickly as they are ready -- screw TCP/IP efficiency
					int lCode;
					int lTrue = 1;
	
					lCode = setsockopt(sNewSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &lTrue, sizeof(int));
	
					ASSERT(lCode != SOCKET_ERROR);
	
					// Reduce output queue to 512 bytes
					int lQueueSize = 512;
	
					lCode = setsockopt(sNewSocket, SOL_SOCKET, SO_SNDBUF, (char *) &lQueueSize, sizeof(int));
	
					ASSERT(lCode != SOCKET_ERROR);
	
					// next instance of this function will be with the MRM_SERVER_CONNECT message
					connect(sNewSocket, (struct sockaddr *) &lAddr, sizeof(lAddr));
				}
			}
			break;

		// sNewSocket has connected to the server successfully, now let's get the game information
		case MRM_SERVER_CONNECT:
			{
				MR_NetMessageBuffer lOutputBuffer;
	
				if(WSAGETSELECTERROR(pLParam) == 0) {
					SetDlgItemText(pWindow, IDC_TEXT, _("Retrieving game info..."));
	
					// mClient[0] is the server (if we're not the server)
					mActiveInterface->mClient[0].Connect(sNewSocket, mActiveInterface->mUDPRecvSocket);
	
					// callback with MRM_CLIENT message once the socket reads data
					WSAAsyncSelect(sNewSocket, pWindow, MRM_CLIENT, FD_READ);
	
					// bind the registry socket to the server
					SOCKADDR_IN lAddr;
					int lSize = sizeof(lAddr);
	
					lAddr.sin_family = AF_INET;
	
					getsockname(mActiveInterface->mClient[0].GetSocket(), (struct sockaddr *) &lAddr, &lSize);
	
					lAddr.sin_family = AF_INET;
					// lAddr.sin_addr.s_addr = INADDR_ANY;
					lAddr.sin_port = htons(mActiveInterface->mTCPRecvPort);
	
					if(bind(mActiveInterface->mRegistrySocket, (LPSOCKADDR) &lAddr, sizeof(lAddr)) != 0) {
						MessageBoxW(pWindow, Str::UW(_("Unable to create socket")), Str::UW(_("TCP Server")), MB_ICONERROR | MB_OK | MB_APPLMODAL);
					}

					unsigned int lOpt = 1;
					setsockopt(mActiveInterface->mRegistrySocket, SOL_SOCKET, SO_REUSEADDR, (const char *) &lOpt, sizeof(lOpt));
	
					lSize = sizeof(lAddr);
					lAddr.sin_family = AF_INET;
	
					getsockname(mActiveInterface->mRegistrySocket, (struct sockaddr *) &lAddr, &lSize);
	
					// send a message asking for the game name
					lOutputBuffer.mMessageType = MRNM_GET_GAME_NAME;
					lOutputBuffer.mClient = mActiveInterface->mId;
					lOutputBuffer.mDataLen = 8;
					*(int *) &(lOutputBuffer.mData[0]) = lAddr.sin_addr.s_addr;
					*(int *) &(lOutputBuffer.mData[4]) = lAddr.sin_port;
	
					mActiveInterface->mClient[0].Send(&lOutputBuffer, MR_NET_REQUIRED);
				}
				else {
					SetDlgItemTextW(pWindow, IDC_TEXT, Str::UW(_("ERROR Unable to connect to server...")));
				}
			}
			lReturnValue = TRUE;
			break;

		case MRM_CLIENT:
			{
				const MR_NetMessageBuffer *lBuffer = NULL;
	
				switch (WSAGETSELECTEVENT(pLParam)) {
					case FD_READ:
						// disable reception of this message
						WSAAsyncSelect(sNewSocket, pWindow, MRM_CLIENT, 0);
						lBuffer = mActiveInterface->mClient[0].Poll(0, TRUE);
	
						if((lBuffer != NULL) && (lBuffer->mMessageType == MRNM_GAME_NAME)) {
							mActiveInterface->mGameName = CString((const char *) lBuffer->mData, lBuffer->mDataLen);
							EndDialog(pWindow, IDOK);
						}
						else {
							// Bad message, reenable reception
							WSAAsyncSelect(sNewSocket, pWindow, MRM_CLIENT, FD_READ);
						}
	
						break;
				}
			}
			lReturnValue = TRUE;
			break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDCANCEL: // user canceled connection
					lReturnValue = TRUE;
					closesocket(sNewSocket);
					mActiveInterface->mClient[0].Disconnect();
					EndDialog(pWindow, IDCANCEL);
					break;
			}
	}

	return lReturnValue;
}

/**
 * This method is called when modifications to the "TCP Connections" dialog (IDD_TCP_CLIENT or IDD_TCP_SERVER) occur.  Four basic sequences of packet
 * reception occur through this method -- one for the server, one for a client connecting to the server, one for a client connecting to another client,
 * and another for a client being connected to by another client.  A basic flowchart:
 *
 * Server:
 *
 *  -#  Dialog is initialized.  Set up the main socket for reception and wait for a new client to connect (MRM_NEW_CLIENT).
 *  -#  A new client has connected: find a place in the list for them, add them to the list, and wait for another message (MRNM_CONN_NAME_GET_SET)
 *  -#  Client has responded (MRNM_CONN_NAME_GET_SET) with player name and UDP recv port; add it to the list and respond with our name and UDP
 *      receive port (MRNM_CONN_NAME_SET) and then send the list of other client IPs and names (MRNM_CLIENT_ADDR).  Wait for client to initiate lag
 *      test (MRNM_LAG_TEST).
 *  -#  Lag test initiation received (MRNM_LAG_TEST); respond with current time (MRNM_LAG_ANSWER).  The client will repeat the lag test 5 times, then
 *      inform us of the lag info.
 *  -#  Lag info received (MRNM_LAG_INFO).  Wait for more connections (go to step 2) or user input to start the race.
 *  -#  User started the race.  If some connections aren't done, ask the user if they really want to continue.  If they do, send an MRNM_READY
 *      message to all clients.  Remove the dialog and send mReturnMessage to the parent window (pWindow).
 *
 * Client connecting to server:
 *
 *  -#  Dialog is initialized.  Set up sockets for reception and ask the server for the game name (MRNM_CONN_NAME_GET_SET), sending the player name
 *      and UDP receive port.
 *  -#  Server responded with their name and UDP receive port (MRNM_CONN_NAME_SET); initiate a lag test (MRNM_LAG_TEST), sending the current time.
 *      Wait for the response.
 *  -#  Server responded with lag (MRNM_LAG_ANSWER).  Do 4 more lag tests (totaling 5 tests).
 *  -#  Send the server lag info (MRNM_LAG_INFO).  Mark this connection as done, update the dialog box as such.
 *  -#  Wait for the MRNM_READY message from the server to start the game.
 *
 * Client connecting to another client:
 *
 *  -#  The server gave us the info to connect to the new client with the MRNM_CLIENT_ADDR message.  Set up the socket and try to connect to the new
 *      client.
 *  -#  (jump to the first step of the "Client connecting to server:" section)
 *  -#  When the lag tests are done, call SendConnectionDoneIfNeeded() to send the MRNM_CONNECTION_DONE message to the server if we are connected to
 *      all the other clients.  
 *  -#  Wait for the MRNM_READY message.
 *
 * Client being connected to by another client:
 *
 *  -#  We received a connection request from a client.
 *  -#  (jump to "Server:", step 2, follow until step 5.
 *  -#  Wait for the MRNM_READY message.
 *
 * @param pWindow Parent window
 * @param pMsgId ID of the message sent by the dialog
 * @param pWParam ID denoting a selected option (for pMsgId WM_COMMAND) 
 * @param pLParam not used but required for MFC callback
 */
BOOL CALLBACK MR_NetworkInterface::ListCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
	BOOL lReturnValue = FALSE;
	HWND lListHandle = NULL;
	MR_NetMessageBuffer lAnswer;
	lAnswer.mClient = mActiveInterface->GetId();

	switch (pMsgId) {
		// Catch environment modification events
		case WM_INITDIALOG: // set up the window
			{
				RECT lRect;
	
				lListHandle = GetDlgItem(pWindow, IDC_LIST);
	
				if(!GetClientRect(lListHandle, &lRect)) {
					ASSERT(FALSE);
				}
				else {
					// Create list columns
					int lWidth = lRect.right;
					LV_COLUMN lSpec;
	
					lSpec.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
	
					CString lPlayerLabel;
					lPlayerLabel = _("Player");
					lSpec.fmt = LVCFMT_LEFT;
					lSpec.cx = lWidth - (lWidth / 6) - (lWidth / 4) - GetSystemMetrics(SM_CXVSCROLL);
					lSpec.pszText = (char *) (const char *) lPlayerLabel;
					lSpec.iSubItem = 0;
	
					ListView_InsertColumn(lListHandle, 0, &lSpec);
	
					CString lLagLabel;
					lLagLabel = _("Lag (ms)");
					lSpec.fmt = LVCFMT_RIGHT;
					lSpec.cx = lWidth / 6;
					lSpec.pszText = (char *) (const char *) lLagLabel;
					lSpec.iSubItem = 1;
	
					ListView_InsertColumn(lListHandle, 1, &lSpec);
	
					CString lStatusLabel;
					lStatusLabel = _("Status");
					lSpec.fmt = LVCFMT_LEFT;
					lSpec.cx = lWidth / 4;
					lSpec.pszText = (char *) (const char *) lStatusLabel;
					lSpec.iSubItem = 2;
	
					ListView_InsertColumn(lListHandle, 2, &lSpec);
				}
	
				// set game information in dialog
				SetDlgItemInt(pWindow, IDC_SERVER_PORT, mActiveInterface->mServerPort, FALSE);
				SetDlgItemText(pWindow, IDC_SERVER_ADDR, mActiveInterface->mServerAddr);
				SetDlgItemText(pWindow, IDC_GAME_NAME, mActiveInterface->mGameName);
	
				// Add the current player to the list
				LV_ITEM lItem;
	
				lItem.mask = LVIF_TEXT;
				lItem.iItem = 0;
				lItem.iSubItem = 0;
				lItem.pszText = "";
	
				ListView_InsertItem(lListHandle, &lItem);
				ListView_SetItemText(lListHandle, 0, 0, (char *) ((const char *) mActiveInterface->mPlayer));
				ListView_SetItemText(lListHandle, 0, 1, _("Local"));
				ListView_SetItemText(lListHandle, 0, 2, _("Connected"));
	
				for(int lCounter = 1; lCounter < eMaxClient; lCounter++) {
					// Add empty entries
					lItem.iItem = lCounter;
					lItem.pszText = "--";
					ListView_InsertItem(lListHandle, &lItem);
				}
	
				// Put the registry socket in listen mode, for the MRM_NEW_CLIENT message
				listen(mActiveInterface->mRegistrySocket, 5);
				WSAAsyncSelect(mActiveInterface->mRegistrySocket, pWindow, MRM_NEW_CLIENT, FD_ACCEPT);
	
				if(!mActiveInterface->mServerMode) {
					// allow reception of messages with id MRM_CLIENT + 0, MRM_CLIENT + 1
					WSAAsyncSelect(mActiveInterface->mClient[0].GetSocket(), pWindow, MRM_CLIENT + 0, FD_READ);
					WSAAsyncSelect(mActiveInterface->mClient[0].GetUDPSocket(), pWindow, MRM_CLIENT + 1, FD_READ);
	
					// Request server name to start sequence
					// also include UDP port number in the request
					lAnswer.mMessageType = MRNM_CONN_NAME_GET_SET;
					lAnswer.mDataLen = mActiveInterface->mPlayer.GetLength() + 4;
					*(unsigned int *) (lAnswer.mData) = htons(mActiveInterface->mUDPRecvPort);
					memcpy(lAnswer.mData + 4, mActiveInterface->mPlayer, lAnswer.mDataLen - 4);
	
					mActiveInterface->mClient[0].Send(&lAnswer, MR_NET_REQUIRED);
				}
			}
	
			break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDCANCEL:
					// if we are the server we must tell the other clients to disconnect
					if(mActiveInterface->mServerMode) {
						// technically this is not an answer but lAnswer is already declared so I will use it
						lAnswer.mMessageType = MRNM_CANCEL_GAME;
						lAnswer.mDataLen = 1;
						mActiveInterface->BroadcastMessage(&lAnswer, MR_NET_REQUIRED); // tell everyone
					}

					mActiveInterface->Disconnect();
					lReturnValue = TRUE;

					if(mActiveInterface->mReturnMessage == 0) {
						EndDialog(pWindow, IDCANCEL);
					}
					else {
						SendMessage(GetParent(pWindow), mActiveInterface->mReturnMessage, IDCANCEL, 0);
						DestroyWindow(pWindow);
					}
					break;

				case IDOK:
					{
						ASSERT(mActiveInterface->mServerMode);
	
						int lCounter;
						BOOL lOk = TRUE;
						// Verify that lag have been computed for all connections
						for(lCounter = 0; lCounter < eMaxClient; lCounter++) {
							if(mActiveInterface->mClient[lCounter].IsConnected() && !mActiveInterface->mConnected[lCounter]) {
								lOk = FALSE;
							}
						}
	
						if(!lOk) {
							HMODULE lModuleHandle = GetModuleHandle(NULL /*"util.dll" */ );
	
							if(DialogBoxW(lModuleHandle, MAKEINTRESOURCEW(IDD_WAIT_ALL), pWindow, WaitConnCallback) == IDOK) {
								lOk = TRUE;
							}
						}
	
						if(lOk) {
							Config *cfg = Config::GetInstance();
							if(cfg->misc.aloneWarning) {
								// check that we have any opponents
								int opponents = 0;
								for(int i = 0; i < eMaxClient; i++) {
									if(mActiveInterface->mClient[i].IsConnected())
										opponents++;
								}

								if(opponents == 0) {
									lOk = FALSE;
									HMODULE lModuleHandle = GetModuleHandle(NULL /*"util.dll" */ );
									// warn
									if(DialogBoxW(lModuleHandle, MAKEINTRESOURCEW(IDD_WARN_ALONE), pWindow, WarnAloneCallback) == IDOK) {
										lOk = TRUE;
									}
								}
							}

							if(lOk) {
								MR_NetMessageBuffer lMessage;
	
								lMessage.mMessageType = MRNM_READY;
								lMessage.mClient = mActiveInterface->mId;
								lMessage.mDataLen = 1;
	
								// Send a READY message to all the clients
								for(lCounter = 0; lCounter < eMaxClient; lCounter++) {
									lMessage.mData[0] = lCounter + 1;
	
									mActiveInterface->mClient[lCounter].Send(&lMessage, MR_NET_REQUIRED);
								}
	
								// Disable all callbacks
								for(lCounter = 0; lCounter < eMaxClient; lCounter++) {
									if(mActiveInterface->mClient[lCounter].IsConnected()) {
										WSAAsyncSelect(mActiveInterface->mClient[lCounter].GetSocket(), pWindow, MRM_CLIENT + lCounter, 0);
										WSAAsyncSelect(mActiveInterface->mClient[lCounter].GetUDPSocket(), pWindow, MRM_CLIENT + lCounter, 0);
									}
								}
								WSAAsyncSelect(mActiveInterface->mRegistrySocket, pWindow, MRM_CLIENT, 0);
	
								if(mActiveInterface->mReturnMessage == 0) {
									EndDialog(pWindow, IDOK);
								}
								else {
									SendMessage(GetParent(pWindow), mActiveInterface->mReturnMessage, IDOK, 0);
									DestroyWindow(pWindow);
								}
							}
							else {
								// MessageBox( pWindow, "Please wait until all clients have all successfully connected", "TCP Server", MB_ICONINFORMATION|MB_OK|MB_APPLMODAL );
							}
						}
					}
					break;
			}
			break;

		case MRM_NEW_CLIENT: // a new client has connected
			{
				int lNewSlot = -1;
	
				// figure out what slot we're going to put them in
				for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
					if(!mActiveInterface->mClient[lCounter].IsConnected()) {
						lNewSlot = lCounter;
						break;
					}
				}

				if(lNewSlot != -1) {
					// accept our new connection
					SOCKET lNewSocket = accept(mActiveInterface->mRegistrySocket, NULL, 0);

					if(lNewSocket != INVALID_SOCKET) {
						int lCode;
						int lTrue = 1;

						lCode = setsockopt(lNewSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &lTrue, sizeof(int));

						ASSERT(lCode != SOCKET_ERROR);

						// Reduce output queue to 512 bytes
						int lQueueSize = 512;

						lCode = setsockopt(lNewSocket, SOL_SOCKET, SO_SNDBUF, (char *) &lQueueSize, sizeof(int));

						ASSERT(lCode != SOCKET_ERROR);

						mActiveInterface->mClient[lNewSlot].Connect(lNewSocket, mActiveInterface->mUDPRecvSocket);

						// wait for new message (MRM_CLIENT + lNewSlot)
						WSAAsyncSelect(lNewSocket, pWindow, MRM_CLIENT + lNewSlot, FD_READ | FD_CLOSE);
						WSAAsyncSelect(mActiveInterface->mClient[lNewSlot].GetUDPSocket(), pWindow, MRM_CLIENT + lNewSlot, FD_READ | FD_CLOSE);
					}
				}
			}
			break;

		case WM_TIMER: // used for lag tests
			{
				if(pWParam - 10 > 0) { // client connect attempt timed out
					// retry
					KillTimer(pWindow, pWParam);

					int lClient = pWParam - 20;

					// try to connect again
					TRACE("attempting reconnect with client %d\n", lClient);
					WSAAsyncSelect(mActiveInterface->mClient[lClient].GetSocket(), pWindow, MRM_CLIENT + lClient, 0);
					
					mActiveInterface->mClient[lClient].Disconnect();
					SOCKET lNewSocket = socket(PF_INET, SOCK_STREAM, 0);

					mActiveInterface->mClient[lClient].Connect(lNewSocket, mActiveInterface->mUDPRecvSocket);
					mActiveInterface->mPreLoguedClient[lClient] = TRUE;

					SOCKADDR_IN lAddr;

					lAddr.sin_family = AF_INET;
					lAddr.sin_addr.s_addr = mActiveInterface->mClientAddr[lClient];
					lAddr.sin_port = mActiveInterface->mClientPort[lClient];

					// connect to the new client
					WSAAsyncSelect(lNewSocket, pWindow, MRM_CLIENT + lClient, FD_CONNECT | FD_READ | FD_CLOSE);
					int lCode = connect(lNewSocket, (struct sockaddr *) &lAddr, sizeof(lAddr));

					// set timeout timer
					SetTimer(pWindow, lClient + 20, MR_CLIENT_RETRY_TIME, NULL);

				} else {
					KillTimer(pWindow, pWParam);
	
					int lClient = pWParam - 10;
	
					if((lClient >= 0) && (lClient < eMaxClient)) {
						if(mActiveInterface->mClient[lClient].LagDone()) {
							// That is a late time-out.. probably stuck in the queue
							TRACE("Late ping message A %d\n", lClient);
						}
						else {
							// Start a time-out timer because the request may fail
							SetTimer(pWindow, lClient + 10, MR_PING_RETRY_TIME, NULL);
	
							// send a new request
							lAnswer.mMessageType = MRNM_LAG_TEST;
							lAnswer.mDataLen = 4;
							*(int *) &(lAnswer.mData[0]) = timeGetTime();
	
							mActiveInterface->UDPSend(lClient, &lAnswer, TRUE);
						}
					}
				}
			}
			break;
	}

	// client message

	/**
	 * Here is how I want to change this:
	 *
	 * 1.  All messages become MRM_CLIENT.
	 * 2.  lClient is not determined until the message is read.
	 * 3.  The next message in the queue is always dealt with (no ignoring).
	 *
	 * But what if lClient is MR_ID_NOT_SET?  How do we differentiate the clients?
	 */
	if((pMsgId >= MRM_CLIENT) && (pMsgId < (MRM_CLIENT + eMaxClient))) {
		int lClient = pMsgId - MRM_CLIENT;
		const MR_NetMessageBuffer *lBuffer = NULL;

		lListHandle = GetDlgItem(pWindow, IDC_LIST);

		switch (WSAGETSELECTEVENT(pLParam)) {
			case FD_CLOSE:
				// client quit this game
				TRACE("Client %d disconnected\n", lClient);
				mActiveInterface->mClient[lClient].Disconnect();
				mActiveInterface->mCanBePreLogued[lClient] = FALSE;
				mActiveInterface->mPreLoguedClient[lClient] = FALSE;
				mActiveInterface->mConnected[lClient] = FALSE;

				ListView_SetItemText(lListHandle, lClient + 1, 1, "");
				ListView_SetItemText(lListHandle, lClient + 1, 2, _("Disconnected"));

				break;

			case FD_READ:			
				// get our message
				lBuffer = mActiveInterface->mClient[lClient].Poll(0, FALSE);

				if(lBuffer != NULL) {
					// we must make sure this message is from the right client
					if(lBuffer->mClient != MR_ID_NOT_SET) {
						// We need to modify mClient.  If we are player #1, player #2 should be client 0, player #3 should be client 1,
						// and so on.  If we are player #2, player #1 is client 0, player #3 is client 1, and so on.
						// What this boils down to is that if pClientId is greater than our own ID, we have to decrement it
						if(lBuffer->mClient > mActiveInterface->mId)
							lClient = lBuffer->mClient - 1;
						else
							lClient = lBuffer->mClient;
					}

					// disable further reception of this message
					WSAAsyncSelect(mActiveInterface->mClient[lClient].GetSocket(), pWindow, MRM_CLIENT + lClient, 0);
					WSAAsyncSelect(mActiveInterface->mClient[lClient].GetUDPSocket(), pWindow, MRM_CLIENT + lClient, 0);
					TRACE("Packet processing: lClient is %d, ID is %d\n", lClient, lBuffer->mMessageType);

					switch (lBuffer->mMessageType) {
						case MRNM_GET_GAME_NAME: // can only occur in server mode: client asked for game name
							ASSERT(mActiveInterface->mServerMode);

							// we want to get the IP ourselves instead of from the client
							// but we want to save what they said
							mActiveInterface->mClientBkAddr[lClient] = *(int *) &(lBuffer->mData[0]);
							{
								SOCKADDR_IN lClientAddr;
								int lSize = sizeof(lClientAddr);
								if(getpeername(mActiveInterface->mClient[lClient].GetSocket(), (SOCKADDR *) &lClientAddr, &lSize) != 0) {
									ASSERT(FALSE);
								} else {
									mActiveInterface->mClientAddr[lClient] = *(int *) &(lClientAddr.sin_addr);
								}
								TRACE("Client addr: %08x (id %d)\n", mActiveInterface->mClientAddr[lClient], lClient);
							}
							mActiveInterface->mClientPort[lClient] = *(int *) &(lBuffer->mData[4]);

							lAnswer.mMessageType = MRNM_GAME_NAME;
							lAnswer.mDataLen = mActiveInterface->mGameName.GetLength();
							memcpy(lAnswer.mData, (const char *) mActiveInterface->mGameName, lAnswer.mDataLen);

							mActiveInterface->mClient[lClient].Send(&lAnswer, MR_NET_REQUIRED);

							break;

						case MRNM_CONN_NAME_GET_SET: // client has given us their name and their UDP recv port
							{
								// Add the item int the list
								LV_ITEM lItem;
								CString lConnectionName((const char *) (lBuffer->mData + 4), lBuffer->mDataLen - 4);
	
								mActiveInterface->mClient[lClient].SetRemoteUDPPort(*(unsigned int *) (lBuffer->mData));
								mActiveInterface->mClientName[lClient] = lConnectionName;
	
								lItem.mask = LVIF_TEXT;
								lItem.iItem = lClient + 1;
								lItem.iSubItem = 0;
								lItem.pszText = (char *) ((const char *) lConnectionName);
	
								if(ListView_GetItemCount(lListHandle) > lItem.iItem) {
									ListView_SetItem(lListHandle, &lItem);
								}
								else {
									ListView_InsertItem(lListHandle, &lItem);
								}
								ListView_SetItemText(lListHandle, lClient + 1, 1, _("Computing lag"));
								ListView_SetItemText(lListHandle, lClient + 1, 2, _("Connecting"));
	
								// return local name as an answer
								// also include UDP port number in the request
								lAnswer.mMessageType = MRNM_CONN_NAME_SET;
								lAnswer.mDataLen = mActiveInterface->mPlayer.GetLength() + 4;
								*(unsigned int *) (lAnswer.mData) = htons(mActiveInterface->mUDPRecvPort);
								memcpy(lAnswer.mData + 4, mActiveInterface->mPlayer, lAnswer.mDataLen - 4);
	
								mActiveInterface->mClient[lClient].Send(&lAnswer, MR_NET_REQUIRED);
	
								// Tell client their ID and send client list if server
								if(mActiveInterface->mServerMode) {
									// send player ID
									lAnswer.mMessageType = MRNM_SET_PLAYER_ID;
									lAnswer.mDataLen = 1;
									lAnswer.mData[0] = lClient + 1;
	
									mActiveInterface->mClient[lClient].Send(&lAnswer, MR_NET_REQUIRED);
									
									// now send the client list
									lAnswer.mMessageType = MRNM_CLIENT_ADDR;
									lAnswer.mClient = 0;
									lAnswer.mDataLen = 12;
	
									// Send the actual client list
									for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
										if((lCounter != lClient)
											&& (mActiveInterface->mClient[lCounter].IsConnected())
											&& (mActiveInterface->mCanBePreLogued[lCounter])) {
											*(int *) &(lAnswer.mData[0]) = mActiveInterface->mClientAddr[lCounter];
											*(int *) &(lAnswer.mData[4]) = mActiveInterface->mClientBkAddr[lCounter];
											*(int *) &(lAnswer.mData[8]) = mActiveInterface->mClientPort[lCounter];
	
											mActiveInterface->mClient[lClient].Send(&lAnswer, MR_NET_REQUIRED);
	
										}
									}
									lAnswer.mDataLen = 0;
									mActiveInterface->mClient[lClient].Send(&lAnswer, MR_NET_REQUIRED);
	
									mActiveInterface->mCanBePreLogued[lClient] = TRUE;
								}
	
							}
							break;

						case MRNM_CONN_NAME_SET: // client returned information on their name and UDP recv port
							{
								LV_ITEM lItem;
								CString lConnectionName((const char *) (lBuffer->mData + 4), lBuffer->mDataLen - 4);
	
								mActiveInterface->mClient[lClient].SetRemoteUDPPort(*(unsigned int *) (lBuffer->mData));
								mActiveInterface->mClientName[lClient] = lConnectionName;
	
								lItem.mask = LVIF_TEXT;
								lItem.iItem = lClient + 1;
								lItem.iSubItem = 0;
								lItem.pszText = (char *) ((const char *) lConnectionName);
	
								// Add the client into the list
								if(ListView_GetItemCount(lListHandle) > lItem.iItem) {
									ListView_SetItem(lListHandle, &lItem);
								}
								else {
									ListView_InsertItem(lListHandle, &lItem);
								}
	
								// Begin lag test
								ListView_SetItemText(lListHandle, lClient + 1, 1, _("Computing lag"));
								ListView_SetItemText(lListHandle, lClient + 1, 2, _("Connecting"));
	
								// Start lag test with that connection
								lAnswer.mMessageType = MRNM_LAG_TEST;
								lAnswer.mDataLen = 4;
								*(DWORD *) lAnswer.mData = timeGetTime();
	
								mActiveInterface->UDPSend(lClient, &lAnswer, TRUE);
								// mActiveInterface->mClient[ lClient ].Send( &lAnswer, MR_NET_REQUIRED );
	
								// Start a time-out timer because the request may fail
								SetTimer(pWindow, lClient + 10, MR_PING_RETRY_TIME, NULL);	
							}
							break;

						case MRNM_REMOVE_ENTRY:
							ASSERT(FALSE);		  // Bad code

							/*
							   mActiveInterface->mClient[lClient].Disconnect();
							   ListView_DeleteItem( lListHandle, lClient+1 );
							 */
							break;

						case MRNM_CLIENT_ADDR: // server sent us a list of other clients and addresses
							{
								if(lBuffer->mDataLen == 0) {
									// end of list
									mActiveInterface->mAllPreLoguedRecv = TRUE;
									mActiveInterface->mPreLoguedClient[0] = TRUE;
	
									// Start a lag test with the server
									// * Already done *
	
									// Report that all timing tests have been done.. if needed
									mActiveInterface->SendConnectionDoneIfNeeded();
								}
								else {
									// The server sent us a new client address
									// Create a connection to that port
									SOCKET lNewSocket = socket(PF_INET, SOCK_STREAM, 0);

									ASSERT(lNewSocket != INVALID_SOCKET);

									SOCKADDR_IN lAddr;
	
									int lSlot = -1;
	
									for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
										if(!mActiveInterface->mClient[lCounter].IsConnected()) {
											lSlot = lCounter;
											break;
										}
									}
									ASSERT(lSlot != -1);

									mActiveInterface->mClient[lSlot].Connect(lNewSocket, mActiveInterface->mUDPRecvSocket);
									mActiveInterface->mPreLoguedClient[lSlot] = TRUE;

									lAddr.sin_family = AF_INET;
									lAddr.sin_addr.s_addr = *(int *) &(lBuffer->mData[0]);
									lAddr.sin_port = *(int *) &(lBuffer->mData[8]);
									
									// save info
									// backup address will be attempted to be used if connection fails
									mActiveInterface->mClientAddr[lSlot] = *(int *) &(lBuffer->mData[0]);
									mActiveInterface->mClientBkAddr[lSlot] = *(int *) &(lBuffer->mData[4]);
									mActiveInterface->mClientPort[lSlot] = *(int *) &(lBuffer->mData[8]);

									// allow messages from the new client (under FD_CONNECT)
									WSAAsyncSelect(lNewSocket, pWindow, MRM_CLIENT + lSlot, FD_CONNECT | FD_READ | FD_CLOSE);
									WSAAsyncSelect(mActiveInterface->mClient[lSlot].GetUDPSocket(), pWindow, MRM_CLIENT + lSlot, FD_READ | FD_CLOSE);

									// connect to the new client
									int lCode = connect(lNewSocket, (struct sockaddr *) &lAddr, sizeof(lAddr));

									// set timeout timer
									SetTimer(pWindow, lSlot + 20, MR_CLIENT_RETRY_TIME, NULL);
								}
							}
							break;

						case MRNM_LAG_TEST: // a client we are connecting to is conducting a lag test
							// return the message and add the current time
							lAnswer.mMessageType = MRNM_LAG_ANSWER;
							lAnswer.mDataLen = 4;
							*(int *) &(lAnswer.mData[0]) = *(int *) &(lBuffer->mData[0]);

							// send the request
							mActiveInterface->UDPSend(lClient, &lAnswer, TRUE);
							// mActiveInterface->mClient[ lClient ].Send( &lAnswer, MR_NET_REQUIRED );

							break;

						case MRNM_LAG_ANSWER: // lag test returned
							// Desactivate time-out
							KillTimer(pWindow, lClient + 10);

							// Verify that it is not a late message
							if(mActiveInterface->mClient[lClient].LagDone()) {
								// That is a late message.. ignore it
								TRACE("Late ping message B %d\n", lClient);
							}
							else {
								// Add the sample to the list
								if(!mActiveInterface->mClient[lClient].AddLagSample(timeGetTime() - *(int *) &(lBuffer->mData[0]))) {
									// Start a time-out timer because the request may fail
									SetTimer(pWindow, lClient + 10, MR_PING_RETRY_TIME, NULL);

									// send a new request
									lAnswer.mMessageType = MRNM_LAG_TEST;
									lAnswer.mDataLen = 4;
									*(int *) &(lAnswer.mData[0]) = timeGetTime();

									mActiveInterface->UDPSend(lClient, &lAnswer, TRUE);
									// mActiveInterface->mClient[ lClient ].Send( &lAnswer, MR_NET_REQUIRED );

								}
								else { // we have 5 lag samples, that's all we need
									// ASSERT( mActiveInterface->mAllPreLoguedRecv ); it is not an important assert.. it only mean that the client list is not completely received
									ASSERT(!mActiveInterface->mServerMode);

									// Send result to remote side
									lAnswer.mMessageType = MRNM_LAG_INFO;
									lAnswer.mDataLen = 8;
									*(int *) &(lAnswer.mData[0]) = mActiveInterface->mClient[lClient].GetAvgLag();
									*(int *) &(lAnswer.mData[4]) = mActiveInterface->mClient[lClient].GetMinLag();
									mActiveInterface->mClient[lClient].Send(&lAnswer, MR_NET_REQUIRED);

									// Update display
									char lStrBuffer[20];

									ListView_SetItemText(lListHandle, lClient + 1, 1, _itoa(mActiveInterface->mClient[lClient].GetAvgLag(), lStrBuffer, 10));
									ListView_SetItemText(lListHandle, lClient + 1, 2, _("Connected"));

									// Verify if weconnected with all the client that we were suppose to
									mActiveInterface->SendConnectionDoneIfNeeded();
								}
							}
							break;

						case MRNM_LAG_INFO: // we are being sent lag info
							{
								mActiveInterface->mClient[lClient].SetLag(*(int *) &(lBuffer->mData[0]), *(int *) &(lBuffer->mData[4]));
	
								// Update display
								char lStrBuffer[20];
	
								ListView_SetItemText(lListHandle, lClient + 1, 1, _itoa(mActiveInterface->mClient[lClient].GetAvgLag(), lStrBuffer, 10));
								if(mActiveInterface->mServerMode) {
									ListView_SetItemText(lListHandle, lClient + 1, 2, _("Waiting acknowledgement"));
								}
								else {
									ListView_SetItemText(lListHandle, lClient + 1, 2, _("Connected"));
								}
							}
							break;

						case MRNM_CONNECTION_DONE: // the client has told us they are connected to everyone
							{
								ASSERT(mActiveInterface->mServerMode);
	
								// Mark the connection as completed
								ListView_SetItemText(lListHandle, lClient + 1, 2, _("Connected"));
								mActiveInterface->mConnected[lClient] = TRUE;
							}
							break;

						case MRNM_READY: // race is beginning
							// Get Client Id
							mActiveInterface->mId = lBuffer->mData[0];

							// Quit this dialog with success

							// Remove all ports callback
							for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
								if(mActiveInterface->mClient[lCounter].IsConnected()) {
									WSAAsyncSelect(mActiveInterface->mClient[lCounter].GetSocket(), pWindow, MRM_CLIENT + lCounter, 0);
									WSAAsyncSelect(mActiveInterface->mClient[lCounter].GetUDPSocket(), pWindow, MRM_CLIENT + lCounter, 0);
								}

							}
							WSAAsyncSelect(mActiveInterface->mRegistrySocket, pWindow, MRM_CLIENT, 0);

							if(mActiveInterface->mReturnMessage == 0) {
								EndDialog(pWindow, IDOK);
							}
							else {
								SendMessage(GetParent(pWindow), mActiveInterface->mReturnMessage, IDOK, 0);
								DestroyWindow(pWindow);
							}

							return TRUE;

						case MRNM_CANCEL_GAME: // game has been cancelled
							// notify the user that the game ended
							MessageBoxW(pWindow, Str::UW(_("Game cancelled by server")), Str::UW(_("TCP Server")), MB_ICONERROR | MB_OK | MB_APPLMODAL);

							mActiveInterface->Disconnect();
							lReturnValue = TRUE;

							if(mActiveInterface->mReturnMessage == 0) {
								EndDialog(pWindow, IDCANCEL);
							}
							else {
								SendMessage(GetParent(pWindow), mActiveInterface->mReturnMessage, IDCANCEL, 0);
								DestroyWindow(pWindow);
							}
							break;

						case MRNM_SET_PLAYER_ID:
							// set mId
							mActiveInterface->mId = lBuffer->mData[0];
							break;
					}
				}

				WSAAsyncSelect(mActiveInterface->mClient[lClient].GetSocket(), pWindow, MRM_CLIENT + lClient, FD_READ | FD_CLOSE);
				WSAAsyncSelect(mActiveInterface->mClient[lClient].GetUDPSocket(), pWindow, MRM_CLIENT + lClient, FD_READ | FD_CLOSE);

				break;

			case FD_CONNECT: // we have successfully connected to the server or another client
				TRACE("received FD_CONNECT message");
				KillTimer(pWindow, lClient + 20); // kill timeout timer
				if(WSAGETSELECTERROR(pLParam)) {
					// Connection error with a client
					//ASSERT(FALSE);
					TRACE("Connection error with client %d: ", lClient);

					switch(WSAGETSELECTERROR(pLParam)) {
						case WSAECONNREFUSED:
							TRACE("connection refused\n");
							// assemble our message
							{
					            std::string lErrorString = boost::str(boost::format("%s %s%s") %
									_("Connection refused by peer") %
									mActiveInterface->mClientName[lClient] %
									_("; perhaps they are behind a firewall and need to forward ports 9530 and 9531 TCP and UDP?"));
                                
								MessageBoxW(pWindow, Str::UW(lErrorString.c_str()), Str::UW(_("TCP Client")), MB_ICONERROR | MB_OK | MB_APPLMODAL);
							}
							break;
						case WSAENETUNREACH:
						case WSAETIMEDOUT:
							/* try a different IP if possible */
							/* but not if connecting to server */
							TRACE("timeout or unreachable\n");
							if(!mActiveInterface->mClient[lClient].mTriedBackupIP && lClient != 0) {
								mActiveInterface->mClient[lClient].Disconnect();
								mActiveInterface->mClient[lClient].mTriedBackupIP = TRUE;

								SOCKET lNewSocket = socket(PF_INET, SOCK_STREAM, 0);

								ASSERT(lNewSocket != INVALID_SOCKET);

								SOCKADDR_IN lAddr;
								lAddr.sin_family = AF_INET;

								mActiveInterface->mClient[lClient].Connect(lNewSocket, mActiveInterface->mUDPRecvSocket);

								if(((mActiveInterface->mClientAddr[lClient] & 0x0000FFFF) == 0x0000A8C0) ||
								   ((mActiveInterface->mClientAddr[lClient] & 0x000000FF) == 0x0000000A)) {
									/* Our server reported an internal address:
									 * This probably means that the server and this client are on the
									 * same internal network.  If this is the case, we need to use the IP
									 * we used for the server, but make sure we are connecting on a different
									 * port.
									 */
									if(mActiveInterface->mClientPort[0] != mActiveInterface->mClientPort[lClient]) {
										lAddr.sin_addr.s_addr = *(int *) &(mActiveInterface->mClientAddr[0]);
										lAddr.sin_port = *(int *) &(mActiveInterface->mClientPort[lClient]);
									} else {
										/* assemble error message */
										std::string lErrorMessage = boost::str(boost::format("%s%s%s") %
											_("Main server and peer ") %
											mActiveInterface->mClientName[lClient] %
											_(" are using the same IP and port!"));

										MessageBoxW(pWindow, Str::UW(lErrorMessage.c_str()), Str::UW(_("TCP Client")), MB_ICONERROR | MB_OK | MB_APPLMODAL);
									}
								} else {
									lAddr.sin_addr.s_addr = *(int *) &(mActiveInterface->mClientBkAddr[lClient]);
									// we try to use the address at lBuffer->mData[4] if we can't connect with the IP the server saw
									lAddr.sin_port = *(int *) &(mActiveInterface->mClientPort[lClient]);
								}

								TRACE("Trying new IP %08x, port %d\n", lAddr.sin_addr.s_addr, lAddr.sin_port);

								// allow messages from the new client (under FD_CONNECT)
								WSAAsyncSelect(lNewSocket, pWindow, MRM_CLIENT + lClient, FD_CONNECT | FD_READ | FD_CLOSE);
								WSAAsyncSelect(mActiveInterface->mClient[lClient].GetUDPSocket(), pWindow, MRM_CLIENT + lClient, FD_READ | FD_CLOSE);

								// connect to the new client
								int lCode = connect(lNewSocket, (struct sockaddr *) &lAddr, sizeof(lAddr));
							} else {
								SOCKADDR_IN lAddr;
								lAddr.sin_addr.s_addr = *(int *) &(mActiveInterface->mClientAddr[lClient]);

								std::string lErrorString = boost::str(boost::format("%s%s%s%s") %
									_("Cannot connect to peer ") %
									mActiveInterface->mClientName[lClient] %
									_(" at IP ") %
									inet_ntoa(lAddr.sin_addr));

								MessageBoxW(pWindow, Str::UW(lErrorString.c_str()), Str::UW(_("TCP Client")), MB_ICONERROR | MB_OK | MB_APPLMODAL);
							}
							break;
						default:
							TRACE("Unknown error");
							{
								std::string lError = boost::str(boost::format("%s%s.") %
									_("Connection error with ") %
									mActiveInterface->mClientName[lClient]);
                                
								MessageBoxW(pWindow, Str::UW(lError.c_str()), Str::UW(_("TCP Client")), MB_ICONERROR | MB_OK | MB_APPLMODAL);
							}
							break;
					}
				}
				else {
					// request client name to start the connection sequence
					// also include UDP port number in the request
					lAnswer.mMessageType = MRNM_CONN_NAME_GET_SET;
					lAnswer.mDataLen = mActiveInterface->mPlayer.GetLength() + 4;
					*(unsigned int *) (lAnswer.mData) = htons(mActiveInterface->mUDPRecvPort);

					memcpy(lAnswer.mData + 4, mActiveInterface->mPlayer, lAnswer.mDataLen - 4);

					mActiveInterface->mClient[lClient].Send(&lAnswer, MR_NET_REQUIRED);
				}

				break;
		}
		lReturnValue = TRUE;
	}

	return lReturnValue;
}

/**
 * This function is the callback for the "Warning, about to launch alone" dialog requested in #101.
 *
 * Its function is to warn the user if they are launching a game alone.
 */
BOOL CALLBACK MR_NetworkInterface::WarnAloneCallback(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam) {
	switch (pMsgId) {
		case WM_INITDIALOG: // set up the window
			// do i18n
			SetWindowTextW(pWindow, Str::UW(_("Warning!")));
			//SetDlgItemTextW(pWindow, IDC_TEXT_WARNING, 
			//	Str::UW(_("It is highly preferable to wait until all players have successfully connected to the game before pressing the START buttton.")));
			SetDlgItemTextW(pWindow, IDC_TEXT_WARNING,
				Str::UW(_("You are about to launch the race without any opponents.  You will be on your own.  It is preferable to wait for other players to join your game.  Are you sure?")));
			SetDlgItemTextW(pWindow, IDC_ASK_AGAIN, Str::UW(_("Ask me this each time I launch a game alone")));
			SetDlgItemTextW(pWindow, IDOK, Str::UW(_("OK")));
			SetDlgItemTextW(pWindow, IDCANCEL, Str::UW(_("Cancel")));
			SendDlgItemMessage(pWindow, IDC_ASK_AGAIN, BM_SETCHECK, BST_CHECKED, 0);
			break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					return TRUE;

				case IDOK:
					EndDialog(pWindow, IDOK);

					Config *cfg = Config::GetInstance();
					cfg->misc.aloneWarning = (SendDlgItemMessage(pWindow, IDC_ASK_AGAIN, BM_GETCHECK, 0, 0) == BST_CHECKED);

					return TRUE;
			}
	}

	return FALSE;
}

/***
 * This callback is used for the dialog displayed if not all players have connected successfully.
 * It is rather simple.
 */
BOOL CALLBACK MR_NetworkInterface::WaitConnCallback(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam) {
	switch (pMsgId) {
		case WM_INITDIALOG: // set up the window
			// do i18n
			SetWindowTextW(pWindow, Str::UW(_("Warning!")));
			SetDlgItemTextW(pWindow, IDC_WARN_TEXT, 
				Str::UW(_("It is highly preferable to wait until all players have successfully connected to the game before pressing the start button.")));
			SetDlgItemTextW(pWindow, IDOK, Str::UW(_("Start NOW!")));
			SetDlgItemTextW(pWindow, IDCANCEL, Str::UW(_("OK")));
			break;

		case WM_COMMAND:
			switch (LOWORD(pWParam)) {
				case IDCANCEL:
					EndDialog(pWindow, IDCANCEL);
					return TRUE;

				case IDOK:
					EndDialog(pWindow, IDOK);
					return TRUE;
			}
	}

	return FALSE;
}

/**
 * This function sends the MRNM_CONNECTION_DONE message to the server, if we have successfully connected to all other clients.  The message is not
 * sent if any clients are not yet finished with connecting.
 */
void MR_NetworkInterface::SendConnectionDoneIfNeeded()
{
	MR_NetMessageBuffer lAnswer;

	if(mAllPreLoguedRecv) {
		BOOL lAllDone = TRUE;
		for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
			if(mPreLoguedClient[lCounter]) {
				if(!mClient[lCounter].LagDone()) {
					lAllDone = FALSE;
					break;
				}
			}
		}

		if(lAllDone) {
			lAnswer.mMessageType = MRNM_CONNECTION_DONE;
			lAnswer.mClient = mId;
			lAnswer.mDataLen = 0;
			mClient[0].Send(&lAnswer, MR_NET_REQUIRED);
		}
	}
}

/**
 * Initialize the sockets to INVALID_SOCKETs.  Also run Disconnect().
 */
MR_NetworkPort::MR_NetworkPort()
{
	mSocket = INVALID_SOCKET;
	mUDPRecvSocket = INVALID_SOCKET;
	mTriedBackupIP = FALSE;

	Disconnect();
}

/**
 * Disconnect before destruction.
 */
MR_NetworkPort::~MR_NetworkPort()
{
	Disconnect();
}

/**
 * Connects to the socket passed as a parameter.  The socket passed should already be set up and connected to the client.  Also sets up the
 * UDP receive socket to receive messages from the client.
 *
 * @param pSocket An already-connected socket.
 */
void MR_NetworkPort::Connect(SOCKET pSocket, SOCKET pUDPRecvSocket)
{
	Disconnect();
	mSocket = pSocket;
	mWatchdog = timeGetTime();

	mUDPRecvSocket = pUDPRecvSocket;

	/*
	// Create UDPPort

	// Create a new UDP Socket for the reception
	mUDPRecvSocket = socket(PF_INET, SOCK_DGRAM, 0);
	ASSERT(mUDPRecvSocket != INVALID_SOCKET);

	// Set this port as non-blocking
	unsigned long lNonBlock = TRUE;
	int lCode = ioctlsocket(mUDPRecvSocket, FIONBIO, &lNonBlock);
	ASSERT(lCode != SOCKET_ERROR);

	// Bind the socket to any available address
	SOCKADDR_IN lLocalAddr;
	lLocalAddr.sin_family = AF_INET;
	lLocalAddr.sin_addr.s_addr = INADDR_ANY;
	lLocalAddr.sin_port = 0;
	lCode = bind(mUDPRecvSocket, (LPSOCKADDR) &lLocalAddr, sizeof(lLocalAddr));
	ASSERT(lCode != SOCKET_ERROR);
	*/
}

/**
 * Set the remote UDP port.
 *
 * @param pPort Remote port to use for UDP
 */
void MR_NetworkPort::SetRemoteUDPPort(unsigned int pPort)
{
	// Determine remote address and port based on the TCP port
	int lSize = sizeof(mUDPRemoteAddr);
	int lCode = getpeername(mSocket, (LPSOCKADDR) &mUDPRemoteAddr, &lSize);
	ASSERT(lCode != SOCKET_ERROR);

	mUDPRemoteAddr.sin_port = pPort;
}

/**
 * Close the main socket and UDP receive socket, and reset variables.  Default lag is 300.  Not sure why.
 */
void MR_NetworkPort::Disconnect()
{
	if(mSocket != INVALID_SOCKET) {
		closesocket(mSocket);
	}

	// Don't close the UDP recv socket!
	
	//if(mUDPRecvSocket != INVALID_SOCKET) {
	//	closesocket(mUDPRecvSocket);
	//}

	mSocket = INVALID_SOCKET;

	mUDPRecvSocket = INVALID_SOCKET;

	mLastSendedDatagramNumber[0] = 0;
	mLastSendedDatagramNumber[1] = 0;
	mLastSendedDatagramNumber[2] = 0;
	mLastSendedDatagramNumber[3] = 0;
	mLastReceivedDatagramNumber[0] = 0;
	mLastReceivedDatagramNumber[1] = 0;
	mLastReceivedDatagramNumber[2] = 0;
	mLastReceivedDatagramNumber[3] = 0;
	mLastClient[0] = MR_ID_NOT_SET;
	mLastClient[1] = MR_ID_NOT_SET;
	mLastClient[2] = MR_ID_NOT_SET;
	mLastClient[3] = MR_ID_NOT_SET;

	mAvgLag = 300;
	mMinLag = 300;
	mNbLagTest = 0;
	mTotalLag = 0;

	mOutQueueLen = 0;
	mOutQueueHead = 0;

	mInputMessageBufferIndex = 0;

	mTriedBackupIP = FALSE;
}

/**
 * Tests if the port is connected (with mSocket).
 */
BOOL MR_NetworkPort::IsConnected() const
{
	return (mSocket != INVALID_SOCKET);
}

/**
 * Returns the TCP socket used for communication with the client.
 */
SOCKET MR_NetworkPort::GetSocket() const
{
	return mSocket;
}

/**
 * Returns the UDP socket that is being used for listening.
 */
SOCKET MR_NetworkPort::GetUDPSocket() const
{
	return mUDPRecvSocket;
}

/**
 * Check if any new messages have been recieved.  If not, NULL is returned.  Otherwise the first message in the queue is returned.  The UDP socket is
 * checked before the TCP socket (TCP messages are not as time-critical).
 *
 * The parameter pClientId is a dirty hack for the one-port UDP hack.  Since this polls the global UDP receive port it is possible we could receive a
 * message not addressed to this particular port; therefore, we peek at the message and make sure it has the same ID as pClientId, or, we ignore it.
 *
 * @param pClientId Id of the client a packet should be from
 */
const MR_NetMessageBuffer *MR_NetworkPort::Poll(int pClientId, BOOL pCheckClientId)
{
	// Socket is assumed to be non-blocking, but it damn well better be because we set it that way
	if((mInputMessageBufferIndex == 0) && (mUDPRecvSocket != INVALID_SOCKET)) {
		while(1) {
			// see if there is a UDP packet addressed to us
			// this is a dirty hack
			int lLen = 0;
			BOOL lPassCheck = TRUE;
			if(pCheckClientId == TRUE) {
				lLen = recv(mUDPRecvSocket, ((char *) &mInputMessageBuffer), sizeof(mInputMessageBuffer), MSG_PEEK);
				if((mInputMessageBuffer.mClient != pClientId) && (mInputMessageBuffer.mClient != MR_ID_NOT_SET)) {
					/* disable this for intensive testing
					sprintf(lErrorBuffer, "Ignoring packet from client %d (looking for %d), message %d\n",
								mInputMessageBuffer.mClient,
								pClientId,
								mInputMessageBuffer.mMessageType);
					OutputDebugString((LPCTSTR) lErrorBuffer);
					*/
					lPassCheck = FALSE;
				} else
					lLen = recv(mUDPRecvSocket, ((char *) &mInputMessageBuffer), sizeof(mInputMessageBuffer), 0);
			}
			else {
				lLen = recv(mUDPRecvSocket, ((char *) &mInputMessageBuffer), sizeof(mInputMessageBuffer), 0);
			}

			if(lLen > 0 && lPassCheck == TRUE) {
				int lQueueId = mInputMessageBuffer.mDatagramQueue;
				// We have received a datagram
				ASSERT(lLen == mInputMessageBuffer.mDataLen + MR_NET_HEADER_LEN);

				// Eliminate duplicate and late datagrams
				if(((MR_Int8) ((MR_Int8) (MR_UInt8) mInputMessageBuffer.mDatagramNumber - (MR_Int8) mLastReceivedDatagramNumber[lQueueId]) > 0)
					|| (mInputMessageBuffer.mClient != mLastClient[lQueueId])) {
					// TRACE( "UDP recv\n" );
					mLastReceivedDatagramNumber[lQueueId] = mInputMessageBuffer.mDatagramNumber;
					mLastClient[lQueueId] = mInputMessageBuffer.mClient;

					mWatchdog = timeGetTime();
					TRACE("UDP recv: client %d, message %d, number %d\n", mInputMessageBuffer.mClient, mInputMessageBuffer.mMessageType, mInputMessageBuffer.mDatagramNumber);
					return &mInputMessageBuffer;
				}
				else {
					TRACE("Late UDP %d %d client %d\n", (MR_Int8) (MR_UInt8) mInputMessageBuffer.mDatagramNumber, (MR_Int8) mLastReceivedDatagramNumber[lQueueId], mInputMessageBuffer.mClient);
				}
			}
			else {
				// No data... try TCP
				break;
			}
		}
	}

	// check for TCP packets
	if(mSocket != INVALID_SOCKET) {
		if(mInputMessageBufferIndex < MR_NET_HEADER_LEN) {
			// Try to read message header
			int lLen = recv(mSocket, ((char *) &mInputMessageBuffer) + mInputMessageBufferIndex, MR_NET_HEADER_LEN - mInputMessageBufferIndex, 0);

			if(lLen > 0) {
				mInputMessageBufferIndex += lLen;
				TRACE("Begin recv, client %d, message %d, number %d\n", mInputMessageBuffer.mClient, mInputMessageBuffer.mMessageType, mInputMessageBuffer.mDatagramNumber);
			}
		}

		if(mInputMessageBufferIndex >= MR_NET_HEADER_LEN && (mInputMessageBuffer.mDataLen > 0)) {
			int lLen = recv(mSocket, ((char *) &mInputMessageBuffer) + mInputMessageBufferIndex, mInputMessageBuffer.mDataLen - (mInputMessageBufferIndex - MR_NET_HEADER_LEN), 0);

			if(lLen > 0) {
				mInputMessageBufferIndex += lLen;
				TRACE("Continue recv, client %d, message %d, number %d\n", mInputMessageBuffer.mClient, mInputMessageBuffer.mMessageType, mInputMessageBuffer.mDatagramNumber);
			}
		}

		if((mInputMessageBufferIndex >= MR_NET_HEADER_LEN) && (mInputMessageBufferIndex == (MR_NET_HEADER_LEN + mInputMessageBuffer.mDataLen))) {
			mInputMessageBufferIndex = 0;
			mWatchdog = timeGetTime();
			TRACE("Done receiving packet from client %d, message %d, number %d\n", mInputMessageBuffer.mClient, mInputMessageBuffer.mMessageType, mInputMessageBuffer.mDatagramNumber);
			return &mInputMessageBuffer;
		}

		if((mLastSendedDatagramNumber[1] > 16) && (timeGetTime() - mWatchdog) > MR_CONNECTION_TIMEOUT) {
			// (mLastSendedDatagramNumber[1]>16) -- this condition avoid disconnecting
			//                                   -- before game start
			TRACE("Reception Timeout %d %d\n", timeGetTime() - mWatchdog, mInputMessageBufferIndex);

			if(mInputMessageBufferIndex != 0) {
				TRACE("Message: %d %d\n", mInputMessageBuffer.mMessageType, mInputMessageBuffer.mDataLen);
			}

			Disconnect();
		}
	}

	return NULL;
}

/**
 * Send a packet via UDP, using the supplied socket.
 *
 * @param pSocket The socket the packet will be sent from (generally MR_NetworkInterface::mUDPOutLongPort or MR_NetworkInterface::mUDPOutShortPort)
 * @param pMessage The message to be sent
 * @param pQueueId The queue ID of the message
 * @param pResendLast TRUE if this is a re-send of the last packet
 */
BOOL MR_NetworkPort::UDPSend(SOCKET pSocket, MR_NetMessageBuffer *pMessage, unsigned pQueueId, BOOL pResendLast)
{
	BOOL lReturnValue = TRUE;

	// Debug lost packet simulation
	/*
	   if( (rand()%3) != 1 )
	   {
	   return TRUE;
	   }
	 */

	if(mUDPRecvSocket != INVALID_SOCKET) {
		int lToSend = MR_NET_HEADER_LEN + pMessage->mDataLen;

		if(!pResendLast) {
			mLastSendedDatagramNumber[pQueueId]++;
		}

		pMessage->mDatagramQueue = pQueueId;
		pMessage->mDatagramNumber = mLastSendedDatagramNumber[pQueueId];

		int lSent = sendto(pSocket, ((const char *) pMessage), lToSend, 0, (LPSOCKADDR) & mUDPRemoteAddr, sizeof(mUDPRemoteAddr));

		lReturnValue = (lSent != SOCKET_ERROR);
	}
	return lReturnValue;
}

/**
 * Send the given message via TCP.
 *
 * @param pMessage The message to be sent
 * @param pReqLevel Should be set to MR_NET_REQUIRED
 */
void MR_NetworkPort::Send(const MR_NetMessageBuffer *pMessage, int pReqLevel)
{
	// First try to send buffered data
	if(mSocket != INVALID_SOCKET) {
		BOOL lEndQueueLoop = (mOutQueueLen == 0);

		while(!lEndQueueLoop) {
			// Determine first batch to send
			int lToSend;

			if((mOutQueueHead + mOutQueueLen) > MR_OUT_QUEUE_LEN) {
				lToSend = MR_OUT_QUEUE_LEN - mOutQueueHead;
			}
			else {
				lToSend = mOutQueueLen;
				lEndQueueLoop = TRUE;
			}

			int lReturnValue = send(mSocket, (const char *) (mOutQueue + mOutQueueHead), lToSend, 0);

			if(lReturnValue >= 0) {
				mOutQueueLen -= lReturnValue;

				mOutQueueHead = (mOutQueueHead + lReturnValue) % MR_OUT_QUEUE_LEN;

				if(lReturnValue != lToSend) {
					lEndQueueLoop = TRUE;
				}
			}
			else {
				lEndQueueLoop = TRUE;

				if(WSAGetLastError() == WSAEWOULDBLOCK) {
					// nothing to do
				}
				else {
					TRACE("Communication error A %d\n", WSAGetLastError());
					Disconnect();
				}
			}
		}
	}

	if(mSocket != INVALID_SOCKET) {
		int lToSend = MR_NET_HEADER_LEN + pMessage->mDataLen;

		int lReturnValue;						  // send return value

		if(mOutQueueLen > 0) {
			lReturnValue = 0;
		}
		else {
			lReturnValue = send(mSocket, ((const char *) pMessage), lToSend, 0);

			// TRACE( "Send %d %d %d\n", lToSend, lToSend-lSent, lReturnValue );

			if((lReturnValue == -1) && (WSAGetLastError() == WSAEWOULDBLOCK)) {
				ASSERT(FALSE);
				lReturnValue = 0;
			}
		}

		if(lReturnValue < 0) {
			TRACE("Communication error B %d\n", WSAGetLastError());
			Disconnect();
		}
		else if(lReturnValue != lToSend) {
			if((lReturnValue > 0) || (pReqLevel == MR_NET_REQUIRED) || ((pReqLevel == MR_NET_TRY) && (mOutQueueLen < (MR_OUT_QUEUE_LEN / 4)))) {
				lToSend -= lReturnValue;

				if(lToSend > (MR_OUT_QUEUE_LEN - mOutQueueLen)) {
					// no space left
					TRACE("Output queue full\n");
					Disconnect();
					ASSERT(FALSE);
				}
				else {
					// put the remaining part of the message in the queue
					int lTail = (mOutQueueHead + mOutQueueLen) % MR_OUT_QUEUE_LEN;

					int lFirstBlocSize = min(lToSend, MR_OUT_QUEUE_LEN - lTail);
					int lSecondBlocSize = max(0, lToSend - lFirstBlocSize);

					ASSERT(lFirstBlocSize > 0);

					memcpy(mOutQueue + lTail, pMessage + lReturnValue, lFirstBlocSize);

					if(lSecondBlocSize > 0) {
						memcpy(mOutQueue, pMessage + lFirstBlocSize + lReturnValue, lSecondBlocSize);
					}

					mOutQueueLen += lToSend;
				}
			}
		}
	}
}

/**
 * Adds an observed lag sample to the lag.  Used in lag testing (if you hadn't guessed).
 *
 * @param pLag Observed lag
 */
BOOL MR_NetworkPort::AddLagSample(int pLag)
{
	mNbLagTest++;

	mTotalLag += pLag;

	mAvgLag = mTotalLag / (mNbLagTest * 2);

	mMinLag = min(mMinLag, pLag / 2);

	return LagDone();
}

/**
 * Returns whether or not five lag tests have been performed.
 */
BOOL MR_NetworkPort::LagDone() const
{
	return (mNbLagTest >= 5);
}

/**
 * Returns the calculated average lag.
 */
int MR_NetworkPort::GetAvgLag() const
{
	return mAvgLag;
}

/**
 * Returns the minimum observed lag.
 */
int MR_NetworkPort::GetMinLag() const
{
	return mMinLag;
} 

/**
 * Sets the lag.
 *
 * @param pAvgLag The average lag
 * @param pMinLag The minimum lag
 */
void MR_NetworkPort::SetLag(int pAvgLag, int pMinLag)
{
	mAvgLag = pAvgLag;
	mMinLag = pMinLag;

	mNbLagTest = 100;							  // Set as completed

}

// Helper functions

/**
 * Returns the local IP in a CString.
 */
CString GetLocalAddrStr()
{
	CString lReturnValue;

	char lHostname[100];
	HOSTENT *lHostEnt;
	int lAdapter = 0;

	gethostname(lHostname, sizeof(lHostname));

	lReturnValue = lHostname;

	lHostEnt = gethostbyname(lHostname);

	if(lHostEnt != NULL) {
		while(lHostEnt->h_addr_list[lAdapter] != NULL) {
			const unsigned char *lHostAddr = (const unsigned char *) lHostEnt->h_addr_list[lAdapter];
			lAdapter++;

			sprintf(lHostname, "  %d.%d.%d.%d", lHostAddr[0], lHostAddr[1], lHostAddr[2], lHostAddr[3]);

			lReturnValue += lHostname;
		}
	}

	if(lAdapter == 0) {
		lReturnValue += " (";
		lReturnValue += _("no network connection");
		lReturnValue += ")";
	}

	return lReturnValue;
}

/**
 * Return a 32-bit integer denoting the IP of the passed parameter string.
 *
 * @param pName IP in string form ( %d.%d.%d.%d)
 */
MR_UInt32 GetAddrFromStr(const char *pName)
{

	MR_UInt32 lReturnValue;
	unsigned int lNibble[4];

	// Verify if the given string is not a numeric addr

	int lNbField = sscanf(pName, " %d.%d.%d.%d", &(lNibble[0]),
		&(lNibble[1]),
		&(lNibble[2]),
		&(lNibble[3]));

	if((lNbField == 4) && (lNibble[0] != 0)) {
		lReturnValue = lNibble[0]
			+ (lNibble[1] << 8)
			+ (lNibble[2] << 16)
			+ (lNibble[3] << 24);

	}
	else {
		lReturnValue = INADDR_ANY;
	}
	return lReturnValue;
}