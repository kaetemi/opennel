/** \file mesh_mrm.cpp
 * <File description>
 *
 * $Id: mesh_mrm.cpp,v 1.20 2001/09/06 07:25:37 corvazier Exp $
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

#include "3d/mesh_mrm.h"
#include "3d/mrm_builder.h"
#include "3d/mrm_parameters.h"
#include "3d/mesh_mrm_instance.h"
#include "3d/scene.h"
#include "3d/skeleton_model.h"
#include "nel/misc/bsphere.h"


using namespace NLMISC;
using namespace std;


namespace NL3D 
{



// ***************************************************************************
// ***************************************************************************
// CMeshMRMGeom::CLod
// ***************************************************************************
// ***************************************************************************

	
// ***************************************************************************
void		CMeshMRMGeom::CLod::buildSkinVertexBlocks()
{
	contReset(SkinVertexBlocks);


	// The list of vertices. true if used by this lod.
	vector<bool>		vertexMap;
	vertexMap.resize(NWedges, false);


	// from InfluencedVertices, aknoledge what vertices are used.
	uint	i;
	for(i=0;i<NL3D_MESH_SKINNING_MAX_MATRIX;i++)
	{
		uint		nInf= InfluencedVertices[i].size();
		if( nInf==0 )
			continue;
		uint32		*infPtr= &(InfluencedVertices[i][0]);

		//  for all InfluencedVertices only.
		for(;nInf>0;nInf--, infPtr++)
		{
			uint	index= *infPtr;
			vertexMap[index]= true;
		}
	}

	// For all vertices, test if they are used, and build the according SkinVertexBlocks;
	CVertexBlock	*vBlock= NULL;
	for(i=0; i<vertexMap.size();i++)
	{
		if(vertexMap[i])
		{
			// preceding block?
			if(vBlock)
			{
				// yes, extend it.
				vBlock->NVertices++;
			}
			else
			{
				// no, append a new one.
				SkinVertexBlocks.push_back();
				vBlock= &SkinVertexBlocks[SkinVertexBlocks.size()-1];
				vBlock->VertexStart= i;
				vBlock->NVertices= 1;
			}
		}
		else
		{
			// Finish the preceding block (if any).
			vBlock= NULL;
		}
	}

}


// ***************************************************************************
// ***************************************************************************
// CMeshMRMGeom.
// ***************************************************************************
// ***************************************************************************




// ***************************************************************************
static	NLMISC::CAABBoxExt	makeBBox(const std::vector<CVector>	&Vertices)
{
	NLMISC::CAABBox		ret;
	nlassert(Vertices.size());
	ret.setCenter(Vertices[0]);
	for(sint i=0;i<(sint)Vertices.size();i++)
	{
		ret.extend(Vertices[i]);
	}

	return ret;
}


// ***************************************************************************
CMeshMRMGeom::CMeshMRMGeom()
{
	_VertexBufferHardDirty= true;
	_Skinned= false;
	_NbLodLoaded= 0;
}


// ***************************************************************************
CMeshMRMGeom::~CMeshMRMGeom()
{
	deleteVertexBufferHard();
}


// ***************************************************************************
void			CMeshMRMGeom::build(CMesh::CMeshBuild &m, uint numMaxMaterial, const CMRMParameters &params)
{

	// Dirt the VBuffer.
	_VertexBufferHardDirty= true;

	// Empty geometry?
	if(m.Vertices.size()==0 || m.Faces.size()==0)
	{
		_VBuffer.setNumVertices(0);
		_VBuffer.reserve(0);
		_Lods.clear();
		_BBox.setCenter(CVector::Null);
		_BBox.setSize(CVector::Null);
		return;
	}
	nlassert(numMaxMaterial>0);


	/// 0. First, make bbox.
	//======================
	// NB: this is equivalent as building BBox from MRM VBuffer, because CMRMBuilder create new vertices 
	// which are just interpolation of original vertices.
	_BBox= makeBBox(m.Vertices);


	/// 1. Launch the MRM build process.
	//================================================
	CMRMBuilder			mrmBuilder;
	CMeshBuildMRM		meshBuildMRM;

	mrmBuilder.compileMRM(m, params, meshBuildMRM, numMaxMaterial);

	// Then just copy result!
	//================================================
	_VBuffer= meshBuildMRM.VBuffer;
	_Lods= meshBuildMRM.Lods;
	_Skinned= meshBuildMRM.Skinned;
	_SkinWeights= meshBuildMRM.SkinWeights;


	// Compute degradation control.
	//================================================
	_DistanceFinest= meshBuildMRM.DistanceFinest;
	_DistanceMiddle= meshBuildMRM.DistanceMiddle;
	_DistanceCoarsest= meshBuildMRM.DistanceCoarsest;
	nlassert(_DistanceFinest>=0);
	nlassert(_DistanceMiddle > _DistanceFinest);
	nlassert(_DistanceCoarsest > _DistanceMiddle);
	// Compute _OODistDelta.
	_OODistanceDelta= 1.0f / (_DistanceCoarsest - _DistanceFinest);
	/* Compute exponent pow, such that 0.5= dMiddle^pow;
		ie 0.5= e(ln dMiddle * pow)
	*/
	float	dMiddle= (_DistanceCoarsest - _DistanceMiddle) * _OODistanceDelta;
	_DistancePow= (float)(log(0.5) / log(dMiddle));



	// Build the _LodInfos.
	//================================================
	_LodInfos.resize(_Lods.size());
	uint32	precNWedges= 0;
	uint	i;
	for(i=0;i<_Lods.size();i++)
	{
		_LodInfos[i].StartAddWedge= precNWedges;
		_LodInfos[i].EndAddWedges= _Lods[i].NWedges;
		precNWedges= _Lods[i].NWedges;
		// LodOffset is filled in serial() when stream is input.
	}
	// After build, all lods are present in memory. 
	_NbLodLoaded= _Lods.size();


	// For load balancing.
	//================================================
	// compute Max Face Used
	_MaxFaceUsed= 0;
	_MinFaceUsed= 0;
	// Count of primitive block
	if(_Lods.size()>0)
	{
		uint	pb;
		// Compute MinFaces.
		CLod	&firstLod= _Lods[0];
		for (pb=0; pb<firstLod.RdrPass.size(); pb++)
		{
			CRdrPass &pass= firstLod.RdrPass[pb];
			// Sum tri
			_MinFaceUsed+= pass.PBlock.getNumTriangles ();
		}
		// Compute MaxFaces.
		CLod	&lastLod= _Lods[_Lods.size()-1];
		for (pb=0; pb<lastLod.RdrPass.size(); pb++)
		{
			CRdrPass &pass= lastLod.RdrPass[pb];
			// Sum tri
			_MaxFaceUsed+= pass.PBlock.getNumTriangles ();
		}
	}


	// For skinning.
	//================================================
	if( _Skinned )
	{
		bkupOriginalSkinVertices();
	}


	// For AGP SKinning optim.
	//================================================
	for(i=0;i<_Lods.size();i++)
	{
		_Lods[i].buildSkinVertexBlocks();
	}


}


