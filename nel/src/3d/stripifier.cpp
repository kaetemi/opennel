/** \file stripifier.cpp
 * <File description>
 *
 * $Id: stripifier.cpp,v 1.2 2001/10/31 10:18:26 berenguier Exp $
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

#include "3d/stripifier.h"
// For now, don't use NVidia stripifier.
//#include "nv_tri_strip_objects.h"
#include <vector>
#include <deque>


using namespace std;

namespace NL3D 
{


// ***************************************************************************
CStripifier::CStripifier()
{
}


// ***************************************************************************
/*
	NVidia(tm) 's method get better performance (8ms on 50K faces meshe, instead of 9.2ms), but 
	precomputing is much slower (1'40 instead of 0'??  :)  ).
*/
/*void		CStripifier::optimizeTriangles(const CPrimitiveBlock &in, CPrimitiveBlock &out, uint cacheSize)
{
	NvStripifier	stripifier;
	WordVec			inIndices;
	NvStripInfoVec	outStrips;
	sint			i;

	// prepare inIndices.
	inIndices.resize(in.getNumTri()*3);
	for(i=0;i< (sint)inIndices.size(); i++)
	{
		inIndices[i]= in.getTriPointer()[i];
	}

	// build strips.
	stripifier.Stripify(inIndices, cacheSize, outStrips);

	// build triangles from strips, and release memory
	out.setNumTri(0);
	out.reserveTri(in.getNumTri());
	for(i= 0;i< (sint)outStrips.size(); i++)
	{
		NvStripInfo		*stripInfo= outStrips[i];

		// build triangle from the strip.
		for(uint j= 0;j< stripInfo->m_faces.size(); j++)
		{
			NvFaceInfo	*face= stripInfo->m_faces[j];
			out.addTri(face->m_v0, face->m_v1, face->m_v2);
			// delete this face.
			delete face;
		}

		// delete this strip.
		// Unref first the edges touched by this strip.
		NvEdgeInfo	*edgeInfo = stripInfo->m_startInfo.m_startEdge;
		while(edgeInfo)
		{
			NvEdgeInfo	*edgeInfoNext= edgeInfo->m_nextV1;
			edgeInfo->Unref();
			edgeInfo= edgeInfoNext;
		}
		// delete
		delete stripInfo;
	}

}*/


// ***************************************************************************
struct	CVertexCache
{
	CVertexCache(sint cacheSize, sint nbVerts)
	{
		_VertexInCache.resize(nbVerts, 0);
		_Cache.resize(cacheSize, 0xFFFFFFFF);
	}

	void	touchVertex(uint vert)
	{
		if(isVertexInCache(vert))
		{
			// do nothing ?????? depends of vcache implementation
		}
		else
		{
			// pop front
			uint	removed= _Cache.front();
			if(removed!=0xFFFFFFFF)
				_VertexInCache[removed]= 0;
			_Cache.pop_front();
			// push_back
			_VertexInCache[vert]= 1;
			_Cache.push_back(vert);
		}
	}

	bool	isVertexInCache(uint vert)
	{
		return _VertexInCache[vert]!=0;
	}

	// return which vertex is at which place in the cache. 0xFFFFFFFF if the entry is empty
	uint	getVertexInCache(uint vertIdInCache)
	{
		return _Cache[vertIdInCache];
	}


private:
	// 0 if not in the cache
	vector<uint8>		_VertexInCache;
	deque<uint32>		_Cache;

};


// ***************************************************************************
struct	COrderFace
{
	sint	v[3];
	bool	Inserted;

	void	insertInPB(CPrimitiveBlock &out, CVertexCache &vertexCache)
	{
		out.addTri(v[0], v[1], v[2]);
		vertexCache.touchVertex(v[0]);
		vertexCache.touchVertex(v[1]);
		vertexCache.touchVertex(v[2]);
		Inserted= true;
	}

	sint	countCacheMiss(CVertexCache &vertexCache)
	{
		sint	ret=0 ;
		if(!vertexCache.isVertexInCache(v[0]))	ret++;
		if(!vertexCache.isVertexInCache(v[1]))	ret++;
		if(!vertexCache.isVertexInCache(v[2]))	ret++;
		return ret;
	}
};


