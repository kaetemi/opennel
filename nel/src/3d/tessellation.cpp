/** \file tessellation.cpp
 * <File description>
 *
 * $Id: tessellation.cpp,v 1.56 2001/10/10 15:48:38 berenguier Exp $
 *
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

#include "3d/tessellation.h"
#include "3d/patch.h"
#include "3d/zone.h"
#include "nel/misc/common.h"
#include "3d/landscape_profile.h"
#include "3d/landscape.h"
using namespace NLMISC;
using namespace std;


namespace NL3D 
{



// ***************************************************************************
// The normal Uvs format.
const	uint8	TileUvFmtNormal1= 0;
const	uint8	TileUvFmtNormal2= 1;
const	uint8	TileUvFmtNormal3= 2;
const	uint8	TileUvFmtNormal4= 3;
const	uint8	TileUvFmtNormal5= 4;


// ***************************************************************************
// \todo yoyo: may change this.
const	float TileSize= 128;




// ***************************************************************************
// ***************************************************************************
// CTileMaterial
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CTileMaterial::CTileMaterial()
{
	// By default, all pass are NULL.
	for(uint i=0; i<NL3D_MAX_TILE_FACE; i++)
	{
		TileFaceVectors[i]= NULL;
	}
}


// ***************************************************************************
void		CTileMaterial::appendTileToEachRenderPass()
{
	for(uint i=0;i<NL3D_MAX_TILE_PASS;i++)
	{
		// If RdrPass exist, add this Material Id
		CPatchRdrPass	*rdrPass= Pass[i].PatchRdrPass;
		if(rdrPass!=NULL)
		{
			rdrPass->appendRdrPatchTile(i, &Pass[i]);
		}
	}
}


// ***************************************************************************
// ***************************************************************************
//	CTessVertex
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CTessVertex::computeGeomPos()
{
	// Compute Basic ErrorMetric.
	float	sqrDist= (StartPos - CLandscapeGlobals::RefineCenter).sqrnorm();
	float	pgeom= MaxFaceSize * CLandscapeGlobals::OORefineThreshold / sqrDist;

	// Compute ErrorMetric modified by TileNear transition, only if TileNear transition.
	if( sqrDist< CLandscapeGlobals::TileDistFarSqr )
	{
		// Soft optim: do it only if necessary, ie result of max(errorMetric, errorMetricModified) is foreseeable here.
		if(pgeom < MaxNearLimit)
		{
			float	f= (CLandscapeGlobals::TileDistFarSqr - sqrDist) * CLandscapeGlobals::OOTileDistDeltaSqr;
			clamp(f, 0, 1);
			// ^4 gives better smooth result
			f= sqr(f);
			f= sqr(f);
			// interpolate the errorMetric
			pgeom= MaxNearLimit*f + pgeom*(1-f);
		}
	}

	// Interpolate StartPos to EndPos, between 1 and 2.
	if(pgeom<=1.0f)
		Pos= StartPos;
	else if(pgeom>=2.0f)
		Pos= EndPos;
	else
	{
		float		f= pgeom - 1.0f;
		Pos= f * (EndPos-StartPos) + StartPos;
	}
}


// ***************************************************************************
// ***************************************************************************
// CTessFace
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CTessFace	CTessFace::CantMergeFace;
CTessFace	CTessFace::MultipleBindFace;


// ***************************************************************************
CTessFace::CTessFace()
{
	// Don't modify any of it!!
	// Patch, SonLeft and SonRight nullity are very usefull for MultiplePatch faces, and CantMergeFace.

	Patch= NULL;
	VBase=VLeft=VRight= NULL;
	FBase=FLeft=FRight= NULL;
	Father=SonLeft=SonRight= NULL;
	Level=0;
	ErrorMetricDate= 0;
	// Size, Center, paramcoord undetermined.

	TileMaterial= NULL;
	// Very important (for split reasons). Init Tilefaces to NULL.
	for(sint i=0;i<NL3D_MAX_TILE_FACE;i++)
	{
		TileFaces[i]=NULL;
	}

	RecursMarkCanMerge=false;
	RecursMarkForceMerge=false;


	NL3D_PROFILE_LAND_ADD(ProfNTessFace, 1);
}


// ***************************************************************************
CTessFace::~CTessFace()
{
	// Old Code. This is not sufficient to clear the CTessFace.
	// Vertices and Uvs must be correctly cleared too (but difficult because of sharing).
	/*
	Patch->getLandscape()->deleteTessFace(SonLeft);
	Patch->getLandscape()->deleteTessFace(SonRight);

	// update neighbors.
	if(FBase)	FBase->changeNeighbor(this, NULL);
	if(FLeft)	FLeft->changeNeighbor(this, NULL);
	if(FRight)	FRight->changeNeighbor(this, NULL);

	FBase=FLeft=FRight= NULL;
	*/

	NL3D_PROFILE_LAND_ADD(ProfNTessFace, -1);
}


// ***************************************************************************
float			CTessFace::computeNearLimit()
{
	// General formula for Level, function of Size, treshold etc...:
	// WantedLevel= log2(BaseSize / sqrdist / RefineThreshold);
	// <=> WantedLevel= log2( CurSize*2^Level / sqrdist / RefineThreshold).
	// <=> WantedLevel= log2( ProjectedSize* 2^Level / RefineThreshold).
	// <=> 2^WantedLevel= ProjectedSize* 2^Level / RefineThreshold.
	// <=> ProjectedSize= (2^WantedLevel) * RefineThreshold / (2^Level);
	// <=> ProjectedSize= (1<<WantedLevel) * RefineThreshold / (1<<Level);
	// UnOptimised formula: limit= (1<<Patch->TileLimitLevel) / (1<<Level);
	nlassert(Level<=20);
	static const uint	BigValue= 1<<20;
	static const float	OOBigValue= 1.0f / BigValue;
	return (1<<Patch->TileLimitLevel) * (OOBigValue*(BigValue>>Level));
}


// ***************************************************************************
void			CTessFace::computeTileErrorMetric()
{
	// We must take a more correct errometric here: We must have sons face which have
	// lower projectedsize than father. This is not the case if Center of face is taken (but when not in
	// tile mode this is nearly the case). So take the min dist from 3 points.
	float	s0= (VBase->EndPos - CLandscapeGlobals::RefineCenter).sqrnorm();
	float	s1= (VLeft->EndPos - CLandscapeGlobals::RefineCenter).sqrnorm();
	float	s2= (VRight->EndPos - CLandscapeGlobals::RefineCenter).sqrnorm();
	float	sqrdist= minof(s0, s1, s2);
	// It is also VERY important to take the min of 3, to ensure the split in TileMode when Far1 vertex begin
	// to blend (see Patch::renderFar1() render).

	// NB: VertexProgram geomorph take sqrdist= (SplitPoint - RefineCenter).sqrnorm();
	// It's OK because geomorph will start "far" after the split.

	if(sqrdist< CLandscapeGlobals::TileDistFarSqr)
	{
		float	nearLimit;
		nearLimit= CLandscapeGlobals::RefineThreshold * computeNearLimit();
		// If we are not so subdivided.
		if(ErrorMetric<nearLimit)
		{
			if(sqrdist< CLandscapeGlobals::TileDistNearSqr)
			{
				ErrorMetric=nearLimit;
			}
			else
			{
				// Smooth transition to the nearLimit of tesselation.
				float	f= ( CLandscapeGlobals::TileDistFarSqr - sqrdist ) * CLandscapeGlobals::OOTileDistDeltaSqr;
				// sqr gives better result, by smoothing more the start of transition.
				f= sqr(f);
				f= sqr(f);
				ErrorMetric= ErrorMetric + (nearLimit-ErrorMetric)*f;

				// If threshold is big like 0.5, transition is still hard, and pops occurs. But The goal is 
				// 0.005 and less, so don't bother. 
			}
		}
	}
}


// ***************************************************************************
void		CTessFace::updateErrorMetric()
{
	// If already updated for this pass...
	if(ErrorMetricDate>= CLandscapeGlobals::CurrentDate)
		return;

	CVector	viewdir= SplitPoint - CLandscapeGlobals::RefineCenter;
	float	sqrdist= viewdir.sqrnorm();

	// trivial formula.
	//-----------------
	ErrorMetric= Size/ sqrdist;


	// Hoppe97 formula:  k²= a² * ("v-e"² - ((v-e).n)²) / "v-e"^4.
	//-----------------
	// Can't do it because geomorph is made on Graphic card, so the simplier is the better.


	// TileMode Impact.
	//-----------------
	// TileMode Impact. We must split at least at TileLimitLevel.
	if(Level<Patch->TileLimitLevel)
	{
		computeTileErrorMetric();
	}

	ErrorMetricDate= CLandscapeGlobals::CurrentDate;
}


// ***************************************************************************
inline float	CTessFace::computeTileEMForUpdateRefine(float distSplitPoint, float distMinFace, float nearLimit)
{
	float	ema;
	// Normal ErrorMetric simulation.
	ema= Size / sqr(distSplitPoint);

	// TileErrorMetric simulation.
	if(distMinFace < CLandscapeGlobals::TileDistFar)
	{
		// If we are not so subdivided.
		if( ema<nearLimit )
		{
			if( distMinFace< CLandscapeGlobals::TileDistNear)
			{
				ema= nearLimit;
			}
			else
			{
				// Smooth transition to the nearLimit of tesselation.
				float	f= ( CLandscapeGlobals::TileDistFarSqr - sqr(distMinFace) ) * CLandscapeGlobals::OOTileDistDeltaSqr;
				// sqr gives better result, by smoothing more the start of transition.
				f= sqr(f);
				f= sqr(f);
				ema= ema + (nearLimit-ema)*f;
			}
		}
	}

	return ema * CLandscapeGlobals::OORefineThreshold;
}


// ***************************************************************************
void	CTessFace::computeSplitPoint()
{
	if(isRectangular())
	{
		// If it is a rectangular triangle, it will be splitted on the middle of VBase/VLeft.
		// see splitRectangular() conventions.
		// So for good geomorph compute per vertex, we must have this SplitPoint on this middle.
		SplitPoint= (VLeft->EndPos + VBase->EndPos)/2;
	}
	else
	{
		// If it is a square triangle, it will be splitted on middle of VLeft/VRight. 
		// So for good geomorph compute per vertex, we must have this SplitPoint on this middle.
		SplitPoint= (VLeft->EndPos + VRight->EndPos)/2;
	}
}

// ***************************************************************************
void	CTessFace::allocTileUv(TTileUvId id)
{
	// TileFaces must have been build.
	nlassert(TileFaces[NL3D_TILE_PASS_RGB0]);

	for(sint i=0;i<NL3D_MAX_TILE_FACE;i++)
	{
		if(TileFaces[i])
		{
			CTessNearVertex		*newNear= Patch->getLandscape()->newTessNearVertex();
			switch(id)
			{
				case IdUvBase: newNear->Src= VBase; TileFaces[i]->VBase= newNear; break;
				case IdUvLeft: newNear->Src= VLeft; TileFaces[i]->VLeft= newNear; break;
				case IdUvRight: newNear->Src= VRight; TileFaces[i]->VRight= newNear; break;
				default: nlstop;
			};
			Patch->appendNearVertexToRenderList(TileMaterial, newNear);

			// May Allocate/Fill VB. Do it after allocTileUv, because UVs are not comuted yet.
		}
	}
}

// ***************************************************************************
void	CTessFace::checkCreateFillTileVB(TTileUvId id)
{
	// TileFaces must have been build.
	nlassert(TileFaces[NL3D_TILE_PASS_RGB0]);

	for(sint i=0;i<NL3D_MAX_TILE_FACE;i++)
	{
		if(TileFaces[i])
		{
			CTessNearVertex		*vertNear;
			switch(id)
			{
				case IdUvBase: vertNear= TileFaces[i]->VBase; break;
				case IdUvLeft: vertNear= TileFaces[i]->VLeft; break;
				case IdUvRight: vertNear= TileFaces[i]->VRight; break;
				default: nlstop;
			};

			// May Allocate/Fill VB.
			Patch->checkCreateVertexVBNear(vertNear);
			Patch->checkFillVertexVBNear(vertNear);
		}
	}
}


// ***************************************************************************
void	CTessFace::checkFillTileVB(TTileUvId id)
{
	// TileFaces must have been build.
	nlassert(TileFaces[NL3D_TILE_PASS_RGB0]);

	for(sint i=0;i<NL3D_MAX_TILE_FACE;i++)
	{
		if(TileFaces[i])
		{
			CTessNearVertex		*vertNear;
			switch(id)
			{
				case IdUvBase: vertNear= TileFaces[i]->VBase; break;
				case IdUvLeft: vertNear= TileFaces[i]->VLeft; break;
				case IdUvRight: vertNear= TileFaces[i]->VRight; break;
				default: nlstop;
			};

			// May Fill VB.
			Patch->checkFillVertexVBNear(vertNear);
		}
	}
}