// ***************************************************************************
float	CMeshMRMGeom::getLevelDetailFromDist(float dist)
{
	if(dist <= _DistanceFinest)
		return 1;
	if(dist >= _DistanceCoarsest)
		return 0;

	float	d= (_DistanceCoarsest - dist) * _OODistanceDelta;
	return  (float)pow(d, _DistancePow);
}


// ***************************************************************************
void	CMeshMRMGeom::applyGeomorph(std::vector<CMRMWedgeGeom>  &geoms, float alphaLod)
{
	// no geomorphs? quit.
	if(geoms.size()==0)
		return;

	uint		i;
	clamp(alphaLod, 0.f, 1.f);
	float		a= alphaLod;
	float		a1= 1 - alphaLod;
	uint		ua= (uint)(a*256);
	clamp(ua, (uint)0, (uint)256);
	uint		ua1= 256 - ua;


	// info from VBuffer.
	uint8		*vertexPtr= (uint8*)_VBuffer.getVertexCoordPointer();
	uint		flags= _VBuffer.getVertexFormat();
	sint32		vertexSize= _VBuffer.getVertexSize();
	// because of the unrolled code for 4 first UV, must assert this.
	nlassert(CVertexBuffer::MaxStage>=4);
	// must have XYZ.
	nlassert(flags & CVertexBuffer::PositionFlag);


	// If VBuffer Hard present
	uint8		*vertexDestPtr;
	if(_VBHard!=NULL)
	{
		// must write into it
		vertexDestPtr= (uint8*)_VBHard->lock();
		nlassert(vertexSize == _VBHard->getVertexSize());
	}
	else
	{
		// else write into vertexPtr.
		vertexDestPtr= vertexPtr;
	}


	// if an offset is 0, it means that the component is not in the VBuffer.
	sint32		normalOff;
	sint32		colorOff;
	sint32		specularOff;
	sint32		uvOff[CVertexBuffer::MaxStage];


	// Compute offset of each component of the VB.
	if(flags & CVertexBuffer::NormalFlag)
		normalOff= _VBuffer.getNormalOff();
	else
		normalOff= 0;
	if(flags & CVertexBuffer::PrimaryColorFlag)
		colorOff= _VBuffer.getColorOff();
	else
		colorOff= 0;
	if(flags & CVertexBuffer::SecondaryColorFlag)
		specularOff= _VBuffer.getSpecularOff();
	else
		specularOff= 0;
	for(i= 0; i<CVertexBuffer::MaxStage;i++)
	{
		if(flags & (CVertexBuffer::TexCoord0Flag<<i))
			uvOff[i]= _VBuffer.getTexCoordOff(i);
		else
			uvOff[i]= 0;
	}


	// For all geomorphs.
	uint			nGeoms= geoms.size();
	CMRMWedgeGeom	*ptrGeom= &(geoms[0]);
	uint8			*destPtr= vertexDestPtr;
	/* NB: optimisation: lot of "if" in this Loop, but because of BTB, they always cost nothing (prediction is good).
	   NB: this also is why we unroll the 4 1st Uv. The other (if any), are done in the other loop.
	   NB: optimisation for AGP write cominers: the order of write (vertex, normal, uvs...) is important for good
	   use of AGP write combiners.
	*/
	for(; nGeoms>0; nGeoms--, ptrGeom++, destPtr+= vertexSize )
	{
		uint8			*startPtr=	vertexPtr + ptrGeom->Start*vertexSize;
		uint8			*endPtr=	vertexPtr + ptrGeom->End*vertexSize;

		// Vertex.
		{
			CVector		*start=	(CVector*)startPtr;
			CVector		*end=	(CVector*)endPtr;
			CVector		*dst=	(CVector*)destPtr;
			*dst= *start * a + *end * a1;
		}

		// Normal.
		if(normalOff)
		{
			CVector		*start= (CVector*)(startPtr + normalOff);
			CVector		*end=	(CVector*)(endPtr   + normalOff);
			CVector		*dst=	(CVector*)(destPtr  + normalOff);
			*dst= *start * a + *end * a1;
		}


		// Uvs.
		// uv[0].
		if(uvOff[0])
		{
			// Uv.
			CUV			*start= (CUV*)(startPtr + uvOff[0]);
			CUV			*end=	(CUV*)(endPtr   + uvOff[0]);
			CUV			*dst=	(CUV*)(destPtr  + uvOff[0]);
			*dst= *start * a + *end * a1;
		}
		// uv[1].
		if(uvOff[1])
		{
			// Uv.
			CUV			*start= (CUV*)(startPtr + uvOff[1]);
			CUV			*end=	(CUV*)(endPtr   + uvOff[1]);
			CUV			*dst=	(CUV*)(destPtr  + uvOff[1]);
			*dst= *start * a + *end * a1;
		}
		// uv[2].
		if(uvOff[2])
		{
			// Uv.
			CUV			*start= (CUV*)(startPtr + uvOff[2]);
			CUV			*end=	(CUV*)(endPtr   + uvOff[2]);
			CUV			*dst=	(CUV*)(destPtr  + uvOff[2]);
			*dst= *start * a + *end * a1;
		}
		// uv[3].
		if(uvOff[3])
		{
			// Uv.
			CUV			*start= (CUV*)(startPtr + uvOff[3]);
			CUV			*end=	(CUV*)(endPtr   + uvOff[3]);
			CUV			*dst=	(CUV*)(destPtr  + uvOff[3]);
			*dst= *start * a + *end * a1;
		}


		// color.
		if(colorOff)
		{
			CRGBA		*start= (CRGBA*)(startPtr + colorOff);
			CRGBA		*end=	(CRGBA*)(endPtr   + colorOff);
			CRGBA		*dst=	(CRGBA*)(destPtr  + colorOff);
			dst->blendFromui(*start, *end,  ua1);
		}
		// specular.
		if(specularOff)
		{
			CRGBA		*start= (CRGBA*)(startPtr + specularOff);
			CRGBA		*end=	(CRGBA*)(endPtr   + specularOff);
			CRGBA		*dst=	(CRGBA*)(destPtr  + specularOff);
			dst->blendFromui(*start, *end,  ua1);
		}

	}


	// Process extra UVs (maybe never, so don't bother optims :)).
	// For all stages after 4.
	for(i=4;i<CVertexBuffer::MaxStage;i++)
	{
		uint			nGeoms= geoms.size();
		CMRMWedgeGeom	*ptrGeom= &(geoms[0]);
		uint8			*destPtr= vertexDestPtr;

		if(uvOff[i]==0)
			continue;

		// For all geomorphs.
		for(; nGeoms>0; nGeoms--, ptrGeom++, destPtr+= vertexSize )
		{
			uint8			*startPtr=	vertexPtr + ptrGeom->Start*vertexSize;
			uint8			*endPtr=	vertexPtr + ptrGeom->End*vertexSize;

			// uv[i].
			// Uv.
			CUV			*start= (CUV*)(startPtr + uvOff[i]);
			CUV			*end=	(CUV*)(endPtr	+ uvOff[i]);
			CUV			*dst=	(CUV*)(destPtr	+ uvOff[i]);
			*dst= *start * a + *end * a1;

		}
	}


	// If _VBHard here, unlock it.
	if(_VBHard)
	{
		_VBHard->unlock();
	}

}


