// MatchReport.cpp
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

#include "MatchReport.h"
#include "Banner.h"

#include <WINSOCK.h>
#include "resource.h"

// Local constants
#define MODULE_NAME           NULL           // For ressources loading
#define MRM_NET_EVENT         (WM_USER+7)

#define ANSWER_HEADER         "[LADDER_ANSWER_1]"
#define QUIT_BTN              "QUIT_BTN"            // More like a OK button  
#define RETRY_BTN             "RETRY_BTN"           // This button is STUPID
#define URL_BTN               "URL_BTN"             // URL login
#define TEXT_LABEL            "TEXT"                // Text to be displayed in the window

#define QUIT_BTN_TYPE         1
#define RETRY_BTN_TYPE        2
#define URL_BTN_TYPE          3

#define OP_TIMEOUT_EVENT      1
#define OP_TIMEOUT           45  // in seconds

// Local classes declaration

class InternetRequest
{
   protected:
      time_t  mStartTime;

      SOCKET  mSocket;
      char    mRequest[800];

      char    mBuffer[4096]; //Space for the result
      int     mBufferIndex;

      void Close();
      BOOL Working()const;

   public:
      InternetRequest();
      ~InternetRequest();

      BOOL Send( HWND pWindow, unsigned long pIP, unsigned int pPort, const char* pURLPath, const char* pRequest );
      void Clear();  // Abort and clean
      BOOL ProcessEvent( WPARAM pWParam, LPARAM pLParam ); // return TRUE if some processing was done

      const char* GetBuffer();
      // const char* GetBuffer( int& pSize )const;
      BOOL IsReady()const;      
};



class MatchReportRequest
{
   private:
     // Class attributes
     static char mPassword[13];
     char        mRequest[500];   // Request seended to the server
     char        mResultStr[500]; // String displayed to the user

     // Temporary data used during Process operation
     InternetRequest  mInternetRequest;
     unsigned long    mIP;
     unsigned int     mPort;
     const char*      mURLPath;
     char             mComment[200];

     // Answer content
     int              mNbButtons;
     int              mButtonType[3];
     char             mButtonLabel[3][16];
     char             mButtonURL[3][80];
     char             mAnswerText[320];

     BOOL ParseAnswer();

     static BOOL CALLBACK SubmitCallBack( HWND pWindow, UINT  pMsgId, WPARAM  pWParam, LPARAM  pLParam );
     static BOOL CALLBACK SubmitProcessCallBack( HWND pWindow, UINT  pMsgId, WPARAM  pWParam, LPARAM  pLParam );
     static BOOL CALLBACK ResultCallBack( HWND pWindow, UINT  pMsgId, WPARAM  pWParam, LPARAM  pLParam );

   public:
      MatchReportRequest();

      // Operations
      BOOL        AddResultString( const char* pString );
      BOOL        AddVariable( const char*pVar, const char* pValue, const char*pLabel = NULL );
      static void SetDefaultPassword( const char* pPassword );
      BOOL Process( HWND pWindow, unsigned long pIP, unsigned int pPort, const char* pURLPath );

      static const char* GetPassword(); 

};

// Local variables
char MatchReportRequest::mPassword[13] = {0};

static MatchReportRequest* gThis = NULL;


// Local functions declarations
static BOOL CALLBACK DefaultCallBack( HWND pWindow, UINT  pMsgId, WPARAM  pWParam, LPARAM  pLParam );
static const char* Pad( const char*pIn, char* pOut, int pOutSize );
static int         GetLineLen( const char* pSrc );
static const char* GetNextLine( const char* pSrc );

// Interface mapping
void MReport_Clear( BOOL pKeepPassword )
{
   if( gThis != NULL )
   {
      delete gThis;
   }

   if( !pKeepPassword )
   {
      MatchReportRequest::SetDefaultPassword( NULL );
   }
}

