/** \file login_service.cpp
 * Login Service (LS)
 *
 * $Id: connection_client.cpp,v 1.5 2001/09/20 08:54:47 lecroart Exp $
 *
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NeL Network Services.
 * NEVRAX NeL Network Services is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX NeL Network Services is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <vector>
#include <map>

#include "nel/misc/types_nl.h"

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/misc/log.h"

#include "nel/net/service.h"
#include "nel/net/net_manager.h"
#include "nel/net/login_cookie.h"
#include "login_service.h"

#ifdef NL_OS_UNIX
extern "C" char *crypt (const char *__key, const char *__salt);
#endif


using namespace std;
using namespace NLMISC;
using namespace NLNET;

// These functions enable crypting password, work only on unix

const uint32 EncryptedSize = 13;

// Get a number between 0 and 64, used by cryptPassword
static uint32 rand64 ()
{
	return (uint32) floor(64.0*(double)rand()/((double)RAND_MAX+1.0));
}

// Crypt a password
string cryptPassword (const string &password)
{
#if defined(NL_OS_UNIX) && CRYPT_PASSWORD
	char Salt[3];
	static char SaltString[65] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

	Salt[0] = SaltString[rand64()];
	Salt[1] = SaltString[rand64()];
	Salt[2] = '\0';

	return string (crypt (password.c_str(), Salt));
#else
	return password;
#endif
}

// Check if a password is valid
bool checkPassword (const string &password, const string &encrypted)
{
#if defined(NL_OS_UNIX) && CRYPT_PASSWORD
	char Salt[3];

	if (encrypted.size() != EncryptedSize)
	{
		nlwarning ("checkPassword(): \"%s\" is not a valid encrypted password", encrypted.c_str());
		return false;
	}

	Salt[0] = encrypted[0];
	Salt[1] = encrypted[1];
	Salt[2] = '\0';

	return encrypted == crypt (password.c_str(), Salt);
#else
	return encrypted == password;
#endif
}

sint findUser (string &login)
{
	for (sint i = 0; i < (sint) Users.size (); i++)
	{
		if (Users[i].Login == login)
		{
			return i;
		}
	}
	// user not found
	return -1;
}

void addUser (string &login, string &password)
{
	if (findUser (login) == -1)
	{
		Users.push_back (CUser (login, cryptPassword(password)));
		writePlayerDatabase ();
	}
	else
	{
		nlwarning ("user '%s' already exists in the base", login.c_str ());
	}
}

sint userToLog(sint userPos)
{
	if (userPos == -1) return userPos;
	else return Users[userPos].Id;
}


bool stringIsStandard(const string &str)
{
	for (sint i = 0; i < (sint) str.size(); i++)
	{
		if (!isalnum (str[i])) return false;
	}
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
/////////////// CONNECTION TO THE CLIENTS //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Callback for service unregistration.
 *
 * Message expected :
 * - nothing
 */
static void cbClientVerifyLoginPassword (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// reason is empty if everything goes right or contains the reason of the failure
	string reason = "";

	//
	// S03: check the validity of the client login/password and send "VLP" message to client
	//

	string Login, Password;
	msgin.serial (Login);
	msgin.serial (Password);

	sint userPos = findUser (Login);
	const CInetAddress &ia = netbase.hostAddress (from);

	Output.displayNL ("***: %3d Login '%s' Ip '%s'", userToLog(userPos), Login.c_str(), ia.asString().c_str());

	// recv the client version and check it

	uint32 ClientVersion;
	msgin.serial (ClientVersion);
	if (ClientVersion < ServerVersion || ClientVersion > ServerVersion)
	{
		// reject the use, bad version
		if (ClientVersion < ServerVersion)
			reason = "Your client is out of date. You have to download the last version.";
		else
			reason = "Your client is too new compare to the server. You have to get an older version of the client.";
		Output.displayNL ("---: %3d Bad Version, ClientVersion: %d ServerVersion: %d", userToLog(userPos), ClientVersion, ServerVersion);
	}

	// recv client hardware info

	string OS, Proc, Mem, Gfx;

	msgin.serial (OS);
	msgin.serial (Proc);
	msgin.serial (Mem);
	msgin.serial (Gfx);

	Output.displayNL ("OS : %3d %s", userToLog(userPos), OS.c_str());
	Output.displayNL ("PRC: %3d %s", userToLog(userPos), Proc.c_str());
	Output.displayNL ("MEM: %3d %s", userToLog(userPos), Mem.c_str());
	Output.displayNL ("GFX: %3d %s", userToLog(userPos), Gfx.c_str());

	// check the login & pass

	if (reason.empty() && !stringIsStandard (Login))
	{
		// reject the new user, bad login format
		reason = "Bad login format, only alphanumeric character";
		Output.displayNL ("---: %3d Bad Login Format", userToLog(userPos));
	}

	if (reason.empty() && !stringIsStandard (Password))
	{
		// reject the new user, bad password format
		reason = "Bad password format, only alphanumeric character";
		Output.displayNL ("---: %3d Bad Password Format", userToLog(userPos));
	}

	if (reason.empty())
	{
		if (userPos == -1)
		{
			// unknown user
#if ACCEPT_NEW_USER
			// add the new user
			addUser (Login, Password);
			// take the new user entry
			userPos = findUser (Login);
			Output.displayNL ("---: %3d New User (new id:%d)", -1, userToLog(userPos));
#else
			// reject the new user
			reason = "Bad login";
			Output.displayNL ("---: %3d Bad Login", userToLog(userPos));
#endif
		}
		else
		{
			if (!checkPassword (Password, Users[userPos].Password))
			{
				// error reason
				reason = "Bad password";
				Output.displayNL ("---: %3d Bad Password", userToLog(userPos));
			}
			else
			{
				Output.displayNL ("---: %3d Ok", userToLog(userPos));
			}
		}
	}

	if (reason.empty())
	{
		reason = Users[userPos].Authorize (from, netbase);
	}

	uint32 nbshard = 0;
	if (reason.empty())
	{
		// count online shards
		for (uint i = 0; i < Shards.size (); i++)
		{
			if (Shards[i].Online)
			{
				nbshard++;
			}
		}
		if (nbshard==0)
		{
			reason = "No shards available";
		}

	}

	CMessage msgout (netbase.getSIDA (), "VLP");

	if (reason.empty())
	{
		uint8 ok = 1;
		msgout.serial (ok);

		// send number of online shard
		msgout.serial (nbshard);

		// send address and name of all online shards
		for (uint i = 0; i < Shards.size (); i++)
		{
			if (Shards[i].Online)
			{
				// serial the name of the shard
				string shardname;
				shardname = Shards[i].Name;

				if (Shards[i].NbPlayers == 0)
				{
					shardname += " (no users)";
				}
				else
				{
					char num[1024];
					smprintf(num, 1024, "%d", Shards[i].NbPlayers);
					shardname += " (";
					shardname += num;
					if (Shards[i].NbPlayers == 1)
						shardname += " user)";
					else
						shardname += " users)";
				}
				msgout.serial (shardname);
				
				// serial the address of the WS service
				msgout.serial (Shards[i].WSAddr);
			}
		}
		netbase.send (msgout, from);

		netbase.authorizeOnly ("CS", from);
	}
	else
	{
		// put the error message
		uint8 ok = 0;
		msgout.serial (ok);
		msgout.serial (reason);
		netbase.send (msgout, from);
// FIX: On linux, when we disconnect now, sometime the other side doesnt receive the message sent just before.
//      So it's the other side to disconnect
//		netbase.disconnect (from);
	}
}

