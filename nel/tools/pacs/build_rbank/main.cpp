/** \file main.cpp
 *
 *
 * $Id: main.cpp,v 1.1 2001/11/28 15:54:46 corvazier Exp $
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

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/config_file.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/common.h"
#include "nel/misc/displayer.h"
#include "nel/misc/file.h"

#include "3d/register_3d.h"

#include "build_surf.h"
#include "build_rbank.h"

#include "pacs/global_retriever.h"
#include "pacs/retriever_bank.h"
#include "pacs/surface_quad.h"
#include "pacs/local_retriever.h"
#include "pacs/retriever_instance.h"

#include <string>
#include <deque>

using namespace std;
using namespace NLMISC;
using namespace NL3D;


#define LOG_ALL_INFO_TO_FILE



string												OutputRootPath;
string												OutputDirectory;
string												OutputPath;
string												TessellationPath;
string												IGBoxes;
bool												ReduceSurfaces;
bool												SmoothBorders;
bool												ComputeElevation;
bool												ComputeLevels;
bool												LinkElements;
bool												CutEdges;
vector<string>										ZoneNames;
string												ZoneExt;
string												ZoneLookUpPath;
bool												TessellateZones;
bool												MoulineZones;
bool												ProcessRetrievers;
string												PreprocessDirectory;
bool												UseZoneSquare;
string												ZoneUL;
string												ZoneDR;
string												GlobalRetriever;
string												RetrieverBank;
string												GlobalUL;
string												GlobalDR;
bool												ProcessGlobal;

string												BanksPath;
string												Bank;

/****************************************************************\
					initMoulinette
\****************************************************************/
int		getInt(CConfigFile &cf, const string &varName)
{
	CConfigFile::CVar &var = cf.getVar(varName);
	return var.asInt();
}

string	getString(CConfigFile &cf, const string &varName)
{
	CConfigFile::CVar &var = cf.getVar(varName);
	return var.asString();
}


void	initMoulinette()
{
	registerSerial3d();

	try
	{
		CConfigFile cf;
		int			i;
	
		cf.load("moulinette.cfg");
	
		OutputRootPath = getString(cf, "OutputRootPath");
		ReduceSurfaces = getInt(cf, "ReduceSurfaces") ? true : false;
		SmoothBorders = getInt(cf, "SmoothBorders") ? true : false;

		if (SmoothBorders)
			OutputDirectory = getString(cf, "SmoothDirectory");
		else
			OutputDirectory = getString(cf, "RawDirectory");

		TessellationPath = getString(cf, "TessellationPath");

		ComputeElevation = getInt(cf, "ComputeElevation") ? true : false;
		ComputeLevels = getInt(cf, "ComputeLevels") ? true : false;
		LinkElements = getInt(cf, "LinkElements") ? true : false;
		CutEdges = getInt(cf, "CutEdges") ? true : false;

		CConfigFile::CVar &cvZones = cf.getVar("Zones");
		for (i=0; i<cvZones.size(); i++)
		{
			ZoneNames.push_back(cvZones.asString(i));
		}

		ZoneExt = getString(cf, "ZoneExt");
		ZoneLookUpPath = getString(cf, "ZonePath");

		CPath::addSearchPath(ZoneLookUpPath);

		IGBoxes = getString(cf, "IGBoxes");

		TessellateZones = getInt(cf, "TessellateZones") ? true : false;
		MoulineZones = getInt(cf, "MoulineZones") ? true : false;
		ProcessRetrievers = getInt(cf, "ProcessRetrievers") ? true : false;
		PreprocessDirectory = getString(cf, "PreprocessDirectory");

		UseZoneSquare = getInt(cf, "UseZoneSquare") ? true : false;
		ZoneUL = getString(cf, "ZoneUL");
		ZoneDR = getString(cf, "ZoneDR");

		GlobalRetriever = getString(cf, "GlobalRetriever");
		RetrieverBank = getString(cf, "RetrieverBank");
		GlobalUL = getString(cf, "GlobalUL");
		GlobalDR = getString(cf, "GlobalDR");
		ProcessGlobal = getInt(cf, "ProcessGlobal") ? true : false;

		BanksPath = getString(cf, "BanksPath");
		Bank = getString(cf, "Bank");
		CPath::addSearchPath(BanksPath);

		nlinfo("OutputRootPath=%s", OutputRootPath.c_str());
		nlinfo("Outputdirectory=%s", OutputDirectory.c_str());
		nlinfo("TessellationPath=%s", TessellationPath.c_str());
		nlinfo("ReduceSurfaces=%s", ReduceSurfaces ? "true" : "false");
		nlinfo("SmoothBorders=%s", SmoothBorders ? "true" : "false");
		nlinfo("ComputeElevation=%s", ComputeElevation ? "true" : "false");
		nlinfo("ComputeLevels=%s", ComputeLevels ? "true" : "false");
		nlinfo("LinkElements=%s", LinkElements ? "true" : "false");
		nlinfo("CutEdges=%s", CutEdges ? "true" : "false");
		nlinfo("TessellateZones=%s", TessellateZones ? "true" : "false");
		nlinfo("MoulineZones=%s", MoulineZones ? "true" : "false");
		nlinfo("ProcessRetrievers=%s", ProcessRetrievers ? "true" : "false");
		nlinfo("ZoneLookUpPath=%s", ZoneLookUpPath.c_str());

		if (UseZoneSquare)
		{
			ZoneNames.clear();
			uint	ul = getZoneIdByName(ZoneUL),
					dr = getZoneIdByName(ZoneDR);
			uint	x0 = ul%256, 
					y0 = ul/256,
					x1 = dr%256, 
					y1 = dr/256;
			uint	x, y;
			for (y=y0; y<=y1; ++y)
				for (x=x0; x<=x1; ++x)
					ZoneNames.push_back(getZoneNameById(x+y*256));
		}
	}
	catch (EConfigFile &e)
	{
		printf ("Problem in config file : %s\n", e.what ());
	}

	OutputPath = OutputRootPath+OutputDirectory;
}

