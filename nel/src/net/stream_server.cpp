/** \file stream_server.cpp
 * Network engine, layer 2, server
 *
 * $Id: stream_server.cpp,v 1.4 2001/12/10 14:34:31 lecroart Exp $
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

#include "nel/misc/debug.h"
#include "nel/misc/mem_stream.h"

#include "nel/net/stream_server.h"

using namespace NLMISC;

namespace NLNET {


/*
 * Send a message to the specified host
 */
void CStreamServer::send (const CMemStream &buffer, TSockId hostid)
{
	nlassert (buffer.length() != 0);  // no size limit anymore

	nldebug ("LNETL2S: send()");

	CBufServer::send (buffer.bufferAsVector(), hostid);
}


void CStreamServer::receive (NLMISC::CMemStream &buffer, TSockId *hostid)
{
	nldebug ("LNETL2S: receive()");

	CBufServer::receive (buffer.bufferAsVector (), hostid);
	buffer.resetBufPos ();
}

} // NLNET
