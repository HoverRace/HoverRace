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

#include "../StdAfx.h"

#include "TrackSpecParser.h"

#ifndef _WIN32
#	include <strings.h>
#	define _stricmp strcasecmp
#endif

namespace HoverRace {
namespace MazeCompiler {

TrackSpecParser::TrackSpecParser(std::istream &in) : in(in)
{
	Reset();
}

TrackSpecParser::~TrackSpecParser()
{
}

void TrackSpecParser::Reset()
{
	in.clear();
	in.seekg(0, std::ios::beg);

	mLineBuffer[0] = 0;
	mParsePtr = mLineBuffer;
	mLineNumber = 0;
	ReadNewLine();
}

/**
 * Read the next meaningful line from the input stream into the buffer.
 * @return @c true if successful,
 *         @c false if the end of the stream has been reached.
 */
bool TrackSpecParser::ReadNewLine()
{
	if (in.eof()) return false;

	std::string ris;
	mParsePtr = mLineBuffer;
	mLineBuffer[0] = 0;

	std::getline(in, ris);

	ASSERT(ris.length() < sizeof(mLineBuffer));
	strncat(mLineBuffer, ris.c_str(), sizeof(mLineBuffer) - 1);

	mLineBuffer[sizeof(mLineBuffer) - 1] = 0;
	mLineNumber++;

	char *lPtr = mLineBuffer;
	bool lJustBlank = true;
	bool lAfterEqual = false;

	while(*lPtr != 0) {

		switch (*lPtr) {

			// Remove comments and ending CR/LF.
			case ';':
			case '#':
			case 10:
			case 13:
				*lPtr = 0;
				break;

			case '=':
				lAfterEqual = true;

			// Convert to uppercase.
			default:
				if(lJustBlank && !isspace(*lPtr)) {
					lJustBlank = false;
				}
				if(!lAfterEqual) {
					*lPtr = (char) toupper(*lPtr);
				}
		}

		if(*lPtr != 0) {
			lPtr++;
		}
	}

	// Make sure the line is not empty.
	if(lJustBlank) {
		return ReadNewLine();
	}
	else {
		return true;
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

	// Copy the word in the buffer back

	// Replace "]" because they can not include them in a set for scanf
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

bool TrackSpecParser::GetNextLine()
{

	if(!ReadNewLine()) {
		return false;
	}

	if(mLineBuffer[0] == '[') {
		return false;
	}
	else {
		return true;
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

	// Remove white space that could be the end of the line
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
