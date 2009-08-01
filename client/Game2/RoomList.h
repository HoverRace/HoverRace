
// RoomList.h
// Header for the server room list.
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

#pragma once

#include "../../engine/Net/CancelFlag.h"

namespace HoverRace {
namespace Client {

class RoomList
{
	public:
		RoomList();
		~RoomList();

	public:
		void LoadFromUrl(const std::string &url,
			Net::CancelFlagPtr cancelFlag=Net::CancelFlagPtr());
		void LoadFromStream(std::istream &in);

	public:
		class IpAddr
		{
			unsigned int ud;
			public:
				operator unsigned int() { return ud; }
				friend std::istream &operator>>(std::istream &in, IpAddr &ip);
		};
		struct Server
		{
			std::string name;  ///< Server name
			IpAddr addr;       ///< IPv4 packed address
			int port;
			std::string path;  ///< Request URI (formerly @c mURL)

			friend std::istream &operator>>(std::istream &in, Server &server);
		};
		struct Banner : public Server
		{
			int delay;             ///< Delay in seconds (formerly @c mDelay)
			std::string clickUrl;  ///< Banner click URL
			bool indirectClick;    ///< Load location first (formerly @c mIndirectClick)
			std::string cookie;    ///< Storage for cookie info after banner is loaded (formerly @c mLastCookie).

			friend std::istream &operator>>(std::istream &in, Banner &server);
		};

	private:
		Server scoreServer;
		typedef std::vector<Server*> rooms_t;
		rooms_t rooms;
		typedef std::vector<Banner*> banners_t;
		banners_t banners;
};
typedef boost::shared_ptr<RoomList> RoomListPtr;

std::istream &operator>>(std::istream &in, RoomList::IpAddr &ip);
std::istream &operator>>(std::istream &in, RoomList::Server &server);
std::istream &operator>>(std::istream &in, RoomList::Banner &banner);

}  // namespace Client
}  // namespace HoverRace

