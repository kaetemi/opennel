/** \file raw_skin.h
 * Packed struct used for faster Software skinning
 *
 * $Id: raw_skin.h,v 1.4 2004/03/19 10:11:36 corvazier Exp $
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

#ifndef NL_RAW_SKIN_H
#define NL_RAW_SKIN_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/uv.h"
#include "nel/misc/object_vector.h"
#include "3d/mesh.h"
#include "3d/mrm_mesh.h"


namespace NL3D 
{


using	NLMISC::CVector;
using	NLMISC::CUV;

/// A simple Vertex Pos/Normal/Uv
class	CRawSkinVertex
{
public:
	CVector		Pos;
	CVector		Normal;
	CUV			UV;
};

/// Vertices influenced by 1 matrix only.
class	CRawVertexNormalSkin1
{
public:
	// The id of the matrix to use.
	uint32			MatrixId[1];
	CRawSkinVertex	Vertex;
};

/// Vertices influenced by 2 matrix only.
class	CRawVertexNormalSkin2
{
public:
	// The id of the matrix to use.
	uint32			MatrixId[2];
	float			Weights[2];
	CRawSkinVertex	Vertex;
};

/// Vertices influenced by 3 matrix only.
class	CRawVertexNormalSkin3
{
public:
	uint32			MatrixId[3];
	float			Weights[3];
	CRawSkinVertex	Vertex;
};

/// Vertices influenced by 4 matrix only.
class	CRawVertexNormalSkin4
{
public:
	uint32			MatrixId[4];
	float			Weights[4];
	CRawSkinVertex	Vertex;
};

/// The array per lod.
class	CRawSkinNormalCache
{
public:
	// The vertices influenced by 1 matrix.
	NLMISC::CObjectVector<CRawVertexNormalSkin1, false>	Vertices1;
	// The vertices influenced by 2 matrix.
	NLMISC::CObjectVector<CRawVertexNormalSkin2, false>	Vertices2;
	// The vertices influenced by 3 matrix.
	NLMISC::CObjectVector<CRawVertexNormalSkin3, false>	Vertices3;
	// The vertices influenced by 4 matrix.
	NLMISC::CObjectVector<CRawVertexNormalSkin4, false>	Vertices4;

	// For Each array, set the max number of vertices to copy in VBSoft (not VBHard directly)
	uint32							SoftVertices[4];
	uint32							HardVertices[4];
	// Total Of SoftVertices
	uint32							TotalSoftVertices;
	uint32							TotalHardVertices;

	// The RawSkin Geomorphs.
	std::vector<CMRMWedgeGeom>		Geomorphs;
	// The Raw Primitives.
	std::vector<CIndexBuffer>	RdrPass;

	/// What RawSkin lod this cache represent. -1 if NULL
	sint					LodId;
	/// To see if same Data than in the CMeshMRMGeom
	uint					MeshDataId;


	// Used only in case of Morphing. Same Size as the original VB. Remap the original VB to the RawSkin.
	NLMISC::CObjectVector<CRawSkinVertex*, false>		VertexRemap;

public:
	// free up the memory
	void				clearArrays();
};



} // NL3D


#endif // NL_RAW_SKIN_H

/* End of raw_skin.h */
