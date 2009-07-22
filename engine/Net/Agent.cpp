
// Agent.cpp
// Network transfer agent (wrapper for libcurl).
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

#include <curl/curl.h>

#include "../Util/Config.h"

#include "BlockingTransfer.h"

#include "Agent.h"

using namespace HoverRace::Net;

using HoverRace::Util::Config;

/**
 * Constructor.
 * @param The URL to retrieve.
 */
Agent::Agent(const std::string &url) :
	url(url)
{
}

Agent::~Agent()
{
}

void Agent::SetUrl(const std::string &url)
{
	this->url = url;
}

/**
 * Easy URL retrieval to a string.
 * This method blocks until the transfer is complete.
 * @param buf The string to store to.
 * @param cancelFlag Optional control to check for cancellation (may be @c NULL).
 */
void Agent::Get(std::string &buf, CancelFlagPtr cancelFlag)
{
	BlockingTransfer(*this, buf, cancelFlag).Go();
}

/**
 * Easy URL retrieval to a stream.
 * This method blocks until the transfer is complete.
 * @param buf The stream to write to.
 * @param cancelFlag Optional control to check for cancellation (may be @c NULL).
 */
void Agent::Get(std::ostream &buf, CancelFlagPtr cancelFlag)
{
	BlockingTransfer(*this, buf, cancelFlag).Go();
}
