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
#include "../Util/StrRes.h"

// Private window messages
#define MRM_SERVER_CONNECT (WM_USER + 1)
#define MRM_NEW_CLIENT     (WM_USER + 2)
#define MRM_CLIENT         (WM_USER + 10)	// 64 next reserved

#define MRNM_GET_GAME_NAME     40
#define MRNM_REMOVE_ENTRY      41
#define MRNM_GAME_NAME         42
#define MRNM_CONN_NAME_GET_SET 43
#define MRNM_CONN_NAME_SET     44
#define MRNM_CLIENT_ADDR_REQ   45
#define MRNM_CLIENT_ADDR       46
#define MRNM_LAG_TEST          47
#define MRNM_LAG_ANSWER        48
#define MRNM_LAG_INFO          49
#define MRNM_CONNECTION_DONE   50
#define MRNM_READY             51

#define MR_CONNECTION_TIMEOUT   21000	// 21 sec

#define MR_PING_RETRY_TIME   750

// Local prototypes
static CString GetLocalAddrStr();
static MR_UInt32 GetAddrFromStr(const char *pName);
static BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

MR_NetworkInterface *MR_NetworkInterface::mActiveInterface = NULL;

MR_NetworkInterface::MR_NetworkInterface()
{
    ASSERT(MR_NET_HEADER_LEN == 3);

    WORD lVersionRequested = MAKEWORD(1, 1);
    WSADATA lWsaData;

    if(WSAStartup(lVersionRequested, &lWsaData))
	ASSERT(FALSE);

    char lNameBuffer[80];
    DWORD lNameLen = sizeof(lNameBuffer);

    if(GetUserName(lNameBuffer, &lNameLen))
	mPlayer = lNameBuffer;

    mId = 0;
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
}

MR_NetworkInterface::~MR_NetworkInterface()
{
    if(mUDPOutShortPort != INVALID_SOCKET)
	closesocket(mUDPOutShortPort);

    if(mUDPOutLongPort != INVALID_SOCKET)
	closesocket(mUDPOutLongPort);

    Disconnect();
    WSACleanup();
}

const char *MR_NetworkInterface::GetPlayerName(int pIndex) const const
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

BOOL MR_NetworkInterface::IsConnected(int pIndex) const const
{
    ASSERT(pIndex < eMaxClient);

    if(pIndex == -1)
	return TRUE;
    else
	return mClient[pIndex].IsConnected();
}

void MR_NetworkInterface::Disconnect()
{
    if(mRegistrySocket != INVALID_SOCKET) {
	closesocket(mRegistrySocket);
	mRegistrySocket = INVALID_SOCKET;
    }
    mServerMode = FALSE;
    mId = 0;
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
}

int MR_NetworkInterface::GetClientCount() const const
{
    int lReturnValue = 0;

    for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
	if(mClient[lCounter].IsConnected())
	    lReturnValue++;
    }
    return lReturnValue;
}

int MR_NetworkInterface::GetId() const const
{
    ASSERT((mId != 0) || mServerMode);
    return mId;
}

int MR_NetworkInterface::GetLagFromServer() const const
{
    if(mServerMode)
	return 0;
    else
	return mClient[0].GetMinLag();
}

int MR_NetworkInterface::GetMinLag(int pClient) const const
{
    ASSERT((pClient >= 0) && (pClient < eMaxClient));
    return mClient[pClient].GetMinLag();
}

int MR_NetworkInterface::GetAvgLag(int pClient) const const
{
    ASSERT((pClient >= 0) && (pClient < eMaxClient));
    return mClient[pClient].GetMinLag();
}

BOOL MR_NetworkInterface::UDPSend(int pClient, MR_NetMessageBuffer * pMessage, BOOL pLongPort, BOOL pResendLast)
{
    ASSERT((pClient >= 0) && (pClient < eMaxClient));
    return mClient[pClient].UDPSend(pLongPort ? mUDPOutLongPort : mUDPOutShortPort, pMessage, pLongPort ? 0 : 1, pResendLast);
}

