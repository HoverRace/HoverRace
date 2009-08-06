
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
			std::string s;     ///< As original string
			unsigned long ud;  ///< As packed 32-bit number
			public:
				operator std::string() const { return s; }
				operator unsigned long() const { return ud; }
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
			bool indirectClick;    ///< Get the click URL from the server instead of navigating directly.
			std::string cookie;    ///< Storage for cookie info after banner is loaded (formerly @c mLastCookie).

			friend std::istream &operator>>(std::istream &in, Banner &server);
		};

		typedef std::vector<Server*> rooms_t;
		typedef std::vector<Banner*> banners_t;

		const Server &GetScoreServer() const { return scoreServer; }
		
		const rooms_t &GetRooms() const { return rooms; }
		const Server *GetSelectedRoom() const { return selectedRoom; }
		void SetSelectedRoom(size_t index);
		
		const banners_t &GetBanners() const { return banners; }
		const bool HasBanners() const { return !banners.empty(); }
		Banner *GetCurrentBanner() const { return curBanner; }
		Banner *NextBanner();
		Banner *PeekNextBanner() const;

	private:
		Server scoreServer;
		rooms_t rooms;
		Server *selectedRoom;
		banners_t banners;
		Banner *curBanner;
		int curBannerIdx;
};
typedef boost::shared_ptr<RoomList> RoomListPtr;

std::istream &operator>>(std::istream &in, RoomList::IpAddr &ip);
std::istream &operator>>(std::istream &in, RoomList::Server &server);
std::istream &operator>>(std::istream &in, RoomList::Banner &banner);

}  // namespace Client
}  // namespace HoverRace

