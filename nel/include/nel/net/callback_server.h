/** \file callback_server.h
 * Network engine, layer 3, server
 *
 * $Id: callback_server.h,v 1.7 2001/06/13 10:22:26 lecroart Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#ifndef NL_CALLBACK_SERVER_H
#define NL_CALLBACK_SERVER_H

#include "nel/misc/types_nl.h"

#include "nel/net/callback_net_base.h"
#include "nel/net/stream_server.h"


namespace NLNET {


/**
 * Server class for layer 3
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CCallbackServer : public CCallbackNetBase, public CStreamServer
{
public:

	CCallbackServer ();

	/// Sends a message to the specified host
	void	send (const CMessage &buffer, TSockId hostid, bool log = true);

	/// Force to send all data pending in the send queue.
	bool	flush (TSockId destid) { checkThreadId (); nlassert( destid != NULL ); return CStreamServer::flush(destid); }

	/// Updates the network (call this method evenly)
	void	update (sint32 timeout=0);

	/// Sets callback for incoming connections (or NULL to disable callback)
	void	setConnectionCallback (TNetCallback cb, void *arg) { checkThreadId (); _ConnectionCallback = cb; _ConnectionCbArg = arg; }

	/// Sets callback for disconnections (or NULL to disable callback)
	void	setDisconnectionCallback (TNetCallback cb, void *arg) { checkThreadId (); CCallbackNetBase::setDisconnectionCallback (cb, arg); }

	/// Returns true if the connection is still connected. on server, we always "connected"
	bool	connected () const { checkThreadId (); return true; } 

	/// Disconnect a connection
	void	disconnect (TSockId hostid) { checkThreadId (); CStreamServer::disconnect (hostid); }

	/// Returns the address of the specified host
	const CInetAddress& hostAddress (TSockId hostid) { checkThreadId (); return CStreamServer::hostAddress (hostid); }

	virtual TSockId	getSockId (TSockId hostid = 0);

private:

	/// This function is public in the base class and put it private here because user cannot use it in layer 2
	void	send (const NLMISC::CMemStream &buffer, TSockId hostid) { nlstop; }

	bool	dataAvailable () { checkThreadId (); return CStreamServer::dataAvailable (); }
	void	receive (CMessage &buffer, TSockId *hostid);

	void	sendAllMyAssociations (TSockId to);

	TNetCallback	 _ConnectionCallback;
	void			*_ConnectionCbArg;

	friend void cbsNewConnection (TSockId from, void *data);
};


} // NLNET


#endif // NL_CALLBACK_SERVER_H

/* End of callback_server.h */
