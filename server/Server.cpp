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
 * Server.cpp
 *
 * Implementation of the dedicated server.
 */
#include "Server.h"

#include <boost/bind.hpp>

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

Server::Server(io_service &io_service, short tcpListenPort, short udpListenPort)
{
	// TODO: make listen ports specifiable
	this->io_service = io_service;
	acceptor(io_service, tcp::endpoint(tcp::v4(), tcpListenPort));

	// this is our connectionless UDP socket, which will get passed to all UDPConnection
	// objects
	udpSocket = udp::socket(io_service, udp::endpoint(udp::v4(), udpListenPort));

	// "blank" connection that gets written to when someone tries to connect, and
	// then we deal with it
	tcpConnection = new TCPConnection(io_service);
	udpSender = udp::endpoint;

	// set up handler for a new received TCP connection
	tcpAcceptor.async_accept(tcpConnection->getSocket(),
		boost::bind(&Server::HandleTCPAccept, this, tcpConnection, placeholders::error));

	// and a handler for a new received UDP connection
	udpSocket->getSocket().async_receive_from(buffer(udpSocket->getBuffer(),
		udpSocket->getMaxBufferLength()), udpSender, boost::bind(&Server::HandleUDPConnect, this,
			placeholders::error, placeholders::bytes_transferred));
}

Server::HandleTCPAccept(TCPConnection *connection, const boost::system::error_code &error)
{
	if(!error) {
		
	} else {
		// HOLY SHIT WHAT DO WE DO CAPTAIN?
	}
}

Server::HandleUDPConnect(UDPConnection *connection, const boost::system::error_code &error)
{
	if(!error) {

	} else {
		// ???
	}
}
