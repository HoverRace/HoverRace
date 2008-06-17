// MatchReport.h
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

#ifndef MATCH_REPORT_H
#define MATCH_REPORT_H

// Exported functions (Ansi C style for portability and DLL creation)

void        MReport_Clear             ( BOOL pKeepPassword = FALSE );
BOOL        MReport_AddResultString   ( const char* pString );
BOOL        MReport_AddVariable       ( const char*pVar, const char* pValue, const char*pLabel = NULL );
void        MReport_SetDefaultPassword( const char* pPassword );
BOOL        MReport_Process           ( HWND pWindow, unsigned long pIP, unsigned int pPort, const char* pURLPath );
const char* MReport_GetPassword       ( ); // To be able to reuse password from one call to an other

#endif