// ***************************************************************************
void	CTessFace::deleteTileUv(TTileUvId id)
{
	// TileFaces must still exist.
	nlassert(TileFaces[NL3D_TILE_PASS_RGB0]);

	for(sint i=0;i<NL3D_MAX_TILE_FACE;i++)
	{
		if(TileFaces[i])
		{
			CTessNearVertex		*oldNear;
			switch(id)
			{
				case IdUvBase : oldNear= TileFaces[i]->VBase;  TileFaces[i]->VBase=NULL; break;
				case IdUvLeft : oldNear= TileFaces[i]->VLeft;  TileFaces[i]->VLeft=NULL; break;
				case IdUvRight: oldNear= TileFaces[i]->VRight; TileFaces[i]->VRight=NULL; break;
				default: nlstop;
			};

			// May delete this vertex from VB.
			Patch->checkDeleteVertexVBNear(oldNear);

			Patch->removeNearVertexFromRenderList(TileMaterial, oldNear);
			Patch->getLandscape()->deleteTessNearVertex(oldNear);
		}
	}
}
// ***************************************************************************
void	CTessFace::copyTileUv(TTileUvId dstId, CTessFace *srcFace, TTileUvId srcId)
{
	// TileFaces must have been build.
	nlassert(TileFaces[NL3D_TILE_PASS_RGB0]);

	// Since this a ptr-copy, no need to add/remove the renderlist of near vertices.

	for(sint i=0;i<NL3D_MAX_TILE_FACE;i++)
	{
		if(TileFaces[i])
		{
			// The srcface should have the same tileFaces enabled.
			nlassert(srcFace->TileFaces[i]);

			// copy from src.
			CTessNearVertex		*copyNear;
			switch(srcId)
			{
				case IdUvBase : copyNear= srcFace->TileFaces[i]->VBase; break;
				case IdUvLeft : copyNear= srcFace->TileFaces[i]->VLeft; break;
				case IdUvRight: copyNear= srcFace->TileFaces[i]->VRight; break;
				default: nlstop;
			};

			// copy to dst.
			switch(dstId)
			{
				case IdUvBase : TileFaces[i]->VBase=  copyNear; break;
				case IdUvLeft : TileFaces[i]->VLeft=  copyNear; break;
				case IdUvRight: TileFaces[i]->VRight= copyNear; break;
				default: nlstop;
			};
		}
	}
}
// ***************************************************************************
void	CTessFace::heritTileUv(CTessFace *baseFace)
{
	// TileFaces must have been build.
	nlassert(TileFaces[NL3D_TILE_PASS_RGB0]);

	for(sint i=0;i<NL3D_MAX_TILE_FACE;i++)
	{
		if(TileFaces[i])
		{
			// The baseface should have the same tileFaces enabled.
			nlassert(baseFace->TileFaces[i]);
			// VBase should be allocated.
			nlassert(TileFaces[i]->VBase);
			TileFaces[i]->VBase->initMiddleUv(*baseFace->TileFaces[i]->VLeft, *baseFace->TileFaces[i]->VRight);
		}
	}
}


// ***************************************************************************
void		CTessFace::buildTileFaces()
{
	nlassert(TileMaterial);

	// Do nothgin for lightmap pass, of course.
	for(sint i=0;i<NL3D_MAX_TILE_FACE;i++)
	{
		if(TileMaterial->Pass[i].PatchRdrPass)
		{
			TileFaces[i]= Patch->getLandscape()->newTileFace();
			TileFaces[i]->VBase= NULL;
			TileFaces[i]->VLeft= NULL;
			TileFaces[i]->VRight= NULL;
		}
	}
}
// ***************************************************************************
void		CTessFace::deleteTileFaces()
{
	nlassert(TileMaterial);

	// Do nothgin for lightmap pass, of course.
	for(sint i=0;i<NL3D_MAX_TILE_FACE;i++)
	{
		if(TileMaterial->Pass[i].PatchRdrPass)
		{
			nlassert(TileFaces[i]);
			Patch->getLandscape()->deleteTileFace(TileFaces[i]);
			TileFaces[i]= NULL;
		}
		else
		{
			nlassert(TileFaces[i]==NULL);
		}
	}
}

// ***************************************************************************
bool		CTessFace::emptyTileFaces()
{
	// Do nothgin for lightmap pass, of course.
	for(sint i=0;i<NL3D_MAX_TILE_FACE;i++)
	{
		// Some TileFace exist??
		if(TileFaces[i])
			return false;
	}

	return true;
}



// ***************************************************************************
void		CTessFace::initTileUvRGBA(sint pass, bool alpha, CParamCoord pointCoord, CParamCoord middle, CUV &uv)
{
	// Get good coordinate according to patch orientation.
	uv.U= pointCoord.S<=middle.S? 0.0f: 1.0f;
	uv.V= pointCoord.T<=middle.T? 0.0f: 1.0f;
	
	// Get Tile Uv info: orientation and scale.
	uint8		orient;
	CVector		uvScaleBias;
	bool		is256;
	uint8		uvOff;
	Patch->getTileUvInfo(TileId, pass, alpha, orient, uvScaleBias, is256, uvOff);

	// Orient the UV.
	float	u= uv.U;
	float	v= uv.V;
	// Speed rotation.
	switch(orient)
	{
		case 0: 
			uv.U= u;
			uv.V= v;
			break;
		case 1: 
			uv.U= 1-v;
			uv.V= u;
			break;
		case 2: 
			uv.U= 1-u;
			uv.V= 1-v;
			break;
		case 3: 
			uv.U= v;
			uv.V= 1-u;
			break;
	}

	// Do the 256x256.
	if(is256)
	{
		uv*= 0.5;
		if(uvOff==2 || uvOff==3)
			uv.U+= 0.5;
		if(uvOff==1 || uvOff==2)
			uv.V+= 0.5;
	}


	// Do the HalfPixel scale bias.
	CVector		hBias;
	float		tsize;
	if(is256)
		tsize= CLandscapeGlobals::TilePixelSize*2.0f;
	else
		tsize= CLandscapeGlobals::TilePixelSize;
	hBias.x= 0.5f/tsize;
	hBias.y= 0.5f/tsize;
	hBias.z= 1-1/tsize;


	// Scale the UV.
	uv.U*= uvScaleBias.z*hBias.z;
	uv.V*= uvScaleBias.z*hBias.z;
	uv.U+= uvScaleBias.x+hBias.x;
	uv.V+= uvScaleBias.y+hBias.y;
}


// ***************************************************************************
void		CTessFace::initTileUvLightmap(CParamCoord pointCoord, CParamCoord middle, CUV &uv)
{
	// Get good coordinate according to patch orientation.
	uv.U= pointCoord.S<=middle.S? 0.0f: 1.0f;
	uv.V= pointCoord.T<=middle.T? 0.0f: 1.0f;
	
	// Get Tile Lightmap Uv info: bias and scale.
	CVector		uvScaleBias;
	Patch->getTileLightMapUvInfo(TileMaterial->TileS, TileMaterial->TileT, uvScaleBias);

	// Scale the UV.
	uv.U*= uvScaleBias.z;
	uv.V*= uvScaleBias.z;
	uv.U+= uvScaleBias.x;
	uv.V+= uvScaleBias.y;
}


// ***************************************************************************
void		CTessFace::computeTileMaterial()
{
	// 0. Compute TileId.
	//-------------------
	// Square Order Patch assumption: assume that when a CTessFace become a tile, his base can ONLY be diagonal...
	/* a Patch:
		A ________
		|\      /|
		| \    / |
	   C|__\B_/  |
		|  /  \  |
		| /    \ |
		|/______\|

		Here, if OrderS*OrderT=2*2, ABC is a new CTessFace of a Tile, and AB is the diagonal of the tile.
		Hence the middle of the tile is the middle of AB.

		C must be created, but A and B may be created or copied from neighbor.
	*/
	CParamCoord	middle(PVLeft,PVRight);
	sint ts= ((sint)middle.S * (sint)Patch->OrderS) / 0x8000;
	sint tt= ((sint)middle.T * (sint)Patch->OrderT) / 0x8000;
	TileId= tt*Patch->OrderS + ts;


	// 1. Compute Tile Material.
	//--------------------------
	// if base neighbor is already at TileLimitLevel just ptr-copy, else create the TileMaterial...
	nlassert(!FBase || FBase->Patch!=Patch || FBase->Level<=Patch->TileLimitLevel);
	bool	copyFromBase;
	copyFromBase= (FBase && FBase->Patch==Patch);
	copyFromBase= copyFromBase && (FBase->Level==Patch->TileLimitLevel && FBase->TileMaterial!=NULL);
	// NB: because of delete/recreateTileUvs(), FBase->TileMaterial may be NULL, even if face is at good TileLimitLevel.
	if(copyFromBase)
	{
		TileMaterial= FBase->TileMaterial;
		nlassert(FBase->TileId== TileId);
	}
	else
	{
		sint	i;
		TileMaterial= Patch->getLandscape()->newTileMaterial();
		TileMaterial->TileS= ts;
		TileMaterial->TileT= tt;

		// Add this new material to the render list.
		Patch->appendTileMaterialToRenderList(TileMaterial);

		// First, get a lightmap for this tile. NB: important to do this after appendTileMaterialToRenderList(), 
		// because use TessBlocks.
		Patch->getTileLightMap(ts, tt, TileMaterial->Pass[NL3D_TILE_PASS_LIGHTMAP].PatchRdrPass);

		// Fill pass of multi pass material.
		for(i=0;i<NL3D_MAX_TILE_PASS;i++)
		{
			// Get the correct render pass, according to the tile number, and the pass.
			if(i!=NL3D_TILE_PASS_LIGHTMAP)
				TileMaterial->Pass[i].PatchRdrPass= Patch->getTileRenderPass(TileId, i);
		}

		// Fill Pass Info.
		for(i=0;i<NL3D_MAX_TILE_PASS;i++)
		{
			TileMaterial->Pass[i].TileMaterial= TileMaterial;
		}

		// Do not append this tile to each RenderPass, done in preRender().
	}


	// 2. Compute Uvs.
	//----------------
	// NB: TileMaterial is already setup. Usefull for initTileUvLightmap() and initTileUvRGBA().

	// First, must create The TileFaces, according to the TileMaterial passes.
	buildTileFaces();

	// Must allocate the Base, and insert into list.
	allocTileUv(IdUvBase);


	// Init LightMap UV, in RGB0 pass, UV1..
	initTileUvLightmap(PVBase, middle, TileFaces[NL3D_TILE_PASS_RGB0]->VBase->PUv1);

	// Init UV RGBA, for all pass (but lightmap).
	for(sint i=0;i<NL3D_MAX_TILE_FACE;i++)
	{
		nlassert(i!=NL3D_TILE_PASS_LIGHTMAP);
		// If pass is valid
		if( TileMaterial->Pass[i].PatchRdrPass)
		{
			// Face must exist.
			nlassert(TileFaces[i]);
			// Compute RGB UV in UV0.
			initTileUvRGBA(i, false, PVBase, middle, TileFaces[i]->VBase->PUv0);
			// If transition tile, compute alpha UV in UV1.
			if(i== NL3D_TILE_PASS_RGB1 || i==NL3D_TILE_PASS_RGB2)
				initTileUvRGBA(i, true, PVBase, middle, TileFaces[i]->VBase->PUv1);
		}
	}

	// UVs are computed, may create and fill VB.
	checkCreateFillTileVB(IdUvBase);


	// if base neighbor is already at TileLimitLevel just ptr-copy, else create the left/right TileUvs...
	if(copyFromBase)
	{
		// Just cross-copy the pointers.
		// Make Left near vertices be the Right vertices of FBase
		copyTileUv(IdUvLeft, FBase, IdUvRight);
		// Make Right near vertices be the Left vertices of FBase
		copyTileUv(IdUvRight, FBase, IdUvLeft);
	}
	else
	{
		// Must allocate the left/right uv (and insert into list).
		allocTileUv(IdUvLeft);
		allocTileUv(IdUvRight);


		// Init LightMap UV, in UvPass 0, UV1..
		initTileUvLightmap(PVLeft, middle, TileFaces[NL3D_TILE_PASS_RGB0]->VLeft->PUv1);
		initTileUvLightmap(PVRight, middle, TileFaces[NL3D_TILE_PASS_RGB0]->VRight->PUv1);

		// Init UV RGBA!
		for(sint i=0;i<NL3D_MAX_TILE_FACE;i++)
		{
			nlassert(i!=NL3D_TILE_PASS_LIGHTMAP);
			// If pass is valid
			if(TileMaterial->Pass[i].PatchRdrPass)
			{
				// Face must exist.
				nlassert(TileFaces[i]);
				// Compute RGB UV in UV0.
				initTileUvRGBA(i, false, PVLeft, middle, TileFaces[i]->VLeft->PUv0);
				initTileUvRGBA(i, false, PVRight, middle, TileFaces[i]->VRight->PUv0);
				// If transition tile, compute alpha UV in UV1.
				if(i== NL3D_TILE_PASS_RGB1 || i==NL3D_TILE_PASS_RGB2)
				{
					initTileUvRGBA(i, true, PVLeft, middle, TileFaces[i]->VLeft->PUv1);
					initTileUvRGBA(i, true, PVRight, middle, TileFaces[i]->VRight->PUv1);
				}
			}
		}

		// UVs are computed, may create and fill VB.
		checkCreateFillTileVB(IdUvLeft);
		checkCreateFillTileVB(IdUvRight);
	}

}
// ***************************************************************************
void	CTessFace::releaseTileMaterial()
{
	// Hence, must release the tile. TileUvBase is differnet for each of leaves.
	deleteTileUv(IdUvBase);

	nlassert(!FBase || FBase->Level<=Patch->TileLimitLevel);
	if(FBase && FBase->Level==Patch->TileLimitLevel && FBase->TileMaterial!=NULL)
	{
		// Do not release Uvs, since neighbor need it...
		// But release faces.
		deleteTileFaces();
		// Do not release TileMaterial, since neighbor need it...
		TileMaterial= NULL;
	}
	else
	{
		// release Uvs.
		deleteTileUv(IdUvLeft);
		deleteTileUv(IdUvRight);

		// After, release Tile faces.
		deleteTileFaces();

		// Release the tile lightmap part. Do it before removeTileMaterialFromRenderList().
		Patch->releaseTileLightMap(TileMaterial->TileS, TileMaterial->TileT);

		// Remove this material from the render list. DO it before deletion of course :).
		// NB: TileS/TileT still valid.
		Patch->removeTileMaterialFromRenderList(TileMaterial);

		Patch->getLandscape()->deleteTileMaterial(TileMaterial);
		TileMaterial= NULL;
	}
}