BOOL MR_NetworkInterface::BroadcastMessage(MR_NetMessageBuffer * pMessage, int pReqLevel)
{
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

BOOL MR_NetworkInterface::FetchMessage(DWORD & pTimeStamp, int &pMessageType, int &pMessageLen, const MR_UInt8 * &pMessage, int &pClientId)
{
    BOOL lReturnValue = FALSE;
    static int sLastClient = 0;

    for(int lCounter = 0; !lReturnValue && (lCounter < eMaxClient); lCounter++) {
	int lClient = (lCounter + sLastClient + 1) % eMaxClient;

	const MR_NetMessageBuffer *lMessage = mClient[lClient].Poll();

	if(lMessage != NULL) {
	    lReturnValue = TRUE;
	    sLastClient = lClient;

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

	    pClientId = lClient;

	}
    }
    return lReturnValue;
}

void MR_NetworkInterface::SetPlayerName(const char *pPlayerName)
{
    mPlayer = pPlayerName;
}

const char *MR_NetworkInterface::GetPlayerName() const const
{
    return mPlayer;
}

BOOL MR_NetworkInterface::MasterConnect(HWND pWindow, const char *pGameName, BOOL pPromptForPort, unsigned pDefaultPort, HWND * pModalessDlg, int pReturnMessage)
{
    BOOL lReturnValue = FALSE;
    mActiveInterface = this;

    Disconnect();

    mGameName = pGameName;
    mServerMode = TRUE;

    // Create the server Socket
    mRegistrySocket = socket(PF_INET, SOCK_STREAM, 0);

    if(mRegistrySocket == INVALID_SOCKET) {
	MessageBox(pWindow, MR_LoadString(IDS_CANT_CREATE_SOCK), MR_LoadString(IDS_TCP_SERVER), MB_ICONERROR | MB_OK | MB_APPLMODAL);
    } else {
	// Ask server port number
	HMODULE lModuleHandle = GetModuleHandle(NULL /*"util.dll" */ );

	mServerPort = pDefaultPort;

	if(pPromptForPort) {
	    lReturnValue = (DialogBox(lModuleHandle, MAKEINTRESOURCE(IDD_SERVER_PORT), pWindow, ServerPortCallBack) == IDOK);
	} else {

	    SOCKADDR_IN lAddr;

	    lAddr.sin_family = AF_INET;
	    lAddr.sin_addr.s_addr = INADDR_ANY;
	    lAddr.sin_port = htons(mServerPort);

	    if(bind(mRegistrySocket, (LPSOCKADDR) & lAddr, sizeof(lAddr)) != 0) {
		lReturnValue = FALSE;
		MessageBox(pWindow, MR_LoadString(IDS_CANT_USE_PORT), MR_LoadString(IDS_TCP_SERVER), MB_ICONERROR | MB_OK | MB_APPLMODAL);
	    } else {
		lReturnValue = TRUE;
	    }
	}


	if(lReturnValue) {
	    // Determine server addr
	    mServerAddr = GetLocalAddrStr();

	    // Pop the Bit gialog
	    if(pModalessDlg == NULL) {
		mReturnMessage = 0;

		// Normal mode ( Modal )
		if(DialogBox(lModuleHandle, MAKEINTRESOURCE(IDD_TCP_SERVER), pWindow, ListCallBack) != IDOK) {
		    lReturnValue = FALSE;
		}
	    } else {
		ASSERT(pReturnMessage != 0);

		mReturnMessage = pReturnMessage;

		// Modaless mode
		*pModalessDlg = CreateDialog(lModuleHandle, MAKEINTRESOURCE(IDD_TCP_SERVER), pWindow, ListCallBack);

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






BOOL MR_NetworkInterface::SlavePreConnect(HWND pWindow, CString & pGameName)
{
    BOOL lReturnValue = FALSE;
    mActiveInterface = this;

    Disconnect();

    pGameName = "";
    mGameName = "";

    // Create the registry Socket
    mRegistrySocket = socket(PF_INET, SOCK_STREAM, 0);

    if(mRegistrySocket == INVALID_SOCKET) {
	MessageBox(pWindow, MR_LoadString(IDS_CANT_CREATE_SOCK), MR_LoadString(IDS_TCP_SERVER), MB_ICONERROR | MB_OK | MB_APPLMODAL);
    } else {
	// Ask server port addr and number
	HMODULE lModuleHandle = GetModuleHandle(NULL /*"util.dll" */ );

	if(DialogBox(lModuleHandle, MAKEINTRESOURCE(IDD_SERVER_ADDR), pWindow, ServerAddrCallBack) == IDOK) {
	    lReturnValue = TRUE;

	    pGameName = mGameName;
	}
    }

    if(!lReturnValue) {
	Disconnect();
    }

    return lReturnValue;
}

BOOL MR_NetworkInterface::SlaveConnect(HWND pWindow, const char *pServerIP, unsigned pDefaultPort, const char *pGameName, HWND * pModalessDlg, int pReturnMessage)
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
	    MessageBox(pWindow, MR_LoadString(IDS_CANT_CREATE_SOCK), MR_LoadString(IDS_TCP_SERVER), MB_ICONERROR | MB_OK | MB_APPLMODAL);
	    lReturnValue = FALSE;
	} else {

	    // There was no pre-connect phase, do the preconnection now
	    mServerAddr = pServerIP;
	    mServerPort = pDefaultPort;
	    mActiveInterface = this;

	    lReturnValue = (DialogBox(lModuleHandle, MAKEINTRESOURCE(IDD_NET_PROGRESS), pWindow, WaitGameNameCallBack) == IDOK);
	}
    }

    ASSERT(mRegistrySocket != INVALID_SOCKET);
    ASSERT(mActiveInterface == this);

    if(lReturnValue) {
	if(pModalessDlg == NULL) {
	    mReturnMessage = 0;

	    // Normal mode ( Modal )
	    if(DialogBox(lModuleHandle, MAKEINTRESOURCE(IDD_TCP_CLIENT), pWindow, ListCallBack) != IDOK) {
		lReturnValue = FALSE;
	    }
	} else {
	    ASSERT(pReturnMessage != 0);

	    mReturnMessage = pReturnMessage;

	    // Modaless mode ( Modal )
	    *pModalessDlg = CreateDialog(lModuleHandle, MAKEINTRESOURCE(IDD_TCP_CLIENT), pWindow, ListCallBack);

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


BOOL CALLBACK MR_NetworkInterface::ServerPortCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
    BOOL lReturnValue = FALSE;

    switch (pMsgId) {
	    // Catch environment modification events
	case WM_INITDIALOG:
	    SetDlgItemText(pWindow, IDC_PLAYER_NAME, mActiveInterface->mPlayer);
	    SetDlgItemInt(pWindow, IDC_SERVER_PORT, MR_DEFAULT_NET_PORT, FALSE);

	    lReturnValue = TRUE;
	    break;

	case WM_COMMAND:
	    switch (LOWORD(pWParam)) {
		case IDC_DEFAULT:
		    SetDlgItemInt(pWindow, IDC_SERVER_PORT, MR_DEFAULT_NET_PORT, FALSE);
		    lReturnValue = TRUE;
		    break;

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
		    mActiveInterface->mServerPort = GetDlgItemInt(pWindow, IDC_SERVER_PORT, NULL, FALSE);

		    if(mActiveInterface->mServerPort <= 0) {
			MessageBox(pWindow, MR_LoadString(IDS_PORT_RANGE), MR_LoadString(IDS_TCP_SERVER), MB_ICONERROR | MB_OK | MB_APPLMODAL);
		    } else {
			SOCKADDR_IN lAddr;

			lAddr.sin_family = AF_INET;
			lAddr.sin_addr.s_addr = INADDR_ANY;
			lAddr.sin_port = htons(mActiveInterface->mServerPort);

			if(bind(mActiveInterface->mRegistrySocket, (LPSOCKADDR) & lAddr, sizeof(lAddr)) != 0) {
			    MessageBox(pWindow, MR_LoadString(IDS_CANT_USE_PORT), MR_LoadString(IDS_TCP_SERVER), MB_ICONERROR | MB_OK | MB_APPLMODAL);
			} else {
			    EndDialog(pWindow, IDOK);
			}
		    }
		    lReturnValue = TRUE;
		    break;
	    }
    }

    return lReturnValue;
}

BOOL CALLBACK MR_NetworkInterface::ServerAddrCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
    BOOL lReturnValue = FALSE;

    switch (pMsgId) {
	    // Catch environment modification events
	case WM_INITDIALOG:
	    SetDlgItemText(pWindow, IDC_PLAYER_NAME, mActiveInterface->mPlayer);

	    SetDlgItemInt(pWindow, IDC_SERVER_PORT, MR_DEFAULT_NET_PORT, FALSE);
	    lReturnValue = TRUE;
	    break;

	case WM_COMMAND:
	    switch (LOWORD(pWParam)) {
		case IDC_DEFAULT:
		    SetDlgItemInt(pWindow, IDC_SERVER_PORT, MR_DEFAULT_NET_PORT, FALSE);
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

			if(DialogBox(lModuleHandle, MAKEINTRESOURCE(IDD_NET_PROGRESS), pWindow, WaitGameNameCallBack) == IDOK) {
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

BOOL CALLBACK MR_NetworkInterface::WaitGameNameCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
    BOOL lReturnValue = FALSE;

    static SOCKET sNewSocket;

    switch (pMsgId) {
	    // Catch environment modification events
	case WM_INITDIALOG:
	    {

		sNewSocket = socket(PF_INET, SOCK_STREAM, 0);

		if(sNewSocket == INVALID_SOCKET) {
		    SetDlgItemText(pWindow, IDC_TEXT, MR_LoadString(IDS_CANT_CREATE_SOCK));
		} else {
		    SetDlgItemText(pWindow, IDC_TEXT, MR_LoadString(IDS_CONNECTING_SERV));

		    SOCKADDR_IN lAddr;

		    lAddr.sin_family = AF_INET;
		    lAddr.sin_addr.s_addr = GetAddrFromStr(mActiveInterface->mServerAddr);
		    lAddr.sin_port = htons(mActiveInterface->mServerPort);

		    WSAAsyncSelect(sNewSocket, pWindow, MRM_SERVER_CONNECT, FD_CONNECT);

		    // Disable Nangle algo
		    int lCode;
		    int lTrue = 1;

		    lCode = setsockopt(sNewSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &lTrue, sizeof(int));

		    ASSERT(lCode != SOCKET_ERROR);

		    // Reduce output queue to 512 bytes
		    int lQueueSize = 512;

		    lCode = setsockopt(sNewSocket, SOL_SOCKET, SO_SNDBUF, (char *) &lQueueSize, sizeof(int));

		    ASSERT(lCode != SOCKET_ERROR);



		    connect(sNewSocket, (struct sockaddr *) &lAddr, sizeof(lAddr));
		}
	    }
	    break;

	case MRM_SERVER_CONNECT:
	    {
		MR_NetMessageBuffer lOutputBuffer;

		if(WSAGETSELECTERROR(pLParam) == 0) {
		    SetDlgItemText(pWindow, IDC_TEXT, MR_LoadString(IDS_GET_GAMEINFO));

		    mActiveInterface->mClient[0].Connect(sNewSocket);


		    WSAAsyncSelect(sNewSocket, pWindow, MRM_CLIENT, FD_READ);

		    SOCKADDR_IN lAddr;
		    int lSize = sizeof(lAddr);

		    lAddr.sin_family = AF_INET;

		    getsockname(mActiveInterface->mClient[0].GetSocket(), (struct sockaddr *) &lAddr, &lSize);

		    lAddr.sin_family = AF_INET;
		    // lAddr.sin_addr.s_addr = INADDR_ANY;
		    lAddr.sin_port = 0;

		    if(bind(mActiveInterface->mRegistrySocket, (LPSOCKADDR) & lAddr, sizeof(lAddr)) != 0) {
			MessageBox(pWindow, MR_LoadString(IDS_CANT_CREATE_SOCK), MR_LoadString(IDS_TCP_CLIENT), MB_ICONERROR | MB_OK | MB_APPLMODAL);
		    }

		    lSize = sizeof(lAddr);
		    lAddr.sin_family = AF_INET;

		    getsockname(mActiveInterface->mRegistrySocket, (struct sockaddr *) &lAddr, &lSize);

		    lOutputBuffer.mMessageType = MRNM_GET_GAME_NAME;
		    lOutputBuffer.mDataLen = 8;
		    *(int *) &(lOutputBuffer.mData[0]) = lAddr.sin_addr.s_addr;
		    *(int *) &(lOutputBuffer.mData[4]) = lAddr.sin_port;

		    mActiveInterface->mClient[0].Send(&lOutputBuffer, MR_NET_REQUIRED);

		} else {
		    SetDlgItemText(pWindow, IDC_TEXT, MR_LoadString(IDS_CANT_CONNECT));
		}
	    }
	    lReturnValue = TRUE;
	    break;

	case MRM_CLIENT:
	    {
		const MR_NetMessageBuffer *lBuffer = NULL;

		switch (WSAGETSELECTEVENT(pLParam)) {
		    case FD_READ:
			WSAAsyncSelect(sNewSocket, pWindow, MRM_CLIENT, 0);
			lBuffer = mActiveInterface->mClient[0].Poll();

			if((lBuffer != NULL) && (lBuffer->mMessageType == MRNM_GAME_NAME)) {
			    mActiveInterface->mGameName = CString((const char *) lBuffer->mData, lBuffer->mDataLen);
			    EndDialog(pWindow, IDOK);
			} else {
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
		case IDCANCEL:
		    lReturnValue = TRUE;
		    closesocket(sNewSocket);
		    mActiveInterface->mClient[0].Disconnect();
		    EndDialog(pWindow, IDCANCEL);
		    break;
	    }
    }

    return lReturnValue;
}

BOOL CALLBACK MR_NetworkInterface::ListCallBack(HWND pWindow, UINT pMsgId, WPARAM pWParam, LPARAM pLParam)
{
    BOOL lReturnValue = FALSE;
    HWND lListHandle = NULL;
    MR_NetMessageBuffer lAnswer;

    switch (pMsgId) {
	    // Catch environment modification events
	case WM_INITDIALOG:
	    {
		RECT lRect;

		lListHandle = GetDlgItem(pWindow, IDC_LIST);

		if(!GetClientRect(lListHandle, &lRect)) {
		    ASSERT(FALSE);
		} else {
		    // Create list columns
		    int lWidth = lRect.right;
		    LV_COLUMN lSpec;

		    lSpec.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;

		    CString lPlayerLabel;
		    lPlayerLabel.LoadString(IDS_PLAYER);
		    lSpec.fmt = LVCFMT_LEFT;
		    lSpec.cx = lWidth - (lWidth / 6) - (lWidth / 4) - GetSystemMetrics(SM_CXVSCROLL);
		    lSpec.pszText = (char *) (const char *) lPlayerLabel;
		    lSpec.iSubItem = 0;

		    ListView_InsertColumn(lListHandle, 0, &lSpec);

		    CString lLagLabel;
		    lLagLabel.LoadString(IDS_LAG);
		    lSpec.fmt = LVCFMT_RIGHT;
		    lSpec.cx = lWidth / 6;
		    lSpec.pszText = (char *) (const char *) lLagLabel;
		    lSpec.iSubItem = 1;

		    ListView_InsertColumn(lListHandle, 1, &lSpec);

		    CString lStatusLabel;
		    lStatusLabel.LoadString(IDS_LAG);
		    lSpec.fmt = LVCFMT_LEFT;
		    lSpec.cx = lWidth / 4;
		    lSpec.pszText = (char *) (const char *) lStatusLabel;
		    lSpec.iSubItem = 2;

		    ListView_InsertColumn(lListHandle, 2, &lSpec);


		}

		SetDlgItemInt(pWindow, IDC_SERVER_PORT, mActiveInterface->mServerPort, FALSE);

		SetDlgItemText(pWindow, IDC_SERVER_ADDR, mActiveInterface->mServerAddr);

		SetDlgItemText(pWindow, IDC_GAME_NAME, mActiveInterface->mGameName);

		// Addthe current player to the list
		LV_ITEM lItem;

		lItem.mask = LVIF_TEXT;
		lItem.iItem = 0;
		lItem.iSubItem = 0;
		lItem.pszText = "";

		ListView_InsertItem(lListHandle, &lItem);
		ListView_SetItemText(lListHandle, 0, 0, (char *) ((const char *) mActiveInterface->mPlayer));
		ListView_SetItemText(lListHandle, 0, 1, (char *) MR_LoadStringBuffered(IDS_LOCAL));
		ListView_SetItemText(lListHandle, 0, 2, (char *) MR_LoadStringBuffered(IDS_CONNECTED));

		for(int lCounter = 1; lCounter < eMaxClient; lCounter++) {
		    // Add empty entries
		    lItem.iItem = lCounter;
		    lItem.pszText = "--";
		    ListView_InsertItem(lListHandle, &lItem);
		}

		// Put the registry socket in listen mode
		listen(mActiveInterface->mRegistrySocket, 5);
		WSAAsyncSelect(mActiveInterface->mRegistrySocket, pWindow, MRM_NEW_CLIENT, FD_ACCEPT);




		if(!mActiveInterface->mServerMode) {
		    WSAAsyncSelect(mActiveInterface->mClient[0].GetSocket(), pWindow, MRM_CLIENT + 0, FD_READ);
		    WSAAsyncSelect(mActiveInterface->mClient[0].GetUDPSocket(), pWindow, MRM_CLIENT + 1, FD_READ);

		    // Request server name to start sequence
		    // also include UDP port number in the request
		    lAnswer.mMessageType = MRNM_CONN_NAME_GET_SET;
		    lAnswer.mDataLen = mActiveInterface->mPlayer.GetLength() + 4;
		    *(unsigned int *) (lAnswer.mData) = mActiveInterface->mClient[0].GetUDPPort();
		    memcpy(lAnswer.mData + 4, mActiveInterface->mPlayer, lAnswer.mDataLen - 4);

		    mActiveInterface->mClient[0].Send(&lAnswer, MR_NET_REQUIRED);
		}

	    }

	    break;

	case WM_COMMAND:
	    switch (LOWORD(pWParam)) {
		case IDCANCEL:
		    mActiveInterface->Disconnect();
		    lReturnValue = TRUE;

		    if(mActiveInterface->mReturnMessage == 0) {
			EndDialog(pWindow, IDCANCEL);
		    } else {
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

			    if(DialogBox(lModuleHandle, MAKEINTRESOURCE(IDD_WAIT_ALL), pWindow, DialogProc) == IDOK) {
				lOk = TRUE;
			    }
			}

			if(lOk) {
			    MR_NetMessageBuffer lMessage;

			    lMessage.mMessageType = MRNM_READY;
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
			    } else {
				SendMessage(GetParent(pWindow), mActiveInterface->mReturnMessage, IDOK, 0);
				DestroyWindow(pWindow);
			    }
			} else {
			    // MessageBox( pWindow, "Please wait until all clients have all successfully connected", "TCP Server", MB_ICONINFORMATION|MB_OK|MB_APPLMODAL );
			}
		    }
		    break;
	    }
	    break;

	case MRM_NEW_CLIENT:
	    {

		int lNewSlot = -1;

		for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
		    if(!mActiveInterface->mClient[lCounter].IsConnected()) {
			lNewSlot = lCounter;
			break;
		    }
		}

		if(lNewSlot != -1) {
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


			mActiveInterface->mClient[lNewSlot].Connect(lNewSocket);

			WSAAsyncSelect(lNewSocket, pWindow, MRM_CLIENT + lNewSlot, FD_READ | FD_CLOSE);
			WSAAsyncSelect(mActiveInterface->mClient[lNewSlot].GetUDPSocket(), pWindow, MRM_CLIENT + lNewSlot, FD_READ | FD_CLOSE);
		    }
		}

	    }
	    break;

	case WM_TIMER:
	    {
		KillTimer(pWindow, pWParam);

		int lClient = pWParam - 10;

		if((lClient >= 0) && (lClient < eMaxClient)) {
		    if(mActiveInterface->mClient[lClient].LagDone()) {
			// That is a late time-out.. probably stuck in the queue
			TRACE("Late ping message A %d\n", lClient);
		    } else {
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
	    break;

    }

    if((pMsgId >= MRM_CLIENT) && (pMsgId < (MRM_CLIENT + eMaxClient))) {
	int lClient = pMsgId - MRM_CLIENT;
	const MR_NetMessageBuffer *lBuffer = NULL;

	lListHandle = GetDlgItem(pWindow, IDC_LIST);

	switch (WSAGETSELECTEVENT(pLParam)) {
	    case FD_CLOSE:
		// Oups..needed because player get exited when playing HoverRace
		// They jump from one game to an other
		TRACE("Client %d disconnected\n", lClient);
		mActiveInterface->mClient[lClient].Disconnect();
		mActiveInterface->mCanBePreLogued[lClient] = FALSE;
		mActiveInterface->mPreLoguedClient[lClient] = FALSE;
		mActiveInterface->mConnected[lClient] = FALSE;

		ListView_SetItemText(lListHandle, lClient + 1, 1, "");
		ListView_SetItemText(lListHandle, lClient + 1, 2, (char *) MR_LoadStringBuffered(IDS_DISCONNECTED));

		break;

	    case FD_READ:
		WSAAsyncSelect(mActiveInterface->mClient[lClient].GetSocket(), pWindow, MRM_CLIENT + lClient, 0);
		WSAAsyncSelect(mActiveInterface->mClient[lClient].GetUDPSocket(), pWindow, MRM_CLIENT + lClient, 0);
		lBuffer = mActiveInterface->mClient[lClient].Poll();

		if(lBuffer != NULL) {
		    switch (lBuffer->mMessageType) {
			case MRNM_GET_GAME_NAME:
			    ASSERT(mActiveInterface->mServerMode);

			    mActiveInterface->mClientAddr[lClient] = *(int *) &(lBuffer->mData[0]);
			    mActiveInterface->mClientPort[lClient] = *(int *) &(lBuffer->mData[4]);


			    lAnswer.mMessageType = MRNM_GAME_NAME;
			    lAnswer.mDataLen = mActiveInterface->mGameName.GetLength();
			    memcpy(lAnswer.mData, (const char *) mActiveInterface->mGameName, lAnswer.mDataLen);

			    mActiveInterface->mClient[lClient].Send(&lAnswer, MR_NET_REQUIRED);

			    break;

			case MRNM_CONN_NAME_SET:
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
				} else {
				    ListView_InsertItem(lListHandle, &lItem);
				}

				ListView_SetItemText(lListHandle, lClient + 1, 1, (char *) MR_LoadStringBuffered(IDS_COMPUTING));
				ListView_SetItemText(lListHandle, lClient + 1, 2, (char *) MR_LoadStringBuffered(IDS_CONNECTING));

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


			case MRNM_CONN_NAME_GET_SET:
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
				} else {
				    ListView_InsertItem(lListHandle, &lItem);
				}
				ListView_SetItemText(lListHandle, lClient + 1, 1, (char *) MR_LoadStringBuffered(IDS_COMPUTING));
				ListView_SetItemText(lListHandle, lClient + 1, 2, (char *) MR_LoadStringBuffered(IDS_CONNECTING));

				// return local name as an answer
				// also include UDP port number in the request
				lAnswer.mMessageType = MRNM_CONN_NAME_SET;
				lAnswer.mDataLen = mActiveInterface->mPlayer.GetLength() + 4;
				*(unsigned int *) (lAnswer.mData) = mActiveInterface->mClient[lClient].GetUDPPort();
				memcpy(lAnswer.mData + 4, mActiveInterface->mPlayer, lAnswer.mDataLen - 4);

				mActiveInterface->mClient[lClient].Send(&lAnswer, MR_NET_REQUIRED);

				// Send client list if server
				if(mActiveInterface->mServerMode) {
				    lAnswer.mMessageType = MRNM_CLIENT_ADDR;
				    lAnswer.mDataLen = 8;

				    // Send the actual client list
				    for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
					if((lCounter != lClient)
					   && (mActiveInterface->mClient[lCounter].IsConnected())
					   && (mActiveInterface->mCanBePreLogued[lCounter])) {
					    *(int *) &(lAnswer.mData[0]) = mActiveInterface->mClientAddr[lCounter];
					    *(int *) &(lAnswer.mData[4]) = mActiveInterface->mClientPort[lCounter];

					    mActiveInterface->mClient[lClient].Send(&lAnswer, MR_NET_REQUIRED);

					}
				    }
				    lAnswer.mDataLen = 0;
				    mActiveInterface->mClient[lClient].Send(&lAnswer, MR_NET_REQUIRED);

				    mActiveInterface->mCanBePreLogued[lClient] = TRUE;
				}

			    }
			    break;


			case MRNM_REMOVE_ENTRY:
			    ASSERT(FALSE);	// Bad code

			    /*
			       mActiveInterface->mClient[lClient].Disconnect();
			       ListView_DeleteItem( lListHandle, lClient+1 );
			     */
			    break;

			case MRNM_CLIENT_ADDR:
			    {
				if(lBuffer->mDataLen == 0) {
				    // end of list
				    mActiveInterface->mAllPreLoguedRecv = TRUE;

				    mActiveInterface->mPreLoguedClient[0] = TRUE;

				    // Start a lag test with the server
				    // * Already done *

				    // Report that all timing tests have been done.. if needed
				    mActiveInterface->SendConnectionDoneIfNeeded();

				} else {
				    // The server sended us a new addr
				    // Create a connection to that port
				    SOCKET lNewSocket = socket(PF_INET, SOCK_STREAM, 0);

				    ASSERT(lNewSocket != INVALID_SOCKET);

				    int lSlot = -1;

				    for(int lCounter = 0; lCounter < eMaxClient; lCounter++) {
					if(!mActiveInterface->mClient[lCounter].IsConnected()) {
					    lSlot = lCounter;
					    break;
					}
				    }
				    ASSERT(lSlot != -1);

				    mActiveInterface->mClient[lSlot].Connect(lNewSocket);
				    mActiveInterface->mPreLoguedClient[lSlot] = TRUE;


				    SOCKADDR_IN lAddr;

				    lAddr.sin_family = AF_INET;
				    lAddr.sin_addr.s_addr = *(int *) &(lBuffer->mData[0]);
				    lAddr.sin_port = *(int *) &(lBuffer->mData[4]);

				    WSAAsyncSelect(lNewSocket, pWindow, MRM_CLIENT + lSlot, FD_CONNECT | FD_READ | FD_CLOSE);
				    WSAAsyncSelect(mActiveInterface->mClient[lSlot].GetUDPSocket(), pWindow, MRM_CLIENT + lSlot, FD_READ | FD_CLOSE);

				    int lCode = connect(lNewSocket, (struct sockaddr *) &lAddr, sizeof(lAddr));

				}
			    }
			    break;


			case MRNM_LAG_TEST:
			    // return the message and add the current time
			    lAnswer.mMessageType = MRNM_LAG_ANSWER;
			    lAnswer.mDataLen = 4;
			    *(int *) &(lAnswer.mData[0]) = *(int *) &(lBuffer->mData[0]);

			    // send the request
			    mActiveInterface->UDPSend(lClient, &lAnswer, TRUE);
			    // mActiveInterface->mClient[ lClient ].Send( &lAnswer, MR_NET_REQUIRED );

			    break;

			case MRNM_LAG_ANSWER:

			    // Desactivate time-out
			    KillTimer(pWindow, lClient + 10);

			    // Verify that it is not a late message
			    if(mActiveInterface->mClient[lClient].LagDone()) {
				// That is a late message.. ignore it
				TRACE("Late ping message B %d\n", lClient);
			    } else {
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

				} else {
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
				    ListView_SetItemText(lListHandle, lClient + 1, 2, (char *) MR_LoadStringBuffered(IDS_CONNECTED));

				    // Verify if weconnected with all the client that we were suppose to
				    mActiveInterface->SendConnectionDoneIfNeeded();
				}
			    }
			    break;

			case MRNM_LAG_INFO:
			    {
				mActiveInterface->mClient[lClient].SetLag(*(int *) &(lBuffer->mData[0]), *(int *) &(lBuffer->mData[4]));

				// Update display
				char lStrBuffer[20];

				ListView_SetItemText(lListHandle, lClient + 1, 1, _itoa(mActiveInterface->mClient[lClient].GetAvgLag(), lStrBuffer, 10));
				if(mActiveInterface->mServerMode) {
				    ListView_SetItemText(lListHandle, lClient + 1, 2, (char *) MR_LoadStringBuffered(IDS_WAITING_ACK));
				} else {
				    ListView_SetItemText(lListHandle, lClient + 1, 2, (char *) MR_LoadStringBuffered(IDS_CONNECTED));
				}
			    }
			    break;

			case MRNM_CONNECTION_DONE:
			    {
				ASSERT(mActiveInterface->mServerMode);

				// Mark the connection as completed
				ListView_SetItemText(lListHandle, lClient + 1, 2, (char *) MR_LoadStringBuffered(IDS_CONNECTED));
				mActiveInterface->mConnected[lClient] = TRUE;
			    }
			    break;


			case MRNM_READY:
			    //Get Client Id
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
			    } else {
				SendMessage(GetParent(pWindow), mActiveInterface->mReturnMessage, IDOK, 0);
				DestroyWindow(pWindow);
			    }

			    return TRUE;
		    }
		}

		WSAAsyncSelect(mActiveInterface->mClient[lClient].GetSocket(), pWindow, MRM_CLIENT + lClient, FD_READ | FD_CLOSE);
		WSAAsyncSelect(mActiveInterface->mClient[lClient].GetUDPSocket(), pWindow, MRM_CLIENT + lClient, FD_READ | FD_CLOSE);

		break;


	    case FD_CONNECT:
		if(WSAGETSELECTERROR(pLParam)) {
		    // Connection error with a client
		    ASSERT(FALSE);
		} else {

		    // request client name to start the connection sequence
		    // also include UDP port number in the request
		    lAnswer.mMessageType = MRNM_CONN_NAME_GET_SET;
		    lAnswer.mDataLen = mActiveInterface->mPlayer.GetLength() + 4;
		    *(unsigned int *) (lAnswer.mData) = mActiveInterface->mClient[lClient].GetUDPPort();

		    memcpy(lAnswer.mData + 4, mActiveInterface->mPlayer, lAnswer.mDataLen - 4);

		    mActiveInterface->mClient[lClient].Send(&lAnswer, MR_NET_REQUIRED);
		}

		break;
	}
	lReturnValue = TRUE;
    }


    return lReturnValue;
}

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
	    lAnswer.mDataLen = 0;
	    mClient[0].Send(&lAnswer, MR_NET_REQUIRED);
	}
    }
}


