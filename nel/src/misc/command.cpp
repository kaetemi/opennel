/** \file command.cpp
 * TODO: File description
 *
 * $Id: command.cpp,v 1.34 2004/11/15 10:25:03 lecroart Exp $
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

#include "stdmisc.h"

#include "nel/misc/command.h"
#include "nel/misc/algo.h"

using namespace std;
using namespace NLMISC;

namespace NLMISC {

ICommand::TCategorySet* ICommand::Categories;
ICommand::TCommand* ICommand::Commands;
bool ICommand::CommandsInit;

ICommand::ICommand(const char *categoryName, const char *commandName, const char *commandHelp, const char *commandArgs)
{
	// self registration

	if (!CommandsInit)
	{
		//nlinfo ("create map");
		Commands = new TCommand;
		Categories = new TCategorySet;
		CommandsInit = true;
	}

	TCommand::iterator comm = (*Commands).find(commandName);

	if (comm != (*Commands).end ())
	{
		// 2 commands have the same name
		nlstopex (("There are 2 commands that have the same name in the project (command name '%s'), skip the second definition", commandName));
	}
	else
	{
		// insert the new command in the map
		//nlinfo ("add command '%s'", commandName);
		CategoryName = categoryName;
		HelpString = commandHelp;
		CommandArgs = commandArgs;
		_CommandName = commandName;
		Type = Command;
		(*Commands)[commandName] = this;
		Categories->insert(categoryName);
	}
}

ICommand::~ICommand()
{
	// self deregistration

	if (!CommandsInit)
	{
		// should never happen
		nlstop;
		return;
	}

	// find the command

	for (TCommand::iterator comm = (*Commands).begin(); comm != (*Commands).end(); comm++)
	{
		if ((*comm).second == this)
		{
			//printf("remove command\n");
			(*Commands).erase (comm);

			if ((*Commands).size() == 0)
			{
				// if the commands map is empty, destroy it
				//printf("delete map\n");
				delete Commands;
				CommandsInit = false;
			}
			
			return;
		}
	}
	// commands is not found
	nlstop;
}

void ICommand::execute (const std::string &commandWithArgs, CLog &log, bool quiet, bool human)
{
	if (!quiet)
	{
		ucstring temp;
		temp.fromUtf8(commandWithArgs);
		string disp = temp.toString();
		log.displayNL ("Executing command : '%s'", disp.c_str());
	}

	// convert the buffer into string vector
	vector<pair<string, vector<string> > > commands;
	
	bool firstArg = true;
	uint i = 0;
	while (true)
	{
		// skip whitespace
		while (true)
		{
			if (i == commandWithArgs.size())
			{
				goto end;
			}
			if (commandWithArgs[i] != ' ' && commandWithArgs[i] != '\t' && commandWithArgs[i] != '\n' && commandWithArgs[i] != '\r')
			{
				break;
			}
			i++;
		}
		
		// get param
		string arg;
		if (commandWithArgs[i] == '\"')
		{
			// starting with a quote "
			i++;
			while (true)
			{
				if (i == commandWithArgs.size())
				{
					if (!quiet) log.displayNL ("Missing end quote character \"");
					return;
				}
				if (commandWithArgs[i] == '"')
				{
					i++;
					break;
				}
				if (commandWithArgs[i] == '\\')
				{
					// manage escape char backslash
					i++;
					if (i == commandWithArgs.size())
					{
						if (!quiet) log.displayNL ("Missing character after the backslash \\ character");
						return;
					}
					switch (commandWithArgs[i])
					{
						case '\\':	arg += '\\'; break; // double backslash
						case 'n':	arg += '\n'; break; // new line
						case '"':	arg += '"'; break; // "
						default:
							if (!quiet) log.displayNL ("Unknown escape code '\\%c'", commandWithArgs[i]);
							return;
					}
					i++;
				}
				else
				{
					arg += commandWithArgs[i++];
				}
			}
		}
		else
		{
			// normal word
			while (true)
			{
				if (commandWithArgs[i] == '\\')
				{
					// manage escape char backslash
					i++;
					if (i == commandWithArgs.size())
					{
						if (!quiet) log.displayNL ("Missing character after the backslash \\ character");
						return;
					}
					switch (commandWithArgs[i])
					{
						case '\\':	arg += '\\'; break; // double backslash
						case 'n':	arg += '\n'; break; // new line
						case '"':	arg += '"'; break; // "
						case ';':	arg += ';'; break; // ;
						default:
							if (!quiet) log.displayNL ("Unknown escape code '\\%c'", commandWithArgs[i]);
							return;
					}
				}
				else if (commandWithArgs[i] == ';')
				{
					// command separator
					break;
				}
				else
				{
					arg += commandWithArgs[i];
				}

				i++;

				if (i == commandWithArgs.size() || commandWithArgs[i] == ' ' || commandWithArgs[i] == '\t' || commandWithArgs[i] == '\n' || commandWithArgs[i] == '\r')
				{
					break;
				}
			}
		}

		if (!arg.empty())
		{
			if (firstArg)
			{
				commands.push_back (make_pair(arg, vector<string> () ));
				firstArg = false;
			}
			else
			{
				commands[commands.size()-1].second.push_back (arg);
			}
		}

		// separator
		if (i < commandWithArgs.size() && commandWithArgs[i] == ';')
		{
			firstArg = true;
			i++;
		}
	}
end:

// displays args for debug purpose
/*	for (uint u = 0; u < commands.size (); u++)
	{
		nlinfo ("c '%s'", commands[u].first.c_str());
		for (uint t = 0; t < commands[u].second.size (); t++)
		{
			nlinfo ("p%d '%s'", t, commands[u].second[t].c_str());
		}
	}
*/

	for (uint u = 0; u < commands.size (); u++)
	{
		// find the command	
		TCommand::iterator comm = (*Commands).find(commands[u].first);
		if (comm == (*Commands).end ())
		{
			// the command doesn't exist
			if (!quiet) log.displayNL("Command '%s' not found, try 'help'", commands[u].first.c_str());
		}
		else
		{
			//nlinfo("execute command '%s'", commands[u].first.c_str());
			if (!(*comm).second->execute (commands[u].second, log, quiet, human))
			{
				if (!quiet) log.displayNL("Bad command usage, try 'help %s'", commands[u].first.c_str());
			}
		}
	}
}