BOOL MReport_AddResultString( const char* pString )
{
   BOOL lReturnValue = FALSE;

   if( gThis == NULL )
   {
      gThis = new MatchReportRequest();
   }

   if( gThis != NULL )
   {
      lReturnValue = gThis->AddResultString( pString );
   }
   return lReturnValue;
}

BOOL MReport_AddVariable( const char*pVar, const char* pValue, const char*pLabel )
{
   BOOL lReturnValue = FALSE;

   if( gThis == NULL )
   {
      gThis = new MatchReportRequest();
   }

   if( gThis != NULL )
   {
      lReturnValue = gThis->AddVariable( pVar, pValue, pLabel );
   }
   return lReturnValue;
}

void MReport_SetDefaultPassword( const char* pPassword )
{
   MatchReportRequest::SetDefaultPassword( pPassword );
}

BOOL MReport_Process( HWND pWindow, unsigned long pIP, unsigned int pPort, const char* pURLPath )
{
   BOOL lReturnValue = FALSE;

   if( gThis == NULL )
   {
      gThis = new MatchReportRequest();
   }

   if( gThis != NULL )
   {
      lReturnValue = gThis->Process( pWindow, pIP, pPort, pURLPath );
   }
   return lReturnValue;

}

const char* MReport_GetPassword()
{
   return MatchReportRequest::GetPassword();
}



// Local class operations
InternetRequest::InternetRequest()
{
   mSocket      = INVALID_SOCKET;
   mBufferIndex = 0;
}

InternetRequest::~InternetRequest()
{
   Close();
}


void InternetRequest::Close()
{
   if( mSocket != INVALID_SOCKET )
   {
      closesocket( mSocket );
      mSocket = INVALID_SOCKET;
   }
}

BOOL InternetRequest::Working()const
{
   return( mSocket!=INVALID_SOCKET );
}

void InternetRequest::Clear()
{
   Close();
   mRequest[0]  = 0;
   mBufferIndex = 0;
}


BOOL InternetRequest::Send( HWND pWindow, unsigned long pIP, unsigned pPort, const char*pURLPath, const char* pRequest )
{
   BOOL lReturnValue = FALSE;

   if( !Working() )
   {
      Clear();

      mStartTime = time( NULL );
      

      sprintf( mRequest,  "GET %s?%s HTTP/1.0\n\r"
                          "Accept: */*\n\r"
                          "UserAgent:  Custom(by GrokkSoft)/0.1\n\r"
                          "\n\r",
                          pURLPath,
                          pRequest   );

      mSocket    = socket( PF_INET, SOCK_STREAM, 0 );

      ASSERT( mSocket != INVALID_SOCKET );

      SOCKADDR_IN lAddr;

      lAddr.sin_family      = AF_INET;
      lAddr.sin_addr.s_addr = pIP;
      lAddr.sin_port        = htons( (unsigned short)pPort );

      WSAAsyncSelect( mSocket, pWindow, MRM_NET_EVENT, FD_CONNECT|FD_READ|FD_CLOSE );
                
      connect( mSocket, (struct sockaddr*)&lAddr, sizeof( lAddr ) );

      lReturnValue = TRUE;
   }
   return lReturnValue;
}


BOOL InternetRequest::ProcessEvent( WPARAM pWParam, LPARAM pLParam )
{
   BOOL lReturnValue = FALSE;

   if( Working()&& (pWParam == mSocket) )
   {
      lReturnValue = TRUE;

      switch( WSAGETSELECTEVENT( pLParam) )
      {
         case FD_CONNECT:
            // We are now connected, send the request
            send( mSocket, mRequest, strlen( mRequest ), 0 );
            mRequest[0] = 0;
            break;

         case FD_READ:
            int lNbRead;

            if( mBufferIndex >= sizeof( mBuffer ) )
            {
               Close();
            }
            else
            {
               lNbRead = recv( mSocket, mBuffer+mBufferIndex, sizeof( mBuffer )-mBufferIndex, 0 );

               if( lNbRead >= 0 )
               {
                  mBufferIndex += lNbRead;
               }
               else
               {
                  Close();
               }
            }
            break;


         case FD_CLOSE:
            Close();
            break;
      }
   }

   return lReturnValue;

}

