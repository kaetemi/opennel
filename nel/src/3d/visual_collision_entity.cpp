/** \file visual_collision_entity.cpp
 * <File description>
 *
 * $Id: visual_collision_entity.cpp,v 1.2 2001/06/11 13:35:01 berenguier Exp $
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

#include "nel/3d/visual_collision_entity.h"
#include "nel/3d/landscape.h"


using namespace std;

namespace NL3D 
{


// ***************************************************************************
// should be at least 2 meters.
const	float	CVisualCollisionEntity::_BBoxRadius= 10;
const	float	CVisualCollisionEntity::_BBoxRadiusZ= 20;
const	uint32	CVisualCollisionEntity::_StartPatchQuadBlockSize= 64;	// 256 octets per entity minimum.
vector<CPatchBlockIdent>		CVisualCollisionEntity::_TmpBlockIds;
vector<CPatchQuadBlock*>		CVisualCollisionEntity::_TmpPatchQuadBlocks;


// ***************************************************************************
CVisualCollisionEntity::CVisualCollisionEntity(CVisualCollisionManager *owner) : _LandscapeQuadGrid(owner)
{
	_Owner= owner;
	_PatchQuadBlocks.reserve(_StartPatchQuadBlockSize);

	_CurrentBBox.setHalfSize(CVector::Null);
}


// ***************************************************************************
CVisualCollisionEntity::~CVisualCollisionEntity()
{
	// delete the _PatchQuadBlocks.
	for(sint i=0; i<(sint)_PatchQuadBlocks.size(); i++)
	{
		_Owner->deletePatchQuadBlock(_PatchQuadBlocks[i]);
	}
	_PatchQuadBlocks.clear();

	// delete the quadgrid.
	_LandscapeQuadGrid.clear();
}


// ***************************************************************************
void		CVisualCollisionEntity::snapToGround(CVector &pos)
{
	// update the cahe of tile info near this position.
	// =================
	testComputeLandscape(pos);


	// find possible faces under the entity.
	// =================
	CVisualTileDescNode		*ptr= _LandscapeQuadGrid.select(pos); 


	// find the better face under the entity.
	// =================
	float	sqrBestDist= sqr(1000.f);
	CVector	res, hit;
	// build the vertical ray.
	CVector		segP0= pos - CVector(0,0,100);
	CVector		segP1= pos + CVector(0,0,100);


	// triangles builded from this list.
	static	vector<CTrianglePatch>		testTriangles;
	// NB: not so many reallocation here, because static.
	testTriangles.clear();
	sint	bestTriangle= 0;


	// For all the faces in this quadgrid node.
	while(ptr)
	{
		// what is the quad block of this tile Id.
		sint				qbId= ptr->PatchQuadBlocId;
		nlassert(qbId>=0 && qbId<(sint)_PatchQuadBlocks.size());
		CPatchQuadBlock		&qb= *_PatchQuadBlocks[qbId];

		// Build the 2 triangles of this tile Id.
		sint	idStart= testTriangles.size();
		testTriangles.resize(idStart+2);
		qb.buildTileTriangles((uint8)ptr->QuadId, &testTriangles[idStart]);

		// Test the 2 triangles.
		for(sint i=0; i<2; i++)
		{
			CTrianglePatch	&tri= testTriangles[idStart+i];
			// test if the ray intersect.
			// NB: triangleIntersect() is faster than CTriangle::intersect().
			if(triangleIntersect(tri, segP0, segP1, hit))
			{
				// find the nearest triangle.
				float sqrdist= (hit-pos).sqrnorm();
				if(sqrdist<sqrBestDist)
				{
					bestTriangle= idStart+i;
					res= hit;
					sqrBestDist= sqrdist;
				}
			}
		}


		// Next in the list.
		ptr= ptr->Next;
	}

	// result. NB: if not found, dot not modify.
	if(sqrBestDist<sqr(1000))
	{
		// snap the position to the highest tesselation.
		pos= res;
		// snap the position to the current rendered tesselation.
		snapToLandscapeCurrentTesselation(pos, testTriangles[bestTriangle]);
	}
}


// ***************************************************************************
void		CVisualCollisionEntity::snapToLandscapeCurrentTesselation(CVector &pos, const CTrianglePatch &tri)
{
	// Must find the Uv under the position.
	// compute UV gradients.
	CVector		Gu;
	CVector		Gv;
	tri.computeGradient(tri.Uv0.U, tri.Uv1.U, tri.Uv2.U, Gu);
	tri.computeGradient(tri.Uv0.V, tri.Uv1.V, tri.Uv2.V, Gv);
	// compute UV for position.
	CUV		uv;
	uv.U= tri.Uv0.U + Gu*(pos-tri.V0);
	uv.V= tri.Uv0.V + Gv*(pos-tri.V0);

	// Ask pos to landscape.
	CVector		posLand;
	posLand= _Owner->_Landscape->getTesselatedPos(tri.PatchId, uv);

	// just keep Z.
	pos.z= posLand.z;
}


// ***************************************************************************
bool		CVisualCollisionEntity::triangleIntersect(CTriangle &tri, const CVector &pos0, const CVector &pos1, CVector &hit)
{
	CVector		&p0= tri.V0;
	CVector		&p1= tri.V1;
	CVector		&p2= tri.V2;

	// Test if the face enclose the pos in X/Y plane.
	// NB: compute and using a BBox to do a rapid test is not a very good idea, since it will 
	// add an overhead which is NOT negligeable compared to the following test.
	float		a,b,c;		// 2D cartesian coefficients of line in plane X/Y.
	// Line p0-p1.
	a= -(p1.y-p0.y);
	b= (p1.x-p0.x);
	c= -(p0.x*a + p0.y*b);
	if( (a*pos0.x + b*pos0.y + c) < 0)	return false;
	// Line p1-p2.
	a= -(p2.y-p1.y);
	b= (p2.x-p1.x);
	c= -(p1.x*a + p1.y*b);
	if( (a*pos0.x + b*pos0.y + c) < 0)	return false;
	// Line p2-p0.
	a= -(p0.y-p2.y);
	b= (p0.x-p2.x);
	c= -(p2.x*a + p2.y*b);
	if( (a*pos0.x + b*pos0.y + c) < 0)	return false;

	// Compute the possible height.
	CVector		tmp;
	// build the plane
	CPlane plane;
	plane.make (p0, p1, p2);
	// intersect the vertical line with the plane.
	tmp= plane.intersect(pos0, pos1);

	float		h= tmp.z;
	// Test if it would fit in the wanted field.
	if(h>pos1.z)	return false;
	if(h<pos0.z)	return false;

	// OK!!
	hit= tmp;
	return true;
}

// ***************************************************************************
void		CVisualCollisionEntity::testComputeLandscape(const CVector &pos)
{
	// if new position is out of the bbox surounding the entity.
	if(_CurrentBBox.getHalfSize()==CVector::Null || !_CurrentBBox.include(pos))
	{
		// must recompute the data around the entity.
		doComputeLandscape(pos);
	}
}

// ***************************************************************************
void		CVisualCollisionEntity::doComputeLandscape(const CVector &pos)
{
	sint	i;

	// setup new bbox.
	//==================
	_CurrentBBox.setCenter(pos);
	_CurrentBBox.setHalfSize(CVector(_BBoxRadius, _BBoxRadius, _BBoxRadiusZ));

	// Search landscape blocks which are in the bbox.
	//==================
	_Owner->_Landscape->buildPatchBlocksInBBox(_CurrentBBox, _TmpBlockIds);



	// Recompute PatchQuadBlockcs.
	//==================
	// This parts try to keeps old patch blocks so they are not recomputed if they already here.

	// sort PatchBlockIdent.
	sort(_TmpBlockIds.begin(), _TmpBlockIds.end());

	// Copy old array of ptr (ptr copy only).
	_TmpPatchQuadBlocks= _PatchQuadBlocks;

	// allocate dest array.
	_PatchQuadBlocks.resize(_TmpBlockIds.size());

	// Traverse all current patchBlocks, deleting old ones no longer needed, and creating new ones.
	// this algorithm suppose both array are sorted.
	uint	iOld=0;
	// parse until dest is filled.
	for(i=0; i<(sint)_PatchQuadBlocks.size();)
	{
		// get requested new BlockIdent.
		CPatchBlockIdent	newBi= _TmpBlockIds[i];

		// get requested old BlockIdent in the array.
		bool				oldEnd= false;
		CPatchBlockIdent	oldBi;
		if(iOld==_TmpPatchQuadBlocks.size())
			oldEnd= true;
		else
			oldBi= _TmpPatchQuadBlocks[iOld]->PatchBlockId;

		// if no more old blocks, or if new Block is < than current, we must create a new block, and insert it.
		if(oldEnd || newBi < oldBi)
		{
			// allocate the patch block.
			_PatchQuadBlocks[i]= _Owner->newPatchQuadBlock();
			// fill the patch block.
			_PatchQuadBlocks[i]->PatchBlockId= _TmpBlockIds[i];
			_Owner->_Landscape->fillPatchQuadBlock(*_PatchQuadBlocks[i]);

			// next new patch block.
			i++;
		}
		// else if current new Block is same than old block, just copy ptr.
		else if(newBi==oldBi)
		{
			// just copy ptr with the old one.
			_PatchQuadBlocks[i]= _TmpPatchQuadBlocks[iOld];

			// next new and old patch block.
			i++;
			iOld++;
		}
		// else, this old block is no longer used, delete it.
		else
		{
			_Owner->deletePatchQuadBlock(_TmpPatchQuadBlocks[iOld]);
			// next old patch block.
			iOld++;
		}
	}
	// Here, must delete old blocks not yet processed.
	for(;iOld<_TmpPatchQuadBlocks.size(); iOld++)
	{
		_Owner->deletePatchQuadBlock(_TmpPatchQuadBlocks[iOld]);
	}
	_TmpPatchQuadBlocks.clear();


	// Compute the quadGrid.
	//==================

	// Compute a delta so elt position for CLandscapeCollisionGrid are positive, and so fastFloor() used will work.
	CVector		delta;
	// center the position on 0.
	// floor may be important for precision when the delta is applied.
	delta.x= (float)floor(-pos.x);
	delta.y= (float)floor(-pos.y);
	delta.z= 0;
	// We must always have positive values for patchBlocks vertices.
	float	val= (float)ceil(_BBoxRadius + 256);
	// NB: 256 is a security. Because of size of patchs, a value of 32 at max should be sufficient (64 for bigger patch (gfx))
	// we are large because doesn't matter, the CLandscapeCollisionGrid tiles.
	delta.x+= val;
	delta.y+= val;

	// rebuild the quadGrid.
	_LandscapeQuadGrid.build(_PatchQuadBlocks, delta);

}


} // NL3D
