/** \file increment_version.cpp
 *	Increment the version number
 *
 * $Id: increment_version.cpp,v 1.1 2002/09/05 12:41:44 coutelas Exp $
 */

/* Copyright, 2002 Nevrax Ltd.
 *
 * This file is part of NEVRAX RYZOM.
 * NEVRAX RYZOM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX RYZOM is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX RYZOM; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/file.h"


#include <string>
#include <fstream>

using namespace std;
using namespace NLMISC;



//-----------------------------------------------
//	main
//
//-----------------------------------------------
sint main( sint argc, char ** argv )
{
	if( argc < 4 )
	{
		printf("\n");
		printf("Increment the version number in a version file\n\n");
		printf("INCREMENT_VERSION <version file> <version motif> <version main number>\n");
		return 1;
	}

	// open the version file
	ifstream input(argv[1], ios::in);
	if( !input.is_open() )
	{
		nlwarning("can't open the file %s",argv[1]);
		return 1;
	}

	// open the output file
	string outputFilename = string(argv[1]) + ".out";
	FILE * output = fopen(outputFilename.c_str(),"w");
	if( output == NULL )
	{
		nlwarning("can't open the output file %s",outputFilename.c_str());
		return 1;
	}
	
	bool motifFound = false;
	bool versionNumberFound = false;

	while( !input.eof() )
	{
		// read a line
		string line;
		getline(input,line,'\n');

		// search the version motif
		sint32 idx = line.find(argv[2]);

		// if motif not found
		if( idx == -1 )
		{
			// we write the line in output and continue to the next line
			fprintf(output,"%s\n",line.c_str());
		}
		// if motif found
		else
		{
			motifFound = true;

			// search the main version number
			idx = line.find(argv[3]);
			if( idx == -1 )
			{
				fprintf(output,"%s\n",line.c_str());
				continue;
			}

			versionNumberFound = true;

			// get old build version number
			sint32 oldBuildVersionStartIdx = line.rfind(".") + 1;
			sint32 oldBuildVersionEndIdx = line.find_first_of(" \t",oldBuildVersionStartIdx) - 1;
			if( oldBuildVersionEndIdx == -1) 
			{
				oldBuildVersionEndIdx = line.size() - 1;
			}
			string oldBuildVersionStr = line.substr( oldBuildVersionStartIdx, oldBuildVersionEndIdx - oldBuildVersionStartIdx + 1 );
			sint oldBuildVersion = atoi( oldBuildVersionStr.c_str() );

			// increment build version
			sint32 buildVersion = oldBuildVersion + 1;
			string buildVersionStr = toString(buildVersion);
			printf("%d\n",buildVersion);

			// replace build version in line
			line.replace( oldBuildVersionStartIdx, oldBuildVersionStr.size(), buildVersionStr.c_str(), buildVersionStr.size() );
			
			// we write the line in output and continue to the next line
			fprintf(output,"%s\n",line.c_str());
		}
	}
	input.close();
	fclose(output);

	if( !motifFound )
	{
		nlwarning("version motif %s not found\n",argv[2]);
		return 1;
	}
	else
	{
		if( !versionNumberFound )
		{
			nlwarning("version number %s not found\n",argv[3]);
			return 1;
		}
	}

	return 0;	
}

