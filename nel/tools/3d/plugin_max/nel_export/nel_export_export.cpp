/** \file nel_export_export.cpp
 * <File description>
 *
 * $Id: nel_export_export.cpp,v 1.6 2001/08/09 13:12:50 corvazier Exp $
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

#include "std_afx.h"
#include "nel_export.h"
#include "nel/misc/file.h"
#include "3d/shape.h"
#include "3d/animation.h"
#include "3d/skeleton_shape.h"
#include "../nel_mesh_lib/export_nel.h"

using namespace NL3D;
using namespace NLMISC;

// --------------------------------------------------

bool CNelExport::exportMesh (const char *sPath, INode& node, Interface& ip, TimeValue time, CExportNelOptions &opt)
{
	// Result to return
	bool bRet=false;

	// Eval the object a time
	ObjectState os = node.EvalWorldState(time);

	// Object exist ?
	if (os.obj)
	{
		// Skeleton shape
		CSkeletonShape *skeletonShape=NULL;
		TInodePtrInt *mapIdPtr=NULL;
		TInodePtrInt mapId;

		// If model skinned ?
		if (CExportNel::isSkin (node))
		{
			// Create a skeleton
			INode *skeletonRoot=CExportNel::getSkeletonRootBone (node);

			// Skeleton exist ?
			if (skeletonRoot)
			{
				// Build a skeleton
				skeletonShape=new CSkeletonShape();

				// Add skeleton bind pos info
				CExportNel::mapBoneBindPos boneBindPos;
				CExportNel::addSkeletonBindPos (node, boneBindPos);

				// Build the skeleton based on the bind pos information
				CExportNel::buildSkeletonShape (*skeletonShape, *skeletonRoot, &boneBindPos, mapId, time);

				// Set the pointer to not NULL
				mapIdPtr=&mapId;

				// Erase the skeleton
				if (skeletonShape)
					delete skeletonShape;
			}
		}

		// Export in mesh format
		IShape*	pShape=CExportNel::buildShape (node, ip, time, mapIdPtr, false, opt, false);

		// Conversion success ?
		if (pShape)
		{
			// Open a file
			COFile file;
			if (file.open (sPath))
			{
				try
				{
					// Create a streamable shape
					CShapeStream shapeStream (pShape);
					
					// Serial the shape
					shapeStream.serial (file);

					// All is good
					bRet=true;
				}
				catch (...)
				{
				}
			}

			// Delete the pointer
			delete pShape;
		}
	}
	return bRet;
}

// --------------------------------------------------

bool CNelExport::exportAnim (const char *sPath, std::vector<INode*>& vectNode, Interface& ip, TimeValue time, bool scene)
{
	// Result to return
	bool bRet=false;

	// Create an animation file
	CAnimation animFile;

	// For each node to export
	for (uint n=0; n<vectNode.size(); n++)
	{
		// Get name
		std::string nodeName="";

		// Set the name only if it is a scene animation
		if (scene)
			nodeName=CExportNel::getName (*vectNode[n])+".";

		// Add animation
		CExportNel::addAnimation (animFile, *vectNode[n], nodeName.c_str(), &ip);
	}

	if (vectNode.size())
	{
		// Open a file
		COFile file;
		if (file.open (sPath))
		{
			try
			{
				// Serial the animation
				animFile.serial (file);

				// All is good
				bRet=true;
			}
			catch (Exception& e)
			{
				MessageBox (NULL, e.what(), "NeL export", MB_OK|MB_ICONEXCLAMATION);
			}
		}
		else
		{
			MessageBox (NULL, "Can't open the file for writing.", "NeL export", MB_OK|MB_ICONEXCLAMATION);
		}
	}
	return bRet;
}

// --------------------------------------------------

bool CNelExport::exportSkeleton	(const char *sPath, INode* pNode, Interface& ip, TimeValue time)
{
	// Result to return
	bool bRet=false;

	// Build the skeleton format
	CSkeletonShape *skeletonShape=new CSkeletonShape();
	TInodePtrInt mapId;
	CExportNel::buildSkeletonShape (*skeletonShape, *pNode, NULL, mapId, time);

	// Open a file
	COFile file;
	if (file.open (sPath))
	{
		try
		{
			// Create a streamable shape
			CShapeStream shapeStream (skeletonShape);
			
			// Serial the shape
			shapeStream.serial (file);

			// All is good
			bRet=true;
		}
		catch (...)
		{
		}
	}

	// Delete the pointer
	delete skeletonShape;

	return bRet;
}

// --------------------------------------------------

