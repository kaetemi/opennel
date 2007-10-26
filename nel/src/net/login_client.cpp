/** \file login_client.cpp
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

#include "stdnet.h"

#include "nel/misc/system_info.h"

#include "nel/net/callback_client.h"

#include "nel/net/login_cookie.h"
#include "nel/net/login_client.h"

#include "nel/net/udp_sock.h"

using namespace std;
using namespace NLMISC;

namespace NLNET {


CLoginClient::TShardList CLoginClient::ShardList;
CCallbackClient *CLoginClient::_LSCallbackClient;


//
// CALLBACK FROM THE FS (Front-end Service)
//

// Callback for answer of the request shard
static bool ShardValidate;
static string ShardValidateReason;
static void cbShardValidate (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// S14: receive "SV" message from FS
	//

	msgin.serial (ShardValidateReason);
	ShardValidate = true;
}

static TCallbackItem FSCallbackArray[] =
{
	{ "SV", cbShardValidate },
};


//
// CALLBACK FROM THE LS (Login Service)
//

// Callback for answer of the login password.
static bool VerifyLoginPassword;
static string VerifyLoginPasswordReason;
static void cbVerifyLoginPassword (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// S04: receive the "VLP" message from LS
	//

	msgin.serial (VerifyLoginPasswordReason);
	if(VerifyLoginPasswordReason.empty())
	{
		uint32 nbshard;
		msgin.serial (nbshard);

		CLoginClient::ShardList.clear ();
		VerifyLoginPasswordReason.clear();

		// get the shard list
		for (uint i = 0; i < nbshard; i++)
		{
			CLoginClient::CShardEntry se;
			msgin.serial (se.Name, se.NbPlayers, se.Id);
			CLoginClient::ShardList.push_back (se);
		}		
	}
	VerifyLoginPassword = true;
}

// Callback for answer of the request shard
static bool ShardChooseShard;
static string ShardChooseShardReason;
static string ShardChooseShardAddr;
static string ShardChooseShardCookie;
static void cbShardChooseShard (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// S11: receive "SCS" message from LS
	//

	msgin.serial (ShardChooseShardReason);

	if (ShardChooseShardReason.empty())
	{
		msgin.serial (ShardChooseShardCookie);
		msgin.serial (ShardChooseShardAddr);
	}
	ShardChooseShard = true;
}

static TCallbackItem LSCallbackArray[] =
{
	{ "VLP", cbVerifyLoginPassword },
	{ "SCS", cbShardChooseShard },
};

string CLoginClient::authenticate (const string &loginServiceAddr, const ucstring &login, const string &cpassword, const string &application)
{
	//
	// S01: connect to the LS
	//
	try
	{
		if(_LSCallbackClient == 0)
		{
			_LSCallbackClient = new CCallbackClient();
			_LSCallbackClient->addCallbackArray (LSCallbackArray, sizeof(LSCallbackArray)/sizeof(LSCallbackArray[0]));
		}

		string addr = loginServiceAddr;
		if(addr.find(":") == string::npos)
			addr += ":49997";
		if(_LSCallbackClient->connected())
			_LSCallbackClient->disconnect();
		_LSCallbackClient->connect (CInetAddress(addr));
	}
	catch (ESocket &e)
	{
		delete _LSCallbackClient;
		_LSCallbackClient = 0;
		nlwarning ("Connection refused to LS (addr:%s): %s", loginServiceAddr.c_str(), e.what ());
		return toString("Connection refused to LS (addr:%s): %s", loginServiceAddr.c_str(), e.what ());
	}

	//
	// S02: create and send the "VLP" message
	//
	CMessage msgout ("VLP");
	msgout.serial (const_cast<ucstring&>(login));
	msgout.serial (const_cast<string&>(cpassword));
	msgout.serial (const_cast<string&>(application));

	_LSCallbackClient->send (msgout);

	// wait the answer from the LS
	VerifyLoginPassword = false;
	while (_LSCallbackClient->connected() && !VerifyLoginPassword)
	{
		_LSCallbackClient->update ();
		nlSleep(10);
	}

	// have we received the answer?
	if (!VerifyLoginPassword)
	{
		delete _LSCallbackClient;
		_LSCallbackClient = 0;
		return "CLoginClient::authenticate(): LS disconnects me";
	}

	if (!VerifyLoginPasswordReason.empty())
	{
		_LSCallbackClient->disconnect ();
		delete _LSCallbackClient;
		_LSCallbackClient = 0;
	}

	return VerifyLoginPasswordReason;
}

string CLoginClient::connectToShard (CLoginCookie &lc, const std::string &addr, CCallbackClient &cnx)
{
	nlassert (!cnx.connected());

	try
	{
		//
		// S12: connect to the FS and send "SV" message to the FS
		//
		cnx.connect (CInetAddress(addr));
		cnx.addCallbackArray (FSCallbackArray, sizeof(FSCallbackArray)/sizeof(FSCallbackArray[0]));

		// send the cookie
		CMessage msgout2 ("SV");
		msgout2.serial (lc);
		cnx.send (msgout2);

		// wait the answer of the connection
		ShardValidate = false;
		while (cnx.connected() && !ShardValidate)
		{
			cnx.update ();
			nlSleep(10);
		}

		// have we received the answer?
		if (!ShardValidate) return "FS disconnect me";
	}
	catch (ESocket &e)
	{
		return string("FS refused the connection (") + e.what () + ")";
	}

	return ShardValidateReason;
}

string CLoginClient::connectToShard (const std::string &addr, CUdpSock &cnx)
{
	nlassert (!cnx.connected());
	
	try
	{
		//
		// S12: connect to the FES. Note: In UDP mode, it's the user that have to send the cookie to the front end
		//
		// If a personal firewall such as ZoneAlarm is installed and permission not granted yet,
		// the connect blocks until the user makes a choice.
		// If the user denies the connection, the exception ESocket is thrown.
		// Other firewalls such as Kerio make the send() fail instead.
		//
		cnx.connect (CInetAddress(addr));
	}
	catch (ESocket &e)
	{
		return string("FES refused the connection (") + e.what () + ")";
	}

	return ShardValidateReason;
}

string CLoginClient::connectToShard (const std::string &addr, CUdpSimSock &cnx)
{
	nlassert (!cnx.connected());
	
	try
	{
		//
		// S12: connect to the FES. Note: In UDP mode, it's the user that have to send the cookie to the front end
		//
		// See firewall comment in connectToShard(string,CUdpSock)
		//
		cnx.connect (CInetAddress(addr));
	}
	catch (ESocket &e)
	{
		return string("FES refused the connection (") + e.what () + ")";
	}

	return ShardValidateReason;
}

string CLoginClient::confirmConnection (sint32 shardId)
{
	nlassert (_LSCallbackClient != 0 && _LSCallbackClient->connected());

	//
	// S05: create and send the "CS" message with the shardid choice to the LS
	//

	CLoginClient::CShardEntry *s = getShard(shardId);
	nlassert(s);

	// send CS
	CMessage msgout ("CS");
	msgout.serial (s->Id);
	_LSCallbackClient->send (msgout);

	// wait the answer
	ShardChooseShard = false;
	while (_LSCallbackClient->connected() && !ShardChooseShard)
	{
		_LSCallbackClient->update ();
		nlSleep(10);
	}

	// have we received the answer?
	if (!ShardChooseShard)
	{
		delete _LSCallbackClient;
		_LSCallbackClient = 0;
		return "CLoginClientMtp::confirmConnection(): LS disconnects me";
	}
	else
	{
		_LSCallbackClient->disconnect ();
		delete _LSCallbackClient;
		_LSCallbackClient = 0;
	}

	if (!ShardChooseShardReason.empty())
	{
		return ShardChooseShardReason;
	}

	// ok, we can try to connect to the good front end

	nlinfo("addr: '%s' cookie: %s", ShardChooseShardAddr.c_str(), ShardChooseShardCookie.c_str());

	return "";
}

string CLoginClient::wantToConnectToShard (sint32 shardId, string &ip, string &cookie)
{
	string res = confirmConnection (shardId);
	if (!res.empty()) return res;

	ip = ShardChooseShardAddr;
	cookie = ShardChooseShardCookie;

	return "";
}

CLoginClient::CShardEntry *CLoginClient::getShard (sint32 shardId)
{
	for(TShardList::iterator it=ShardList.begin();it!=ShardList.end();it++)
	{
		if((*it).Id == shardId)
			return &(*it);
	}
	return 0;
}


} // NLNET