// class MR_NetworkPort
MR_NetworkPort::MR_NetworkPort()
{
    mSocket = INVALID_SOCKET;
    mUDPRecvSocket = INVALID_SOCKET;

    Disconnect();
}

MR_NetworkPort::~MR_NetworkPort()
{
    Disconnect();
}

void MR_NetworkPort::Connect(SOCKET pSocket)
{
    Disconnect();
    mSocket = pSocket;
    mWatchdog = timeGetTime();

    // Create UDPPort

    // Create a new UDP Socket for the reception
    mUDPRecvSocket = socket(PF_INET, SOCK_DGRAM, 0);
    ASSERT(mUDPRecvSocket != INVALID_SOCKET);

    // Set this port as non-blocking
    unsigned long lNonBlock = TRUE;
    int lCode = ioctlsocket(mUDPRecvSocket, FIONBIO, &lNonBlock);
    ASSERT(lCode != SOCKET_ERROR);



    // Bind the socket to any availlable address
    SOCKADDR_IN lLocalAddr;
    lLocalAddr.sin_family = AF_INET;
    lLocalAddr.sin_addr.s_addr = INADDR_ANY;
    lLocalAddr.sin_port = 0;
    lCode = bind(mUDPRecvSocket, (LPSOCKADDR) & lLocalAddr, sizeof(lLocalAddr));
    ASSERT(lCode != SOCKET_ERROR);

}

