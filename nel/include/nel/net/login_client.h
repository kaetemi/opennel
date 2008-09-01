/** \file login_client.h
 * CLoginClient is the interface used by the client to identifies itself to the login_sytem and
 * connects to the shard.
 *
 * $Id$
 * 
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

#ifndef NL_LOGIN_CLIENT_H
#define NL_LOGIN_CLIENT_H

#include "nel/misc/types_nl.h"

#include <string>
#include <vector>

#include "callback_client.h"
#include "udp_sim_sock.h"

namespace NLNET
{

class CLoginCookie;
class CUdpSock;
class IDisplayer;

/**
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2002
 */
class CLoginClient {
public:

	struct CShardEntry
	{
		CShardEntry() { NbPlayers = 0; Id = -1; }
		CShardEntry(const ucstring &name, uint8 nbp, sint32 sid) : Id(sid), Name(name), NbPlayers(nbp) { }
		sint32		Id;
		ucstring	Name;
		uint8		NbPlayers;
	};

	typedef std::vector<CShardEntry> TShardList;

	// This list is filled after a successful authenticate call
	static TShardList ShardList;

	/** Try to login with login and password. cpassword must be md5 crypted (that's why it's a string and not an ucstring)
	* application is the name of the application. the LS will return all shards that is available for this application (sample, snowballs, ...)
	* If the authentication is ok, the function return an empty string else it returns the reason of the failure.
	*/
	static std::string authenticate(const std::string &loginServiceAddr, const ucstring &login, const std::string &cpassword, const std::string &application);
	static std::string authenticateBegin(const std::string &loginServiceAddr, const ucstring &login, const std::string &cpassword, const std::string &application);
	static bool authenticateUpdate(std::string &error);
	/** Todo: fix comment.
	*/
	static std::string wantToConnectToShard (sint32 shardId, std::string &ip, std::string &cookie);
	static std::string selectShardBegin(sint32 shardId);
	static bool selectShardUpdate(std::string &error, std::string &ip, std::string &cookie);

	/** Try to connect to the shard and return a TCP connection to the shard.
	 */
	static std::string connectToShard (CLoginCookie &lc, const std::string &addr, CCallbackClient &cnx);

	/** Try to connect to the shard and return an UDP connection to the shard.
	 */
	static std::string connectToShard (const std::string &addr, CUdpSock &cnx);

	/** Try to connect to the shard and return an UDP simulate connection to the shard.
	 */
	static std::string connectToShard (const std::string &addr, CUdpSimSock &cnx);

private:

	static std::string confirmConnection (sint32 sharId);
	static CLoginClient::CShardEntry *getShard (sint32 shardId);

	static NLNET::CCallbackClient *_LSCallbackClient;

};


} // NLNET

#endif // NL_LOGIN_CLIENT_H

/* End of login_client.h */

/* Merge OpenNeL SVN
 */