// ***************************************************************************
void		CTessFace::updateNearFarVertices()
{
	nlassert(VBase && FVBase);
	nlassert(VLeft && FVLeft);
	nlassert(VRight && FVRight);

	FVBase->Src= VBase;
	FVLeft->Src= VLeft;
	FVRight->Src= VRight;
	FVBase->PCoord= PVBase;
	FVLeft->PCoord= PVLeft;
	FVRight->PCoord= PVRight;


	// Near Vertices update (Src only).
	for(sint i=0; i<NL3D_MAX_TILE_FACE; i++)
	{
		if(TileFaces[i])
		{
			TileFaces[i]->VBase->Src= VBase;
			TileFaces[i]->VLeft->Src= VLeft;
			TileFaces[i]->VRight->Src= VRight;
		}
	}
}


// ***************************************************************************
void		CTessFace::splitRectangular(bool propagateSplit)
{
	CTessFace	*f0= this;
	CTessFace	*f1= FBase;
	// Rectangular case: FBase must exist.
	nlassert(f1);

	// In rectangular case, we split at the same time this and FBase (f0 and f1).


	/*
		Tesselation is:
        
       lt                    rt        lt        top         rt
		---------------------        	---------------------
		|----               |        	|\        |\        |
		|    ----      f1   |        	|  \  f1l |  \  f1r |
		|        ----       |    --> 	|    \    |    \    |
		|   f0       ----   |        	| f0r  \  | f0l  \  |
		|                ---|        	|        \|        \|
		---------------------        	---------------------
	   lb                    rb        lb        bot         rb

		Why? For symetry and bind/split reasons: FBase->SonLeft->VBase is always the good vertex to take 
		(see vertex binding).
	*/
	CParamCoord		pclt= f1->PVLeft;
	CParamCoord		pclb= f0->PVBase;
	CParamCoord		pcrt= f1->PVBase;
	CParamCoord		pcrb= f1->PVRight;
	CTessVertex		*vlt= f1->VLeft;
	CTessVertex		*vlb= f0->VBase;
	CTessVertex		*vrt= f1->VBase;
	CTessVertex		*vrb= f1->VRight;

	CTessFarVertex	*farvlt= f1->FVLeft;
	CTessFarVertex	*farvlb= f0->FVBase;
	CTessFarVertex	*farvrt= f1->FVBase;
	CTessFarVertex	*farvrb= f1->FVRight;


	// 1. create new vertices.
	//------------------------

	// Create splitted vertices.
	CParamCoord		pctop(f1->PVBase, f1->PVLeft);
	CParamCoord		pcbot(f0->PVBase, f0->PVLeft);
	CTessVertex		*vtop= NULL;
	CTessVertex		*vbot= NULL;
	// Compute top.
	if(f1->FLeft==NULL || f1->FLeft->isLeaf())
	{
		// The base neighbor is a leaf or NULL. So must create the new vertex.
		vtop= Patch->getLandscape()->newTessVertex();

		// Compute pos.
		vtop->StartPos= (f1->VLeft->EndPos + f1->VBase->EndPos)/2;
		vtop->EndPos= f1->Patch->computeVertex(pctop.getS(), pctop.getT());
		// Init Pos= InitialPos. Important in the case of enforced split.
		vtop->Pos= vtop->StartPos;

		// For geomorph (VertexProgram or soft), compute MaxFaceSize and MaxNearLimit.
		vtop->MaxFaceSize= f1->Size;
		vtop->MaxNearLimit= f1->computeNearLimit();
	}
	else
	{
		// Else, get from neighbor.
		// NB: since *FLeft is not a leaf, FBase->SonLeft!=NULL...
		// NB: this work with both rectangular and square triangles.
		vtop= f1->FLeft->SonLeft->VBase;

		// For geomorph (VertexProgram or soft), compute MaxFaceSize and MaxNearLimit.
		vtop->MaxFaceSize= max( vtop->MaxFaceSize, f1->Size);
		vtop->MaxNearLimit= max( vtop->MaxNearLimit, f1->computeNearLimit());
	}
	// Compute bot.
	if(f0->FLeft==NULL || f0->FLeft->isLeaf())
	{
		// The base neighbor is a leaf or NULL. So must create the new vertex.
		vbot= Patch->getLandscape()->newTessVertex();

		// Compute pos.
		vbot->StartPos= (f0->VLeft->EndPos + f0->VBase->EndPos)/2;
		vbot->EndPos= Patch->computeVertex(pcbot.getS(), pcbot.getT());
		// Init Pos= InitialPos. Important in the case of enforced split.
		vbot->Pos= vbot->StartPos;

		// For geomorph (VertexProgram or soft), compute MaxFaceSize and MaxNearLimit.
		vbot->MaxFaceSize= f0->Size;
		vbot->MaxNearLimit= f0->computeNearLimit();
	}
	else
	{
		// Else, get from neighbor.
		// NB: since *FLeft is not a leaf, FBase->SonLeft!=NULL...
		// NB: this work with both rectangular and square triangles.
		vbot= f0->FLeft->SonLeft->VBase;

		// For geomorph (VertexProgram or soft), compute MaxFaceSize and MaxNearLimit.
		vbot->MaxFaceSize= max( vbot->MaxFaceSize, f0->Size);
		vbot->MaxNearLimit= max( vbot->MaxNearLimit, f0->computeNearLimit());
	}

	// In all case, must create new FarVertices, since rect split occurs on border!!
	CTessFarVertex	*farvtop= Patch->getLandscape()->newTessFarVertex();
	CTessFarVertex	*farvbot= Patch->getLandscape()->newTessFarVertex();
	farvtop->Src= vtop;
	farvbot->Src= vbot;
	farvtop->PCoord= pctop;
	farvbot->PCoord= pcbot;
	Patch->appendFarVertexToRenderList(farvtop);
	Patch->appendFarVertexToRenderList(farvbot);
	// May Allocate/Fill VB.
	// NB: vtop / vbot are well computed  and ready for the fill in VB.
	Patch->checkCreateVertexVBFar(farvtop);
	Patch->checkFillVertexVBFar(farvtop);
	Patch->checkCreateVertexVBFar(farvbot);
	Patch->checkFillVertexVBFar(farvbot);

	// For VertexProgram only, must refill the Far vertex of neighbor(s), 
	// because MaxFaceSize, and MaxNearLimit may have change.
	if( CLandscapeGlobals::VertexProgramEnabled )
	{
		// f0
		if( ! (f0->FLeft==NULL || f0->FLeft->isLeaf()) )
			f0->FLeft->Patch->checkFillVertexVBFar(f0->FLeft->SonLeft->FVBase);
		// f1
		if( ! (f1->FLeft==NULL || f1->FLeft->isLeaf()) )
			f1->FLeft->Patch->checkFillVertexVBFar(f1->FLeft->SonLeft->FVBase);
	}

	
	// 2. Create sons, and update links.
	//----------------------------------

	CTessFace	*f0l, *f0r;
	CTessFace	*f1l, *f1r;

	// create and bind Sons.
	f0l= f0->SonLeft= Patch->getLandscape()->newTessFace();
	f0r= f0->SonRight= Patch->getLandscape()->newTessFace();
	f1l= f1->SonLeft= Patch->getLandscape()->newTessFace();
	f1r= f1->SonRight= Patch->getLandscape()->newTessFace();

	// subdivision left.
	f0l->Patch= f0->Patch;
	f0l->Father= f0;
	f0l->Level= f0->Level+1;
	f0l->Size= f0->Size*0.5f;
	// subdivision right.
	f0r->Patch= f0->Patch;
	f0r->Father= f0;
	f0r->Level= f0->Level+1;
	f0r->Size= f0->Size*0.5f;
	// subdivision left.
	f1l->Patch= f1->Patch;
	f1l->Father= f1;
	f1l->Level= f1->Level+1;
	f1l->Size= f1->Size*0.5f;
	// subdivision right.
	f1r->Patch= f1->Patch;
	f1r->Father= f1;
	f1r->Level= f1->Level+1;
	f1r->Size= f1->Size*0.5f;

	// Patch coordinates.
	f0r->PVRight= pclt;
	f0r->PVBase= pclb;
	f0r->PVLeft= pcbot;
	f1l->PVBase= pctop;
	f1l->PVLeft= f0r->PVRight;
	f1l->PVRight= f0r->PVLeft;

	f0l->PVRight= pctop;
	f0l->PVBase= pcbot;
	f0l->PVLeft= pcrb;
	f1r->PVBase= pcrt;
	f1r->PVLeft= f0l->PVRight;
	f1r->PVRight= f0l->PVLeft;

	// link existing vertex.
	f0r->VRight= vlt;
	f0r->VBase= vlb;
	f0r->VLeft= vbot;
	f1l->VBase= vtop;
	f1l->VLeft= f0r->VRight;
	f1l->VRight= f0r->VLeft;

	f0l->VRight= vtop;
	f0l->VBase= vbot;
	f0l->VLeft= vrb;
	f1r->VBase= vrt;
	f1r->VLeft= f0l->VRight;
	f1r->VRight= f0l->VLeft;

	// link Far vertices.
	f0r->FVRight= farvlt;
	f0r->FVBase= farvlb;
	f0r->FVLeft= farvbot;
	f1l->FVBase= farvtop;
	f1l->FVLeft= f0r->FVRight;
	f1l->FVRight= f0r->FVLeft;

	f0l->FVRight= farvtop;
	f0l->FVBase= farvbot;
	f0l->FVLeft= farvrb;
	f1r->FVBase= farvrt;
	f1r->FVLeft= f0l->FVRight;
	f1r->FVRight= f0l->FVLeft;

	// link neigbhor faces.
	f0r->FBase= f1l;
	f1l->FBase= f0r;
	f0l->FBase= f1r;
	f1r->FBase= f0l;
	f1l->FRight= f0l;
	f0l->FRight= f1l;
	f0r->FRight= f0->FRight;
	if(f0->FRight)
		f0->FRight->changeNeighbor(f0, f0r);
	f1r->FRight= f1->FRight;
	if(f1->FRight)
		f1->FRight->changeNeighbor(f1, f1r);
	// 4 links (all FLeft sons ) are stil invalid here.
	f0l->FLeft= NULL;
	f0r->FLeft= NULL;
	f1l->FLeft= NULL;
	f1r->FLeft= NULL;

	// Neigbors pointers of undetermined splitted face are not changed. Must Doesn't change this. 
	// Used and Updated in section 5. ...


	// 3. Update Tile infos.
	//----------------------
	// There is no update tileinfo with rectangular patch, since tiles are always squares. (TileLimitLevel>SquareLimitLevel).

	// NB: but must test update of tile info for neighboring, ie 2 faces around the splits.
	// For Vertex program only
	if( CLandscapeGlobals::VertexProgramEnabled )
	{
		// if neighbor face splitted, and if 2 different patchs, we must update the Tile vertices
		// because MaxFaceSize and MaxNearLimit may have changed.
		if( f0->FLeft!=NULL && !f0->FLeft->isLeaf() && f0->FLeft->Patch!=Patch )
		{
			// If neighbors sons at tile level, must update their Tile vertices.
			if( f0->FLeft->SonLeft->Level >= f0->FLeft->Patch->TileLimitLevel )
			{
				f0->FLeft->SonLeft->checkFillTileVB(IdUvBase);
				f0->FLeft->SonRight->checkFillTileVB(IdUvBase);
			}
		}
		// idem for f1.
		if( f1->FLeft!=NULL && !f1->FLeft->isLeaf() && f1->FLeft->Patch!=Patch )
		{
			// If neighbors sons at tile level, must update their Tile vertices.
			if( f1->FLeft->SonLeft->Level >= f1->FLeft->Patch->TileLimitLevel )
			{
				f1->FLeft->SonLeft->checkFillTileVB(IdUvBase);
				f1->FLeft->SonRight->checkFillTileVB(IdUvBase);
			}
		}
	}


	// 4. Compute centers.
	//--------------------
	f0r->computeSplitPoint();
	f0l->computeSplitPoint();
	f1r->computeSplitPoint();
	f1l->computeSplitPoint();


	// 5. Propagate, or link sons of base.
	//------------------------------------
	for(sint i=0;i<2;i++)
	{
		CTessFace	*f, *fl, *fr;
		// TOP face.
		if(i==0)
		{
			f= f1;
			fl= f1l;
			fr= f1r;
		}
		// then BOT face.
		else
		{
			f= f0;
			fl= f0l;
			fr= f0r;
		}

		// If current face and FBase has sons, just links.
		if(f->FLeft==NULL)
		{
			// Just update sons neighbors.
			fl->FLeft= NULL;
			fr->FLeft= NULL;
		}
		else if(!f->FLeft->isLeaf())
		{
			CTessFace	*toLeft, *toRight;
			toLeft= f->FLeft->SonLeft;
			toRight= f->FLeft->SonRight;
			// Cross connection of sons.
			if( !f->FLeft->isRectangular() )
			{
				// Case neigbhor is square.
				fl->FLeft= toLeft;
				fr->FLeft= toRight;
				toLeft->FRight= fl;
				toRight->FLeft= fr;
			}
			else
			{
				// Case neigbhor is rectangle.
				fl->FLeft= toRight;
				fr->FLeft= toLeft;
				toLeft->FLeft= fr;
				toRight->FLeft= fl;
			}
		}
		else if (propagateSplit)
		{
			// Warning: at each iteration, the pointer of FLeft may change (because of split() which can change the neighbor 
			// and so f).
			while(f->FLeft->isLeaf())
				f->FLeft->split();

			// There is a possible bug here (maybe easily patched). Sons may have be propagated splitted.
			// And problems may arise because this face hasn't yet good connectivity (especially for rectangles!! :) ).
			nlassert(fl->isLeaf() && fr->isLeaf());
		}
	}

	
	// 6. Must remove father from rdr list, and insert sons.
	//------------------------------------------------------
	// UGLY REFCOUNT SIDE EFFECT: do the append first.
	Patch->appendFaceToRenderList(f0l);
	Patch->appendFaceToRenderList(f0r);
	Patch->appendFaceToRenderList(f1l);
	Patch->appendFaceToRenderList(f1r);
	Patch->removeFaceFromRenderList(f0);
	Patch->removeFaceFromRenderList(f1);


	// 7. Update priority list.
	//------------------------------------------------------
	// Since we are freshly splitted, unlink from any list, and link to the MergePriorityList, because must look 
	// now when should merge.
	Patch->getLandscape()->_MergePriorityList.insert(0, f0);
	Patch->getLandscape()->_MergePriorityList.insert(0, f1);

	// Since we are split, no need to test father for merge, because it cannot!
	if(f0->Father)
	{
		// remove father from any priority list.
		f0->Father->unlinkInPList();
	}
	if(f1->Father)
	{
		// remove father from any priority list.
		f1->Father->unlinkInPList();
	}

}