void MR_NetworkPort::SetRemoteUDPPort(unsigned int pPort)
{
    // Determine remote address and port based on the TCP port
    int lSize = sizeof(mUDPRemoteAddr);
    int lCode = getpeername(mSocket, (LPSOCKADDR) & mUDPRemoteAddr, &lSize);
    ASSERT(lCode != SOCKET_ERROR);

    mUDPRemoteAddr.sin_port = pPort;
}

unsigned int MR_NetworkPort::GetUDPPort() const const
{
    // Get UDP local addr
    SOCKADDR_IN lLocalAddr;
    int lSize = sizeof(lLocalAddr);
    int lCode = getsockname(mUDPRecvSocket, (LPSOCKADDR) & lLocalAddr, &lSize);
    ASSERT(lCode != SOCKET_ERROR);

    return lLocalAddr.sin_port;
}


void MR_NetworkPort::Disconnect()
{
    if(mSocket != INVALID_SOCKET) {
	closesocket(mSocket);
    }
    if(mUDPRecvSocket != INVALID_SOCKET) {
	closesocket(mUDPRecvSocket);
    }

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

    mAvgLag = 300;
    mMinLag = 300;
    mNbLagTest = 0;
    mTotalLag = 0;

    mOutQueueLen = 0;
    mOutQueueHead = 0;

    mInputMessageBufferIndex = 0;

}

