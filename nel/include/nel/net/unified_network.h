/** \file unified_network.h
 * Network engine, layer 5
 *
 * $Id: unified_network.h,v 1.1 2001/10/16 09:23:10 legros Exp $
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

#ifndef NL_UNIFIED_NETWORD_H
#define NL_UNIFIED_NETWORD_H

#include <hash_map>

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/net/callback_client.h"
#include "nel/net/callback_server.h"
#include "nel/net/naming_client.h"
#include "nel/misc/mutex.h"
#include "nel/misc/reader_writer.h"

namespace NLNET {

/** Callback function type for service up/down processing
 * \param serviceName name of the service that is un/registered to the naming service
 * \param arg a pointer initialized by the user
 */
typedef void (*TUnifiedNetCallback) (const std::string &serviceName, uint16 sid, void *arg);

/** Callback function type for message processing
 * \param msgin message received
 * \param serviceName name of the service that sent the message
 * \param sid id of the service that sent the message
 */
typedef void (*TUnifiedMsgCallback) (CMessage &msgin, const std::string &serviceName, uint16 sid);

/// Callback items. See CMsgSocket::update() for an explanation on how the callbacks are called.
struct TUnifiedCallbackItem
{
	/// Key C string. It is a message type name, or "C" for connection or "D" for disconnection
	char				*Key;
	/// The callback function
	TUnifiedMsgCallback	Callback;

};

/**
 * Layer 5
 *
 * \author Vianney Lecroart, Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CUnifiedNetwork
{
	friend void	cbConnection(TSockId from, void *arg);
	friend void	cbDisconnection(TSockId from, void *arg);
	friend void	cbServiceIdentification(CMessage &msgin, TSockId from, CCallbackNetBase &netbase);
	friend void	cbMsgProcessing(CMessage &msgin, TSockId from, CCallbackNetBase &netbase);

public:

	/** Returns the singleton instance of the CNetManager class.
	 */
	static CUnifiedNetwork *getInstance ();

	/** Creates the connection to the Naming Service.
	 * If the connection failed, ESocketConnectionFailed exception is generated.
	 * This function is called automatically called by the service class at the beginning.
	 *
	 * \param addr address of the naming service (NULL is you don't want to use the naming service)
	 * \param rec recorging state to know if we have to record or replay messages
	 */
	void	init (const CInetAddress *addr, CCallbackNetBase::TRecordingState rec, const std::string &shortName, uint16 port, TServiceId &sid );

	/** Registers to the Naming Service, and connects to the present services
	 */
	void	connect();

	/** Closes the connection to the naming service, every other connection and free.
	 */
	void	release ();

	/** Adds a specific service to the list of connected services.
	 */
	void	addService(const std::string &name, const CInetAddress &addr, bool sendId = true, bool external = true, uint16 sid=0, bool autoRetry = true);

	/** Adds a callback array in the system. You can add callback only *after* adding the server, the client or the group.
	 */
	void	addCallbackArray (const TUnifiedCallbackItem *callbackarray, NLMISC::CStringIdArray::TStringId arraysize);

	/** Call it evenly. the parameter select the timeout value in milliseconds for each update. You are absolutely certain that this
	 * function will not be returns before this amount of time you set.
	 * If you set the update timeout value higher than 0, all messages in queues will be process until the time is greater than the timeout user update().
	 * If you set the update timeout value to 0, all messages in queues will be process one time before calling the user update(). In this case, we don't nlSleep(1).
	 */
	void	update (sint32 timeout = 0);

	/** Sends a message to a specific serviceName. If there's more than one service with this name, all services of this name will receive the message.
	 * \param serviceName name of the service you want to send the message (may not be unique.)
	 * \param the message you want to send.
	 */
	void	send (const std::string &serviceName, const CMessage &msg);

	/** Sends a message to a specific serviceId.
	 * \param serviceId Id of the service you want to send the message.
	 * \param the message you want to send.
	 */
	void	send (uint16 serviceId, const CMessage &msg);

	/** Broadcasts a message to all connected services.
	 * \param the message you want to send.
	 */
	void	send (const CMessage &msg);


	/** Sets callback for incoming connections (or NULL to disable callback).
	 * On a client, the callback will be call when the connection to the server is established (the first connection or after the server shutdown and started)
	 * On a server, the callback is called each time a new client is connected to him
	 */
	void	setServiceUpCallback (const std::string &serviceName, TUnifiedNetCallback cb, void *arg);

	/** Sets callback for disconnections (or NULL to disable callback).
	 * On a client, the callback will be call each time the connection to the server is lost.
	 * On a server, the callback is called each time a client is disconnected.
	 */
	void	setServiceDownCallback (const std::string &serviceName, TUnifiedNetCallback cb, void *arg);


	/// Gets the CCallbackNetBase of the service
	CCallbackNetBase	*getNetBase(const std::string &name, TSockId &host);

	/// Gets the total number of bytes sent
	uint64				getBytesSent ();

	/// Gets the total number of bytes received
	uint64				getBytesReceived ();

	/// Gets the total number of bytes queued for sending
	uint64				getSendQueueSize ();

	/// Gets the total number of bytes queued after receiving
	uint64				getReceiveQueueSize ();

