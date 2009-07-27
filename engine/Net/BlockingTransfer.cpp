
// BlockingTransfer.h
// The simple blocking transfer.
//
// Copyright (c) 2009 Michael Imamura.
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

#include "StdAfx.h"

#include "BlockingTransfer.h"

using namespace HoverRace::Net;

/**
 * Constructor (for storing to a string).
 * @param agent The transfer agent (request parameters).
 * @param buf The string to store to.
 * @param cancelFlag Control object to check for cancellation (may be @c NULL).
 */
BlockingTransfer::BlockingTransfer(const Agent &agent, std::string &buf,
                                   CancelFlagPtr cancelFlag) :
	SUPER(agent), cancelFlag(cancelFlag)
{
	Init();
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StringWriteFunc);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
}

/**
 * Constructor (for storing to a stream).
 * @param agent The transfer agent (request parameters).
 * @param buf The stream to write to.
 * @param cancelFlag Control object to check for cancellation (may be @c NULL).
 */
BlockingTransfer::BlockingTransfer(const Agent &agent, std::ostream &buf,
                                   CancelFlagPtr cancelFlag) :
	SUPER(agent), cancelFlag(cancelFlag)
{
	Init();
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StreamWriteFunc);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
}

void BlockingTransfer::Init()
{
	if (cancelFlag == NULL) {
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
	}
	else {
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, ProgressFunc);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, cancelFlag.get());
	}
}

/**
 * Start the transfer and return when the transfer is complete.
 * @throw NetExn If an error occurs during the transfer.
 */
void BlockingTransfer::Go()
{
	AssertCurlSuccess(curl_easy_perform(curl), cancelFlag);
}

size_t BlockingTransfer::StringWriteFunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
	std::string *out = (std::string*)stream;
	size_t sz = size * nmemb;
	if (sz > 0) out->append((const char*)ptr, sz);
	return sz;
}

size_t BlockingTransfer::StreamWriteFunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
	std::ostream *out = (std::ostream*)stream;
	size_t sz = size * nmemb;
	if (sz > 0) out->write((const char*)ptr, size * nmemb);
	return sz;
}

size_t BlockingTransfer::ProgressFunc(void *cancelFlag, double, double, double, double)
{
	// Abort transfer if cancel flag is set.
	CancelFlag *cf = (CancelFlag*)cancelFlag;
	return cf->IsCanceled() ? 1 : 0;
}
