/** \file vegetable_shape.cpp
 * <File description>
 *
 * $Id: vegetable_shape.cpp,v 1.8 2002/04/04 14:45:32 berenguier Exp $
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

#include "std3d.h"

#include "3d/vegetable_shape.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"


using namespace std;
using namespace NLMISC;


namespace NL3D 
{


// ***************************************************************************
CVegetableShape::CVegetableShape()
{
	Lighted= false;
	DoubleSided= false;
	PreComputeLighting= false;
	AlphaBlend= false;
	BestSidedPreComputeLighting= false;
}

// ***************************************************************************
void		CVegetableShape::build(CVegetableShapeBuild &vbuild)
{
	// Must have TexCoord0.
	nlassert( vbuild.VB.getVertexFormat() & CVertexBuffer::TexCoord0Flag );

	// Header
	//---------

	// Lighted ?
	if(vbuild.Lighted && ( vbuild.VB.getVertexFormat() & CVertexBuffer::NormalFlag) )
		Lighted= true;
	else
		Lighted= false;

	// DoubleSided
	DoubleSided= vbuild.DoubleSided;

	// PreComputeLighting.
	PreComputeLighting= Lighted && vbuild.PreComputeLighting;

	// AlphaBlend: valid only for 2Sided and Unlit (or similar PreComputeLighting) mode
	AlphaBlend= vbuild.AlphaBlend && DoubleSided && (!Lighted || PreComputeLighting);

	// BestSidedPreComputeLighting
	BestSidedPreComputeLighting= PreComputeLighting && vbuild.BestSidedPreComputeLighting;

	// BendCenterMode
	BendCenterMode= vbuild.BendCenterMode;

	// Format of the VB.
	uint32	format;
	format= CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag | CVertexBuffer::TexCoord1Flag;
	// lighted?
	if(Lighted)
		format|= CVertexBuffer::NormalFlag;
	// set VB.
	VB.setVertexFormat(format);


	// Fill triangles.
	//---------
	uint	i;
	// resisz
	TriangleIndices.resize(vbuild.PB.getNumTri() * 3);
	uint32	*srcTri= vbuild.PB.getTriPointer();
	// fill
	for(i=0; i<vbuild.PB.getNumTri(); i++)
	{
		TriangleIndices[i*3+0]= *(srcTri++);
		TriangleIndices[i*3+1]= *(srcTri++);
		TriangleIndices[i*3+2]= *(srcTri++);
	}


	// Fill vertices.
	//---------
	// resize
	uint32		nbVerts= vbuild.VB.getNumVertices();
	VB.setNumVertices(nbVerts);
	// if no vertex color, 
	float	maxZ= 0;
	bool	bendFromColor= true;
	if(! (vbuild.VB.getVertexFormat() & CVertexBuffer::PrimaryColorFlag) )
	{
		// must compute bendWeight from z.
		bendFromColor= false;
		// get the maximum Z.
		for(i=0;i<nbVerts;i++)
		{
			float	z= ((CVector*)vbuild.VB.getVertexCoordPointer(i))->z;
			maxZ= max(z, maxZ);
		}
		// if no positive value, bend will always be 0.
		if(maxZ==0)
			maxZ= 1;
	}

	// For all vertices, fill
	for(i=0;i<nbVerts;i++)
	{
		// Position.
		CVector		*srcPos= (CVector*)vbuild.VB.getVertexCoordPointer(i);
		CVector		*dstPos= (CVector*)VB.getVertexCoordPointer(i);
		*dstPos= *srcPos;

		// Normal
		if(Lighted)
		{
			CVector		*srcNormal= (CVector*)vbuild.VB.getNormalCoordPointer(i);
			CVector		*dstNormal= (CVector*)VB.getNormalCoordPointer(i);
			*dstNormal= *srcNormal;
		}

		// Texture.
		CUV		*srcUV= (CUV*)vbuild.VB.getTexCoordPointer(i, 0);
		CUV		*dstUV= (CUV*)VB.getTexCoordPointer(i, 0);
		*dstUV= *srcUV;

		// Bend.
		// copy to texture stage 1.
		CUV		*dstUVBend= (CUV*)VB.getTexCoordPointer(i, 1);
		if(bendFromColor)
		{
			CRGBA	*srcColor= (CRGBA*)vbuild.VB.getColorPointer(i);
			// Copy and scale by MaxBendWeight
			dstUVBend->U= (srcColor->R / 255.f) * vbuild.MaxBendWeight;
		}
		else
		{
			float	w= srcPos->z / maxZ;
			w= max(w, 0.f);
			// Copy and scale by MaxBendWeight
			dstUVBend->U= w * vbuild.MaxBendWeight;
		}
	}


	// Misc.
	//---------
	// prepare for instanciation
	InstanceVertices.resize(VB.getNumVertices());

}

// ***************************************************************************
void		CVegetableShape::loadShape(const std::string &shape)
{
	string	path= CPath::lookup(shape);
	// read this file
	CIFile	f(path);
	serial(f);
}

// ***************************************************************************
void		CVegetableShape::serial(NLMISC::IStream &f)
{
	/*
	Version 1: 
		- BestSidedPreComputeLighting
	*/
	sint	ver= f.serialVersion(1);
	f.serialCheck((uint32)'_LEN');
	f.serialCheck((uint32)'GEV_');
	f.serialCheck((uint32)'BATE');
	f.serialCheck((uint32)'__EL');

	f.serial(Lighted);
	f.serial(DoubleSided);
	f.serial(PreComputeLighting);
	f.serial(AlphaBlend);
	f.serialEnum(BendCenterMode);
	f.serial(VB);
	f.serialCont(TriangleIndices);

	if(ver>=1)
		f.serial(BestSidedPreComputeLighting);
	else if(f.isReading())
		BestSidedPreComputeLighting= false;

	// if reading
	if(f.isReading())
	{
		// prepare for instanciation
		InstanceVertices.resize(VB.getNumVertices());
	}
}



} // NL3D