// ***************************************************************************
bool	CMeshMRMGeom::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
{
	// Speed Clip: clip just the sphere.
	CBSphere	localSphere(_BBox.getCenter(), _BBox.getRadius());
	CBSphere	worldSphere;

	// transform the sphere in WorldMatrix (with nearly good scale info).
	localSphere.applyTransform(worldMatrix, worldSphere);

	// if out of only plane, entirely out.
	for(sint i=0;i<(sint)pyramid.size();i++)
	{
		// We are sure that pyramid has normalized plane normals.
		// if SpherMax OUT return false.
		float	d= pyramid[i]*worldSphere.Center;
		if(d>worldSphere.Radius)
			return false;
	}
	return true;
}


// ***************************************************************************
void	CMeshMRMGeom::render(IDriver *drv, CTransformShape *trans, bool passOpaque, float polygonCount, float globalAlpha)
{
	nlassert(drv);
	if(_Lods.size()==0)
		return;

	// get the meshMRM instance.
	CMeshBaseInstance	*mi= safe_cast<CMeshBaseInstance*>(trans);


	// get the result of the Load Balancing.
	float	alphaMRM;
	if(_MaxFaceUsed > _MinFaceUsed)
	{
		// compute the level of detail we want.
		alphaMRM= (polygonCount - _MinFaceUsed) / (_MaxFaceUsed - _MinFaceUsed);
		clamp(alphaMRM, 0, 1);
	}
	else
		alphaMRM= 1;


	// Choose what Lod to draw.
	alphaMRM*= _Lods.size()-1;
	sint	numLod= (sint)ceil(alphaMRM);
	float	alphaLod;
	if(numLod==0)
	{
		numLod= 0;
		alphaLod= 0;
	}
	else
	{
		// Lerp beetween lod i-1 and lod i.
		alphaLod= alphaMRM-(numLod-1);
	}


	// If lod chosen is not loaded, take the best loaded.
	if(numLod>=(sint)_NbLodLoaded)
	{
		numLod= _NbLodLoaded-1;
		alphaLod= 1;
	}


	// Render the choosen Lod.
	CLod	&lod= _Lods[numLod];
	if(lod.RdrPass.size()==0)
		return;


	// Update the vertexBufferHard (if possible).
	// \toto yoyo: TODO_OPTIMIZE: allocate only what is needed for the current Lod (Max of all instances, like
	// the loading....) (see loadHeader()).
	updateVertexBufferHard(drv, _VBuffer.getNumVertices());


	// Skinning.
	//===========
	// get the skeleton model to which I am binded (else NULL).
	CSkeletonModel		*skeleton;
	skeleton= mi->getSkeletonModel();
	// Is this mesh skinned?? true only if mesh is skinned, skeletonmodel is not NULL, and isSkinApply().
	bool	skinOk= _Skinned && mi->isSkinApply() && skeleton;

	// if ready to skin.
	if(skinOk)
	{
		// apply skin for this Lod only.
		applySkin(lod, skeleton->Bones);
	}
	// if instance skin is invalid but mesh is skinned , we must copy vertices/normals from original vertices.
	else if(!skinOk && _Skinned)
	{
		// do it for this Lod only, and if cache say it is necessary.
		if(!lod.OriginalSkinRestored)
			restoreOriginalSkinPart(lod);
	}

	// If skinning, Setup the skeleton matrix
	if(skinOk)
	{
		drv->setupModelMatrix(skeleton->getWorldMatrix());
	}
	// else set the instance worldmatrix.
	else
	{
		drv->setupModelMatrix(trans->getWorldMatrix());
	}


	// Geomorph.
	//===========
	// Geomorph the choosen Lod (if not the coarser mesh).
	if(numLod>0)
	{
		applyGeomorph(lod.Geomorphs, alphaLod);
	}


	// Render the lod.
	//===========

	// force normalisation of normals..
	bool	bkupNorm= drv->isForceNormalize();
	drv->forceNormalize(true);


	// active VB.
	if(_VBHard)
		drv->activeVertexBufferHard(_VBHard);
	else
		drv->activeVertexBuffer(_VBuffer);


	// Global alpha used ?
	bool globalAlphaUsed=globalAlpha!=1;
	uint8 globalAlphaInt=(uint8)(globalAlpha*255);

	// Render all pass.
	if (globalAlphaUsed)
	{
		for(uint i=0;i<lod.RdrPass.size();i++)
		{
			CRdrPass	&rdrPass= lod.RdrPass[i];

			// CMaterial Ref
			CMaterial &material=mi->Materials[rdrPass.MaterialId];

			// Backup opacity
			uint8 opacity=material.getOpacity ();

			// Backup blend
			bool blend=material.getBlend ();
			material.setBlend (true);

			// New opacity
			material.setOpacity (globalAlphaInt);

			// Backup the zwrite
			bool zwrite=material.getZWrite ();

			// New zwrite
			material.setZWrite (false);

			// Render
			drv->render(rdrPass.PBlock, material);

			// Resetup backuped opacity
			material.setOpacity (opacity);

			// Resetup backuped zwrite
			material.setZWrite (zwrite);

			// Resetup backuped blend
			material.setBlend (blend);
		}
	}
	else
	{
		for(uint i=0;i<lod.RdrPass.size();i++)
		{
			CRdrPass	&rdrPass= lod.RdrPass[i];
			// Render with the Materials of the MeshInstance.
			drv->render(rdrPass.PBlock, mi->Materials[rdrPass.MaterialId]);
		}
	}


	// bkup force normalisation.
	drv->forceNormalize(bkupNorm);

}