const char* InternetRequest::GetBuffer()
{
   mBuffer[ min(mBufferIndex, sizeof( mBuffer )-1) ] = 0;
   return mBuffer;
}

/*
const char* InternetRequest::GetBuffer( int& pSize )const
{
   pSize = mBufferIndex;
   return mBuffer;
}
*/


BOOL InternetRequest::IsReady()const
{
   return( (mBufferIndex!=0) && !Working() );
}




// MatchReportRequest

MatchReportRequest::MatchReportRequest()
{
   mRequest[0]   = 0;
   mResultStr[0] = 0;
}


BOOL MatchReportRequest::AddResultString( const char* pString )
{
   BOOL lReturnValue = FALSE;

   if( strlen( pString )+strlen( mResultStr ) + 3 < sizeof( mResultStr ) )
   {
      if( strlen( mResultStr ) != 0 )
      {
         strcat( mResultStr, "\n" );
      }
      strcat( mResultStr, pString );

      lReturnValue = TRUE;
   }
   return lReturnValue;
}

BOOL MatchReportRequest::AddVariable( const char*pVar, const char* pValue, const char*pLabel )
{
   BOOL lReturnValue = FALSE;

   char lPaddedValue[500];

   Pad( pValue, lPaddedValue, sizeof( lPaddedValue ) );

   if( strlen( pVar )+strlen(lPaddedValue)+strlen( mRequest ) + 3 < sizeof( mRequest ) )
   {
      strcat( mRequest, "&" );
      strcat( mRequest, pVar );
      strcat( mRequest, "=" );
      strcat( mRequest, lPaddedValue );

      lReturnValue = TRUE;
   }


   if( lReturnValue && pLabel != NULL )
   {
      if( strlen( pValue )+strlen( pLabel )+strlen( mResultStr ) + 5 < sizeof( mResultStr ) )
      {
         if( strlen( mResultStr ) != 0 )
         {
            strcat( mResultStr, "\n" );
         }
         strcat( mResultStr, pLabel );
         strcat( mResultStr, ": " );
         strcat( mResultStr, pValue );
      }
   }
   return lReturnValue;
}

void MatchReportRequest::SetDefaultPassword( const char* pPassword )
{
   strncpy( mPassword, pPassword, sizeof( mPassword ) );
   mPassword[ sizeof( mPassword )-1 ] = 0;
}

const char* MatchReportRequest::GetPassword()
{
   return mPassword;
}

BOOL MatchReportRequest::Process( HWND pWindow, unsigned long pIP, unsigned int pPort, const char* pURLPath )
{
   BOOL             lReturnValue    = FALSE;

   // Save these parameters in the static local storage
   mIP              = pIP;
   mPort            = pPort;
   mURLPath         = pURLPath;
   mComment[0]      = 0;

   // Pop the window 

   lReturnValue = (DialogBox( GetModuleHandle( MODULE_NAME ), MAKEINTRESOURCE( IDD_LADDER_SUBMIT ), pWindow, SubmitCallBack  )==IDOK);

   return lReturnValue;
}

