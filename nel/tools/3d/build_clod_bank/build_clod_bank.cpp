/** \file build_clod_bank.cpp
 * build a .clodbank with a config file.
 *
 * $Id: build_clod_bank.cpp,v 1.3 2002/05/17 09:20:04 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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


#include "nel/misc/config_file.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/debug.h"
#include "3d/lod_character_shape_bank.h"
#include "3d/lod_character_shape.h"
#include "3d/lod_character_builder.h"
#include "3d/animation.h"
#include "3d/skeleton_shape.h"
#include "3d/register_3d.h"


using namespace std;
using namespace NLMISC;
using namespace NL3D;

int	main(int argc, char *argv[])
{
	uint	i;

	// Avoid warnings.
	NLMISC::createDebug();
	DebugLog->addNegativeFilter("Exception will be launched");
	WarningLog->addNegativeFilter("Exception will be launched");
	InfoLog->addNegativeFilter("FEHTIMER>");
	InfoLog->addNegativeFilter("adding the path");

	// Init serial
	registerSerial3d();

	if(argc<4)
	{
		puts("Usage:    build_clod_bank  path_file.cfg  config_file.cfg  destfile.clodbank  [bakeFrameRate=20] ");
		return 0;
	}


	try
	{
		// The bank to fill
		CLodCharacterShapeBank	lodShapeBank;


		// Read the frameRate to process bake of anims
		float	bakeFrameRate= 20;
		if(argc>=5)
		{
			bakeFrameRate= (float)atof(argv[4]);
			if(bakeFrameRate<=1)
			{
				nlwarning("bad bakeFrameRate value, use a default of 20");
				bakeFrameRate= 20;
			}
		}


		// parse the path file.
		//==================

		// try to load the config file.
		CConfigFile pathConfig;
		pathConfig.load (argv[1]);

		// Get the search pathes
		CConfigFile::CVar &search_pathes = pathConfig.getVar ("search_pathes");
		for (i = 0; i < (uint)search_pathes.size(); i++)
		{
			// Add to search path
			CPath::addSearchPath (search_pathes.asString(i));
		}

		// parse the config file.
		//==================

		// try to load the config file.
		CConfigFile config;
		config.load (argv[2]);

		// For all .clod to process
		//==================
		CConfigFile::CVar &clod_list = config.getVar ("clod_list");
		uint	lodId;
		for (lodId = 0; lodId < (uint)clod_list.size(); lodId++)
		{
			string	lodFileName= clod_list.asString(lodId);
			string	lodName= CFile::getFilenameWithoutExtension(lodFileName);

			printf("Process LOD: %s\n", lodFileName.c_str());

			// Search the variable with this name.
			try
			{
				CIFile		iFile;

				// get the anim list.
				CConfigFile::CVar &clod_anim_list = config.getVar (lodName);

				// Correct format?
				if(clod_anim_list.size()<2)
				{
					nlwarning("%s skipped. Must have at least the skeleton name, and one animation", lodFileName.c_str());
					// go to next.
					continue;
				}

				// Init lod shape process
				//===========================

				// The first variable is the name of the skeleton.
				string	skeletonName= clod_anim_list.asString(0);
				CSmartPtr<CSkeletonShape>	skeletonShape;

				// Load it.
				iFile.open(CPath::lookup(skeletonName));
				CShapeStream		strShape;
				strShape.serial(iFile);
				iFile.close();

				// Get the pointer, check it's a skeleton
				if(dynamic_cast<CSkeletonShape*>(strShape.getShapePointer()) == NULL)
					throw Exception("%s is not a Skeleton", skeletonName.c_str());
				skeletonShape= (CSkeletonShape*)strShape.getShapePointer();

				// Load the shape.
				CLodCharacterShapeBuild		lodShapeBuild;
				iFile.open( CPath::lookup(lodFileName) );
				iFile.serial(lodShapeBuild);
				iFile.close();

				// Prepare to build the lod.
				CLodCharacterBuilder		lodBuilder;
				lodBuilder.setShape(lodName, skeletonShape, &lodShapeBuild);


				// Traverse all anim in the list.
				//===========================
				uint	animId;
				for (animId = 1; animId < (uint)clod_anim_list.size(); animId++)
				{
					string	animFileName= clod_anim_list.asString(animId);

					// display.
					printf("Process Anim: %d/%d\r", animId, clod_anim_list.size()-1);

					// Try to load the animation
					CAnimation			*anim= new CAnimation;
					// NB: fatal error if anim not found
					iFile.open( CPath::lookup(animFileName) );
					iFile.serial(*anim);
					iFile.close();
					// Add to the builder. NB: animation will be delete in this method.
					// NB: the key name here is the entire file, with the .anim, for easier georges editing.
					lodBuilder.addAnim(animFileName.c_str(), anim, bakeFrameRate);
				}
				printf("\n");

				// Add to the bank.
				//===========================
				uint32	shapeId= lodShapeBank.addShape();
				*lodShapeBank.getShapeFullAcces(shapeId)= lodBuilder.getLodShape();
			}
			catch(EUnknownVar &evar)
			{
				nlwarning(evar.what());
				// Any other exception will make the program quit.
			}

		}

		// Save bank.
		//===========================

		// compile
		lodShapeBank.compile();

		// Save
		COFile	oFile(argv[3]);
		oFile.serial(lodShapeBank);
		oFile.close();
	}
	catch (Exception& except)
	{
		// Error message
		printf ("ERROR %s.\n Aborting.\n", except.what());
	}


	return 0;
}