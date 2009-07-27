
// BaseTransfer.h
// Base class of standard transfers.
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

#include "../Util/Config.h"

#include "BaseTransfer.h"

using namespace HoverRace::Net;
using HoverRace::Util::Config;

/**
 * Constructor.
 * @param agent The transfer agent (request parameters).
 */
BaseTransfer::BaseTransfer(const Agent &agent) :
	curl(curl_easy_init())
{
	const Config *cfg = Config::GetInstance();

	errorBuf[0] = '\0';
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuf);

	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);

	curl_easy_setopt(curl, CURLOPT_USERAGENT, cfg->GetUserAgentId().c_str());

	curl_easy_setopt(curl, CURLOPT_URL, agent.GetUrl().c_str());
}

BaseTransfer::~BaseTransfer()
{
	curl_easy_cleanup(curl);
}

void BaseTransfer::AssertCurlSuccess(CURLcode code, CancelFlagPtr cancelFlag)
{
	// One last check of the cancel flag.
	if (cancelFlag != NULL && cancelFlag->IsCanceled()) {
		code = CURLE_ABORTED_BY_CALLBACK;
	}
	switch (code) {
		case 0: break;
		case CURLE_ABORTED_BY_CALLBACK: throw CanceledExn();
		default: throw NetExn(errorBuf);
	}
}