BOOL MatchReportRequest::ParseAnswer( )
{
   mNbButtons     = 0;
   mAnswerText[0] = 0;

   if( mInternetRequest.IsReady() )
   {
      // Find the start
      const char* lLinePtr = mInternetRequest.GetBuffer();

      while( lLinePtr != NULL )
      {
         if( !strncmp( lLinePtr, ANSWER_HEADER, strlen( ANSWER_HEADER ) ) )
         {
            lLinePtr = GetNextLine( lLinePtr );
            break;
         }
         else
         {
            lLinePtr = GetNextLine( lLinePtr );
         }
      }

      while( lLinePtr != NULL )
      {
         if( !strncmp( lLinePtr, TEXT_LABEL, strlen( TEXT_LABEL) ) )
         {
            char lBuffer[150];
            int  lCurrentLen = strlen( mAnswerText );
            sscanf( lLinePtr, "%*s \"%150[^\"]", &mAnswerText );
            
            if( (lCurrentLen > 0) )
            {
               if( lCurrentLen < sizeof(mAnswerText)-5 )
               {
                  strcat( mAnswerText, "\n\r" );
                  lCurrentLen += 2;
            
                  strncpy( mAnswerText+lCurrentLen, lBuffer, sizeof( mAnswerText )-lCurrentLen );
               }
            }
            else
            {
               strncpy( mAnswerText+lCurrentLen, lBuffer, sizeof( mAnswerText ) );
            }

            mAnswerText[ sizeof( mAnswerText )-1 ] = 0;
         }
         else if( mNbButtons < 3 )
         {
            if( !strncmp( lLinePtr, QUIT_BTN, strlen(QUIT_BTN) ) ) 
            {
               mButtonType[mNbButtons] = QUIT_BTN_TYPE;
               sscanf( lLinePtr, "%*s \"%16[^\"]", mButtonLabel[mNbButtons] );               
               mNbButtons++;
            }
            else if( !strncmp( lLinePtr, RETRY_BTN, strlen(RETRY_BTN) ) )
            {
               mButtonType[mNbButtons] = RETRY_BTN_TYPE;
               sscanf( lLinePtr, "%*s \"%16[^\"]", mButtonLabel[mNbButtons] );
               mNbButtons++;
            }
            else if( !strncmp( lLinePtr, URL_BTN, strlen(URL_BTN) ) )
            {
               mButtonType[mNbButtons] = URL_BTN_TYPE;
               sscanf( lLinePtr, "%*s \"%16[^\"] \"%80[^\"]", mButtonLabel[mNbButtons], mButtonURL[mNbButtons] );               
               mNbButtons++;
            }
         }            
         lLinePtr = GetNextLine( lLinePtr );
      }
   }
   return( mNbButtons>0 );
}

BOOL CALLBACK MatchReportRequest::SubmitCallBack( HWND pWindow, UINT  pMsgId, WPARAM  pWParam, LPARAM  pLParam )
{
   BOOL lReturnValue = FALSE;

   switch( pMsgId )
   {
      // Catch environment modification events
      case WM_INITDIALOG:
         // Setup window content
         SetDlgItemText( pWindow, IDC_PASSWORD, gThis->mPassword );
         SetDlgItemText( pWindow, IDC_RESULT,   gThis->mResultStr );
         lReturnValue = TRUE;
         break;


      case WM_COMMAND:
         switch(LOWORD( pWParam))
         {
            case IDCANCEL:
               // Ask the User if he is sure he want to do that
               if( DialogBox( GetModuleHandle( MODULE_NAME ), MAKEINTRESOURCE( IDD_LADDER_SUBMIT_QUIT ), pWindow, DefaultCallBack  )==IDYES )
               {
                  EndDialog( pWindow, IDCANCEL );
               }
               lReturnValue = TRUE;
               break;

            case IDOK:
               {
                  BOOL lRetry   = TRUE;
                  BOOL lSuccess = FALSE;

                  // Fetch the password and comment from the control
                  GetDlgItemText( pWindow, IDC_PASSWORD, gThis->mPassword, sizeof(gThis->mPassword) );
                  GetDlgItemText( pWindow, IDC_COMMENT,  gThis->mComment,  sizeof(gThis->mComment) );


                  while( lRetry )
                  {
                     lRetry = FALSE;
                     
                     int lResult = DialogBox( GetModuleHandle( MODULE_NAME ), MAKEINTRESOURCE( IDD_LADDER_SUBMIT_PROCESS ), pWindow, SubmitProcessCallBack );

                     // Parse the result
                     switch( lResult )
                     {
                        case 1: // Cancel
                           // Nothing special
                           // Break this loop without success
                           break;

                        case 2: // Timeout
                           // Offer to immediately retry
                           // do not terminate there is no success
                           if( DialogBox( GetModuleHandle( MODULE_NAME ), MAKEINTRESOURCE( IDD_LADDER_SUBMIT_QUIT ), pWindow, DefaultCallBack  )==IDRETRY )
                           {
                              lRetry = TRUE;
                           }
                           break;
                        
                        case 3: // We got an answer..parse that answer
                           {
                              if( gThis->ParseAnswer() )
                              {
                                 int lDlg = IDD_LADDER_RES_3;

                                 if( gThis->mNbButtons == 1 )
                                 {
                                    lDlg = IDD_LADDER_RES_1;
                                 }
                                 else if( gThis->mNbButtons == 2 )
                                 {
                                    lDlg = IDD_LADDER_RES_2;
                                 }

                                 int lResult = DialogBox( GetModuleHandle( MODULE_NAME ), MAKEINTRESOURCE( lDlg ), pWindow, ResultCallBack  );

                                 switch( lResult )
                                 {
                                    case IDRETRY:
                                       lRetry = TRUE;
                                       break;

                                    case IDCANCEL:
                                       // Do nothing special just end this loop
                                       break;

                                    case IDOK:
                                       lSuccess = TRUE;
                                       break;
                                 }
                              }
                              else
                              {
                                 if( DialogBox( GetModuleHandle( MODULE_NAME ), MAKEINTRESOURCE( IDD_LADDER_RETRY ), pWindow, DefaultCallBack  )==IDRETRY )
                                 {
                                    lRetry = TRUE;
                                 }
                              }
                           }
                           break;

                        default: // -1 or any other error
                           ASSERT( FALSE );
                           // This make no sence ..anyway just display a standard error
                           if( DialogBox( GetModuleHandle( MODULE_NAME ), MAKEINTRESOURCE( IDD_LADDER_SUBMIT_QUIT ), pWindow, DefaultCallBack  )==IDRETRY )
                           {
                              lRetry = TRUE;
                           }
                     }
                  }

                  if( lSuccess )
                  {
                     EndDialog( pWindow, IDOK );
                  }
               }
               lReturnValue = TRUE;
               break;

         }
         break;
   }
   return lReturnValue;
}