private:

	/// A map of service ids, referred by a service name
	struct TNameMappedConnection : public std::hash_multimap<std::string, uint16> {};

	/// A map of callbacks, reffered by message name
	typedef std::map<std::string, TUnifiedMsgCallback>			TMsgMappedCallback;


	/// A callback and its user data
	typedef std::pair<TUnifiedNetCallback, void *>				TCallbackArgItem;
	/// A Map of service up/down callbacks with their user data.
	typedef std::hash_multimap<std::string, TCallbackArgItem>	TNameMappedCallback;



	/// This may contains a CCallbackClient or a TSockId, depending on which type of connection it is.
	class CUnifiedConnection
	{
	public:
		/// The name of the service (may not be unique)
		std::string				ServiceName;
		/// The id of the service (is unique)
		uint16					ServiceId;
		/// If the current service is connect to the other service as a server or a client
		bool					IsServerConnection;
		/// If the service entry is used
		bool					EntryUsed;
		/// The connection state
		bool					IsConnected;
		/// If the connection is extern to the naming service
		bool					IsExternal;
		/// Auto-retry mode
		bool					AutoRetry;
		/// The external connection address
		CInetAddress			ExtAddress;
		/// The connection
		union TConnection
		{
			CCallbackClient		*CbClient;
			TSockId				HostId;
		};
		TConnection				Connection;

		CUnifiedConnection() : EntryUsed(false) {}

		CUnifiedConnection(const std::string &name, uint16 id, CCallbackClient *cb) : ServiceName(name), ServiceId(id), IsServerConnection(false), EntryUsed(true), IsConnected(false), IsExternal(false) { Connection.CbClient = cb; }

		CUnifiedConnection(const std::string &name, uint16 id, TSockId host) : ServiceName(name), ServiceId(id), IsServerConnection(true), EntryUsed(true), IsConnected(false), IsExternal(false) { Connection.HostId = host; }
	};

	class CConnectionId
	{
	public:
		string		SName;
		uint16		SId;
		TSockId		SHost;

		CConnectionId() {}
		CConnectionId(const std::string &name, uint16 sid, TSockId hid = 0) : SName(name), SId(sid), SHost(hid) {}
	};

	//

	/// Vector of connections by service id
	NLMISC::CRWSynchronized< std::vector<CUnifiedConnection> >	_IdCnx;

	/// Map of connections by service name
	NLMISC::CRWSynchronized<TNameMappedConnection>				_NamedCnx;

	/// The callback server
	CCallbackServer												_CbServer;

	/// The server port
	uint16														_ServerPort;

	/// Map of the up/down service callbacks
	TNameMappedCallback											_UpCallbacks;
	TNameMappedCallback											_DownCallbacks;

	/// Recording state
	CCallbackNetBase::TRecordingState							_RecordingState;

	/// Service id of the running service
	TServiceId													_SId;

	/// Service name
	std::string													_Name;

	/// Map of callbacks
	TMsgMappedCallback											_Callbacks;

	/// Used for external service
	uint16														_ExtSId;

	/// Last time of retry
	NLMISC::TTime												_LastRetry;


	/// internal stacks used for adding/removing connections in multithread
	std::vector<CConnectionId>									_ConnectionStack;
	std::vector<uint16>											_DisconnectionStack;
	std::vector<uint16>											_ConnectionRetriesStack;


	/// The main instance
	static CUnifiedNetwork										*_Instance;


	//
	CUnifiedNetwork() : _ExtSId(256), _LastRetry(0) { }
	~CUnifiedNetwork() { }

	//
	void	updateConnectionTable();
};


} // NLNET


#endif // NL_UNIFIED_NETWORK_H

/* End of unified_network.h */
