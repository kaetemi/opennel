/** \file script.cpp
 * MaxScript extension for ligo plugins
 *
 * $Id: script.cpp,v 1.3 2001/11/14 15:16:00 corvazier Exp $
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

#define EXPORT_GET_ALLOCATOR

#include "max_to_ligo.h"

// From ligo library
#include "../lib/zone_template.h"
#include "../lib/ligo_config.h"
#include "../lib/ligo_error.h"
#include "../lib/material.h"
#include "../lib/transition.h"
#include "../lib/zone_bank.h"

// From nel patch lib
#include "../../plugin_max/nel_patch_lib/rpo.h"

// From nel 3d
#include <3d/zone.h>
#include <3d/nelu.h>
#include <3d/landscape_model.h>

// From nel misc
#include <nel/misc/file.h>
#include <nel/misc/o_xml.h>
#include <nel/misc/i_xml.h>

#include "MAXScrpt.h"
#include "3dmath.h"

// Various MAX and MXS includes
#include "Numbers.h"
#include "MAXclses.h"
#include "Streams.h"
#include "MSTime.h"
#include "MAXObj.h"
#include "Parser.h"
#include "max.h"
#include "stdmat.h"
#include "definsfn.h"

// Visual
#include <direct.h>

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NL3D;

// ***************************************************************************

/// Definition of new MAXScript functions
def_visible_primitive( export_material,						"NeLLigoExportMaterial");
def_visible_primitive( export_transition,					"NeLLigoExportTransition");
def_visible_primitive( export_zone,							"NeLLigoExportZone");
def_visible_primitive( get_error_zone_template,				"NeLLigoGetErrorZoneTemplate");
def_visible_primitive( get_snap ,							"NeLLigoGetSnap");
def_visible_primitive( get_cell_size ,						"NeLLigoGetCellSize");
def_visible_primitive( check_zone_with_material ,			"NeLLigoCheckZoneWithMaterial");
def_visible_primitive( get_error_string,					"NeLLigoGetErrorString");
def_visible_primitive( set_directory,						"NeLLigoSetDirectory");
def_visible_primitive( get_zone_mask,						"NeLLigoGetZoneMask");

// ***************************************************************************

/// Zone template
CLigoError ScriptErrors[10];

// ***************************************************************************

bool MakeSnapShot (NL3D::CZone &zone, NLMISC::CBitmap &snapshot, const NL3D::CTileBank &tileBank, 
				   uint xmax, uint ymax, const CLigoConfig &config, bool errorInDialog);

// ***************************************************************************

/// Export a material
Value* export_material_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(export_material, 4, count);

	// Check to see if the arguments match up to what we expect
	char *message = "NeLLigoExportMaterial [Object] [Filename] [CheckOnly] [Error in dialog]";
	type_check(arg_list[0], MAXNode, message);
	type_check(arg_list[1], String, message);
	type_check(arg_list[2], Boolean, message);
	type_check(arg_list[3], Boolean, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// The first arg
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// The second arg
	const char *fileName = arg_list[1]->to_string();

	// The third arg
	bool checkOnly = (arg_list[2]->to_bool() != FALSE);

	// The fourth arg
	bool errorInDialog = (arg_list[3]->to_bool() != FALSE);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// Ok ?
	if (os.obj)
	{
		// Convert in 3ds NeL patch mesh
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			// Load the ligofile
			CLigoConfig config;
			if (!CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, errorInDialog))
			{
				// Output an error
				CMaxToLigo::errorMessage ("Error: can't load the config file ligoscape.cfg", "NeL Ligo export material", 
					*MAXScript_interface, errorInDialog);
			}
			else
			{
				// The zone template
				CZoneTemplate zoneTemplate;

				// Build the zone template
				bool res = CMaxToLigo::buildZoneTemplate (node, tri->patch, zoneTemplate, config, ScriptErrors[0], ip->GetTime());

				// Success ?
				if (res)
				{
					// Build a zone
					NL3D::CZone zone;
					if (tri->rpatch->exportZone (node, &tri->patch, zone, 0))
					{
						// Build a material
						NLLIGO::CMaterial material;

						// No error ?
						if (res = material.build (zoneTemplate, config, ScriptErrors[0]))
						{
							// Save it ?
							if (!checkOnly)
							{

								// Make a name for the zone
								char drive[512];
								char dir[512];
								char name[512];
								char path[512];
								_splitpath (fileName, drive, dir, name, NULL);
								_makepath (path, drive, dir, name, ".zone");

								// Ok ?
								bool ok = true;

								// Catch exception
								try
								{
									// Open a stream file
									COFile outputfile;
									if (outputfile.open (fileName))
									{
										// Create an xml stream
										COXml outputXml;
										nlverify (outputXml.init (&outputfile));

										// Serial the class
										material.serial (outputXml);

										// Another the stream
										COFile outputfile2;

										// Open another file
										if (outputfile2.open (path))
										{
											// Serial the zone
											zone.serial (outputfile2);
										}
										else
										{
											// Error message
											char tmp[512];
											smprintf (tmp, 512, "Can't open the file %s for writing.", path);
											CMaxToLigo::errorMessage (tmp, "NeL Ligo export material", *MAXScript_interface, errorInDialog);
											ok = false;
										}
									}
									else
									{
										// Error message
										char tmp[512];
										smprintf (tmp, 512, "Can't open the file %s for writing.", fileName);
										CMaxToLigo::errorMessage (tmp, "NeL Ligo export material", *MAXScript_interface, errorInDialog);
										ok = false;
									}
								}
								catch (Exception &e)
								{
									// Error message
									char tmp[512];
									smprintf (tmp, 512, "Error while save the file %s : %s", fileName, e.what());
									CMaxToLigo::errorMessage (tmp, "NeL Ligo export material", *MAXScript_interface, errorInDialog);
									ok = false;
								}

								// Remove the files
								if (!ok)
								{
									remove (fileName);
									remove (path);
								}
							}
						}
					}
					else
					{
						// Error, zone can't be exported
						CMaxToLigo::errorMessage ("Error: can't export the Nel zone, check bind errors.", "NeL Ligo export material", 
							*MAXScript_interface, errorInDialog);
					}
				}

				// Return the result
				return res?&true_value:&false_value;
			}
		}
		else
		{
			// Output an error
			CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
				"NeL Ligo export material", *MAXScript_interface, errorInDialog);
		}
	}

	// Return false
	return &false_value;
}

// ***************************************************************************

/// Export a transition
Value* export_transition_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (6)
	check_arg_count(export_transition, 6, count);

	// Check to see if the arguments match up to what we expect
	char *message = "NeLLigoExportTransition [Object array (count=9)] [Output filename] [First material filename] [Second material filename] [CheckOnly] [Error in dialog]";
	type_check(arg_list[0], Array, message);
	type_check(arg_list[1], String, message);
	type_check(arg_list[2], String, message);
	type_check(arg_list[3], String, message);
	type_check(arg_list[4], Boolean, message);
	type_check(arg_list[5], Boolean, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// The first arg
	Array *nodes = (Array*)arg_list[0];
	nlassert (is_array(nodes));

	// The second arg
	const char *fileName = arg_list[1]->to_string();

	// The second arg
	string matFilename[2];
	matFilename[0] = arg_list[2]->to_string();
	matFilename[1] = arg_list[3]->to_string();

	// The third arg
	bool checkOnly = (arg_list[4]->to_bool() != FALSE);

	// The fourth arg
	bool errorInDialog = (arg_list[5]->to_bool() != FALSE);

	// Ok ?
	bool ok = true;

	// All zone are present ?
	bool allPresent = true;

	// Clear error message
	for (uint error=0; error<10; error++)
		ScriptErrors[error].clear ();

	// Load the ligofile
	CLigoConfig config;
	if (!CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, errorInDialog))
	{
		// Output an error
		CMaxToLigo::errorMessage ("Error: can't load the config file ligoscape.cfg", "NeL Ligo export transition", 
			*MAXScript_interface, errorInDialog);
	}
	else
	{
		// CTransition::TransitionZoneCount elements in the array ?
		if (nodes->size == CTransition::TransitionZoneCount)
		{
			// Build a node array
			std::vector<CZoneTemplate>			arrayTemplate (CTransition::TransitionZoneCount);
			std::vector<const CZoneTemplate*>	arrayTemplatePtr (CTransition::TransitionZoneCount, NULL);
			std::vector<RPO*>					arrayTri (CTransition::TransitionZoneCount, NULL);
			std::vector<INode*>					arrayNode (CTransition::TransitionZoneCount, NULL);

			for (uint i=0; i<(uint)nodes->size; i++)
			{
				// Get a node zone
				if (nodes->get (i+1) != &undefined)
				{
					arrayNode[i] = nodes->get (i+1)->to_node();
					// Node builded ?
					bool builded = false;
					
					// Get a Object pointer
					ObjectState os=arrayNode[i]->EvalWorldState(ip->GetTime()); 

					// Ok ?
					if (os.obj)
					{
						// Convert in 3ds NeL patch mesh
						arrayTri[i] = (RPO *) os.obj->ConvertToType(ip->GetTime(), RYKOLPATCHOBJ_CLASS_ID);
						if (arrayTri[i])
						{
							// Build a zone template
							CZoneTemplate zoneTemplate;

							// Build the zone template
							if (CMaxToLigo::buildZoneTemplate (arrayNode[i], arrayTri[i]->patch, arrayTemplate[i], config, ScriptErrors[i], ip->GetTime()))
							{
								// Success, put the pointer
								arrayTemplatePtr[i] = &arrayTemplate[i];
								builded = true;
							}
						}
					}

					// Ok ?
					if (!builded)
						ok = false;
				}
				else
					allPresent = false;
			}

			// Ok, continue
			if (ok)
			{
				// Load the materials
				NLLIGO::CMaterial materials[2];

				// For each material
				for (uint mat=0; mat<2; mat++)
				{
					// Inputfile 0
					CIFile input;
					if (input.open (matFilename[mat]))
					{
						// Catch some errors
						try
						{
							// XML stream
							CIXml inputXml;
							nlverify (inputXml.init (input));

							// Serial
							materials[mat].serial (inputXml);
						}
						catch (Exception &e)
						{
							// Error message
							char tmp[2048];
							smprintf (tmp, 2048, "Error while loading material file %s : %s", matFilename[mat], e.what());
							CMaxToLigo::errorMessage (tmp, "NeL Ligo export transition", *MAXScript_interface, errorInDialog);
							ok = false;
						}
					}
					else
					{
						// Error message
						char tmp[512];
						smprintf (tmp, 512, "Can't open the file %s for reading.", matFilename[mat]);
						CMaxToLigo::errorMessage (tmp, "NeL Ligo export transition", *MAXScript_interface, errorInDialog);
						ok = false;
					}
				}

				// Ok ?
				if (ok)
				{
					// Build the transition
					CTransition transition;
					if (ok = transition.build (materials[0], materials[1], arrayTemplatePtr, config, ScriptErrors, ScriptErrors[CTransition::TransitionZoneCount]))
					{
						// Export ?
						if (!checkOnly)
						{
							// All transitions are ok ?
							if (allPresent)
							{
								// Build the zones
								NL3D::CZone zones[CTransition::TransitionZoneCount];
								uint zone;
								for (zone=0; zone<CTransition::TransitionZoneCount; zone++)
								{
									// Build the zone
									if (!arrayTri[zone]->rpatch->exportZone (arrayNode[zone], &arrayTri[zone]->patch, zones[zone], 0))
									{
										// Error, zone can't be exported
										CMaxToLigo::errorMessage ("Error: can't export the Nel zone, check bind errors.", "NeL Ligo export material", 
											*MAXScript_interface, errorInDialog);
										ok = false;
									}
								}

								// Catch exceptions
								if (ok)
								{
									// File names
									vector<string> createdfiles;
									try
									{
										// Open a stream file
										COFile outputfile;
										if (outputfile.open (fileName))
										{
											// Add the filename
											createdfiles.push_back (fileName);

											// Create an xml stream
											COXml outputXml;
											nlverify (outputXml.init (&outputfile));

											// Serial the class
											transition.serial (outputXml);

											// Make a name for the zone
											char drive[512];
											char dir[512];
											char name[512];

											// Export each zones
											for (zone=0; zone<CTransition::TransitionZoneCount; zone++)
											{
												// Final path
												char path[512];
												_splitpath (fileName, drive, dir, path, NULL);
												sprintf (name, "%s-%d", path, zone);
												_makepath (path, drive, dir, name, ".zone");

												// Another the stream
												COFile outputfile2;

												// Open another file
												if (outputfile2.open (path))
												{
													// Add the filename
													createdfiles.push_back (path);

													// Serial the zone
													zones[zone].serial (outputfile2);
												}
												else
												{
													// Error message
													char tmp[512];
													smprintf (tmp, 512, "Can't open the file %s for writing.", path);
													CMaxToLigo::errorMessage (tmp, "NeL Ligo export transition", *MAXScript_interface, errorInDialog);
													ok = false;
												}
											}
										}
										else
										{
											// Error message
											char tmp[512];
											smprintf (tmp, 512, "Can't open the file %s for writing.", fileName);
											CMaxToLigo::errorMessage (tmp, "NeL Ligo export transition", *MAXScript_interface, errorInDialog);
											ok = false;
										}
									}
									catch (Exception &e)
									{
										// Error message
										char tmp[512];
										smprintf (tmp, 512, "Error while save the file %s : %s", fileName, e.what());
										CMaxToLigo::errorMessage (tmp, "NeL Ligo export transition", *MAXScript_interface, errorInDialog);
										ok = false;
									}

									// Not ok ?
									if (!ok)
									{
										// Erase the files
										for (uint file=0; file<createdfiles.size(); file++)
										{
											// Removing files
											remove (createdfiles[file].c_str ());
										}
									}
								}
							}
							else
							{
								// Error message
								char tmp[512];
								smprintf (tmp, 512, "All transition are not present. Can't export..");
								CMaxToLigo::errorMessage (tmp, "NeL Ligo export transition", *MAXScript_interface, errorInDialog);
								ok = false;
							}
						}
					}
				}
			}
		}
		else
		{
			// Output an error
			CMaxToLigo::errorMessage ("Error: must have 9 cell in the node array", "NeL Ligo export transition", 
				*MAXScript_interface, errorInDialog);
		}
	}

	// Ok ?
	return ok?&true_value:&false_value;
}

// ***************************************************************************
 
Value* get_error_zone_template_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(get_error_zone_template, 4, count);

	// Check to see if the arguments match up to what we expect
	char *message = "NeLLigoGetErrorZoneTemplate [Array error codes] [Array vertex id] [Array messages] [Error index]";
	type_check(arg_list[0], Array, message);
	type_check(arg_list[1], Array, message);
	type_check(arg_list[2], Array, message);
	type_check(arg_list[3], Integer, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// The first arg
	Array *errorCodes = (Array *)arg_list[0];
	nlassert (is_array(errorCodes));

	// The second arg
	Array *vertexId = (Array *)arg_list[1];
	nlassert (is_array(vertexId));

	// The third arg
	Array *messages = (Array *)arg_list[2];
	nlassert (is_array(messages));

	// The third arg
	int errorIndex = arg_list[3]->to_int() - 1;
	clamp (errorIndex, 0, 8);

	// Num error
	uint numError = ScriptErrors[errorIndex].numVertexError ();

	// For each error
	for (uint i=0; i<numError; i++)
	{
		// Id and edge
		uint id, edge;
		uint error = (uint) ScriptErrors[errorIndex].getVertexError (i, id, edge);

		// Append error code
		errorCodes->append (Integer::intern (error+1));

		// Append vertex id
		vertexId->append (Integer::intern (id+1));

		// Append messages
		messages->append (new String("[LIGO DEBUG] Opened edge"));
	}

	// Return the main error message
	return Integer::intern ((int)ScriptErrors[errorIndex].MainError+1);
}

// ***************************************************************************
 
Value* get_snap_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (0)
	check_arg_count(get_snap, 0, count);

	// Load the ligofile
	CLigoConfig config;
	if (CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, false))
	{
		return Float::intern ((float)config.Snap);
	}
	else
		return &undefined;
}

// ***************************************************************************
 
Value* get_cell_size_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (0)
	check_arg_count(get_cell_size, 0, count);

	// Load the ligofile
	CLigoConfig config;
	if (CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, false))
	{
		return Float::intern ((float)config.CellSize);
	}
	else
		return &undefined;
}

// ***************************************************************************
 
/// Check a ligo zone with a ligo material
Value* check_zone_with_material_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (3)
	check_arg_count(check_zone_with_template, 3, count);

	// Check to see if the arguments match up to what we expect
	char *message = "NeLLigoCheckZoneWithMaterial [Object] [Material filename] [Error in dialog]";
	type_check(arg_list[0], MAXNode, message);
	type_check(arg_list[1], String, message);
	type_check(arg_list[2], Boolean, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// The first arg
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// The second arg
	string fileName = arg_list[1]->to_string();

	// The fourth arg
	bool errorInDialog = (arg_list[2]->to_bool() != FALSE);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// Ok ?
	if (os.obj)
	{
		// Convert in 3ds NeL patch mesh
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			// Load the ligofile
			CLigoConfig config;
			if (!CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, errorInDialog))
			{
				// Output an error
				CMaxToLigo::errorMessage ("Error: can't load the config file ligoscape.cfg", "NeL Ligo check zone", 
					*MAXScript_interface, errorInDialog);
			}
			else
			{
				// The zone template
				CZoneTemplate zoneTemplate;

				// Build the zone template
				bool res = CMaxToLigo::buildZoneTemplate (node, tri->patch, zoneTemplate, config, ScriptErrors[0], ip->GetTime());

				// Success ?
				if (res)
				{
					// The material
					NLLIGO::CMaterial material;

					// Read the template
					CIFile inputfile;

					// Catch exception
					try
					{
						// Open the selected template file
						if (inputfile.open (fileName))
						{
							// Create an xml stream
							CIXml inputXml;
							inputXml.init (inputfile);

							// Serial the class
							material.serial (inputXml);

							// Get the zone edges
							const std::vector<CZoneEdge> &zoneEdges = zoneTemplate.getEdges ();

							// All edge ok
							res = true;

							// For each zone edge
							for (uint edge=0; edge<zoneEdges.size(); edge++)
							{
								// Check it
								if (!material.getEdge().isTheSame (zoneEdges[edge], config, ScriptErrors[0]))
									res = false;
							}
						}
						else
						{
							// Error message
							char tmp[512];
							smprintf (tmp, 512, "Can't open the skeleton template file %s for reading.", fileName);
							CMaxToLigo::errorMessage (tmp, "NeL Ligo check zone", *MAXScript_interface, errorInDialog);
						}
					}
					catch (Exception &e)
					{
						// Error message
						char tmp[512];
						smprintf (tmp, 512, "Error while loading the file %s : %s", fileName, e.what());
						CMaxToLigo::errorMessage (tmp, "NeL Ligo check zone", *MAXScript_interface, errorInDialog);
					}
				}

				// Return the result
				return res?&true_value:&false_value;
			}
		}
		else
		{
			// Output an error
			CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
				"NeL Ligo check zone", *MAXScript_interface, errorInDialog);
		}
	}
	else
	{
		// Output an error
		CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
			"NeL Ligo check zone", *MAXScript_interface, errorInDialog);
	}

	// Return false
	return &false_value;
}

// ***************************************************************************
 
/// Export a ligo zone
Value* export_zone_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(check_zone_with_template, 4, count);

	// Check to see if the arguments match up to what we expect
	char *message = "NeLLigoExportZone [Object] [Ligozone filename] [Category Array] [Error in dialog]";
	type_check(arg_list[0], MAXNode, message);
	type_check(arg_list[1], String, message);
	type_check(arg_list[2], Array, message);
	type_check(arg_list[3], Boolean, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// The first arg
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// The second arg
	string fileName = arg_list[1]->to_string();

	// The thrid arg
	Array *array = (Array*)arg_list[2];

	// Build an array of category
	vector<pair<string, string> > categories;
	categories.resize (array->size);

	// The fourth arg
	bool errorInDialog = (arg_list[3]->to_bool() != FALSE);

	// For each array elements
	uint i;
	for (i=0; i<(uint)array->size; i++)
	{
		// Check that we have an array
		type_check(array->get (i+1), Array, message);

		// Check we have 2 strings
		Array *cell = (Array*)array->get (i+1);
		if (cell->size != 2)
		{
			// Output an error
			CMaxToLigo::errorMessage ("Error: category arguments are not 2 strings", "NeL Ligo export zone", 
				*MAXScript_interface, errorInDialog);
			return &false_value;
		}
		type_check (cell->get(1), String, message);
		type_check (cell->get(2), String, message);

		// Get the strings
		categories[i].first = cell->get(1)->to_string();
		categories[i].second = cell->get(2)->to_string();
	}

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// Ok ?
	if (os.obj)
	{
		// Convert in 3ds NeL patch mesh
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			// Load the ligofile
			CLigoConfig config;
			if (!CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, errorInDialog))
			{
				// Output an error
				CMaxToLigo::errorMessage ("Error: can't load the config file ligoscape.cfg", "NeL Ligo export zone", 
					*MAXScript_interface, errorInDialog);
			}
			else
			{
				// Build a filename
				char drive[512];
				char path[512];
				char name[512];
				char ext[512];
				_splitpath (fileName.c_str(), drive, path, name, ext);

				// Build it
				char outputFilename[512];
				_makepath (outputFilename, drive, path, name, ".zone");

				// Build the zone
				CZone zone;
				if (!tri->rpatch->exportZone (node, &tri->patch, zone, 0))
				{
					// Error, zone can't be exported
					CMaxToLigo::errorMessage ("Error: can't export the Nel zone, check bind errors.", "NeL Ligo export zone", 
						*MAXScript_interface, errorInDialog);
				}
				else
				{
					// The zone template
					CZoneTemplate zoneTemplate;

					// Build the zone template
					bool res = CMaxToLigo::buildZoneTemplate (node, tri->patch, zoneTemplate, config, ScriptErrors[0], ip->GetTime());

					// Success ?
					if (res)
					{
						// Build the zone mask
						std::vector<bool> mask;
						uint width;
						uint height;
						zoneTemplate.getMask (mask, width, height);

						// The bank
						CTileBank tileBank;

						// Catch exception
						try
						{
							// Load the bank
							CIFile fileBank;
							if (fileBank.open (GetBankPathName ()))
							{
								// Create an xml stream
								tileBank.serial (fileBank);

								// Build a screenshot of the zone
								CBitmap snapshot;
								if (MakeSnapShot (zone, snapshot, tileBank, width, height, config, errorInDialog))
								{
									// Build the snap shot filename
									char outputFilenameSnapShot[512];
									_makepath (outputFilenameSnapShot, drive, path, name, ".tga");

									// Output the snap shot
									COFile outputSnapShot;
									if (outputSnapShot.open (outputFilenameSnapShot))
									{
										// Write the tga file
										snapshot.writeTGA (outputSnapShot, 32);

										// Output stream
										COFile output;
										if (output.open (outputFilename))
										{
											// Serial the NeL zone
											zone.serial (output);

											// Is filled ?
											uint j;
											for (j=0; j<mask.size(); j++)
												if (!mask[j]) break;
											
											// Add filled zone	
											if (j >= mask.size())
											{
												categories.push_back (pair<string,string> ("filled", "yes"));
											}
											else
											{
												categories.push_back (pair<string,string> ("filled", "no"));
											}

											// Add the zone categorie
											if (width == height)
											{
												categories.push_back (pair<string,string> ("square", "yes"));
											}
											else
											{
												categories.push_back (pair<string,string> ("square", "no"));
											}

											// Add the size category
											char size[30];
											smprintf (size, 30, "%dx%d", width, height);
											categories.push_back (pair<string,string> ("size", size));

											// Create the zone bank element
											CZoneBankElement bankElm;
											bankElm.setMask (mask, width,height);

											// Add the category
											for (j=0; j<categories.size(); j++)
											{
												bankElm.addCategory (strlwr (categories[j].first), strlwr (categories[j].second));
											}

											// Write the zone
											COFile outputLigoZone;

											// Catch exception
											try
											{
												// Open the selected zone file
												if (outputLigoZone.open (fileName))
												{
													// Create an xml stream
													COXml outputXml;
													outputXml.init (&outputLigoZone);

													// Serial the class
													bankElm.serial (outputXml);

													// Return true
													return &true_value;
												}
												else
												{
													// Error message
													char tmp[512];
													smprintf (tmp, 512, "Can't open the ligozone file %s for writing.", fileName.c_str() );
													CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone", *MAXScript_interface, errorInDialog);
												}
											}
											catch (Exception &e)
											{
												// Error message
												char tmp[512];
												smprintf (tmp, 512, "Error while loading the file %s : %s", fileName, e.what());
												CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone", *MAXScript_interface, errorInDialog);
											}
										}
										else
										{
											// Error message
											char tmp[512];
											smprintf (tmp, 512, "Can't open the tga file %s for writing.", outputFilenameSnapShot);
											CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone", *MAXScript_interface, errorInDialog);
										}
									}
									else
									{
										// Error message
										char tmp[512];
										smprintf (tmp, 512, "Can't open the NeL zone file %s for writing.", outputFilename);
										CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone", *MAXScript_interface, errorInDialog);
									}
								}
							}
							else
							{
								// Error message
								char tmp[512];
								smprintf (tmp, 512, "Can't open the bank file %s for writing.", GetBankPathName ().c_str() );
								CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone", *MAXScript_interface, errorInDialog);
							}
						}
						catch (Exception &e)
						{
							// Error message
							char tmp[512];
							smprintf (tmp, 512, "Error while loading the file bank %s : %s", GetBankPathName ().c_str(), e.what());
							CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone", *MAXScript_interface, errorInDialog);
						}
					}
				}
			}
		}
		else
		{
			// Output an error
			CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
				"NeL Ligo export zone", *MAXScript_interface, errorInDialog);
		}
	}
	else
	{
		// Output an error
		CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
			"NeL Ligo check zone", *MAXScript_interface, errorInDialog);
	}

	// Return false
	return &false_value;
}

// ***************************************************************************

Value* get_error_string_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (1)
	check_arg_count(get_error_string, 1, count);

	// Checks arg
	char *message = "NeLLigoGetErrorString [error code]";
	type_check(arg_list[0], Integer, message);

	// The first arg
	int errorCode = arg_list[0]->to_int()-1;

	// Error code
	return new String ((char*)CLigoError::getStringError ((CLigoError::TError)errorCode));
}

// ***************************************************************************

Value* set_directory_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (1)
	check_arg_count(set_directory, 1, count);

	// Checks arg
	char *message = "NeLLigoDirectory [path]";
	type_check(arg_list[0], String, message);

	// The first arg
	const char *dir = arg_list[0]->to_string();

	// Set the directory
	return (chdir (dir)==0)?&true_value:&false_value;
}

// ***************************************************************************

Value* get_zone_mask_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (5)
	check_arg_count(check_zone_with_template, 5, count);

	// Check to see if the arguments match up to what we expect
	char *message = "NeLLigoGetZoneMask [Object] [Mask Array] [Width Array] [Height Array] [Error in dialog]";
	type_check(arg_list[0], MAXNode, message);
	type_check(arg_list[1], Array, message);
	type_check(arg_list[2], Array, message);
	type_check(arg_list[3], Array, message);
	type_check(arg_list[4], Boolean, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// The first arg
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// The first array
	Array *maskArray = (Array*)arg_list[1];

	// The second array
	Array *widthArray = (Array*)arg_list[2];

	// The second array
	Array *heightArray = (Array*)arg_list[3];

	// The fourth arg
	bool errorInDialog = (arg_list[4]->to_bool() != FALSE);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// Ok ?
	if (os.obj)
	{
		// Convert in 3ds NeL patch mesh
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			// Load the ligofile
			CLigoConfig config;
			if (!CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, errorInDialog))
			{
				// Output an error
				CMaxToLigo::errorMessage ("Error: can't load the config file ligoscape.cfg", "NeL Ligo export zone", 
					*MAXScript_interface, errorInDialog);
			}
			else
			{
				// Build the zone
				CZone zone;
				if (!tri->rpatch->exportZone (node, &tri->patch, zone, 0))
				{
					// Error, zone can't be exported
					CMaxToLigo::errorMessage ("Error: can't export the Nel zone, check bind errors.", "NeL Ligo export zone", 
						*MAXScript_interface, errorInDialog);
				}
				else
				{
					// Get the object matrix
					Matrix3 nodeTM;
					nodeTM = node->GetObjectTM (ip->GetTime());

					// max x and y
					int width = 1;
					int height = 1;

					// For each vertex
					for (uint vert=0; vert<(uint)tri->patch.numVerts; vert++)
					{
						// Transform in the world
						Point3 worldPt = tri->patch.verts[vert].p * nodeTM;

						// Get cell coordinates
						int x = (int)(worldPt.x / config.CellSize) + 1;
						int y = (int)(worldPt.y / config.CellSize) + 1;

						// Max ?
						if (x>width)
							width = x;
						if (y>height)
							height = y;
					}

					// The second array
					width++;
					height++;
					widthArray->append (Float::intern ((float)width));
					heightArray->append (Float::intern ((float)height));

					// Cells
					vector<bool> cells (width * height, false);

					// For each patch
					for (uint patch=0; patch<(uint)tri->patch.numPatches; patch++)
					{
						// Average of the patch
						Point3 average (0,0,0);

						// For each vertex
						for (uint vert=0; vert<4; vert++)
						{
							// Index of the vertex
							int vertId = tri->patch.patches[patch].v[vert];

							// Sum
							average += tri->patch.verts[vertId].p * nodeTM;
						}

						// Average
						average /= 4;

						// Coordinates
						int x = (int)(average.x / config.CellSize) + 1;
						int y = (int)(average.y / config.CellSize) + 1;

						// Clip
						if ((x>=1) && (y>=1) && (x<width) && (y<height))
						{
							// Set this ok
							cells[x+y*width]=true;
						}
					}

					// For each cell
					for (uint k=0; k<cells.size(); k++)
					{
						// Build the result mask
						maskArray->append (cells[k]?&true_value:&false_value);
					}

					// ok
					return &true_value;
				}
			}
		}
		else
		{
			// Output an error
			CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
				"NeL Ligo export zone", *MAXScript_interface, errorInDialog);
		}
	}
	else
	{
		// Output an error
		CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
			"NeL Ligo check zone", *MAXScript_interface, errorInDialog);
	}

	// Return false
	return &false_value;
}

// Make a snap shot of a zone

bool MakeSnapShot (NL3D::CZone &zone, NLMISC::CBitmap &snapshot, const NL3D::CTileBank &tileBank, 
				   uint xmax, uint ymax, const CLigoConfig &config, bool errorInDialog)
{
	// Result
	bool result = false;
		
	try
	{
		// Resolution
		uint widthPixel = config.ZoneSnapShotRes * xmax;
		uint heightPixel = config.ZoneSnapShotRes * ymax;

		// Region
		float width = config.CellSize * (float)xmax;
		float height = config.CellSize * (float)ymax;

		// Use NELU
		CNELU::init (widthPixel, heightPixel, CViewport(), 32, true, NULL);

		// Setup the camera
		CNELU::Camera->setTransformMode (ITransformable::DirectMatrix);
		CMatrix view;
		view.setPos (CVector (width/2, height/2, width));
		view.setRot (CVector::I, -CVector::K, CVector::J);
		CNELU::Camera->setFrustum (width, height, 0.1f, 1000.f, false);
		CNELU::Camera->setMatrix (view);

		// Create a Landscape.
		CLandscapeModel	*theLand= (CLandscapeModel*)CNELU::Scene.createModel(LandscapeModelId);
		theLand->Landscape.setTileNear (1000.f);
		theLand->Landscape.TileBank=tileBank;

		// Enbable automatique lighting
#ifndef NL_DEBUG
		theLand->Landscape.enableAutomaticLighting (true);
		theLand->Landscape.setupAutomaticLightDir (CVector (0.5, 0.5, -1));
#endif // NL_DEBUG

		// Add the zone
		theLand->Landscape.addZone (zone);

		// Clear the backbuffer and the alpha
		CNELU::clearBuffers(CRGBA(255,0,255,0));

		// Render the scene
		CNELU::Scene.render();
	
		// Snapshot
		CNELU::Driver->getBuffer (snapshot);
		snapshot.flipV ();

		// Release the driver
		CNELU::Scene.getDriver()->release ();

		// Release NELU
		CNELU::release();

		// Ok
		result = true;
	}
	catch (Exception &e)
	{
		// Error
		char tmp[512];
		smprintf (tmp, 512, "Error during the snapshot: %s", e.what());

		// Output an error
		CMaxToLigo::errorMessage (tmp, "NeL Ligo check zone", *MAXScript_interface, errorInDialog);
	}

	// Return result
	return result;
}


// ***************************************************************************

/// MAXScript Plugin Initialization

// ***************************************************************************

__declspec( dllexport ) void LibInit() { }

// ***************************************************************************