BOOL CALLBACK MatchReportRequest::ResultCallBack( HWND pWindow, UINT  pMsgId, WPARAM  pWParam, LPARAM  pLParam )
{
   BOOL lReturnValue = FALSE;

   switch( pMsgId )
   {
      case WM_INITDIALOG:
         {
            // Setup window content
            SetDlgItemText( pWindow, IDC_TEXT, gThis->mAnswerText );

            for( int lCounter = 0; lCounter < gThis->mNbButtons; lCounter++ )
            {
               SetDlgItemText( pWindow, ID_BTN1+lCounter, gThis->mButtonLabel[lCounter] );
            }
            lReturnValue = TRUE;
         }
         break;

      case WM_COMMAND:
         if( LOWORD(pWParam)==IDCANCEL )
         {
            EndDialog( pWindow, IDCANCEL );
            lReturnValue = TRUE;
         }
         else if( (LOWORD(pWParam)>=ID_BTN1)&&((LOWORD(pWParam)-ID_BTN1)<gThis->mNbButtons ))
         {
            int lButton = LOWORD(pWParam)-ID_BTN1;

            switch( gThis->mButtonType[ lButton ] )
            {
               case QUIT_BTN_TYPE:
                  EndDialog( pWindow, IDOK );
                  break;

               case RETRY_BTN_TYPE:
                  EndDialog( pWindow, IDCANCEL );
                  break;

               case URL_BTN_TYPE:
                  // Start the Browser
                  LoadURL( pWindow, gThis->mButtonURL[lButton ] );
                  break;
            }
            lReturnValue = TRUE;
         }
         break;      
   }

   return lReturnValue;
}




