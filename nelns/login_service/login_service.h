/** \file login_service.h
 * <File description>
 *
 * $Id: login_service.h,v 1.2 2001/05/18 16:51:01 lecroart Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX NeL Network Services.
 * NEVRAX NeL Network Services is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NeL Network Services is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_LOGIN_SERVICE_H
#define NL_LOGIN_SERVICE_H

#include "nel/misc/types_nl.h"

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/misc/log.h"

#include "nel/net/service.h"
#include "nel/net/net_manager.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

// Constantes


/// Set the version of the server. you have to increase it each time the client-server protocol changes.
/// You have to increment the client too (the server and client version must be the same)
static const uint32 ServerVersion = 1;

// 1 if you want to add new player, 0 if you want to eject new player
#define ACCEPT_NEW_USER 1

// 1 is you accept external shard (not referenced in the config file), 0 if you want to eject unknown shards
#define ACCEPT_EXTERNAL_SHARD 1

// 1 is you want to store password in crypted format, 0 for plain text
#define CRYPT_PASSWORD 1



// Structures


struct CUser
{
	// next user id for new users
	static uint32	NextUserId;

	std::string		Login;
	std::string		Password;
	uint32			Id;				// User ID, it's a unique number linked to a player for all his live

	/**  Different states of a user
	 * Offline		: default state, not here at all
	 * Authorized	: the user gave a good login password and the LS waits the user shard selection
	 * Awaiting		: the user gave the shard, the LS wait the effective connection answer from WS
	 * Online		: the user effectively connected to the shard
	 */
	enum TState { Offline, Authorized, Awaiting, Online };

	TState			State;

	NLNET::TSockId	SockId;			// Connection to the user

	CLoginCookie	Cookie;			// the cookie for the current authentication

	TSockId			ShardId;		// connection to the WS where the user is
	

	CUser (string login) : Login(login), State(Offline), SockId(NULL), ShardId(NULL), Loaded(false) { }
	CUser (string login, string password) : Login(login), Password(password), Id(NextUserId++), State(Offline), SockId(NULL), ShardId(NULL), Loaded(true) { }
	CUser (string login, string password, uint32 id) : Login(login), Password(password), Id(id), State(Offline), SockId(NULL), ShardId(NULL), Loaded(true)
	{
		if (id >= CUser::NextUserId)
			CUser::NextUserId = id + 1;
	}

	bool Authorize (TSockId sender, CCallbackNetBase &netbase);

	// used when reloading config file
	bool Loaded;
};

struct CShard
{
	string			Name;
	string			WSAddr;
	bool			Online;
	TSockId			SockId;		// Connection of to the shard
	sint			NbPlayers;

	CShard (string addr, string name) : Name(name), WSAddr(addr), NbPlayers(0), Loaded(true), Online(false), SockId(NULL) { }
	CShard (const CInetAddress &ia) : NbPlayers(0), Loaded(true), Online(false), SockId(NULL)
	{
		Name = "External Shard '"+ia.hostName()+"'";
		WSAddr = ia.ipAddress();
	}

	// used when reloading config file
	bool Loaded;
};




// Variables

extern CFileDisplayer	Fd;
extern CLog				Output;
extern vector<CUser>	Users;
extern vector<CShard>	Shards;

// Functions

void readPlayerDatabase ();
void writePlayerDatabase ();
void displayShards ();
void displayUsers ();
void disconnectClient (CUser &user, bool disconnectClient, bool disconnectShard);
sint findUser (uint32 Id);


#endif // NL_LOGIN_SERVICE_H

/* End of login_service.h */