// ***************************************************************************
void	CMeshMRMGeom::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// because of complexity, serial is separated in save / load.

	if(f.isReading())
		load(f);
	else
		save(f);

}



// ***************************************************************************
void	CMeshMRMGeom::loadHeader(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/*
	Version 0:
		- base version.
	*/
	sint	ver= f.serialVersion(0);


	// serial Basic info.
	// ==================
	f.serial(_Skinned);
	f.serial(_BBox);
	f.serial(_MaxFaceUsed);
	f.serial(_MinFaceUsed);
	f.serial(_DistanceFinest);
	f.serial(_DistanceMiddle);
	f.serial(_DistanceCoarsest);
	f.serial(_OODistanceDelta);
	f.serial(_DistancePow);
	// preload the Lods.
	f.serialCont(_LodInfos);

	// read/save number of wedges.
	/* NB: prepare memory space too for vertices.
		\todo yoyo: TODO_OPTIMIZE. for now there is no Lod memory profit with vertices / skinWeights.
		But resizing arrays is a problem because of reallocation...
	*/
	uint32	nWedges;
	f.serial(nWedges);
	// Prepare the VBuffer.
	_VBuffer.serialHeader(f);
	// If skinned, must allocate skinWeights.
	contReset(_SkinWeights);
	if(_Skinned)
	{
		_SkinWeights.resize(nWedges);
	}


	// Serial lod offsets.
	// ==================
	// This is the reference pos, to load / save relative offsets.
	sint32			startPos = f.getPos();
	// Those are the lodOffsets, relative to startPos.
	vector<sint32>	lodOffsets;
	lodOffsets.resize(_LodInfos.size(), 0);

	// read all relative offsets, and build the absolute offset of LodInfos.
	for(uint i=0;i<_LodInfos.size(); i++)
	{
		f.serial(lodOffsets[i]);
		_LodInfos[i].LodOffset= startPos + lodOffsets[i];
	}


	// resest the Lod arrays. NB: each Lod is empty, and ready to receive Lod data.
	// ==================
	contReset(_Lods);
	_Lods.resize(_LodInfos.size());

	// Flag the fact that no lod is loaded for now.
	_NbLodLoaded= 0;
}


// ***************************************************************************
void	CMeshMRMGeom::load(NLMISC::IStream &f) throw(NLMISC::EStream)
{

	// because loading, flag the VertexBufferHard.
	_VertexBufferHardDirty= true;


	// Load the header of the stream.
	// ==================
	loadHeader(f);

	// Read All lod subsets.
	// ==================
	for(uint i=0;i<_LodInfos.size(); i++)
	{
		// read the lod face data.
		f.serial(_Lods[i]);
		// read the lod vertex data.
		serialLodVertexData(f, _LodInfos[i].StartAddWedge, _LodInfos[i].EndAddWedges);
		// if reading, must bkup all original vertices from VB.
		// this is done in serialLodVertexData(). by subset
	}


	// Now, all lods are loaded.
	_NbLodLoaded= _Lods.size();

}


// ***************************************************************************
void	CMeshMRMGeom::save(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/*
	Version 0:
		- base version.
	*/
	sint	ver= f.serialVersion(0);
	uint	i;

	// must have good original Skinned Vertex before writing.
	if( _Skinned )
	{
		restoreOriginalSkinVertices();
	}


	// serial Basic info.
	// ==================
	f.serial(_Skinned);
	f.serial(_BBox);
	f.serial(_MaxFaceUsed);
	f.serial(_MinFaceUsed);
	f.serial(_DistanceFinest);
	f.serial(_DistanceMiddle);
	f.serial(_DistanceCoarsest);
	f.serial(_OODistanceDelta);
	f.serial(_DistancePow);
	f.serialCont(_LodInfos);

	// save number of wedges.
	uint32	nWedges;
	nWedges= _VBuffer.getNumVertices();
	f.serial(nWedges);
	// Save the VBuffer header.
	_VBuffer.serialHeader(f);


	// Serial lod offsets.
	// ==================
	// This is the reference pos, to load / save relative offsets.
	sint32			startPos = f.getPos();
	// Those are the lodOffsets, relative to startPos.
	vector<sint32>	lodOffsets;
	lodOffsets.resize(_LodInfos.size(), 0);

	// write all dummy offset. For now (since we don't know what to set), compute the offset of 
	// the sint32 to come back in serial lod parts below.
	for(i=0;i<_LodInfos.size(); i++)
	{
		lodOffsets[i]= f.getPos();
		f.serial(lodOffsets[i]);
	}

	// Serial lod subsets.
	// ==================

	// Save all the lods.
	for(i=0;i<_LodInfos.size(); i++)
	{
		// get current absolute position.
		sint32	absCurPos= f.getPos();

		// come back to "relative lodOffset" absolute position in the stream. (temp stored in lodOffset[i]).
		f.seek(lodOffsets[i], IStream::begin);

		// write the relative position of the lod to the stream.
		sint32	relCurPos= absCurPos - startPos;
		f.serial(relCurPos);

		// come back to absCurPos, to save the lod.
		f.seek(absCurPos, IStream::begin);

		// And so now, save the lod.
		// write the lod face data.
		f.serial(_Lods[i]);
		// write the lod vertex data.
		serialLodVertexData(f, _LodInfos[i].StartAddWedge, _LodInfos[i].EndAddWedges);
	}


}