BOOL MR_NetworkPort::IsConnected() const const
{
    return (mSocket != INVALID_SOCKET);
}


SOCKET MR_NetworkPort::GetSocket() const const
{
    return mSocket;
}

SOCKET MR_NetworkPort::GetUDPSocket() const const
{
    return mUDPRecvSocket;
}


const MR_NetMessageBuffer *MR_NetworkPort::Poll()
{
    // Socket is assume to be non-blocking
    if((mInputMessageBufferIndex == 0) && (mUDPRecvSocket != INVALID_SOCKET)) {
	while(1) {

	    int lLen = recv(mUDPRecvSocket, ((char *) &mInputMessageBuffer), sizeof(mInputMessageBuffer), 0);

	    if(lLen > 0) {
		int lQueueId = mInputMessageBuffer.mDatagramQueue;
		// We have received a datagram
		ASSERT(lLen == mInputMessageBuffer.mDataLen + MR_NET_HEADER_LEN);

		// Eliminate double and late datagram
		if((MR_Int8) ((MR_Int8) (MR_UInt8) mInputMessageBuffer.mDatagramNumber - (MR_Int8) mLastReceivedDatagramNumber[lQueueId]) > 0) {
		    // TRACE( "UDP recv\n" );
		    mLastReceivedDatagramNumber[lQueueId] = mInputMessageBuffer.mDatagramNumber;
		    mWatchdog = timeGetTime();
		    return &mInputMessageBuffer;
		} else {
		    TRACE("Late UDP %d %d\n", (MR_Int8) (MR_UInt8) mInputMessageBuffer.mDatagramNumber, (MR_Int8) mLastReceivedDatagramNumber[lQueueId]);
		}
	    } else {
		// No data.. continue with TCP
		break;
	    }
	}
    }


    if(mSocket != INVALID_SOCKET) {
	if(mInputMessageBufferIndex < MR_NET_HEADER_LEN) {
	    // Try to read message header
	    int lLen = recv(mSocket, ((char *) &mInputMessageBuffer) + mInputMessageBufferIndex, MR_NET_HEADER_LEN - mInputMessageBufferIndex, 0);

	    if(lLen > 0) {
		mInputMessageBufferIndex += lLen;
	    }
	}

	if(mInputMessageBufferIndex >= MR_NET_HEADER_LEN && (mInputMessageBuffer.mDataLen > 0)) {
	    int lLen = recv(mSocket, ((char *) &mInputMessageBuffer) + mInputMessageBufferIndex, mInputMessageBuffer.mDataLen - (mInputMessageBufferIndex - MR_NET_HEADER_LEN), 0);

	    if(lLen > 0) {
		mInputMessageBufferIndex += lLen;
	    }
	}

	if((mInputMessageBufferIndex >= MR_NET_HEADER_LEN) && (mInputMessageBufferIndex == (MR_NET_HEADER_LEN + mInputMessageBuffer.mDataLen))) {
	    mInputMessageBufferIndex = 0;
	    mWatchdog = timeGetTime();
	    return &mInputMessageBuffer;
	}

	if((mLastSendedDatagramNumber[1] > 16) && (timeGetTime() - mWatchdog) > MR_CONNECTION_TIMEOUT) {
	    // (mLastSendedDatagramNumber[1]>16) -- this condition avoid disconnecting 
	    //                                   -- before game start
	    TRACE("Reception TimeOut %d %d\n", timeGetTime() - mWatchdog, mInputMessageBufferIndex);

	    if(mInputMessageBufferIndex != 0) {
		TRACE("Message: %d %d\n", mInputMessageBuffer.mMessageType, mInputMessageBuffer.mDataLen);
	    }

	    Disconnect();
	}
    }

    return NULL;
}


