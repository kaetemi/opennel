/** \file admin.cpp
 * 
 *
 * $Id: admin.cpp,v 1.5 2001/05/31 16:41:59 lecroart Exp $
 *
 * \warning the admin client works *only* on Windows because we use kbhit() and getch() functions that are not portable.
 *
 */

/* Copyright, 2001 Nevrax Ltd.
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

#include <string>

#include "nel/misc/debug.h"
#include "nel/misc/command.h"
#include "nel/misc/config_file.h"
 
#include "nel/net/net_manager.h"

#include "interf.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;

int main (int argc, char **argv)
{
	nlinfo("Admin client for NeL Shard administration ("__DATE__" "__TIME__")\n");

//	DebugLog->addNegativeFilter ("L0:");
//	DebugLog->addNegativeFilter ("L1:");
//	DebugLog->addNegativeFilter ("L2:");

	CNetManager::init (NULL);

	initInterf ();

	CConfigFile ConfigFile;
	ConfigFile.load ("admin.cfg");
	CConfigFile::CVar &host = ConfigFile.getVar ("ASHosts");
	
	for (sint i = 0 ; i < host.size (); i += 2)
	{
		string ASName = host.asString(i);
		string ASAddr = host.asString(i+1);

		// add the AS in the list
		AdminServices.push_back (CAdminService());
		CAdminService *as = &(AdminServices.back());
		as->ASAddr = ASAddr;
		as->ASName = ASName;
		interfAddAS (as);
	}

	runInterf ();

	CNetManager::release ();

	return EXIT_SUCCESS;
}
