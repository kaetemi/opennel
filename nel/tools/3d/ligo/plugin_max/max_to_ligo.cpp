/** \file max_to_ligo.cpp
 * Convert a 3dsmax nel patch mesh in ligo data
 *
 * $Id: max_to_ligo.cpp,v 1.3 2002/02/18 15:22:12 corvazier Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "max_to_ligo.h"

// From nel misc
#include "nel/misc/stream.h"

// From ligo library
#include "ligo/zone_template.h"
#include "ligo/ligo_config.h"
#include "ligo/ligo_error.h"

// From MAXSDK
#include "maxscrpt.h"

using namespace std;
using namespace NLMISC;

#define NEL3D_APPDATA_ZONE_SYMMETRY		1266703979

namespace NLLIGO
{

// ***************************************************************************

int getScriptAppDataPatchMesh (Animatable *node, uint32 id, int def)
{
	// Get the chunk
	AppDataChunk *ap=node->GetAppDataChunk (MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, id);

	// Not found ? return default
	if (ap==NULL)
		return def;

	// String to int
	int value;
	if (sscanf ((const char*)ap->data, "%d", &value)==1)
		return value;
	else
		return def;
}

// ***************************************************************************

bool CMaxToLigo::buildZoneTemplate (INode* pNode, const PatchMesh &patchMesh, CZoneTemplate &zoneTemplate, const CLigoConfig &config, CLigoError &errors, TimeValue time)
{	
	// Vertices
	std::vector<NLMISC::CVector> vertices;
	vertices.resize (patchMesh.numVerts);

	// Indexies
	std::vector< std::pair<uint, uint> > indexes;

	// Get node matrix
	Matrix3 local = pNode->GetObjectTM (time);

	// For each vertices
	for (uint vert=0; vert<(uint)patchMesh.numVerts; vert++)
	{
		// Transform the vertex
		Point3 v = local * patchMesh.verts[vert].p;

		// Copy it
		vertices[vert].x = v.x;
		vertices[vert].y = v.y;
		vertices[vert].z = v.z;
	}

	// Symetric ?
	bool sym = getScriptAppDataPatchMesh (pNode, NEL3D_APPDATA_ZONE_SYMMETRY, 0) != 0;

	// For each edges
	for (uint edge=0; edge<(uint)patchMesh.numEdges; edge++)
	{
		// Open edge ?
		if (patchMesh.edges[edge].patch2<0)
		{
			// Add this edge
			if (sym)
				indexes.push_back (pair<uint, uint> (patchMesh.edges[edge].v2, patchMesh.edges[edge].v1));
			else
				indexes.push_back (pair<uint, uint> (patchMesh.edges[edge].v1, patchMesh.edges[edge].v2));
		}
	}

	// Build it
	return zoneTemplate.build (vertices, indexes, config, errors);
}

// ***************************************************************************

bool CMaxToLigo::loadLigoConfigFile (CLigoConfig& config, Interface& it, bool dialog)
{
	// Get the module path
	HMODULE hModule = GetModuleHandle("nelligo.dlx");
	if (hModule)
	{
		// Get the path
		char sModulePath[256];
		int res=GetModuleFileName(hModule, sModulePath, 256);

		// Success ?
		if (res)
		{
			// Path
			char sDrive[256];
			char sDir[256];
			_splitpath (sModulePath, sDrive, sDir, NULL, NULL);
			_makepath (sModulePath, sDrive, sDir, "ligoscape", ".cfg");

			try
			{
				// Load the config file
				config.read (sModulePath);

				// ok
				return true;
			}
			catch (Exception& e)
			{
				// Print an error message
				char msg[512];
				smprintf (msg, 512, "Error loading the config file ligoscape.cfg: %s", e.what());
				errorMessage (msg, "NeL Ligo load config file", it, dialog);
			}
		}
	}

	// Can't found the module
	return false;
}

// ***************************************************************************

void CMaxToLigo::errorMessage (const char *msg, const char *title, Interface& it, bool dialog)
{
	// Text or dialog ?
	if (dialog)
	{
		// Dialog message
		MessageBox (it.GetMAXHWnd(), msg, title, MB_OK|MB_ICONEXCLAMATION);
	}
	else
	{
		// Text message
		mprintf ((string(msg) + "\n").c_str());
	}
}

// ***************************************************************************

}