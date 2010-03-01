// GrokkSoft HoverRace SourceCode License v1.0, November 29, 2008
// 
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
// - Redistributions in source code must retain the accompanying copyright notice,
//   this list of conditions, and the following disclaimer. 
// - Redistributions in binary form must reproduce the accompanying copyright
//   notice, this list of conditions, and the following disclaimer in the
//   documentation and/or other materials provided with the distribution. 
// - Names of the copyright holders (Richard Langlois and Grokksoft inc.) must not
//   be used to endorse or promote products derived from this software without
//   prior written permission from the copyright holders. 
// - This software, or its derivates must not be used for commercial activities
//   without prior written permission from the copyright holders. 
// - If any files are modified, you must cause the modified files to carry
//   prominent notices stating that you changed the files and the date of any
//   change. 
// 
// Disclaimer: 
//   The author makes no representations about the suitability of this software for
//   any purpose.  It is provided "AS IS", WITHOUT WARRANTIES OR CONDITIONS OF ANY
//   KIND, either express or implied.
// 

/***
 * Server.h
 * Declaration of Server class, the "overseer" class in the dedicated server
 *
 * @author Ryan Curtin
 */

#include <vector>

#include <boost/asio.hpp>

#include "Connection.h"
#include "TCPConnection.h"
#include "UDPConnection.h"

class Server {
	public:
		Server(boost::asio::io_service &io_service);

		void run();

	private:
		void HandleTCPAccept(TCPConnection *newConn, const boost::system::error_code &error);

		std::vector<Connection> connections;
		//std::vector<Game> games; // games we are managing

		TCPConnection *nextConnection;

		boost::asio::io_service io_service;
		boost::asio::ip::tcp::acceptor tcpAccept;
//		udp::acceptor udpAccept;
};
