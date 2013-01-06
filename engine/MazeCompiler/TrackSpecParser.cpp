// TrackSpecParser.cpp
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

#include "StdAfx.h"

#include "TrackSpecParser.h"

#ifndef _WIN32
#	include <strings.h>
#	define _stricmp strcasecmp
#endif

namespace HoverRace {
namespace MazeCompiler {

TrackSpecParser::TrackSpecParser(FILE * pFile)
{
	mFile = pFile;
	Reset();
}

TrackSpecParser::~TrackSpecParser()
{
}

void TrackSpecParser::Reset()
{
	ASSERT(mFile != NULL);

	fseek(mFile, 0, SEEK_SET);

	mLineBuffer[0] = 0;
	mParsePtr = mLineBuffer;
	mLineNumber = 0;
	ReadNewLine();
}

BOOL TrackSpecParser::ReadNewLine()
{
	ASSERT(mFile != NULL);

	mParsePtr = mLineBuffer;
	mLineBuffer[0] = 0;

	if(fgets(mLineBuffer, sizeof(mLineBuffer), mFile) != NULL) {
		mLineBuffer[sizeof(mLineBuffer) - 1] = 0;
		mLineNumber++;

		// Enlever le CR a la fin de la ligne et la mettre en majuscule
		// Enlever les commentaires
		char *lPtr = mLineBuffer;
		BOOL lJustBlank = TRUE;
		BOOL lAfterEqual = FALSE;

		while(*lPtr != 0) {

			switch (*lPtr) {

				case ';':
				case '#':
				case 10:
				case 13:
					*lPtr = 0;
					break;

				case '=':
					lAfterEqual = TRUE;

				default:
					if(lJustBlank && !isspace(*lPtr)) {
						lJustBlank = FALSE;
					}
					if(!lAfterEqual) {
						*lPtr = (char) toupper(*lPtr);
					}
			}

			if(*lPtr != 0) {
				lPtr++;
			}
		}

		// S'assurer que la ligne n'est pas vide
		if(lJustBlank) {
			return ReadNewLine();
		}
		else {
			return TRUE;
		}
	}
	else {
		return FALSE;
	}
}

const char *TrackSpecParser::InternalGetNextClass()
{

	if(mParsePtr != mLineBuffer) {
		if(!ReadNewLine()) {
			return NULL;
		}
	}

	while(mLineBuffer[0] != '[') {
		if(!ReadNewLine()) {
			return NULL;
		}
	}

	// Copier le mot dans le buffer de retour

	// Remplacer les ] car ils est impossible de les inclures dans un set pour scanf
	char *lBracketPtr = strchr(mLineBuffer, ']');

	if(lBracketPtr != NULL) {
		*lBracketPtr = '|';
	}

	sscanf(mLineBuffer + 1, " %29[^|]", mReturnBuffer);
	mParsePtr++;

	return mReturnBuffer;

}

const char *TrackSpecParser::GetNextClass(const char *pClassType)
{
	const char *lReturnValue;

	do {
		lReturnValue = InternalGetNextClass();
	}
	while(pClassType != NULL && lReturnValue != NULL && _stricmp(pClassType, lReturnValue));

	return lReturnValue;
}

const char *TrackSpecParser::GetNextAttrib(const char *pAttrib)
{

	if((mParsePtr != mLineBuffer) || (mLineNumber == 0)) {
		if(!ReadNewLine()) {
			return NULL;
		}
	}

	while(strchr(mLineBuffer, '=') == NULL) {
		if(mLineBuffer[0] == '[') {
			return NULL;
		}

		if(!ReadNewLine()) {
			return NULL;
		}
	}

	// Copy the answer in the return buffer
	sscanf(mLineBuffer, " %29[^ =]", mReturnBuffer);
	mParsePtr = strchr(mLineBuffer, '=') + 1;

	// Verify that the word can be accepted by the user
	const char *lReturnValue = mReturnBuffer;

	if(pAttrib != NULL) {
		if(_stricmp(pAttrib, mReturnBuffer)) {
			lReturnValue = GetNextAttrib(pAttrib);
			// Warning, this code can easily lead to a stack overflow
		}
	}

	return lReturnValue;
}

BOOL TrackSpecParser::GetNextLine()
{

	if(!ReadNewLine()) {
		return FALSE;
	}

	if(mLineBuffer[0] == '[') {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

const char *TrackSpecParser::GetParams()
{
	return mParsePtr;
}

const char *TrackSpecParser::GetNextStrParam(const char *pDefault)
{
	sscanf(mParsePtr, " %29[^,]", mReturnBuffer);
	mParsePtr = strchr(mParsePtr, ',');

	if(mParsePtr == NULL) {
		mParsePtr = mLineBuffer + strlen(mLineBuffer);
	}
	else {
		mParsePtr++;
	}

	// Enlever les espaces blancs que l'on pourrait avoir en fin de ligne
	for(int lCounter = strlen(mReturnBuffer) - 1; lCounter >= 0; lCounter--) {
		if(mReturnBuffer[lCounter] == ' ') {
			mReturnBuffer[lCounter] = 0;
		}
		else {
			break;
		}
	}

	if(strlen(mReturnBuffer) > 0) {
		return mReturnBuffer;
	}
	else {
		return pDefault;
	}
}

double TrackSpecParser::GetNextNumParam(double pDefaultValue)
{
	const char *lReturnValue = GetNextStrParam();

	if(lReturnValue == NULL) {
		return pDefaultValue;
	}
	else {
		return atof(lReturnValue);
	}

}

int TrackSpecParser::GetErrorLine() const
{
	return mLineNumber;
}

}  // namespace MazeCompiler
}  // namespace HoverRace