static void cbClientChooseShard (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// S06: receive "CS" message from client
	//

	// first find if the user is authorized
	for (vector<CUser>::iterator it = Users.begin (); it != Users.end (); it++)
	{
		if ((*it).Authorized && (*it).SockId == from)
		{
			// it's ok, so we found the wanted shard
			string WSAddr;
			msgin.serial (WSAddr);

			for (sint32 i = 0; i < (sint32) Shards.size (); i++)
			{
				if (Shards[i].Online && Shards[i].WSAddr == WSAddr)
				{
					CMessage msgout (CNetManager::getNetBase("WSLS")->getSIDA (), "CS");
					const CInetAddress &ia = netbase.hostAddress ((*it).SockId);
					msgout.serial ((*it).Cookie);
					CNetManager::send("WSLS", msgout, Shards[i].SockId);
					return;
				}
			}
			// the shard is not available, denied the user
			nlwarning("User try to choose a shard without authorization");

			CMessage msgout (netbase.getSIDA (), "SCS");
			uint8 ok = false;
			string reason = "Selected shard is not available";
			msgout.serial (ok);
			msgout.serial (reason);
			netbase.send (msgout, from);
// FIX: On linux, when we disconnect now, sometime the other side doesnt receive the message sent just before.
//      So it's the other side to disconnect
//			netbase.disconnect (from);
			return;
		}
	}

	// the user isn t authorized
	nlwarning("User try to choose a shard without authorization");
	// disconnect him
	netbase.disconnect (from);
}

static void cbClientConnection (const string &serviceName, TSockId from, void *arg)
{
	CCallbackNetBase *cnb = CNetManager::getNetBase("LS");
	const CInetAddress &ia = cnb->hostAddress (from);

	nldebug("new client connection: %s", ia.asString ().c_str ());

	cnb->authorizeOnly ("VLP", from);
}

static void cbClientDisconnection (const string &serviceName, TSockId from, void *arg)
{
	CCallbackNetBase *cnb = CNetManager::getNetBase("LS");
	const CInetAddress &ia = cnb->hostAddress (from);

	nldebug("new client disconnection: %s", ia.asString ().c_str ());

	// remove the user if necessary
	for (vector<CUser>::iterator it = Users.begin (); it != Users.end (); it++)
	{
		if ((*it).SockId == from)
		{
			// remove the authorized user because he's not here anymore
		  // bug? the second param was NULL??? really?
		        disconnectClient (*it, false, false);
			return;
		}
	}
}


const TCallbackItem ClientCallbackArray[] =
{
	{ "VLP", cbClientVerifyLoginPassword },
	{ "CS", cbClientChooseShard },
};

// if you add callback in the client side, don't forget to add it here!!!
static const char *OtherSideAssociations[] =
{
	"AA",
	"RA",
	"RAA",
	"VLP",
	"SCS",
};

void connectionClientInit ()
{
	CNetManager::addCallbackArray ("LS", ClientCallbackArray, sizeof(ClientCallbackArray)/sizeof(ClientCallbackArray[0]));
	CNetManager::setConnectionCallback ("LS", cbClientConnection, NULL);
	CNetManager::setDisconnectionCallback ("LS", cbClientDisconnection, NULL);
	CNetManager::getNetBase("LS")->setOtherSideAssociations(OtherSideAssociations, sizeof(OtherSideAssociations)/sizeof(OtherSideAssociations[0]));
}