// ***************************************************************************
void	CMeshMRMGeom::serialLodVertexData(NLMISC::IStream &f, uint startWedge, uint endWedge)
{
	sint	ver= f.serialVersion(0);

	// VBuffer part.
	_VBuffer.serialSubset(f, startWedge, endWedge);

	// SkinWeights.
	if(_Skinned)
	{
		for(uint i= startWedge; i<endWedge; i++)
		{
			f.serial(_SkinWeights[i]);
		}
		// if reading, must copy original vertices from VB.
		if( f.isReading())
		{
			bkupOriginalSkinVerticesSubset(startWedge, endWedge);
		}
	}
}



// ***************************************************************************
void	CMeshMRMGeom::loadFirstLod(NLMISC::IStream &f)
{

	// because loading, flag the VertexBufferHard.
	_VertexBufferHardDirty= true;

	// Load the header of the stream.
	// ==================
	loadHeader(f);


	// If empty MRM, quit.
	if(_LodInfos.size()==0)
		return;


	// Read only the first lod subset.
	// ==================
	for(uint i=0;i<1; i++)
	{
		// read the lod face data.
		f.serial(_Lods[i]);
		// read the lod vertex data.
		serialLodVertexData(f, _LodInfos[i].StartAddWedge, _LodInfos[i].EndAddWedges);
		// if reading, must bkup all original vertices from VB.
		// this is done in serialLodVertexData(). by subset
	}


	// Now, just first lod is loaded.
	_NbLodLoaded= 1;

}


// ***************************************************************************
void	CMeshMRMGeom::loadNextLod(NLMISC::IStream &f)
{

	// because loading, flag the VertexBufferHard.
	_VertexBufferHardDirty= true;

	// If all is loaded, quit.
	if(getNbLodLoaded() == getNbLod())
		return;

	// Set pos to good lod.
	f.seek(_LodInfos[_NbLodLoaded].LodOffset, IStream::begin);

	// Serial this lod data.
	// read the lod face data.
	f.serial(_Lods[_NbLodLoaded]);
	// read the lod vertex data.
	serialLodVertexData(f, _LodInfos[_NbLodLoaded].StartAddWedge, _LodInfos[_NbLodLoaded].EndAddWedges);
	// if reading, must bkup all original vertices from VB.
	// this is done in serialLodVertexData(). by subset


	// Inc LodLoaded count.
	_NbLodLoaded++;
}


// ***************************************************************************
void	CMeshMRMGeom::unloadNextLod(NLMISC::IStream &f)
{
	// If just first lod remain (or no lod), quit
	if(getNbLodLoaded() <= 1)
		return;

	// Reset the entire Lod object. (Free Memory).
	contReset(_Lods[_NbLodLoaded-1]);


	// Dec LodLoaded count.
	_NbLodLoaded--;
}


// ***************************************************************************
void	CMeshMRMGeom::bkupOriginalSkinVertices()
{
	nlassert(_Skinned);

	// bkup the entire array.
	bkupOriginalSkinVerticesSubset(0, _VBuffer.getNumVertices());
}


// ***************************************************************************
void	CMeshMRMGeom::bkupOriginalSkinVerticesSubset(uint wedgeStart, uint wedgeEnd)
{
	nlassert(_Skinned);

	// Copy VBuffer content into Original vertices normals.
	if(_VBuffer.getVertexFormat() & CVertexBuffer::PositionFlag)
	{
		// copy vertices from VBuffer. (NB: unusefull geomorphed vertices are still copied, but doesn't matter).
		_OriginalSkinVertices.resize(_VBuffer.getNumVertices());
		for(uint i=wedgeStart; i<wedgeEnd;i++)
		{
			_OriginalSkinVertices[i]= *(CVector*)_VBuffer.getVertexCoordPointer(i);
		}
	}
	if(_VBuffer.getVertexFormat() & CVertexBuffer::NormalFlag)
	{
		// copy normals from VBuffer. (NB: unusefull geomorphed normals are still copied, but doesn't matter).
		_OriginalSkinNormals.resize(_VBuffer.getNumVertices());
		for(uint i=wedgeStart; i<wedgeEnd;i++)
		{
			_OriginalSkinNormals[i]= *(CVector*)_VBuffer.getNormalCoordPointer(i);
		}
	}
}


// ***************************************************************************
void	CMeshMRMGeom::restoreOriginalSkinVertices()
{
	nlassert(_Skinned);

	// Copy VBuffer content into Original vertices normals.
	if(_VBuffer.getVertexFormat() & CVertexBuffer::PositionFlag)
	{
		// copy vertices from VBuffer. (NB: unusefull geomorphed vertices are still copied, but doesn't matter).
		for(uint i=0; i<_VBuffer.getNumVertices();i++)
		{
			*(CVector*)_VBuffer.getVertexCoordPointer(i)= _OriginalSkinVertices[i];
		}
	}
	if(_VBuffer.getVertexFormat() & CVertexBuffer::NormalFlag)
	{
		// copy normals from VBuffer. (NB: unusefull geomorphed normals are still copied, but doesn't matter).
		for(uint i=0; i<_VBuffer.getNumVertices();i++)
		{
			*(CVector*)_VBuffer.getNormalCoordPointer(i)= _OriginalSkinNormals[i];
		}
	}
}