// ***************************************************************************
void		CTessFace::split(bool propagateSplit)
{

	// 0. Some easy ending.
	//---------------------
	// Already splitted??
	if(!isLeaf())
		return;
	// Don't do this!!!
	//if(Level>=LS_MAXLEVEL)
	//	return;
	// since split() may reach LS_MAXLEVEL, but enforce splits which outpass this stage!!

	NL3D_PROFILE_LAND_ADD(ProfNSplits, 1);


	// Special Rectangular case.
	if(isRectangular())
	{
		splitRectangular(propagateSplit);
		return;
	}

	// 1. Create sons, and update links.
	//----------------------------------

	// create and bind Sons.
	SonLeft= Patch->getLandscape()->newTessFace();
	SonRight= Patch->getLandscape()->newTessFace();

	// subdivision left.
	SonLeft->Patch= Patch;
	SonLeft->Father= this;
	SonLeft->Level= Level+1;
	SonLeft->Size= Size*0.5f;
	// subdivision right.
	SonRight->Patch= Patch;
	SonRight->Father= this;
	SonRight->Level= Level+1;
	SonRight->Size= Size*0.5f;


	// link Left Son.
	// link neighbor face.
	SonLeft->FBase= FLeft;
	if(FLeft)	FLeft->changeNeighbor(this, SonLeft);
	SonLeft->FLeft= SonRight;
	SonLeft->FRight= NULL;		// Temporary. updated later.
	// link neighbor vertex.
	SonLeft->VLeft= VBase;
	SonLeft->VRight= VLeft;
	// link neighbor Far vertex.
	SonLeft->FVLeft= FVBase;
	SonLeft->FVRight= FVLeft;
	// Patch coordinates.
	SonLeft->PVBase= CParamCoord(PVLeft, PVRight);
	SonLeft->PVLeft= PVBase;
	SonLeft->PVRight= PVLeft;

	// linkRight Son.
	// link neighbor face.
	SonRight->FBase= FRight;
	if(FRight)	FRight->changeNeighbor(this, SonRight);
	SonRight->FLeft= NULL;		// Temporary. updated later.
	SonRight->FRight= SonLeft;
	// link neighbor vertex.
	SonRight->VLeft= VRight;
	SonRight->VRight= VBase;
	// link neighbor Far vertex.
	SonRight->FVLeft= FVRight;
	SonRight->FVRight= FVBase;
	// Patch coordinates.
	SonRight->PVBase= CParamCoord(PVLeft, PVRight);
	SonRight->PVLeft= PVRight;
	SonRight->PVRight= PVBase;


	// FBase->FBase==this. Must Doesn't change this. Used and Updated in section 5. ...


	// 2. Update/Create Vertex infos.
	//-------------------------------

	// Must create/link *->VBase.
	if(FBase==NULL || FBase->isLeaf())
	{
		// The base neighbor is a leaf or NULL. So must create the new vertex.
		CTessVertex	*newVertex= Patch->getLandscape()->newTessVertex();
		SonRight->VBase= newVertex;
		SonLeft->VBase= newVertex;

		// Compute pos.
		newVertex->StartPos= (VLeft->EndPos + VRight->EndPos)/2;
		newVertex->EndPos= Patch->computeVertex(SonLeft->PVBase.getS(), SonLeft->PVBase.getT());

		// Init Pos= InitialPos. Important in the case of enforced split.
		newVertex->Pos= newVertex->StartPos;

		// For geomorph (VertexProgram or soft), compute MaxFaceSize and MaxNearLimit.
		newVertex->MaxFaceSize= Size;
		newVertex->MaxNearLimit= computeNearLimit();
	}
	else
	{
		// Else, get from neighbor.
		// NB: since *FBase is not a leaf, FBase->SonLeft!=NULL...
		// NB: this work with both rectangular and square triangles (see splitRectangular()).
		SonRight->VBase= FBase->SonLeft->VBase;
		SonLeft->VBase= FBase->SonLeft->VBase;

		// For geomorph (VertexProgram or soft), compute MaxFaceSize and MaxNearLimit.
		SonLeft->VBase->MaxFaceSize= max( SonLeft->VBase->MaxFaceSize, Size);
		SonLeft->VBase->MaxNearLimit= max( SonLeft->VBase->MaxNearLimit, computeNearLimit());
	}


	// Must create/link *->FVBase.
	// HERE, we must create a FarVertex too if the neighbor is not of the same patch as me.
	if(FBase==NULL || FBase->isLeaf() || FBase->Patch!=Patch)
	{
		// The base neighbor is a leaf or NULL. So must create the new far vertex.
		CTessFarVertex	*newFar= Patch->getLandscape()->newTessFarVertex();
		SonRight->FVBase= newFar;
		SonLeft->FVBase= newFar;

		// Compute.
		newFar->Src= SonLeft->VBase;
		newFar->PCoord= SonLeft->PVBase;

		// Append.
		Patch->appendFarVertexToRenderList(newFar);

		// May Allocate/Fill VB.
		// NB: SonLeft->VBase->Pos is well computed and ready for the fill in VB.
		Patch->checkCreateVertexVBFar(newFar);
		Patch->checkFillVertexVBFar(newFar);

		// For VertexProgram only, must refill the Far vertex of neighbor, 
		// because MaxFaceSize, and MaxNearLimit may have change.
		if( CLandscapeGlobals::VertexProgramEnabled && ! (FBase==NULL || FBase->isLeaf()) )
			FBase->Patch->checkFillVertexVBFar(FBase->SonLeft->FVBase);
	}
	else
	{
		// Else, get from neighbor.
		// NB: since *FBase is not a leaf, FBase->SonLeft!=NULL...
		// NB: this work with both rectangular and square triangles (see splitRectangular()).
		SonRight->FVBase= FBase->SonLeft->FVBase;
		SonLeft->FVBase= FBase->SonLeft->FVBase;

		// NB For VertexProgram only: no need to refill the Far vertex of neighbor, because neighbor is of same Patch
		// So MaxNearLimit and MaxFaceSize should be the same.
	}


	// 3. Update Tile infos.
	//----------------------
	// NB: must do it before appendFaceToRenderList().
	// NB: must do it after compute of SonLeft->VBase->Pos for good filling in VBuffer.
	// There is no problem with rectangular patch, since tiles are always squares.
	// If new tile ....
	if(SonLeft->Level==Patch->TileLimitLevel)
	{
		SonLeft->computeTileMaterial();
		SonRight->computeTileMaterial();
	}
	// else Tile herit.
	else if(SonLeft->Level > Patch->TileLimitLevel)
	{
		heritTileMaterial();
	}

	// For Vertex program only
	if( CLandscapeGlobals::VertexProgramEnabled )
	{
		// if neighbor face splitted, and if 2 different patchs, we must update the Tile vertices
		// because MaxFaceSize and MaxNearLimit may have changed.
		if( FBase!=NULL && !FBase->isLeaf() && FBase->Patch!=Patch )
		{
			// If neighbors sons at tile level, must update their Tile vertices.
			if( FBase->SonLeft->Level >= FBase->Patch->TileLimitLevel )
			{
				FBase->SonLeft->checkFillTileVB(IdUvBase);
				FBase->SonRight->checkFillTileVB(IdUvBase);
			}
		}
	}


	// 4. Compute centers.
	//--------------------
	SonRight->computeSplitPoint();
	SonLeft->computeSplitPoint();


	// 5. Propagate, or link sons of base.
	//------------------------------------
	// If current face and FBase has sons, just links.
	if(FBase==NULL)
	{
		// Just update sons neighbors.
		SonLeft->FRight= NULL;
		SonRight->FLeft= NULL;
	}
	else if(!FBase->isLeaf())
	{
		CTessFace	*toLeft, *toRight;
		CTessFace	*fl, *fr;
		fl= SonLeft;
		fr= SonRight;
		toLeft= FBase->SonLeft;
		toRight= FBase->SonRight;
		// Cross connection of sons.
		if(!FBase->isRectangular())
		{
			// Case neigbhor is square.
			fl->FRight= toRight;
			fr->FLeft= toLeft;
			toLeft->FRight= fr;
			toRight->FLeft= fl;
		}
		else
		{
			// Case neigbhor is rectangular.
			fl->FRight= toLeft;
			fr->FLeft= toRight;
			toLeft->FLeft= fl;
			toRight->FLeft= fr;
		}
	}
	else if (propagateSplit)
	{
		// Warning: at each iteration, the pointer of FBase may change (because of split() which can change the neighbor 
		// and so "this").
		while(FBase->isLeaf())
			FBase->split();

		// There is a possible bug here (maybe easily patched). Sons may have be propagated splitted.
		// And problems may arise because this face hasn't yet good connectivity.
		nlassert(SonLeft->isLeaf() && SonRight->isLeaf());
	}


	// 6. Must remove father from rdr list, and insert sons.
	//------------------------------------------------------
	// UGLY REFCOUNT SIDE EFFECT: do the append first.
	Patch->appendFaceToRenderList(SonLeft);
	Patch->appendFaceToRenderList(SonRight);
	Patch->removeFaceFromRenderList(this);


	// 7. Update priority list.
	//------------------------------------------------------
	// Since we are freshly splitted, unlink from any list, and link to the MergePriorityList, because must look 
	// now when should merge.
	Patch->getLandscape()->_MergePriorityList.insert(0, this);

	// Since we are split, no need to test father for merge, because it cannot!
	if(Father)
	{
		// remove father from any priority list.
		Father->unlinkInPList();
	}
}