/*
 * Command name completion.
 * Case-sensitive. Displays the list after two calls with the same non-unique completion.
 * Completes commands used with prefixes (such as "help " for example) as well.
 */
void ICommand::expand (std::string &commandName, NLMISC::CLog &log)
{
	// Take out the string before the last separator and remember it as a prefix
	uint32 lastseppos = commandName.find_last_of( " " );
	string prefix;
	bool useprefix;
	if ( lastseppos != string::npos )
	{
		prefix = commandName.substr( 0, lastseppos+1 );
		commandName.erase( 0, lastseppos+1 );
		useprefix = true;
	}
	else
	{
		useprefix = false;
	}

	string lowerCommandName = toLower(commandName);
	// Build the list of matching command names
	vector<string> matchingnames;
	for (TCommand::iterator comm = (*Commands).begin(); comm != (*Commands).end(); comm++)
	{
		string first = toLower((*comm).first);
		if (first.find( lowerCommandName ) == 0)
		{
			matchingnames.push_back( (*comm).first );
		}
	}

	// Do not complete if there is no result
	if ( matchingnames.empty() )
	{
		log.displayNL( "No matching command" );
		goto returnFromExpand;
	}

	// Complete if there is a single result
	if ( matchingnames.size() == 1 )
	{
		commandName = matchingnames.front() + " ";
		goto returnFromExpand;
	}

	// Try to complete to the common part if there are several results
	{
		// Stop loop when a name size is i or names[i] are different
		string commonstr = commandName;
		uint i = commandName.size();
		while ( true )
		{
			char letter = 0;
			vector<string>::iterator imn;
			for ( imn=matchingnames.begin(); imn!=matchingnames.end(); ++imn )
			{
				// Return common string if the next letter is not the same in all matching names
				if ( ((*imn).size() == i) || ( (letter!=0) && ((*imn)[i] != letter) ) )
				{
					log.displayNL( "(Matching command not unique)" );
					static string lastCommandName;
					commandName = commonstr;
					if ( lastCommandName == commandName )
					{
						// Display all the matching names 
						vector<string>::iterator imn2;
						//stringstream ss;
						string str;
						//ss << "Matching commands:" << endl;
						str += "Matching commands:\n";
						for ( imn2=matchingnames.begin(); imn2!=matchingnames.end(); ++imn2 )
						{
							//ss << " " << (*imn2);
							str += " " + (*imn2);
						}
						log.displayNL( "%s", str.c_str() );
					}
					lastCommandName = commandName;
					goto returnFromExpand;
				}
				// Add the next letter to the common string if it is the same in all matching names
				else if ( letter == 0 )
				{
					letter = (*imn)[i];
				}
			}
			commonstr += letter;
			++i;
		}
	}

returnFromExpand:

	// Put back the prefix
	if ( useprefix )
	{
		commandName = prefix + commandName;
	}
}