// ***************************************************************************
void	CMeshMRMGeom::restoreOriginalSkinPart(CLod &lod)
{
	nlassert(_Skinned);


	/* NB: this copies into RAM, and not AGP.
		This is because Geomorph needs to read Data in RAM. So the easiest way is to copy all date into RAM,
		then duplicate  (see fillAGPSkinPart()) in AGP.
	*/


	// get vertexPtr / normalOff.
	//===========================
	uint8		*destVertexPtr= (uint8*)_VBuffer.getVertexCoordPointer();
	uint		flags= _VBuffer.getVertexFormat();
	sint32		vertexSize= _VBuffer.getVertexSize();
	// must have XYZ.
	nlassert(flags & CVertexBuffer::PositionFlag);

	// Compute offset of each component of the VB.
	sint32		normalOff;
	if(flags & CVertexBuffer::NormalFlag)
		normalOff= _VBuffer.getNormalOff();
	else
		normalOff= 0;


	// compute src array.
	CVector				*srcVertexPtr;
	CVector				*srcNormalPtr= NULL;
	srcVertexPtr= &_OriginalSkinVertices[0];
	if(normalOff)
		srcNormalPtr= &(_OriginalSkinNormals[0]);


	// copy skinning.
	//===========================
	for(uint i=0;i<NL3D_MESH_SKINNING_MAX_MATRIX;i++)
	{
		uint		nInf= lod.InfluencedVertices[i].size();
		if( nInf==0 )
			continue;
		uint32		*infPtr= &(lod.InfluencedVertices[i][0]);

		//  for all InfluencedVertices only.
		for(;nInf>0;nInf--, infPtr++)
		{
			uint	index= *infPtr;
			CVector				*srcVertex= srcVertexPtr + index;
			CVector				*srcNormal= srcNormalPtr + index;
			uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);


			// Vertex.
			*dstVertex= *srcVertex;
			// Normal.
			if(normalOff)
				*dstNormal= *srcNormal;
		}
	}


	// Fill the usefull AGP memory (if any one loaded).
	fillAGPSkinPart(lod);


	// clean this lod part. (NB: this is not optimal, but sufficient :) ).
	lod.OriginalSkinRestored= true;
}

// ***************************************************************************

float CMeshMRMGeom::getNumTriangles (float distance)
{
	// NB: this is an approximation, but this is continious.
	// return the lod detail [0,1].
	float	ld= getLevelDetailFromDist(distance);
	// return in nb face.
	return _MinFaceUsed + ld * (_MaxFaceUsed - _MinFaceUsed);
}



// ***************************************************************************
// For fast vector/point multiplication.
struct	CMatrix3x4
{
	// Order them in memory line first, for faster memory access.
	float	a11, a12, a13, a14;
	float	a21, a22, a23, a24;
	float	a31, a32, a33, a34;

	// Copy from a matrix.
	void	set(const CMatrix &mat)
	{
		const float	*m =mat.get();
		a11= m[0]; a12= m[4]; a13= m[8] ; a14= m[12]; 
		a21= m[1]; a22= m[5]; a23= m[9] ; a24= m[13]; 
		a31= m[2]; a32= m[6]; a33= m[10]; a34= m[14]; 
	}


	// mulSetvector. NB: in should be different as v!! (else don't work).
	void	mulSetVector(const CVector &in, CVector &out)
	{
		out.x= (a11*in.x + a12*in.y + a13*in.z);
		out.y= (a21*in.x + a22*in.y + a23*in.z);
		out.z= (a31*in.x + a32*in.y + a33*in.z);
	}
	// mulSetpoint. NB: in should be different as v!! (else don't work).
	void	mulSetPoint(const CVector &in, CVector &out)
	{
		out.x= (a11*in.x + a12*in.y + a13*in.z + a14);
		out.y= (a21*in.x + a22*in.y + a23*in.z + a24);
		out.z= (a31*in.x + a32*in.y + a33*in.z + a34);
	}


	// mulSetvector. NB: in should be different as v!! (else don't work).
	void	mulSetVector(const CVector &in, float scale, CVector &out)
	{
		out.x= (a11*in.x + a12*in.y + a13*in.z) * scale;
		out.y= (a21*in.x + a22*in.y + a23*in.z) * scale;
		out.z= (a31*in.x + a32*in.y + a33*in.z) * scale;
	}
	// mulSetpoint. NB: in should be different as v!! (else don't work).
	void	mulSetPoint(const CVector &in, float scale, CVector &out)
	{
		out.x= (a11*in.x + a12*in.y + a13*in.z + a14) * scale;
		out.y= (a21*in.x + a22*in.y + a23*in.z + a24) * scale;
		out.z= (a31*in.x + a32*in.y + a33*in.z + a34) * scale;
	}


	// mulAddvector. NB: in should be different as v!! (else don't work).
	void	mulAddVector(const CVector &in, float scale, CVector &out)
	{
		out.x+= (a11*in.x + a12*in.y + a13*in.z) * scale;
		out.y+= (a21*in.x + a22*in.y + a23*in.z) * scale;
		out.z+= (a31*in.x + a32*in.y + a33*in.z) * scale;
	}
	// mulAddpoint. NB: in should be different as v!! (else don't work).
	void	mulAddPoint(const CVector &in, float scale, CVector &out)
	{
		out.x+= (a11*in.x + a12*in.y + a13*in.z + a14) * scale;
		out.y+= (a21*in.x + a22*in.y + a23*in.z + a24) * scale;
		out.z+= (a31*in.x + a32*in.y + a33*in.z + a34) * scale;
	}



};