// ***************************************************************************
bool		CTessFace::canMerge(bool testEm)
{
	if(this== &CantMergeFace)
		return false;

	nlassert(!isLeaf());

	// Test diamond config (sons must be leaves).
	if(!SonLeft->isLeaf())
		return false;
	if(!SonRight->isLeaf())
		return false;
	// If Errormetric must be considered for this test.
	if(testEm)
	{
		updateErrorMetric();
		float	ps2= ErrorMetric;
		ps2*= CLandscapeGlobals::OORefineThreshold;
		if(ps2>=1.0f)
			return false;
	}

	// Then test neighbors.
	RecursMarkCanMerge= true;
	bool	ok= true;
	if(!isRectangular())
	{
		if(FBase && !FBase->RecursMarkCanMerge)
		{
			if(!FBase->canMerge(testEm))
				ok= false;
		}
	}
	else
	{
		// Rectangular case. May have a longer propagation...
		if(FBase && !FBase->RecursMarkCanMerge)
		{
			if(!FBase->canMerge(testEm))
				ok= false;
		}
		if(ok && FLeft && !FLeft->RecursMarkCanMerge)
		{
			if(!FLeft->canMerge(testEm))
				ok= false;
		}
	}
	// Must not return false in preceding tests, because must set RecursMarkCanMerge to false.
	RecursMarkCanMerge= false;

	return ok;
}


// ***************************************************************************
void		CTessFace::doMerge()
{
	// Assume that canMerge() return true.
	// And Assume that !isLeaf().
	nlassert(!isLeaf());

	if(!isRectangular())
	{
		// 1. Let's merge vertex.
		//-----------------------
		// Delete vertex, only if not already done by the neighbor (ie neighbor not already merged to a leaf).
		// NB: this work even if neigbor is rectnagular.
		if(!FBase || !FBase->isLeaf())
			Patch->getLandscape()->deleteTessVertex(SonLeft->VBase);

		// Delete Far Vertex. Idem, but test too if != patch...
		if(!FBase || !FBase->isLeaf() || FBase->Patch!=Patch)
		{
			// May delete this vertex from VB.
			Patch->checkDeleteVertexVBFar(SonLeft->FVBase);

			Patch->removeFarVertexFromRenderList(SonLeft->FVBase);
			Patch->getLandscape()->deleteTessFarVertex(SonLeft->FVBase);
		}


		// 2. Must remove sons from rdr list, and insert father.
		//------------------------------------------------------
		// Must do it BEFORE the TileFaces are released.
		// UGLY REFCOUNT SIDE EFFECT: do the append first.
		Patch->appendFaceToRenderList(this);
		Patch->removeFaceFromRenderList(SonLeft);
		Patch->removeFaceFromRenderList(SonRight);

		
		// 3. Let's merge Uv.
		//-------------------
		// Delete Uv.
		// Must do it for this and FBase separately, since they may not have same tile level (if != patch).
		if(SonLeft->Level== Patch->TileLimitLevel)
		{
			// Square patch assumption: the sons are not of the same TileId/Patch.
			nlassert(!sameTile(SonLeft, SonRight));
			// release tiles: NearVertices, TileFaces, and TileMaterial.
			SonLeft->releaseTileMaterial();
			SonRight->releaseTileMaterial();
		}
		else if(SonLeft->Level > Patch->TileLimitLevel)
		{
			// Delete Uv, only if not already done by the neighbor (ie neighbor not already merged to a leaf).
			// But Always delete if neighbor exist and has not same tile as me.
			// NB: this work with rectangular neigbor patch, since sameTile() will return false if different patch.
			if(!FBase || !FBase->isLeaf() || !sameTile(this, FBase))
			{
				SonLeft->deleteTileUv(IdUvBase);
			}
			// In all case, must delete the tilefaces of those face.
			SonLeft->deleteTileFaces();
			SonRight->deleteTileFaces();
		}


		// 4. Let's merge Face.
		//-------------------
		// Change father 's neighbor pointers.
		FLeft= SonLeft->FBase;
		if(FLeft)	FLeft->changeNeighbor(SonLeft, this);
		FRight= SonRight->FBase;
		if(FRight)	FRight->changeNeighbor(SonRight, this);
		// delete sons.
		Patch->getLandscape()->deleteTessFace(SonLeft);
		Patch->getLandscape()->deleteTessFace(SonRight);
		SonLeft=NULL;
		SonRight=NULL;

		// If not already done, merge the neighbor.
		if(FBase!=NULL && !FBase->isLeaf())
		{
			FBase->doMerge();
		}

	}
	else
	{
		// Rectangular case.
		// Since minimum Order is 2, Sons of rectangular face are NEVER at TileLimitLevel. => no Uv merge to do.
		nlassert(SonLeft->Level< Patch->TileLimitLevel);
		nlassert(FBase);

		// 1. Let's merge vertex.
		//-----------------------
		// Delete vertex, only if not already done by the neighbor (ie neighbor not already merged to a leaf).
		// NB: this work even if neigbor is rectangular (see tesselation rules in splitRectangular()).
		if(!FLeft || !FLeft->isLeaf())
			Patch->getLandscape()->deleteTessVertex(SonLeft->VBase);

		// Delete Far Vertex. Rect patch: neightb must be of a != pathc as me => must delete FarVertex.
		nlassert(!FLeft || FLeft->Patch!=Patch);
		// May delete this vertex from VB.
		Patch->checkDeleteVertexVBFar(SonLeft->FVBase);

		Patch->removeFarVertexFromRenderList(SonLeft->FVBase);
		Patch->getLandscape()->deleteTessFarVertex(SonLeft->FVBase);


		// 2. Must remove sons from rdr list, and insert father.
		//------------------------------------------------------
		// UGLY REFCOUNT SIDE EFFECT: do the append first.
		Patch->appendFaceToRenderList(this);
		Patch->removeFaceFromRenderList(SonLeft);
		Patch->removeFaceFromRenderList(SonRight);


		// 3. Let's merge Face.
		//-------------------
		// Change father 's neighbor pointers (see splitRectangular()).
		FRight= SonRight->FRight;
		if(FRight)	FRight->changeNeighbor(SonRight, this);
		// delete sons.
		Patch->getLandscape()->deleteTessFace(SonLeft);
		Patch->getLandscape()->deleteTessFace(SonRight);
		SonLeft=NULL;
		SonRight=NULL;

		// First, do it for my rectangular co-worker FBase (if not already done).
		if(!FBase->isLeaf())
		{
			FBase->doMerge();
		}
		// If not already done, merge the neighbor.
		if(FLeft!=NULL && !FLeft->isLeaf())
		{
			FLeft->doMerge();
		}
	}


	// Update priority list.
	//------------------------------------------------------
	// Since we are freshly merged, unlink from any list, and link to the SplitPriorityList, because must look 
	// now when we should split again.
	Patch->getLandscape()->_SplitPriorityList.insert(0, this);

	// since we are now merged maybe re-insert father in priority list.
	if(Father)
	{
		nlassert(!Father->isLeaf());
		// If sons of father are both leaves (ie this, and the other (complexe case if rectangle) )
		if( Father->SonLeft->isLeaf() && Father->SonRight->isLeaf() )
		{
			Patch->getLandscape()->_MergePriorityList.insert(0, Father);
		}
	}

}


// ***************************************************************************
bool		CTessFace::merge()
{
	// Must not be a leaf.
	nlassert(!isLeaf());

	// 0. Verify if merge is posible.
	//----------------------------
	if(!canMerge(false))
		return false;

	NL3D_PROFILE_LAND_ADD(ProfNMerges, 1);

	// 1. Let's merge the face.
	//-----------------------
	// Propagation is done in doMerge().
	doMerge();

	return true;
}

// ***************************************************************************
void		CTessFace::refineAll()
{
	NL3D_PROFILE_LAND_ADD(ProfNRefineFaces, 1);
	NL3D_PROFILE_LAND_ADD(ProfNRefineLeaves, isLeaf()?1:0);

	/*
		if(ps<RefineThreshold), the face must be merged (ie have no leaves).
		if(ps E [RefineThreshold, RefineTreshold*2]), the face must be splitted (ave leaves), and is geomorphed.
		if(ps>RefineThreshold*2), the face is fully splitted/geomoprhed (tests reported on sons...).
	*/

	// Test for Split or merge.
	//-----------------------
	{
		NL3D_PROFILE_LAND_ADD(ProfNRefineComputeFaces, 1);

		updateErrorMetric();
		float	ps=ErrorMetric;
		ps*= CLandscapeGlobals::OORefineThreshold;
		// 1.0f is the point of split().
		// 2.0f is the end of geomorph.


		// Test split/merge.
		//---------------------
		// If wanted, not already done, and limit not reached, split().
		if(isLeaf())
		{
			if(ps>1.0f && Level< (Patch->TileLimitLevel + CLandscapeGlobals::TileMaxSubdivision) )
				split();
		}
		else
		{
			// Else, if splitted, must merge (if not already the case).
			if(ps<1.0f)
			{
				// Merge only if agree, and neighbors agree.
				// canMerge() test all the good thing: FBase==CantMergeFace, or this is rectangular etc...
				// The test is propagated to neighbors.
				if(canMerge(true))
				{
					merge();
				}
			}
		}
	}

	// Recurs.
	//-----------------------
	if(SonLeft)
	{
		SonLeft->refineAll();
		SonRight->refineAll();
	}

}


// ***************************************************************************
// Some updateRefine***() Doc:

// Split or merge, and meaning of errorMetric:
/*
	if(errorMetric<RefineThreshold), the face must be merged (ie have no leaves).
	if(errorMetric E [RefineThreshold, RefineTreshold*2]), the face must be splitted (ave leaves), and is geomorphed.
	if(errorMetric>RefineThreshold*2), the face is fully splitted/geomoprhed.
*/


// Compute distNormalSplitMerge: distance from refineCenter to normal split/merge (ie without tile transition):
/* 
	normal ErrorMetric formula is:
		em = Size*OORefineThreshold/ dist^2;	with dist == (SplitPoint - CLandscapeGlobals::RefineCenter).norm()
	So inverse this function and we have:
		dist= sqrt(Size*OORefineThreshold/em).
	Split or merge is when em==1, so 
		distSplitMerge= sqrt(Size*OORefineThreshold)
*/


// Compute distTileTransSplitMerge.
/* When we are sure that CLandscapeGlobals::TileDistNear < distMinFace < CLandscapeGlobals::TileDistFar, 
	the clamp in the original formula is skipped
	
	So the TileErrorMetric formula is:

	{
		ema= Sife*OORefineThreshold / distSP^2.
		f= (TileDistFar^2 - distMinFace^2) * OOTileDeltaDist^2
		f= f ^ 4.		// no clamp. see above.
		emb= NL*f + ema*(1-f)
		emFinal= max(ema, emb).
	}

	The problem is that the formula is too complex (degree 8 equation). 
	So search for the result recursively.
*/