// ***************************************************************************
struct	CCornerNode
{
	// next in the list
	CCornerNode		*Next;
	// corner == tuple face/vertex.
	uint			FaceId;
	uint			VertexId;
};


// ***************************************************************************
void		CStripifier::optimizeTriangles(const CPrimitiveBlock &in, CPrimitiveBlock &out, uint cacheSize)
{
	vector<COrderFace>	inFaces;
	sint			i;
	sint			numTris= in.getNumTri();

	// prepare inIndices.
	//--------------------
	inFaces.resize(numTris);
	for(i=0;i< numTris; i++)
	{
		inFaces[i].v[0]= in.getTriPointer()[i*3 + 0];
		inFaces[i].v[1]= in.getTriPointer()[i*3 + 1];
		inFaces[i].v[2]= in.getTriPointer()[i*3 + 2];
		inFaces[i].Inserted= false;
	}


	// build our cache, and compute max number of vertices.
	//--------------------
	int	numVerts=0;
	for (i = 0; i < numTris; i++)
	{
		numVerts= max(numVerts, (int)inFaces[i].v[0]);
		numVerts= max(numVerts, (int)inFaces[i].v[1]);
		numVerts= max(numVerts, (int)inFaces[i].v[2]);
	}
	numVerts++;
	CVertexCache	vertexCache(cacheSize, numVerts);


	// Compute vertex connectivity.
	//--------------------
	vector<CCornerNode*>	vertexConnectivity;
	vector<CCornerNode>		cornerAllocator;
	cornerAllocator.resize(numTris * 3);
	vertexConnectivity.resize(numVerts, NULL);
	// For all triangles.
	for (i = 0; i < numTris; i++)
	{
		COrderFace	*ordFace= &inFaces[i];
		// For each corner, allocate and fill
		for(sint j=0; j<3;j++)
		{
			sint	vertexId= ordFace->v[j];

			// allocate a corner
			CCornerNode		*corner= &cornerAllocator[i*3 + j];

			// fill it.
			corner->FaceId= i;
			corner->VertexId= vertexId;
			// Link it to the vertex list of faces.
			corner->Next= vertexConnectivity[vertexId];
			vertexConnectivity[vertexId]= corner;
		}
	}


	// build output optimized triangles
	//--------------------
	out.setNumTri(0);
	out.reserveTri(numTris);

	for(i=0; i<numTris; i++)
	{
		// force insertion of the ith face.
		sint	nextToInsert= i;
		bool	nextToInsertFound= true;
		while( nextToInsertFound )
		{
			nextToInsertFound= false;

			// if the face is not yet inserted.
			if(!inFaces[nextToInsert].Inserted)
			{
				// must insert this face.
				inFaces[nextToInsert].insertInPB(out, vertexCache);

				sint	minC= 3;

				// look only for faces which use vertices in VertexCache, to get a face with at least one vertex.
				for(uint j=0; j<cacheSize; j++)
				{
					// get a vertex from the vertex cache.
					uint	vertexId= vertexCache.getVertexInCache(j);
					// if empty entry
					if(vertexId==0xFFFFFFFF)
						continue;

					// parse list of faces which use this vertex.
					CCornerNode		*corner= vertexConnectivity[vertexId];
					while(corner)
					{
						uint	faceId= corner->FaceId;

						// if the face is not yet inserted.
						if(!inFaces[faceId].Inserted)
						{
							sint	c= inFaces[faceId].countCacheMiss(vertexCache);
							// insert first any face which don't add any vertex in the cache.
							if(c==0)
							{
								inFaces[faceId].insertInPB(out, vertexCache);
							}
							// else the one which add the minimum of vertex possible: nextToInsert
							else
							{
								if(c<minC)
								{
									nextToInsert= faceId;
									nextToInsertFound= true;
									minC= c;
								}
							}
						}

						// next corner
						corner= corner->Next;
					}

				}

				// if nextToInsertFound, then nextToInsert has the face which add the minimum of vertex possible in the cache
			}
		}

	}

}


} // NL3D
