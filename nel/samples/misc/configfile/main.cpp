/** \file configfile/main.cpp
 * example of the CConfigfile class
 *
 * $Id: main.cpp,v 1.2 2002/11/29 10:01:40 lecroart Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>

// look at the debug example
#include "nel/misc/debug.h"

// contains the config class
#include "nel/misc/config_file.h"

using namespace std;
using namespace NLMISC;

// this function will be called when the configfile will be modified by
// an external program
void mainCallback ()
{
	nlinfo ("The file was modified by external program!");
}

// this function will be called when the variable var12 will be modified by
// an external program
void var12Callback (CConfigFile::CVar &var)
{
	stringstream str;
	
	for (int i = 0; i < var.size (); i++)
		str << var.asInt (i) << " ";

	nlinfo("%s modified, new value: %s\n", var.Name.c_str (), str.str().c_str());
}


// the configfile
CConfigFile cf;

int main (int argc, char **argv)
{

	// look at the debug example
	createDebug();

	// load and parse the configfile
	cf.load ("simpletest.txt");

	// CConfigFile checks if the config file has been modified by an external program.
	// in this case, the configfile automatically reloads and reparses the file.
	// you can connect a callback function that will be called in this case if
	// you want, for example, reset variables
	
	// link a callback with this configfile.
	cf.setCallback (mainCallback);

	// link a callback with the var12. If and only if the var12 changed, this
	// callback will be called
	cf.setCallback ("var12", var12Callback);

	// display all variables
	cf.print ();

	// get the value of var1 as int
	int var1 = cf.getVar ("var1").asInt();

	// get the value of var1 as double, in this case, a conversion from int
	// to double will be done
	double var2 = cf.getVar ("var1").asDouble();

	// get the value of var2 as int, in this case, a conversion from string
	// to int will be done
	int var3 = cf.getVar ("var2").asInt();

	// if the variable is an array of values, you can access them putting the
	// index of the variable you want. Example, get and print all value of var12:
	for (sint i = 0; i < cf.getVar ("var12").size(); i++)
	{
		int val = cf.getVar ("var12").asInt(i);
		nlinfo("%d -> %d", i, val);
	}

	// if you try to access an unknown variable or if something goes wrong, an
	// exception will be called, you can catch them individually or all together in a try/catch block

	try
	{
		// try to access an unknown variable
		int val = cf.getVar ("unknown_variable").asInt();
	}
	catch (EConfigFile &e)
	{
		nlinfo("something goes wrong with configfile: %s", e.what());
	}

	cf.getVar ("var13").asInt (0);
	cf.getVar ("var13").asInt (1);
	cf.getVar ("var13").asInt (2);
	
	printf("Try to modify the var12 in the configfile or any other variable.\n\nPress CTRL-C to exit\n");

	while(true)
	{
		CConfigFile::checkConfigFiles();
	}
	return EXIT_SUCCESS;
}