// ***************************************************************************
void		CTessFace::updateRefineSplit()
{
	NL3D_PROFILE_LAND_ADD(ProfNRefineFaces, 1);

	nlassert(Patch);
	// The face must be not splitted, because tested for split.
	nlassert(isLeaf());

	/*
		NB: see above for some updateRefine*** doc.
	*/

	// Test for Split.
	//-----------------------
	bool	splitted= false;
	{
		updateErrorMetric();
		float	ps=ErrorMetric;
		ps*= CLandscapeGlobals::OORefineThreshold;
		// 1.0f is the point of split().
		// 2.0f is the end of geomorph.


		// Test split.
		//---------------------
		// If wanted and limit not reached, split().
		if(ps>1.0f && Level< (Patch->TileLimitLevel + CLandscapeGlobals::TileMaxSubdivision) )
		{
			split();

			// if split ok
			if(!isLeaf())
			{
				splitted= true;
			}
		}
	}

	// If splitted, then insertion in Landscape->MergePriorityList at 0 has been done. so nothing to update.
	// Else, must compute when whe should re-test.
	if(!splitted)
	{
		// the face is not splitted here.
		nlassert(isLeaf());

		// Insert the face in the priority list.
		//-----------------------
		float	minDeltaDistToUpdate;


		// The distance of SplitPoint to center.
		float	distSplitPoint= (SplitPoint - CLandscapeGlobals::RefineCenter).norm();
		// The distance where we should split/merge. see updateRefin() doc.
		float	distNormalSplitMerge= (float)sqrt(Size*CLandscapeGlobals::OORefineThreshold);


		// If the face is at its max subdivision
		if(Level>=Patch->TileLimitLevel+CLandscapeGlobals::TileMaxSubdivision)
		{
			// special case: the face do not need to be tested for splitting, because Max subdivision reached.
			// Hence just unlink from any list, and return.
			unlinkInPList();
			return;
		}
		else if(Level>=Patch->TileLimitLevel)
		{
			// Always normal ErrorMetric. Because Faces at Tile level decide to split or merge their sons independently 
			// of "Tile ErrorMetric".
			// compute distance to split.
			minDeltaDistToUpdate= distSplitPoint - distNormalSplitMerge;
		}
		else
		{
			// Compute Distance of the face from RefineCenter. It is the min of the 3 points, as in computeTileErrorMetric().
			float	s0= (VBase->EndPos - CLandscapeGlobals::RefineCenter).sqrnorm();
			float	s1= (VLeft->EndPos - CLandscapeGlobals::RefineCenter).sqrnorm();
			float	s2= (VRight->EndPos - CLandscapeGlobals::RefineCenter).sqrnorm();
			float	distMinFace= (float)sqrt( minof(s0, s1, s2) );

			// compute the delta distance to the normal split point. See above for doc.
			float	normalEMDeltaDist;
			normalEMDeltaDist= distSplitPoint - distNormalSplitMerge;


			/* 
			There is 3 possibles cases, according to level, and the distances minFaceDist:
			*/
			///	TileDistFar to +oo.
			if( distMinFace > CLandscapeGlobals::TileDistFar )
			{
				// normal geomorph. Any face compute the distance to the SplitPoint, and take min with distance to
				// the TileDistFar sphere. 
				minDeltaDistToUpdate= normalEMDeltaDist;

				// We must know when we enter in TileErrorMetric zone, because the computing is different.
				minDeltaDistToUpdate= min(minDeltaDistToUpdate, distMinFace - CLandscapeGlobals::TileDistFar);
			}
			/// TileDistNear to TileDistFar.
			else if( distMinFace > CLandscapeGlobals::TileDistNear )
			{
				// Profile
				NL3D_PROFILE_LAND_ADD(ProfNRefineInTileTransition, 1);

				// Compute distance to split/Merge in TileTransition
				float	distTileTransSplitMerge;
				float	maxDeltaDist= 8;
				float	minDeltaDist= 0;
				uint	nbRecurs= 6;
				float	nearLimit;
				nearLimit= CLandscapeGlobals::RefineThreshold * computeNearLimit();
				// search the distance to split recursively.
				for(uint i=0; i< nbRecurs; i++)
				{
					float	pivotDeltaDist= (maxDeltaDist-minDeltaDist)/2;
					// If the em computed with this distance is still <1 (ie merged), then we can move further.
					if ( computeTileEMForUpdateRefine(distSplitPoint-pivotDeltaDist, distMinFace-pivotDeltaDist, nearLimit ) < 1)
						minDeltaDist= pivotDeltaDist;
					// else we must move not as far
					else
						maxDeltaDist= pivotDeltaDist;
				}
				// And so take the minimum resulting delta distance
				distTileTransSplitMerge= minDeltaDist;
				
				// take the min with distance of distMinFace to the TileDistNear and TileDistFar sphere, because formula change at
				// those limits.
				minDeltaDistToUpdate= min(distTileTransSplitMerge, CLandscapeGlobals::TileDistFar - distMinFace );
				minDeltaDistToUpdate= min(minDeltaDistToUpdate, distMinFace - CLandscapeGlobals::TileDistNear);
			}
			/// 0 to TileDistNear.
			else
			{
				// because the face is not a Tile Level (ie Level<Patch->TileLimitLevel), it should be splitted, 
				// and won't merge until reaching at least the TileDistNear sphere.
				// if not splited (should not arise), force the split next time.
				minDeltaDistToUpdate= 0;
			}

		}

		// Profile.
		if(minDeltaDistToUpdate<0.0625)
		{
			NL3D_PROFILE_LAND_ADD(ProfNRefineWithLowDistance, 1);
		}


		// insert in the Split priority list.
		// Until the RefineCenter move under minDeltaDistToUpdate, we don't need to test face.
		Patch->getLandscape()->_SplitPriorityList.insert(minDeltaDistToUpdate, this);
	}
}


// ***************************************************************************
void		CTessFace::updateRefineMerge()
{
	NL3D_PROFILE_LAND_ADD(ProfNRefineFaces, 1);

	nlassert(Patch);
	// The face must be splitted, because tested for merge.
	nlassert(!isLeaf());

	/*
		NB: see above for some updateRefine*** doc.
	*/

	// Test for merge.
	//-----------------------
	bool	merged= false;
	{
		updateErrorMetric();
		float	ps=ErrorMetric;
		ps*= CLandscapeGlobals::OORefineThreshold;
		// 1.0f is the point of split().
		// 2.0f is the end of geomorph.


		// Test merge.
		//---------------------
		// Else, must merge ??
		if(ps<1.0f)
		{
			// Merge only if agree, and neighbors agree.
			// canMerge() test all the good thing: FBase==CantMergeFace, or this is rectangular etc...
			// The test is propagated to neighbors.
			if(canMerge(true))
			{
				merge();

				// NB: here, merge() is not propagated to fathers (supposed to be not very usefull).

				if(isLeaf())
				{
					merged= true;
				}
			}
		}
	}

	// If merged, then insertion in Landscape->SplitPriorityList at 0 has been done. so nothing to update.
	// Else, must compute when whe should re-test.
	if(!merged)
	{
		// the face is splitted here.
		nlassert(!isLeaf());


		// Insert the face in the priority list.
		//-----------------------
		float	minDeltaDistToUpdate;


		// The distance of SplitPoint to center.
		float	distSplitPoint= (SplitPoint - CLandscapeGlobals::RefineCenter).norm();
		// Compute distance from refineCenter to normal split/merge (ie without tile transition).
		float	distNormalSplitMerge= (float)sqrt(Size*CLandscapeGlobals::OORefineThreshold);


		// If the face is at its max subdivision
		if(Level>=Patch->TileLimitLevel+CLandscapeGlobals::TileMaxSubdivision)
		{
			// since the face is splitted, then must test always this face, because we must merge it (as soon as it is possible).
			minDeltaDistToUpdate= 0;
		}
		else if(Level>=Patch->TileLimitLevel)
		{
			// Always normal ErrorMetric. Because Faces at Tile level decide to split or merge their sons independently 
			// of "Tile ErrorMetric".
			// since splitted, compute distance to merge.
			minDeltaDistToUpdate= distNormalSplitMerge - distSplitPoint;
			// NB: it is possible that minDeltaDistToUpdate<0. A good example is when we are enforced split.
			// Then, distSplitMerge may be < distSplitPoint, meaning we should have not split, but a neigbhor has enforced us.
			// So now, must test every frame if we can merge....
			minDeltaDistToUpdate= max( 0.f, minDeltaDistToUpdate );
		}
		else
		{
			// Compute Distance of the face from RefineCenter. It is the min of the 3 points, as in computeTileErrorMetric().
			float	s0= (VBase->EndPos - CLandscapeGlobals::RefineCenter).sqrnorm();
			float	s1= (VLeft->EndPos - CLandscapeGlobals::RefineCenter).sqrnorm();
			float	s2= (VRight->EndPos - CLandscapeGlobals::RefineCenter).sqrnorm();
			float	distMinFace= (float)sqrt( minof(s0, s1, s2) );

			// compute the delta distance to the normal split point. See above for doc.
			float	normalEMDeltaDist;
			normalEMDeltaDist= distNormalSplitMerge - distSplitPoint;
			normalEMDeltaDist= max( 0.f, normalEMDeltaDist );


			/* 
			There is 3 possibles cases, according to level, and the distances minFaceDist:
			*/
			///	TileDistFar to +oo.
			if( distMinFace > CLandscapeGlobals::TileDistFar )
			{
				// normal geomorph. Any face compute the distance to the SplitPoint, and take min with distance to
				// the TileDistFar sphere. 
				minDeltaDistToUpdate= normalEMDeltaDist;

				// We must know when we enter in TileErrorMetric zone, because the computing is different.
				minDeltaDistToUpdate= min(minDeltaDistToUpdate, distMinFace - CLandscapeGlobals::TileDistFar);
			}
			/// TileDistNear to TileDistFar.
			else if( distMinFace > CLandscapeGlobals::TileDistNear )
			{
				// Profile
				NL3D_PROFILE_LAND_ADD(ProfNRefineInTileTransition, 1);


				// Compute distance to split/Merge in TileTransition
				float	distTileTransSplitMerge;
				float	maxDeltaDist= 8;
				float	minDeltaDist= 0;
				uint	nbRecurs= 6;
				float	nearLimit;
				nearLimit= CLandscapeGlobals::RefineThreshold * computeNearLimit();
				// Since splitted, compute distance to merge.
				// search the distance recursively.
				for(uint i=0; i< nbRecurs; i++)
				{
					float	pivotDeltaDist= (maxDeltaDist-minDeltaDist)/2;
					// If the em computed with this distance is still >1 (ie splitted), then we can move further.
					if ( computeTileEMForUpdateRefine(distSplitPoint+pivotDeltaDist, distMinFace+pivotDeltaDist, nearLimit ) > 1)
						minDeltaDist= pivotDeltaDist;
					// else we must move not as far
					else
						maxDeltaDist= pivotDeltaDist;
				}
				// And so take the minimum resulting delta distance
				distTileTransSplitMerge= minDeltaDist;
				
				// take the min with distance of distMinFace to the TileDistNear and TileDistFar sphere, because formula change at
				// those limits.
				minDeltaDistToUpdate= min(distTileTransSplitMerge, CLandscapeGlobals::TileDistFar - distMinFace );
				minDeltaDistToUpdate= min(minDeltaDistToUpdate, distMinFace - CLandscapeGlobals::TileDistNear);
			}
			/// 0 to TileDistNear.
			else
			{
				// because the face is not a Tile Level (ie Level<Patch->TileLimitLevel), it should be splitted, 
				// and won't merge until reaching at least the TileDistNear sphere.
				// Since splitted, Must enter in TileErrorMetric area to know when to merge.
				minDeltaDistToUpdate= CLandscapeGlobals::TileDistNear - distMinFace;
			}

		}

		// Merge Refine Threshold: because of enforced splits, we have lot of faces whit minDeltaDistToUpdate<0, because
		// they alwayas want to merge. To avoid this, add a delta, which delay the test for merge.
		// The caveat is that faces which do not need this may merge later. But 2 meters won't add too many faces.
		minDeltaDistToUpdate+= NL3D_REFINE_MERGE_THRESHOLD;

		// insert in the Merge priority list.
		// Until the RefineCenter move under minDeltaDistToUpdate, we don't need to test face.
		Patch->getLandscape()->_MergePriorityList.insert(minDeltaDistToUpdate, this);
	}
}


// ***************************************************************************
bool		CTessFace::exceptPatch(CPatch *p, CPatch *except[4])
{
	// Warning: except[i] may be NULL, so test if p is NULL.
	// Must unbind from a CantMergeFace.
	if(p== NULL) return false;
	if(p== except[0]) return true;
	if(p== except[1]) return true;
	if(p== except[2]) return true;
	if(p== except[3]) return true;
	return false;
}

