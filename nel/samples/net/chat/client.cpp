/** \file login_system/client.cpp
 * Login system example
 *
 * $Id: client.cpp,v 1.2 2005/04/13 12:37:16 cado Exp $
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


/*
 * Login system example, client.
 *
 * This client connects to a front-end server using login system.
 *
 * Before running this client, the front end service sample must run,
 * and also the NeL naming_service, time_service, login_service, welcome_service.
 *
 */

//
// Includes
//

#include <string>

#ifdef NL_OS_WINDOWS
#include <conio.h>
#else
#include "kbhit.h"
#endif

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/bit_mem_stream.h"

#include "nel/net/callback_client.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

// Really simple chat
// Do not display what you are typing

#ifdef NL_OS_WINDOWS
#define KEY_ESC		27
#define KEY_ENTER	13
#else
#define KEY_ESC		27
#define KEY_ENTER	10
#endif

string CurrentEditString;

// ***************************************************************************
void serverSentChat (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// Called when the server sent a CHAT message
	string text;
	msgin.serial(text);
	printf("%s\n", text.c_str());
}

// ***************************************************************************
// All messages handled by this server
#define NB_CB 1
TCallbackItem CallbackArray[NB_CB] =
{
	{ "CHAT", serverSentChat }
};

/*
 * main
 */
int main (int argc, char **argv)
{
	CCallbackClient *Client;

	// Read the host where to connect in the client.cfg file
	CConfigFile ConfigFile;	
	ConfigFile.load ("client.cfg");
	string LSHost(ConfigFile.getVar("LSHost").asString());

	// Init and Connect the client to the server located on port 3333
	Client = new CCallbackClient();
	Client->addCallbackArray (CallbackArray, NB_CB);

	printf("Please wait connecting...\n");
	try
	{
		CInetAddress addr(LSHost+":3333");
		Client->connect(addr);
	}
	catch(ESocket &e)
	{
		printf("%s\n", e.what());
		return 0;
	}

	if (!Client->connected())
	{
		printf("Connection Error\n");
		return 0;
	}
	printf("Connected.\n");

#ifdef NL_OS_UNIX
	init_keyboard();
#endif
	// The main loop
	do
	{
		int c;
		if (kbhit())
		{
			c = getch();
			if (c == KEY_ESC)
			{
				break; // FINSIH
			}
			else if (c == KEY_ENTER)
			{
				CMessage msg;
				msg.setType("CHAT");
				msg.serial (CurrentEditString);
				Client->send(msg);
				CurrentEditString = "";
			}
			else
			{
				CurrentEditString += c;
			}
		}
		Client->update();
	}
	while (Client->connected());

#ifdef NL_OS_UNIX
	close_keyboard();
#endif
	// Finishing
	delete Client;
}