// ***************************************************************************
void	CMeshMRMGeom::applySkin(CLod &lod, const std::vector<CBone> &bones)
{
	nlassert(_Skinned);
	if(_SkinWeights.size()==0)
		return;

	// get vertexPtr / normalOff.
	//===========================
	uint8		*destVertexPtr= (uint8*)_VBuffer.getVertexCoordPointer();
	uint		flags= _VBuffer.getVertexFormat();
	sint32		vertexSize= _VBuffer.getVertexSize();
	// must have XYZ.
	nlassert(flags & CVertexBuffer::PositionFlag);


	// Compute offset of each component of the VB.
	sint32		normalOff;
	if(flags & CVertexBuffer::NormalFlag)
		normalOff= _VBuffer.getNormalOff();
	else
		normalOff= 0;


	// compute src array.
	CMesh::CSkinWeight	*srcSkinPtr;
	CVector				*srcVertexPtr;
	CVector				*srcNormalPtr= NULL;
	srcSkinPtr= &_SkinWeights[0];
	srcVertexPtr= &_OriginalSkinVertices[0];
	if(normalOff)
		srcNormalPtr= &(_OriginalSkinNormals[0]);



	// Compute usefull Matrix for this lod.
	//===========================
	uint	i;
	// Those arrays map the array of bones in skeleton.
	static	vector<CMatrix3x4>			boneMat3x4;
	static	vector<CMatrix3x4>			boneMatNormal3x4;
	// For all matrix this lod use.
	for(i= 0; i<lod.MatrixInfluences.size(); i++)
	{
		// Get Matrix info.
		uint	matId= lod.MatrixInfluences[i];
		const CMatrix		&boneMat= bones[matId].getBoneSkinMatrix();
		CMatrix				boneMatNormal;

		// build the good boneMatNormal (with good scale inf).
		// copy only the rot matrix.
		boneMatNormal.setRot(boneMat);
		// If matrix has scale...
		if(boneMatNormal.hasScalePart())
		{
			// Must compute the transpose of the invert matrix. (10 times slower if not uniform scale!!)
			boneMatNormal.invert();
			boneMatNormal.transpose3x3();
		}

		// compute "fast" matrix 3x4.
		// resize Matrix3x4.
		if(matId>=boneMat3x4.size())
		{
			boneMat3x4.resize(matId+1);
			boneMatNormal3x4.resize(matId+1);
		}
		boneMat3x4[matId].set(boneMat);
		boneMatNormal3x4[matId].set(boneMatNormal);
	}


	// apply skinning.
	//===========================
	// assert, code below is written especially for 4 per vertex.
	nlassert(NL3D_MESH_SKINNING_MAX_MATRIX==4);
	for(i=0;i<NL3D_MESH_SKINNING_MAX_MATRIX;i++)
	{
		uint		nInf= lod.InfluencedVertices[i].size();
		if( nInf==0 )
			continue;
		uint32		*infPtr= &(lod.InfluencedVertices[i][0]);

		switch(i)
		{
		//=========
		case 0:
			// Special case for Vertices influenced by one matrix. Just copy result of mul.
			//  for all InfluencedVertices only.
			for(;nInf>0;nInf--, infPtr++)
			{
				uint	index= *infPtr;
				CMesh::CSkinWeight	*srcSkin= srcSkinPtr + index;
				CVector				*srcVertex= srcVertexPtr + index;
				CVector				*srcNormal= srcNormalPtr + index;
				uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
				CVector				*dstVertex= (CVector*)(dstVertexVB);
				CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);


				// Vertex.
				boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, *dstVertex);
				// Normal.
				if(normalOff)
					boneMatNormal3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, *dstNormal);
			}
			break;

		//=========
		case 1:
			//  for all InfluencedVertices only.
			for(;nInf>0;nInf--, infPtr++)
			{
				uint	index= *infPtr;
				CMesh::CSkinWeight	*srcSkin= srcSkinPtr + index;
				CVector				*srcVertex= srcVertexPtr + index;
				CVector				*srcNormal= srcNormalPtr + index;
				uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
				CVector				*dstVertex= (CVector*)(dstVertexVB);
				CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);


				// Vertex.
				boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
				boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
				// Normal.
				if(normalOff)
				{
					boneMatNormal3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, srcSkin->Weights[0], *dstNormal);
					boneMatNormal3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcNormal, srcSkin->Weights[1], *dstNormal);
				}
			}
			break;

		//=========
		case 2:
			//  for all InfluencedVertices only.
			for(;nInf>0;nInf--, infPtr++)
			{
				uint	index= *infPtr;
				CMesh::CSkinWeight	*srcSkin= srcSkinPtr + index;
				CVector				*srcVertex= srcVertexPtr + index;
				CVector				*srcNormal= srcNormalPtr + index;
				uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
				CVector				*dstVertex= (CVector*)(dstVertexVB);
				CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);


				// Vertex.
				boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
				boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
				boneMat3x4[ srcSkin->MatrixId[2] ].mulAddPoint( *srcVertex, srcSkin->Weights[2], *dstVertex);
				// Normal.
				if(normalOff)
				{
					boneMatNormal3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, srcSkin->Weights[0], *dstNormal);
					boneMatNormal3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcNormal, srcSkin->Weights[1], *dstNormal);
					boneMatNormal3x4[ srcSkin->MatrixId[2] ].mulAddVector( *srcNormal, srcSkin->Weights[2], *dstNormal);
				}
			}
			break;

		//=========
		case 3:
			//  for all InfluencedVertices only.
			for(;nInf>0;nInf--, infPtr++)
			{
				uint	index= *infPtr;
				CMesh::CSkinWeight	*srcSkin= srcSkinPtr + index;
				CVector				*srcVertex= srcVertexPtr + index;
				CVector				*srcNormal= srcNormalPtr + index;
				uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
				CVector				*dstVertex= (CVector*)(dstVertexVB);
				CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);


				// Vertex.
				boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
				boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
				boneMat3x4[ srcSkin->MatrixId[2] ].mulAddPoint( *srcVertex, srcSkin->Weights[2], *dstVertex);
				boneMat3x4[ srcSkin->MatrixId[3] ].mulAddPoint( *srcVertex, srcSkin->Weights[3], *dstVertex);
				// Normal.
				if(normalOff)
				{
					boneMatNormal3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, srcSkin->Weights[0], *dstNormal);
					boneMatNormal3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcNormal, srcSkin->Weights[1], *dstNormal);
					boneMatNormal3x4[ srcSkin->MatrixId[2] ].mulAddVector( *srcNormal, srcSkin->Weights[2], *dstNormal);
					boneMatNormal3x4[ srcSkin->MatrixId[3] ].mulAddVector( *srcNormal, srcSkin->Weights[3], *dstNormal);
				}
			}
			break;

		}

	}


	// Fill the usefull AGP memory (if any one loaded).
	fillAGPSkinPart(lod);


	// dirt this lod part. (NB: this is not optimal, but sufficient :) ).
	lod.OriginalSkinRestored= false;
}