// ***************************************************************************
void		CTessFace::unbind(CPatch *except[4])
{
	// NB: since CantMergeFace has a NULL patch ptr, it is unbound too.

	// Square case.
	//=============
	if(!isRectangular())
	{
		// Change Left/Right neighbors.
		if(isLeaf())
		{
			// FLeft and FRight pointers are only valid in Leaves nodes.
			if(FLeft && FLeft->Patch!=Patch)
			{
				if(!exceptPatch(FLeft->Patch, except))
				{
					FLeft->changeNeighbor(this, NULL);
					FLeft= NULL;
				}
			}
			if(FRight && FRight->Patch!=Patch)
			{
				if(!exceptPatch(FRight->Patch, except))
				{
					FRight->changeNeighbor(this, NULL);
					FRight= NULL;
				}
			}
		}
		// Change Base neighbors.
		if(FBase && FBase->Patch!=Patch)
		{
			if(!exceptPatch(FBase->Patch, except))
			{
				CTessFace	*oldNeigbhorFace= FBase;

				FBase->changeNeighbor(this, NULL);
				FBase= NULL;
				if(!isLeaf())
				{
					// Duplicate the VBase of sons, so the unbind is correct and no vertices are shared.
					CTessVertex	*old= SonLeft->VBase;
					SonLeft->VBase= Patch->getLandscape()->newTessVertex();
					*(SonLeft->VBase)= *old;
					SonRight->VBase= SonLeft->VBase;

					// For geomorph (VertexProgram or soft), compute good MaxFaceSize and MaxNearLimit (change since unbinded)
					// update us.
					SonLeft->VBase->MaxFaceSize= Size;
					SonLeft->VBase->MaxNearLimit= computeNearLimit();
					// update our neigbhor, only if not a multiple patch face.
					if(oldNeigbhorFace->Patch)
					{
						old->MaxFaceSize= oldNeigbhorFace->Size;
						old->MaxNearLimit= oldNeigbhorFace->computeNearLimit();
					}
				}
			}
		}
	}
	// Rectangular case.
	//==================
	else
	{
		// Doens't need to test FBase, since must be same patch.
		// In rectangular, FLeft has the behavior of FBase in square case.
		if(FLeft && FLeft->Patch!=Patch)
		{
			if(!exceptPatch(FLeft->Patch, except))
			{
				CTessFace	*oldNeigbhorFace= FLeft;

				FLeft->changeNeighbor(this, NULL);
				FLeft= NULL;
				if(!isLeaf())
				{
					// Duplicate the VBase of sons, so the unbind is correct and no vertices are shared.
					// NB: this code is a bit different from square case.
					CTessVertex	*old= SonLeft->VBase;
					SonLeft->VBase= Patch->getLandscape()->newTessVertex();
					*(SonLeft->VBase)= *old;
					// This is the difference:  (see rectangle tesselation rules).
					SonRight->VLeft= SonLeft->VBase;
					// Yoyo_patch_himself (12/02/2001): I forgot this one!!
					nlassert(FBase && FBase->SonLeft);
					FBase->SonLeft->VRight= SonLeft->VBase;


					// For geomorph (VertexProgram or soft), compute good MaxFaceSize and MaxNearLimit (change since unbinded)
					// update us.
					SonLeft->VBase->MaxFaceSize= Size;
					SonLeft->VBase->MaxNearLimit= computeNearLimit();
					// update our neigbhor, only if not a multiple patch face.
					if(oldNeigbhorFace->Patch)
					{
						old->MaxFaceSize= oldNeigbhorFace->Size;
						old->MaxNearLimit= oldNeigbhorFace->computeNearLimit();
					}
				}
			}
		}
		// But FRight still valid in leaves nodes only.
		if(isLeaf())
		{
			if(FRight && FRight->Patch!=Patch)
			{
				if(!exceptPatch(FRight->Patch, except))
				{
					FRight->changeNeighbor(this, NULL);
					FRight= NULL;
				}
			}
		}
	}

	// Propagate unbind.
	//==================
	if(!isLeaf())
	{
		// update sons vertex pointers (since they may have been updated by me or my grandfathers).
		if(!isRectangular())
		{
			SonLeft->VLeft= VBase;
			SonLeft->VRight= VLeft;
			SonRight->VLeft= VRight;
			SonRight->VRight= VBase;
		}
		else
		{
			// Rectangular case. Update only ptrs which may have changed.
			SonLeft->VLeft= VLeft;
			SonRight->VBase= VBase;
			SonRight->VRight= VRight;
		}

		// Must re-create good Vertex links for Far and Near Vertices!!!
		SonLeft->updateNearFarVertices();
		SonRight->updateNearFarVertices();
		if(isRectangular())
		{
			//NB: must do this for Base neighbor (see unbind() rectangular case...).
			nlassert(FBase && FBase->SonLeft && FBase->SonRight);
			FBase->SonLeft->updateNearFarVertices();
			FBase->SonRight->updateNearFarVertices();
		}

		// unbind the sons.
		SonLeft->unbind(except);
		SonRight->unbind(except);
	}

}
// ***************************************************************************
void		CTessFace::forceMerge()
{
	if(this== &CantMergeFace)
		return;

	if(!isLeaf())
	{
		// First, force merge of Sons and neighbor sons, to have a diamond configuration.
		SonLeft->forceMerge();
		SonRight->forceMerge();

		// forceMerge of necessary neighbors.
		RecursMarkForceMerge=true;
		if(!isRectangular())
		{
			if(FBase && !FBase->RecursMarkForceMerge)
				FBase->forceMerge();
		}
		else
		{
			// Rectangular case. May have a longer propagation...
			if(FBase && !FBase->RecursMarkForceMerge)
				FBase->forceMerge();
			if(FLeft && !FLeft->RecursMarkForceMerge)
				FLeft->forceMerge();
		}
		RecursMarkForceMerge=false;

		// If still a parent, merge.
		if(!isLeaf())
			merge();
	}
}


// ***************************************************************************
void		CTessFace::forceMergeAtTileLevel()
{
	if(this== &CantMergeFace)
		return;

	if(!isLeaf())
	{
		SonLeft->forceMergeAtTileLevel();
		SonRight->forceMergeAtTileLevel();
	}
	else
	{
		// If we are at tile subdivision, we must force our sons to merge.
		if(Level==Patch->TileLimitLevel)
			forceMerge();
	}
}


// ***************************************************************************
void		CTessFace::averageTesselationVertices()
{
	// If we are not splitted, no-op.
	if(isLeaf())
		return;


	CTessFace	*neighbor;
	// Normal square case.
	if(!isRectangular())
	{
		neighbor= FBase;
	}
	// Special Rectangular case.
	else
	{
		// NB: here, just need to compute average of myself with FLeft, because my neighbor FBase 
		// is on same patch (see splitRectangular()), and is average with its FLeft neighbor is done 
		// on an other branch of the recurs call.
		neighbor= FLeft;
	}


	/* Try to average with neighbor.
		- if no neighbor, no-op :).
		- if neighbor is bind 1/N (CantMergeFace), no-op too, because the vertex is a BaseVertex, so don't modify.
		- if my patch is same than my neighbor, then we are on a same patch :), and so no need to average.
	*/
	if(neighbor!=NULL && neighbor!=&CantMergeFace && Patch!= neighbor->Patch)
	{
		nlassert(neighbor->Patch);
		nlassert(!neighbor->isLeaf());
		// must compute average beetween me and my neighbor.
		// NB: this work with both rectangular and square triangles (see split*()).
		nlassert(SonLeft->VBase == neighbor->SonLeft->VBase);

		CVector		v0= Patch->computeVertex(SonLeft->PVBase.getS(), SonLeft->PVBase.getT());
		CVector		v1= neighbor->Patch->computeVertex(neighbor->SonLeft->PVBase.getS(), neighbor->SonLeft->PVBase.getT());

		// And so set the average.
		SonLeft->VBase->EndPos= (v0+v1)/2;
	}


	// Do same thing for sons. NB: see above, we are not a leaf.
	SonLeft->averageTesselationVertices();
	SonRight->averageTesselationVertices();
}


// ***************************************************************************
void		CTessFace::refreshTesselationGeometry()
{
	// must enlarge the little tessBlock (if any), for clipping.
	Patch->extendTessBlockWithEndPos(this);

	// If we are not splitted, no-op.
	if(isLeaf())
		return;


	/* NB: rectangular case: just need to take SonLeft->VBase, because my neighbor on FBase will compute his son
		on an other branch of the recurs call.
	*/
	// re-compute this position (maybe with new noise geometry in Tile Edition).
	SonLeft->VBase->EndPos= Patch->computeVertex(SonLeft->PVBase.getS(), SonLeft->PVBase.getT());
	// overwrite cur Pos (NB: specialy hardcoded for Tile edition).
	SonLeft->VBase->Pos= SonLeft->VBase->EndPos;

	// Do same thing for sons. NB: see above, we are not a leaf.
	SonLeft->refreshTesselationGeometry();
	SonRight->refreshTesselationGeometry();
}


// ***************************************************************************
bool		CTessFace::updateBindEdge(CTessFace	*&edgeFace, bool &splitWanted)
{
	// Return true, when the bind should be Ok, or if a split has occured.
	// Return false only if pointers are updated, without splits.

	if(edgeFace==NULL)
		return true;

	if(edgeFace->isLeaf())
		return true;

	/*
		Look at the callers, and you'll see that "this" is always a leaf.
		Therefore, edgeFace is a valid pointer (either if it is FLeft, FRight or FBase).
	*/

	// MultiPatch face case.
	//======================
	// If neighbor is a multiple face.
	if(edgeFace->Patch==NULL && edgeFace->FBase==this)
	{
		splitWanted= true;
		return true;
	}


	// neighbor is a "Square face" case.
	//==================================
	if(!edgeFace->isRectangular())
	{
		// NB: this code works either if I AM a rectangular face or a square face.

		// If the neighbor is splitted  on ourself, split...
		if(edgeFace->FBase==this)
		{
			splitWanted= true;
			return true;
		}
		else
		{
			// Just update pointers...
			if(edgeFace->FLeft==this)
			{
				CTessFace	*sonLeft= edgeFace->SonLeft;
				sonLeft->FBase= this;
				edgeFace= sonLeft;
			}
			else if(edgeFace->FRight==this)
			{
				CTessFace	*sonRight= edgeFace->SonRight;
				sonRight->FBase= this;
				edgeFace= sonRight;
			}
			else
			{
				// Look at the callers, and you'll see that "this" is always a leaf.
				// Therefore, we should never be here.
				nlstop;
			}
		}
	}
	// neighbor is a "Rectangle face" case.
	//=====================================
	else
	{
		// NB: this code works either if I AM a rectangular face or a square face.

		// If the neighbor is splitted  on ourself, split...
		// Test FLeft because of rectangular case... :)
		// FBase should be tested too. If edgeFace->FBase==this, I should be myself a rectangular face.
		if(edgeFace->FLeft==this || edgeFace->FBase==this)
		{
			splitWanted= true;
			return true;
		}
		else
		{
			if(edgeFace->FRight==this)
			{
				// See rectangular tesselation rules, too know why we do this.
				CTessFace	*sonRight= edgeFace->SonRight;
				sonRight->FRight= this;
				edgeFace= sonRight;
			}
			else
			{
				// Look at the callers, and you'll see that "this" is always a leaf.
				// Therefore, we should never be here.
				nlstop;
			}
		}
	}

	return false;
}



// ***************************************************************************
void		CTessFace::updateBindAndSplit()
{
	bool	splitWanted= false;
	CTessFace	*f0= NULL;
	CTessFace	*f1= NULL;
	/*
		Look at the callers, and you'll see that "this" is always a leaf.
		Therefore, FBase, FLeft and FRight are good pointers, and *FLeft and *FRight should be Ok too.
	*/
	nlassert(isLeaf());
	while(!updateBindEdge(FBase, splitWanted));
	// FLeft and FRight pointers are only valid in Leaves nodes.
	while(!updateBindEdge(FLeft, splitWanted));
	while(!updateBindEdge(FRight, splitWanted));
	// In rectangular case, we MUST also update edges of FBase.
	// Because splitRectangular() split those two faces at the same time.
	if(isRectangular())
	{
		f0= this;
		f1= FBase;
		nlassert(FBase);
		nlassert(FBase->isLeaf());
		// Doesn't need to update FBase->FBase, since it's me!
		// FLeft and FRight pointers are only valid in Leaves nodes.
		while(!FBase->updateBindEdge(FBase->FLeft, splitWanted));
		while(!FBase->updateBindEdge(FBase->FRight, splitWanted));
	}



	CTessFace	*fmult= NULL;
	CTessFace	*fmult0= NULL;
	CTessFace	*fmult1= NULL;
	// If multipatch face case.
	//=========================
	if(!isRectangular())
	{
		// multipatch face case are detected when face->Patch==NULL !!!
		if(FBase && FBase->Patch==NULL)
		{
			fmult= FBase;
			// First, trick: FBase is NULL, so during the split. => no ptr problem.
			FBase= NULL;
		}
	}
	else
	{
		// multipatch face case are detected when face->Patch==NULL !!!
		if(f0->FLeft && f0->FLeft->Patch==NULL)
		{
			fmult0= f0->FLeft;
			// First, trick: neighbor is NULL, so during the split. => no ptr problem.
			f0->FLeft= NULL;
		}
		// multipatch face case are detected when face->Patch==NULL !!!
		if(f1->FLeft && f1->FLeft->Patch==NULL)
		{
			fmult1= f1->FLeft;
			// First, trick: neighbor is NULL, so during the split. => no ptr problem.
			f1->FLeft= NULL;
		}
	}

	// Then split, and propagate.
	//===========================
	split(false);
	if(!isRectangular())
	{
		if(FBase)
		{
			while(FBase->isLeaf())
				FBase->updateBindAndSplit();
		}
		// There is a possible bug here (maybe easily patched). Sons may have be propagated splitted.
		// And problems may arise because this face hasn't yet good connectivity.
		nlassert(SonLeft->isLeaf() && SonRight->isLeaf());
	}
	else
	{
		if(f0->FLeft)
		{
			while(f0->FLeft->isLeaf())
				f0->FLeft->updateBindAndSplit();
		}
		if(f1->FLeft)
		{
			while(f1->FLeft->isLeaf())
				f1->FLeft->updateBindAndSplit();
		}
		// There is a possible bug here (maybe easily patched). Sons may have be propagated splitted.
		// And problems may arise because this face hasn't yet good connectivity.
		nlassert(f0->SonLeft->isLeaf() && f0->SonRight->isLeaf());
		nlassert(f1->SonLeft->isLeaf() && f1->SonRight->isLeaf());
	}


	// If multipatch face case, update neighbors.
	//===========================================
	if(!isRectangular() && fmult)
	{
		// Update good Face neighbors.
		//============================
		SonLeft->FRight= fmult->SonRight;
		fmult->SonRight->changeNeighbor(&CTessFace::MultipleBindFace, SonLeft);

		SonRight->FLeft= fmult->SonLeft;
		fmult->SonLeft->changeNeighbor(&CTessFace::MultipleBindFace, SonRight);

		// NB: this work auto with 1/2 or 1/4. See CPatch::bind(), to understand.
		// In 1/4 case, fmult->SonLeft and fmult->SonRight are themselves MultiPatch face. So it will recurse.

		// Update good vertex pointer.
		//============================
		CTessVertex	*vert= fmult->VBase;

		// Copy the good coordinate: those splitted (because of noise).
		vert->Pos= vert->StartPos= vert->EndPos= SonLeft->VBase->EndPos;
		// But delete the pointer.
		Patch->getLandscape()->deleteTessVertex(SonLeft->VBase);
		// And update sons pointers, to good vertex.
		SonLeft->VBase= vert;
		SonRight->VBase= vert;
		// Compute correct centers.
		SonRight->computeSplitPoint();
		SonLeft->computeSplitPoint();


		// Update good Far vertex pointer.
		//================================
		// Because *->VBase may have been merged to the multiple bind face, Near/FarVertices which pointed on it must
		// be setup.
		// We do not have to propagate this vertex ptr change since sons are leaves!!
		nlassert(SonLeft->isLeaf() && SonRight->isLeaf());
		// update pointers on vertex.
		SonLeft->updateNearFarVertices();
		SonRight->updateNearFarVertices();


		// Bind FBase to a false face which indicate a bind 1/N.
		// This face prevent for "this" face to be merged...
		FBase= &CantMergeFace;

		// Therefore, the vertex will be never deleted (since face not merged).
		// The only way to do this, is to unbind the patch from all (then the vertex is cloned), then the merge will be Ok.
	}
	// Else if rectangular.
	else if(fmult0 || fmult1)
	{
		CTessFace	*f;
		sint		i;

		// Same reasoning for rectangular patchs, as above.
		for(i=0;i<2;i++)
		{
			if(i==0)
				f= f0, fmult= fmult0;
			else
				f= f1, fmult= fmult1;
			if(fmult)
			{
				// Update good Face neighbors (when I am a rectangle).
				//============================
				// Consider the fmult face as a square face.
				CTessFace	*toLeft, *toRight;
				CTessFace	*fl=f->SonLeft, *fr=f->SonRight;
				toLeft= fmult->SonLeft;
				toRight= fmult->SonRight;
				// Cross connection of sons.
				fl->FLeft= toLeft;
				fr->FLeft= toRight;
				toLeft->changeNeighbor(&CTessFace::MultipleBindFace, fl);
				toRight->changeNeighbor(&CTessFace::MultipleBindFace, fr);

				// Update good vertex pointer.
				//============================
				CTessVertex	*vert= fmult->VBase;

				// Copy the good coordinate: those splitted (because of noise).
				// NB: this work too with rectangular patch (see tesselation rules).
				vert->Pos= vert->StartPos= vert->EndPos= fl->VBase->EndPos;
				// But delete the pointer.
				Patch->getLandscape()->deleteTessVertex(fl->VBase);
				// And update sons pointers, to good vertex (rectangular case, see tesselation rules).
				fl->VBase= vert;
				fr->VLeft= vert;
				f->FBase->SonLeft->VRight= vert;

				// Point to a bind 1/N indicator.
				f->FLeft= &CantMergeFace;
			}
		}
		// After all updates done. recompute centers of both sons 's faces, and update far vertices pointers.
		for(i=0;i<2;i++)
		{
			if(i==0)
				f= f0;
			else
				f= f1;
			// Compute correct centers.
			f->SonRight->computeSplitPoint();
			f->SonLeft->computeSplitPoint();

			// Update good Far vertex pointer.
			//================================
			// Because *->VBase may have been merged to the multiple bind face, Near/FarVertices which pointed on it must
			// be setup.
			// We do not have to propagate this vertex ptr change, since sons are leaves!!
			nlassert(f->SonLeft->isLeaf() && f->SonRight->isLeaf());
			// update pointers on vertex.
			f->SonLeft->updateNearFarVertices();
			f->SonRight->updateNearFarVertices();
		}
	}
}