BOOL CALLBACK MatchReportRequest::SubmitProcessCallBack( HWND pWindow, UINT  pMsgId, WPARAM  pWParam, LPARAM  pLParam )
{
   BOOL lReturnValue = FALSE;

   switch( pMsgId )
   {
      // Catch environment modification events
      case WM_INITDIALOG:
         {
            char lRequest[1024];
            char lPaddedPasswd[20];
            char lPaddedComment[220];


            Pad( gThis->mPassword, lPaddedPasswd, sizeof( lPaddedPasswd ) );
            Pad( gThis->mComment,  lPaddedComment, sizeof( lPaddedComment ) );


            sprintf( lRequest, "password=%s%scomment=%s", lPaddedPasswd, gThis->mRequest, lPaddedComment );


            // Initiate the request
            gThis->mInternetRequest.Send( pWindow, gThis->mIP, gThis->mPort, gThis->mURLPath, lRequest );

            // start a timeout timer
            SetTimer( pWindow, OP_TIMEOUT_EVENT, OP_TIMEOUT, NULL );            
         }
         break;

      case WM_TIMER:
         {
            // Timeout
            gThis->mInternetRequest.Clear();
            KillTimer( pWindow, pWParam );

            EndDialog( pWindow, 1 );

            lReturnValue = TRUE;
         }
         break;


      case MRM_NET_EVENT:
         gThis->mInternetRequest.ProcessEvent( pWParam, pLParam );

         if( gThis->mInternetRequest.IsReady() )
         {
            KillTimer( pWindow, OP_TIMEOUT_EVENT );

            EndDialog( pWindow, 3 );
         }
         lReturnValue = TRUE;
         break;

      case WM_COMMAND:
         switch(LOWORD( pWParam))
         {
            case IDCANCEL:
               EndDialog( pWindow, 1 );
               KillTimer( pWindow, OP_TIMEOUT_EVENT );
               gThis->mInternetRequest.Clear();

               lReturnValue = TRUE;
               break;
         }
         break;
   }
   return lReturnValue;
}



// Local functions
static BOOL CALLBACK DefaultCallBack( HWND pWindow, UINT  pMsgId, WPARAM  pWParam, LPARAM  pLParam )
{
   BOOL lReturnValue = FALSE;

   switch( pMsgId )
   {
      // Catch environment modification events
      case WM_INITDIALOG:
         // Setup window content
         lReturnValue = TRUE;
         break;


      case WM_COMMAND:
         EndDialog( pWindow, LOWORD( pWParam) );
         break;
   }
   return lReturnValue;
}

static const char* Pad( const char*pIn, char* pOut, int pOutSize )
{
   int lOutIndex = 0;

   while( (*pIn != 0)&&(lOutIndex<pOutSize-4) )
   {
      if( *pIn <= ' ' )
      {
         pOut[ lOutIndex++ ] = '+';
      }
      else if( isalnum( *pIn ) )
      {
         pOut[ lOutIndex++ ] = *pIn;
      }
      else
      {
         switch( *pIn )
         {
            case '$':
            case '-':
            case '_':
            case '.':
            // case '+':
            case '!':
            case '*':
            case '\'':
            case '(':
            case ')':
            case ',':
            case ':':
            case '@':
            // case '&':
            // case '=':

            pOut[ lOutIndex++ ] = *pIn;
            break;

            default:
               sprintf( pOut+(lOutIndex++), "%%%02x", (unsigned)*pIn );
         }

         pIn++;         
      }
   }
   pOut[ lOutIndex ] = 0;

   return pOut;
}

const char* GetNextLine( const char* pSrc )
{
   const char* lReturnValue = NULL;

   if( pSrc != NULL )
   {
      lReturnValue = pSrc+GetLineLen( pSrc );

      if( *lReturnValue == 0 )
      {
         lReturnValue = NULL;
      }
      else
      {
         lReturnValue++;
      }
   }
   return lReturnValue;
}

int         GetLineLen( const char* pSrc )
{
   int lReturnValue = 0;

   if( pSrc != NULL )
   {
      const char* lEoL = strchr( pSrc, '\n' );

      if( lEoL != NULL )
      {
         lReturnValue = lEoL - pSrc;
      }
      else
      {
         lReturnValue = strlen( pSrc );
      }
   }
   return lReturnValue;
}