// ***************************************************************************
void				CMeshMRMGeom::fillAGPSkinPart(CLod &lod)
{
	// Fill AGP vertices used by this lod from RAM. (not geomorphed ones).
	if(_VBHard && lod.SkinVertexBlocks.size()>0 )
	{
		// Get VB info, and lock buffers.
		uint8		*vertexSrc= (uint8*)_VBuffer.getVertexCoordPointer();
		uint8		*vertexDst= (uint8*)_VBHard->lock();
		uint32		vertexSize= _VBuffer.getVertexSize();
		nlassert(vertexSize == _VBHard->getVertexSize());


		// For all block of vertices.
		CVertexBlock	*vBlock= &lod.SkinVertexBlocks[0];
		uint	n= lod.SkinVertexBlocks.size();
		for(;n>0; n--, vBlock++)
		{
			// For all vertices of this block, copy it from RAM to VRAM.
			uint8		*src= vertexSrc + vertexSize * vBlock->VertexStart;
			uint8		*dst= vertexDst + vertexSize * vBlock->VertexStart;

			// big copy of all vertices and their data.
			// NB: this not help RAM bandwidth, but this help AGP write combiners.
			// For the majority of mesh (vertex/normal/uv), this is better (6/10).
			memcpy(dst, src, vBlock->NVertices * vertexSize);
		}


		_VBHard->unlock();
	}
}


// ***************************************************************************
void				CMeshMRMGeom::deleteVertexBufferHard()
{
	// test (refptr) if the object still exist in memory.
	if(_VBHard!=NULL)
	{
		// A vbufferhard should still exist only if driver still exist.
		nlassert(_Driver!=NULL);

		// delete it from driver.
		_Driver->deleteVertexBufferHard(_VBHard);
		_VBHard= NULL;
	}
}

// ***************************************************************************
void				CMeshMRMGeom::updateVertexBufferHard(IDriver *drv, uint32 numVertices)
{
	if(!drv->supportVertexBufferHard() || numVertices==0)
		return;


	// If the vbufferhard is not here, or if diryt, or if do not have enough vertices.
	if(_VBHard==NULL || _VertexBufferHardDirty || _VBHard->getNumVertices() < numVertices)
	{
		_VertexBufferHardDirty= false;

		// delete possible old _VBHard.
		if(_VBHard!=NULL)
		{
			// VertexBufferHard lifetime < Driver lifetime.
			nlassert(_Driver!=NULL);
			_Driver->deleteVertexBufferHard(_VBHard);
		}

		// bkup drv in a refptr. (so we know if the vbuffer hard has to be deleted).
		_Driver= drv;
		// try to create new one, in AGP Ram
		_VBHard= _Driver->createVertexBufferHard(_VBuffer.getVertexFormat(), _VBuffer.getValueTypePointer (), numVertices, IDriver::VBHardAGP);


		// If KO, use normal VertexBuffer, else, Fill it with VertexBuffer.
		if(_VBHard!=NULL)
		{
			void	*vertexPtr= _VBHard->lock();

			nlassert(_VBuffer.getVertexFormat() == _VBHard->getVertexFormat());
			nlassert(_VBuffer.getNumVertices() >= numVertices);
			nlassert(_VBuffer.getVertexSize() == _VBHard->getVertexSize());

			// \todo yoyo: TODO_DX8 and DX8 ???
			// Because same internal format, just copy all block.
			memcpy(vertexPtr, _VBuffer.getVertexCoordPointer(), numVertices * _VBuffer.getVertexSize() );

			_VBHard->unlock();
		}
	}

}



// ***************************************************************************
// ***************************************************************************
// CMeshMRM.
// ***************************************************************************
// ***************************************************************************




// ***************************************************************************
CMeshMRM::CMeshMRM()
{
}
// ***************************************************************************
void			CMeshMRM::build (CMeshBase::CMeshBaseBuild &mBase, CMesh::CMeshBuild &m, const CMRMParameters &params)
{
	/// copy MeshBase info: materials ....
	CMeshBase::buildMeshBase (mBase);

	// Then build the geom.
	_MeshMRMGeom.build (m, mBase.Materials.size(), params);
}
// ***************************************************************************
void			CMeshMRM::build (CMeshBase::CMeshBaseBuild &m, const CMeshMRMGeom &mgeom)
{
	/// copy MeshBase info: materials ....
	CMeshBase::buildMeshBase(m);

	// Then copy the geom.
	_MeshMRMGeom= mgeom;
}



// ***************************************************************************
CTransformShape		*CMeshMRM::createInstance(CScene &scene)
{
	// Create a CMeshMRMInstance, an instance of a mesh.
	//===============================================
	CMeshMRMInstance		*mi= (CMeshMRMInstance*)scene.createModel(NL3D::MeshMRMInstanceId);
	mi->Shape= this;


	// instanciate the material part of the MeshMRM, ie the CMeshBase.
	CMeshBase::instanciateMeshBase(mi);


	return mi;
}


// ***************************************************************************
bool	CMeshMRM::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
{
	return _MeshMRMGeom.clip(pyramid, worldMatrix);
}


// ***************************************************************************
void	CMeshMRM::render(IDriver *drv, CTransformShape *trans, bool passOpaque)
{
	_MeshMRMGeom.render(drv, trans, passOpaque, trans->getNumTrianglesAfterLoadBalancing());
}


// ***************************************************************************
void	CMeshMRM::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/*
	Version 0:
		- base version.
	*/
	sint	ver= f.serialVersion(0);

	// serial Materials infos contained in CMeshBase.
	CMeshBase::serialMeshBase(f);


	// serial the geometry.
	_MeshMRMGeom.serial(f);
}


// ***************************************************************************
float	CMeshMRM::getNumTriangles (float distance)
{
	return _MeshMRMGeom.getNumTriangles (distance);
}

} // NL3D

