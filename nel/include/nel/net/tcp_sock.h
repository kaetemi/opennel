/** \file tcp_sock.h
 * Network engine, layer 0, tcp socket
 *
 * $Id: tcp_sock.h,v 1.5 2005/02/22 10:14:13 besson Exp $
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

#ifndef NL_TCP_SOCK_H
#define NL_TCP_SOCK_H

#include "sock.h"


namespace NLNET {


/**
 * CTcpSock: Reliable socket via TCP.
 * See base class CSock.
 * 
 * When to set No Delay mode on ?
 * Set TCP_NODELAY (call setNoDelay(true)) *only* if you have to send small buffers that need to
 * be sent *immediately*. It should only be set for applications that send frequent small bursts
 * of information without getting an immediate response, where timely delivery of data is
 * required (the canonical example is mouse movements). Setting TCP_NODELAY on increases
 * the network traffic (more overhead).
 * In the normal behavior of CSock, TCP_NODELAY is off i.e. the Nagle buffering algorithm is enabled.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000-2001
 */
class CTcpSock : public CSock
{
public:

	/// @name Socket setup
	//@{

	/**
	 * Constructor.
	 * \param logging Disable logging if the server socket object is used by the logging system, to avoid infinite recursion
	 */
	CTcpSock( bool logging = true );

	/// Construct a CTcpSock object using an already connected socket descriptor and its associated remote address
	CTcpSock( SOCKET sock, const CInetAddress& remoteaddr );

	/** Connection. You can reconnect a socket after being disconnected.
	 * This method does not return a boolean, otherwise a programmer could ignore the result and no
	 * exception would be thrown if connection fails :
	 * - If addr is not valid, an exception ESocket is thrown
	 * - If connect() fails for another reason, an exception ESocketConnectionFailed is thrown
	 */
	virtual void		connect( const CInetAddress& addr );

	/** Sets a custom TCP Window size (SO_RCVBUF and SO_SNDBUF).
	 * You must close the socket is necessary, before calling this method.
	 *
	 * See http://www.ncsa.uiuc.edu/People/vwelch/net_perf/tcp_windows.html
	 */
	void				connectWithCustomWindowSize( const CInetAddress& addr, int windowsize );

	/// Returns the TCP Window Size for the current socket
	uint32				getWindowSize();

	/** Sets/unsets TCP_NODELAY (by default, it is off, i.e. the Nagle buffering algorithm is enabled).
	 * You must call this method *after* connect().
	 */
	virtual void		setNoDelay( bool value );

	/// Active disconnection for download way only (partial shutdown)
	void				shutdownReceiving();

	/// Active disconnection for upload way only (partial shutdown)
	void				shutdownSending();

	/// Active disconnection (shutdown) (mutexed). connected() becomes false.
	virtual void		disconnect();


	//@}

};


} // NLNET


#endif // NL_TCP_SOCK_H

/* End of tcp_sock.h */