/****************************************************************\
					moulineZones
\****************************************************************/
void	moulineZones(vector<string> &zoneNames)
{
	uint	i;

	NLPACS::StatsSurfaces.init();

	if (TessellateZones)
	{
		for (i=0; i<zoneNames.size(); ++i)
			tessellateZone(zoneNames[i]);
	}

	if (MoulineZones)
	{
		for (i=0; i<zoneNames.size(); ++i)
			moulineZone(zoneNames[i]);
	}

	if (ProcessRetrievers)
	{
		for (i=0; i<zoneNames.size(); ++i)
			processRetriever(zoneNames[i]);
	}

	if (ProcessGlobal)
	{
		processGlobalRetriever();
	}

//	updateRetrieverBank();

/*
	NLPACS::StatsSurfaces.XBBSpanList.dump("X Bounding Box Span List :");
	NLPACS::StatsSurfaces.YBBSpanList.dump("Y Bounding Box Span List :");

	NLPACS::StatsSurfaces.XBBSpan.dump("X Bounding Box Span :");
	NLPACS::StatsSurfaces.YBBSpan.dump("Y Bounding Box Span :");
*/
	nlinfo("Total Span : %d", NLPACS::StatsSurfaces.TotalSpan);
	nlinfo("Total SpanList : %d", NLPACS::StatsSurfaces.TotalSpanList);
//	nlinfo("Span per SpanList : %g", (double)NLPACS::StatsSurfaces.TotalSpan/(double)NLPACS::StatsSurfaces.TotalSpanList);
}

/****************************************************************\
							MAIN
\****************************************************************/
CFileDisplayer fd("evallog.log", true);

int main(int argc, char **argv)
{

#ifdef LOG_ALL_INFO_TO_FILE
	createDebug();
	DebugLog->addDisplayer (&fd);
	ErrorLog->addDisplayer (&fd);
	WarningLog->addDisplayer (&fd);
	InfoLog->addDisplayer (&fd);
	AssertLog->addDisplayer (&fd);

	ErrorLog->removeDisplayer("DEFAULT_MBD");
#endif

	try
	{
		// Init the moulinette
		initMoulinette();

		// Compute the zone surfaces
		TTime	before, after;

		uint	i;
		if (argc > 1)
		{
			ZoneNames.clear();
			for (i=1; i<(uint)argc; ++i)
			{
				if (argv[i][0] != '-')
				{
					ZoneNames.push_back(string(argv[i]));
				}
				else
				{
					switch (argv[i][1])
					{
					case 'T':
						TessellateZones = true;
						break;
					case 't':
						TessellateZones = false;
						break;
					case 'M':
						MoulineZones = true;
						break;
					case 'm':
						MoulineZones = false;
						break;
					case 'L':
						ProcessRetrievers = true;
						break;
					case 'l':
						ProcessRetrievers = false;
						break;
					case 'G':
						ProcessGlobal = true;
						break;
					case 'g':
						ProcessGlobal = false;
						break;
					}
				}
			}
		}

		before = CTime::getLocalTime();
		moulineZones(ZoneNames);
		after = CTime::getLocalTime();

		uint	totalSeconds = (uint)((after-before)/1000);
		uint	workDay = totalSeconds/86400,
				workHour = (totalSeconds-86400*workDay)/3600,
				workMinute = (totalSeconds-86400*workDay-3600*workHour)/60,
				workSecond = (totalSeconds-86400*workDay-3600*workHour-60*workMinute);
		nlinfo("total computation time: %d days, %d hours, %d minutes and %d seconds", workDay, workHour, workMinute, workSecond);
	}
	catch (Exception &e)
	{
		fprintf (stderr,"main trapped an exception: '%s'\n", e.what ());
	}
#ifndef NL_DEBUG
	catch (...)
	{
		fprintf(stderr,"main trapped an unknown exception\n");
	}
#endif // NL_DEBUG

	return 0;
}
