// TextParsing.h
//
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

#ifndef TEXT_PARSING_H
#define TEXT_PARSING_H


BOOL          MR_ReadPredefinedConstants( const char* pFileName );

CString       MR_PreProcLine( const char* pLine );

BOOL          MR_BeginByKeyword( const char* pLine, const char* pKeyword );

int           MR_ContainsKeyword( const char* pLine, const char** pKeywordList );

const char*   MR_SkipLeadingSpaces( const char* );


#endif

