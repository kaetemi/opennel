/** \file export_vegetable.cpp
 * Export from 3dsmax a NeL vegetable mesh
 *
 * $Id: export_vegetable.cpp,v 1.2 2001/12/11 10:19:55 corvazier Exp $
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

#include "stdafx.h"
#include "export_nel.h"
#include "export_lod.h"

#include "../nel_export/std_afx.h"
#include "../nel_export/nel_export.h"

#include <3d/vegetable_shape.h>

using namespace NLMISC;
using namespace NL3D;

// ***************************************************************************

bool  CExportNel::buildVegetableShape (NL3D::CVegetableShape& skeletonShape, INode& node, TimeValue time, Interface *ip, bool view, bool messageError)
{
	// Is it a tri object ?
	bool res = false;

	// Get a pointer on the object's node
    Object *obj = node.EvalWorldState(time).obj;

	// Check if there is an object
	if (obj)
	{
		if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) 
		{ 
			// Get a triobject from the node
			TriObject *tri = (TriObject *) obj->ConvertToType(time, Class_ID(TRIOBJ_CLASS_ID, 0));

			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			bool deleteIt=false;
			if (obj != tri) 
				deleteIt = true;

			// Build a mesh base structure
			CMeshBase::CMeshBaseBuild buildBaseMesh;
			CMaxMeshBaseBuild maxBaseBuild;

			// Get the node matrix
			Matrix3 nodeMatrixMax;
			CMatrix nodeMatrix;
			getLocalMatrix (nodeMatrixMax, node, time);
			convertMatrix (nodeMatrix, nodeMatrixMax);

			buildBaseMeshInterface (buildBaseMesh, maxBaseBuild, node, time, view, nodeMatrix);

			// Build a mesh
			CMesh::CMeshBuild buildMesh;
			buildMeshInterface (*tri, buildMesh, buildBaseMesh, maxBaseBuild, node, time, NULL, view);

			// Has UV 1
			if ((buildMesh.VertexFlags & CVertexBuffer::TexCoord0Flag) == 0)
			{
				// Error
				outputErrorMessage (ip, "Can't build vegetable mesh: need UV1 coordinates", "NeL Export", messageError);
			}
			else
			{
				// Build a mesh
				CMesh mesh;
				mesh.build (buildBaseMesh, buildMesh);

				// Number of matrix block
				if (mesh.getNbMatrixBlock () != 1)
				{
					// Error
					outputErrorMessage (ip, "The object can't be skinned", "NeL Export", messageError);
				}
				else
				{
					// Number of render pass
					if (mesh.getNbRdrPass (0) != 1)
					{
						// Error
						outputErrorMessage (ip, "The object must have less than one material!", "NeL Export", messageError);
					}
					else
					{
						// Build a vegetable mesh
						CVegetableShapeBuild vegetableBuild;

						// Copy the vertex buffer
						vegetableBuild.VB = mesh.getVertexBuffer ();

						// Copy the primitive block
						vegetableBuild.PB = mesh.getRdrPassPrimitiveBlock (0, 0);

						// Get the appdata
						vegetableBuild.AlphaBlend = CExportNel::getScriptAppData (&node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND, 0) == 0;

						// Alpha blend ?
						if (vegetableBuild.AlphaBlend)
						{
							// Default options
							vegetableBuild.PreComputeLighting = true;
							vegetableBuild.DoubleSided = true;

							// Lighted ?
							vegetableBuild.Lighted = CExportNel::getScriptAppData (&node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_ON_LIGHTED, 0) == 0;
						}
						else
						{
							// Lighted ?
							vegetableBuild.Lighted = CExportNel::getScriptAppData (&node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_LIGHTED, 0) != 2;

							// Precompute light ?
							vegetableBuild.PreComputeLighting = CExportNel::getScriptAppData (&node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_LIGHTED, 0) == 1;

							// Double sided ?
							vegetableBuild.DoubleSided = CExportNel::getScriptAppData (&node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_DOUBLE_SIDED, 0) != BST_UNCHECKED;

							// Lighted ?
							vegetableBuild.Lighted = CExportNel::getScriptAppData (&node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_ON_LIGHTED, 0) == 0;
						}

						// Max bend weight
						vegetableBuild.MaxBendWeight = CExportNel::getScriptAppData (&node, NEL3D_APPDATA_BEND_FACTOR, NEL3D_APPDATA_BEND_FACTOR_DEFAULT);

						// BendMode
						vegetableBuild.BendCenterMode = (CVegetableShapeBuild::TBendCenterMode)CExportNel::getScriptAppData (&node, NEL3D_APPDATA_BEND_CENTER, 0);

						// Build it
						skeletonShape.build (vegetableBuild);

						// Ok
						res = true;
					}
				}
			}

			if (deleteIt)
				delete tri;
		}
	}

	return res;
}

// ***************************************************************************
