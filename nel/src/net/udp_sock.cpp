/** \file udp_sock.cpp
 * Network engine, layer 0, udp socket
 *
 * $Id: udp_sock.cpp,v 1.9 2001/12/28 10:17:21 lecroart Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "stdnet.h"

#include "nel/net/udp_sock.h"

#ifdef NL_OS_WINDOWS
#include <winsock2.h>
#define socklen_t int
#define ERROR_NUM WSAGetLastError()

#elif defined NL_OS_UNIX
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
//#include <fcntl.h>
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define ERROR_NUM errno
#define ERROR_MSG strerror(errno)
typedef int SOCKET;

#endif

using namespace NLMISC;

namespace NLNET {


/*
 * Constructor
 */
CUdpSock::CUdpSock( bool logging ) :
	CSock( logging ),
	_Bound( false )
{
	// Socket creation
	createSocket( SOCK_DGRAM, IPPROTO_UDP );
}


/** Binds the socket to the specified port. Call bind() for an unreliable socket if the host acts as a server and waits for
 * messages. If the host acts as a client, call sendTo(), there is no need to bind the socket.
 */
void CUdpSock::bind( uint16 port )
{
	CInetAddress addr; // any IP address
	addr.setPort( port );
	bind( addr );
	setLocalAddress(); // will not set the address if the host is multihomed, use bind(CInetAddress) instead
}


/*
 * Same as bind(uint16) but binds on a specified address/port (useful when the host has several addresses)
 */
void CUdpSock::bind( const CInetAddress& addr )
{
#ifndef NL_OS_WINDOWS
	// Set Reuse Address On (does not work on Win98 and is useless on Win2000)
	int value = true;
	if ( setsockopt( _Sock, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value) ) == SOCKET_ERROR )
	{
		throw ESocket( "ReuseAddr failed" );
	}
#endif

	_LocalAddr = addr;

	// Bind the socket
	if ( ::bind( _Sock, (sockaddr*)(_LocalAddr.sockAddr()), sizeof(sockaddr) ) == SOCKET_ERROR )
	{
		throw ESocket( "Bind failed" );
	}
	_Bound = true;
	if ( _Logging )
	{
		nldebug( "LNETL0: Socket %d bound at %s", _Sock, _LocalAddr.asString().c_str() );
	}
}


/*
 * Sends a message
 */
void CUdpSock::sendTo( const uint8 *buffer, uint len, const CInetAddress& addr )
{
	
	//  Send
	if ( ::sendto( _Sock, (const char*)buffer, len, 0, (sockaddr*)(addr.sockAddr()), sizeof(sockaddr) ) != (sint32)len )
	{
		throw ESocket( "Unable to send datagram" );
	}
	_BytesSent += len;

	if ( _Logging )
	{
		nldebug( "LNETL0: Socket %d sent %d bytes to %s", _Sock, len, addr.asString().c_str() );
	}

	// If socket is unbound, retrieve local address
	if ( ! _Bound )
	{
		setLocalAddress();
		_Bound = true;
	}

#ifdef NL_OS_WINDOWS
	// temporary by ace to know size of SO_MAX_MSG_SIZE
	static bool first = true;
	if (first)
	{
		uint MMS, SB;
		int  size = sizeof (MMS);
		getsockopt (_Sock, SOL_SOCKET, SO_SNDBUF, (char *)&SB, &size);
		getsockopt (_Sock, SOL_SOCKET, SO_MAX_MSG_SIZE, (char *)&MMS, &size);
		nlinfo ("the udp SO_MAX_MSG_SIZE=%u, SO_SNDBUF=%u", MMS, SB);
		first = false;
	}
#endif
}


/*
 * Receives data from the peer. (blocking function)
 */
void CUdpSock::receive( uint8 *buffer, uint32& len )
{
	nlassert( _Connected && (buffer!=NULL) );

	// Receive incoming message
	len = ::recv( _Sock, (char*)buffer, len , 0 );

	// Check for errors (after setting the address)
	if ( len == SOCKET_ERROR )
	{
		throw ESocket( "Cannot receive data" );
	}

	_BytesReceived += len;
	if ( _Logging )
	{
		nldebug( "LNETL0: Socket %d received %d bytes from peer %s", _Sock, len, _RemoteAddr.asString().c_str() );
	}
}


/*
 * Receives data and say who the sender is. (blocking function)
 */
void CUdpSock::receivedFrom( uint8 *buffer, uint& len, CInetAddress& addr )
{
	// Receive incoming message
	sockaddr_in saddr;
	socklen_t saddrlen = sizeof(saddr);

	len = ::recvfrom( _Sock, (char*)buffer, len , 0, (sockaddr*)&saddr, &saddrlen );

	// Get sender's address
	addr.setSockAddr( &saddr );

	// Check for errors (after setting the address)
	if ( len == SOCKET_ERROR )
	{
		throw ESocket( "Cannot receive data" );
	}

	_BytesReceived += len;
	if ( _Logging )
	{
		nldebug( "LNETL0: Socket %d received %d bytes from %s", _Sock, len, addr.asString().c_str() );
	}
}


} // NLNET