BOOL MR_NetworkPort::UDPSend(SOCKET pSocket, MR_NetMessageBuffer * pMessage, unsigned pQueueId, BOOL pResendLast)
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

void MR_NetworkPort::Send(const MR_NetMessageBuffer * pMessage, int pReqLevel)
{
    // First try to send buffered data
    if(mSocket != INVALID_SOCKET) {
	BOOL lEndQueueLoop = (mOutQueueLen == 0);

	while(!lEndQueueLoop) {

	    // Determine first batch to send
	    int lToSend;;

	    if((mOutQueueHead + mOutQueueLen) > MR_OUT_QUEUE_LEN) {
		lToSend = MR_OUT_QUEUE_LEN - mOutQueueHead;
	    } else {
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
	    } else {
		lEndQueueLoop = TRUE;

		if(WSAGetLastError() == WSAEWOULDBLOCK) {
		    // notting to do
		} else {
		    TRACE("Communication error A %d\n", WSAGetLastError());
		    Disconnect();
		}
	    }
	}
    }

    if(mSocket != INVALID_SOCKET) {
	int lToSend = MR_NET_HEADER_LEN + pMessage->mDataLen;

	int lReturnValue;	// send return value

	if(mOutQueueLen > 0) {
	    lReturnValue = 0;
	} else {
	    lReturnValue = send(mSocket, ((const char *) pMessage), lToSend, 0);

	    // TRACE( "Send %d %d %d\n", lToSend, lToSend-lSent, lReturnValue );

	    if((lReturnValue == -1) && (WSAGetLastError() == WSAEWOULDBLOCK)) {
		lReturnValue = 0;
	    }
	}


	if(lReturnValue < 0) {
	    TRACE("Communication error B %d\n", WSAGetLastError());
	    Disconnect();
	} else if(lReturnValue != lToSend) {
	    if((lReturnValue > 0) || (pReqLevel == MR_NET_REQUIRED) || ((pReqLevel == MR_NET_TRY) && (mOutQueueLen < (MR_OUT_QUEUE_LEN / 4)))
		) {

		lToSend -= lReturnValue;

		if(lToSend > (MR_OUT_QUEUE_LEN - mOutQueueLen)) {
		    // no space left
		    TRACE("Output queue full\n");
		    Disconnect();
		    ASSERT(FALSE);
		} else {
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

BOOL MR_NetworkPort::AddLagSample(int pLag)
{
    mNbLagTest++;

    mTotalLag += pLag;

    mAvgLag = mTotalLag / (mNbLagTest * 2);

    mMinLag = min(mMinLag, pLag / 2);

    return LagDone();
}

BOOL MR_NetworkPort::LagDone() const const
{
    return (mNbLagTest >= 5);
}

int MR_NetworkPort::GetAvgLag() const const
{
    return mAvgLag;
}

int MR_NetworkPort::GetMinLag() const const
{
    return mMinLag;
}

void MR_NetworkPort::SetLag(int pAvgLag, int pMinLag)
{
    mAvgLag = pAvgLag;
    mMinLag = pMinLag;

    mNbLagTest = 100;		// Set as completed

}



// Helper functions

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
	lReturnValue += " ";
	lReturnValue += MR_LoadString(IDS_NO_NET_CONNECT);
    }

    return lReturnValue;
}



MR_UInt32 GetAddrFromStr(const char *pName)
{

    MR_UInt32 lReturnValue;
    unsigned int lNible[4];

    // Verify if the given string is not a numeric addr

    int lNbField = sscanf(pName, " %d.%d.%d.%d", &(lNible[0]),
			  &(lNible[1]),
			  &(lNible[2]),
			  &(lNible[3]));

    if((lNbField == 4) && (lNible[0] != 0)) {
	lReturnValue = lNible[0]
	    + (lNible[1] << 8)
	    + (lNible[2] << 16)
	    + (lNible[3] << 24);

    } else {
	lReturnValue = INADDR_ANY;
    }
    return lReturnValue;
}

static BOOL CALLBACK DialogProc(HWND pWindow, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
	case WM_COMMAND:
	    switch (LOWORD(wParam)) {
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