// ***************************************************************************
void		CTessFace::updateBind()
{
	/*
		Remind that updateBind() is called ONLY on the patch which is binded (not the neighbors).
		Since updateBind() is called on the bintree, and that precedent propagated split may have occur, we may not
		be a leaf here. So we are not sure that FLeft and FRight are good, and we doesn't need to update them (since we have
		sons).
		Also, since we are splitted, and correctly linked (this may not be the case in updateBindAndSplit()), FBase IS
		correct. His FBase neighbor and him form a diamond. So we don't need to update him.

		Same remarks for rectangular patchs.
	*/
	if(isLeaf())
	{
		bool	splitWanted= false;
		while(!updateBindEdge(FBase, splitWanted));
		// FLeft and FRight pointers are only valid in Leaves nodes.
		while(!updateBindEdge(FLeft, splitWanted));
		while(!updateBindEdge(FRight, splitWanted));
		if(splitWanted)
			updateBindAndSplit();
	}


	// Recurse to sons.
	if(!isLeaf())
	{
		// Update bind of sons.
		SonLeft->updateBind();
		SonRight->updateBind();
	}
}


// ***************************************************************************
bool		CTessFace::isRectangular() const
{
	return Level<Patch->SquareLimitLevel;
}



// ***************************************************************************
// ***************************************************************************
// For changePatchTexture.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CTessFace::deleteTileUvs()
{
	// NB: NearVertices are removed from renderlist with deleteTileUv (called in releaseTileMaterial()).

	if(!isLeaf())
	{
		// Must delete the materials of leaves first.
		SonLeft->deleteTileUvs();
		SonRight->deleteTileUvs();
		if(SonLeft->Level== Patch->TileLimitLevel)
		{
			// Square patch assumption: the sons are not of the same TileId/Patch.
			nlassert(!sameTile(SonLeft, SonRight));
			// release tiles.
			SonLeft->releaseTileMaterial();
			SonRight->releaseTileMaterial();
		}
		else if(SonLeft->Level > Patch->TileLimitLevel)
		{
			nlassert(!FBase || !FBase->isLeaf());

			// Delete Uv, only if not already done by the neighbor (ie neighbor has yet TileFaces!!).
			// But Always delete if neighbor exist and has not same tile as me.
			// NB: this work with rectangular neigbor patch, since sameTile() will return false if different patch.
			if(!FBase || !FBase->SonLeft->emptyTileFaces() || !sameTile(this, FBase))
			{
				SonLeft->deleteTileUv(IdUvBase);
			}
			// In all case, must delete the tilefaces of those face.
			SonLeft->deleteTileFaces();
			SonRight->deleteTileFaces();
			// For createTileUvs, it is important to mark those faces as NO TileMaterial.
			SonLeft->TileMaterial= NULL;
			SonRight->TileMaterial= NULL;
		}
	}
	else
	{
		// NB: this is done always BELOW tile creation (see above).
		// Do this only for tiles.
		if(TileMaterial)
			Patch->removeFaceFromTileRenderList(this);
	}

}


// ***************************************************************************
void		CTessFace::recreateTileUvs()
{
	// NB: NearVertices are append to renderlist with allocTileUv (called in computeTileMaterial()/heritTileMaterial()).

	if(!isLeaf())
	{
		// Must recreate the materials of parent first.

		// There is no problem with rectangular patch, since tiles are always squares.
		// If new tile ....
		if(SonLeft->Level==Patch->TileLimitLevel)
		{
			SonLeft->computeTileMaterial();
			SonRight->computeTileMaterial();
		}
		// else Tile herit.
		else if(SonLeft->Level > Patch->TileLimitLevel)
		{
			heritTileMaterial();
		}

		SonLeft->recreateTileUvs();
		SonRight->recreateTileUvs();
	}
	else
	{
		// NB: this is done always AFTER tile creation (see above).
		// Do this only for tiles.
		if(TileMaterial)
			Patch->appendFaceToTileRenderList(this);
	}
}



// ***************************************************************************
void		CTessFace::heritTileMaterial()
{
	SonLeft->TileMaterial= TileMaterial;
	SonLeft->TileId= TileId;
	SonLeft->buildTileFaces();
	SonLeft->copyTileUv(IdUvLeft, this, IdUvBase);
	SonLeft->copyTileUv(IdUvRight, this, IdUvLeft);

	SonRight->TileMaterial= TileMaterial;
	SonRight->TileId= TileId;
	SonRight->buildTileFaces();
	SonRight->copyTileUv(IdUvLeft, this, IdUvRight);
	SonRight->copyTileUv(IdUvRight, this, IdUvBase);

	// Create, or link to the tileUv.
	// Try to link to a neighbor TileUv.
	// Can only work iff exist, and iff FBase is same patch, and same TileId.
	if(FBase!=NULL && !FBase->isLeaf() && FBase->SonLeft->TileMaterial!=NULL && sameTile(this, FBase) )
	{
		// Ok!! link to the (existing) TileUv.
		// FBase->SonLeft!=NULL since FBase->isLeaf()==false.
		SonLeft->copyTileUv(IdUvBase, FBase->SonLeft, IdUvBase);
		SonRight->copyTileUv(IdUvBase, FBase->SonLeft, IdUvBase);
	}
	else
	{
		// Allocate a new vertex, and copy it to SonLeft and SonRight.
		SonLeft->allocTileUv(IdUvBase);
		SonRight->copyTileUv(IdUvBase, SonLeft, IdUvBase);

		// Fill the new near vertex, with middle of Left/Right father.
		SonLeft->heritTileUv(this);

		// UVs are computed, may create and fill VB.
		SonLeft->checkCreateFillTileVB(IdUvBase);
	}

}


// ***************************************************************************
// ***************************************************************************
// For getTesselatedPos
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CTessFace::getTesselatedPos(const CUV &uv, bool verifInclusion, CVector &ret)
{
	CVector		uvPos(uv.U, uv.V, 0);

	// may verif if uv is In this triangle. supposed true if rectangular branch.
	if(verifInclusion && !(isRectangular() && !isLeaf()) )
	{
		CVector		uvs[3];
		uvs[0].set( PVBase.getS(), PVBase.getT(), 0);
		uvs[1].set( PVLeft.getS(), PVLeft.getT(), 0);
		uvs[2].set( PVRight.getS(), PVRight.getT(), 0);
		for(sint i=0; i<3; i++)
		{
			CVector		dUv= uvs[(i+1)%3] - uvs[i];
			CVector		normalUv(dUv.y, -dUv.x, 0);
			// if out this 2D plane, uv is out this triangle
			if(normalUv * (uvPos-uvs[i]) <0)
				return;
		}
	}

	// compute tesselated pos in this face.
	if(isLeaf())
		// ok, no more sons, let's do it.
		computeTesselatedPos(uv, ret);
	else
	{
		// must subdivide.
		// if we are rectangular (strange tesselation), must test in both leaves, else, choose only one.
		if(isRectangular())
		{
			SonLeft->getTesselatedPos(uv, true, ret);
			SonRight->getTesselatedPos(uv, true, ret);
		}
		else
		{
			// Compute the uv plane which separate the 2 leaves.
			CVector		uvBase, uvMiddle;
			uvBase.set  ( PVBase.getS(), PVBase.getT(), 0);
			uvMiddle.set( SonLeft->PVBase.getS(), SonLeft->PVBase.getT(), 0);
			CVector		dUv= uvMiddle - uvBase;
			CVector		normalUv(dUv.y, -dUv.x, 0);
			// choose what leaf to recurs.
			if(normalUv * (uvPos - uvBase) <0)
				SonLeft->getTesselatedPos(uv, false, ret);
			else
				SonRight->getTesselatedPos(uv, false, ret);

		}
	}

}


// ***************************************************************************
void			CTessFace::computeTesselatedPos(const CUV &uv, CVector &ret)
{
	CVector		uvPos(uv.U, uv.V, 0);

	// compute the UV triangle of this face.
	CTriangle	uvTri;
	uvTri.V0.set( PVBase.getS(), PVBase.getT(), 0);
	uvTri.V1.set( PVLeft.getS(), PVLeft.getT(), 0);
	uvTri.V2.set( PVRight.getS(), PVRight.getT(), 0);

	// must interpolate the position with given UV, so compute XYZ gradients.
	CVector		Gx;
	CVector		Gy;
	CVector		Gz;
	// NB: take geomorphed position.
	uvTri.computeGradient(VBase->Pos.x, VLeft->Pos.x, VRight->Pos.x, Gx);
	uvTri.computeGradient(VBase->Pos.y, VLeft->Pos.y, VRight->Pos.y, Gy);
	uvTri.computeGradient(VBase->Pos.z, VLeft->Pos.z, VRight->Pos.z, Gz);

	// Compute interpolated position.
	ret= VBase->Pos;
	uvPos-= uvTri.V0;
	ret.x+= Gx*uvPos;
	ret.y+= Gy*uvPos;
	ret.z+= Gz*uvPos;

}


// ***************************************************************************
void			CTessFace::appendTessellationLeaves(std::vector<const CTessFace*>  &leaves) const
{
	if(isLeaf())
		leaves.push_back(this);
	else
	{
		SonLeft->appendTessellationLeaves(leaves);
		SonRight->appendTessellationLeaves(leaves);
	}
}


} // NL3D