void ICommand::serialCommands (IStream &f)
{
	vector<CSerialCommand> cmd;
	for (TCommand::iterator comm = (*Commands).begin(); comm != (*Commands).end(); comm++)
	{
		cmd.push_back (CSerialCommand ((*comm).first, (*comm).second->Type));
	}
	f.serialCont (cmd);
}

bool ICommand::exists (std::string const &commandName)
{
	return ((*Commands).find(commandName) != (*Commands).end ());
}

NLMISC_CATEGORISED_COMMAND(nel,help,"display help on a specific variable/commands or on all variables and commands", "[<variable>|<command>]")
{	
	nlassert (Commands != NULL);
	
	// make sure we have a valid number of paramaters
	if (args.size()>1)
		return false;

	// treat the case where we have no parameters
	if (args.size() == 0)
	{
		// display a list of all command categories
		log.displayNL("Help commands:");
		log.displayNL("- help all");
		for (TCategorySet::iterator it=Categories->begin();it!=Categories->end();++it)
		{
			log.displayNL("- help %s",(*it).c_str());
		}
		log.displayNL("- help <wildcard>");
		log.displayNL("- help <command name>");
		return true;
	}

	// treat the case where the supplied parameter is "all"
	if (args[0]=="all")
	{
		// display all commands
		log.displayNL("Displaying all %d variables and commands: ", (*Commands).size());
		uint i = 0;
		for (TCommand::iterator comm = (*Commands).begin(); comm != (*Commands).end(); comm++, i++)
		{
			log.displayNL("%2d %-15s: %s", i, (*comm).first.c_str(), (*comm).second->HelpString.c_str());
		}
		return true;
	}

	// treat the case where the supplied parameter is a category name
	if (Categories->find(args[0])!=Categories->end())
	{
		log.displayNL("Displaying commands and variables from category: %s", args[0].c_str());
		uint i = 0;
		for (TCommand::iterator comm = (*Commands).begin(); comm != (*Commands).end(); comm++)
		{
			if ((*comm).second->CategoryName==args[0])
			{
				log.displayNL("%2d %-15s: %s", i, (*comm).first.c_str(), (*comm).second->HelpString.c_str());
				i++;
			}
		}
		return true;
	}

	// treat the case where the supplied parameter is a wildcard
	if (args[0].find('*')!=std::string::npos || args[0].find('?')!=std::string::npos)
	{
		log.displayNL("Displaying commands and variables matching wildcard: %s", args[0].c_str());
		uint i = 0;
		for (TCommand::iterator comm = (*Commands).begin(); comm != (*Commands).end(); comm++)
		{
			if (testWildCard((*comm).first,args[0]))
			{
				log.displayNL("%2d %-15s: %s", i, (*comm).first.c_str(), (*comm).second->HelpString.c_str());
				i++;
			}
		}
		return true;
	}

	// treat the case where we're looking at help on a given command
	if ((*Commands).find(args[0]) != (*Commands).end())
	{
		TCommand::iterator comm = (*Commands).find(args[0]);
		log.displayNL("%s", (*comm).second->HelpString.c_str());
		log.displayNL("usage: %s %s", (*comm).first.c_str(), (*comm).second->CommandArgs.c_str(), (*comm).second->HelpString.c_str());
		return true;
	}

	// we've failed to find a case that works so display an error message and prompt the player
	log.displayNL("'%s' is not a command, category or wildcard. Type 'help' for more information", args[0].c_str());
	return true;
}

} // NLMISC
