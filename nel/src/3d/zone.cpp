/** \file zone.cpp
 * <File description>
 *
 * $Id: zone.cpp,v 1.50 2001/09/10 13:21:47 berenguier Exp $
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

#include "3d/zone.h"
#include "3d/landscape.h"
#include "nel/misc/common.h"


using namespace NLMISC;
using namespace std;


// define it only for debug bind.
//#define	NL3D_DEBUG_DONT_BIND_PATCH


namespace NL3D 
{



// ***************************************************************************
// ***************************************************************************
// CPatchInfo
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CPatchInfo::setCornerSmoothFlag(uint corner, bool smooth)
{
	nlassert(corner<=3);
	uint	mask= 1<<corner;
	if(smooth)
		_CornerSmoothFlag|= mask;
	else
		_CornerSmoothFlag&= ~mask;
}

// ***************************************************************************
bool			CPatchInfo::getCornerSmoothFlag(uint corner) const
{
	nlassert(corner<=3);
	uint	mask= 1<<corner;
	return	(_CornerSmoothFlag & mask)!=0;
}


// ***************************************************************************
// ***************************************************************************
// CZone
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CZone::CZone()
{
	ComputeTileErrorMetric= false;
	ZoneId= 0;
	Compiled= false;
	Landscape= NULL;
	ClipResult= ClipOut;
}
// ***************************************************************************
CZone::~CZone()
{
	// release() must have been called.
	nlassert(!Compiled);
}


// ***************************************************************************
void			CZone::computeBBScaleBias(const CAABBox	&bb)
{
	ZoneBB= bb;
	// Take a security for noise. (usefull for zone clipping).
	ZoneBB.setHalfSize(ZoneBB.getHalfSize()+CVector(NL3D_NOISE_MAX, NL3D_NOISE_MAX, NL3D_NOISE_MAX));
	CVector	hs= ZoneBB.getHalfSize();
	float	rmax= maxof(hs.x, hs.y, hs.z);
	PatchScale= rmax / 32760;		// Prevent from float imprecision by taking 32760 and not 32767.
	PatchBias= ZoneBB.getCenter();
}


// ***************************************************************************
void			CZone::build(uint16 zoneId, const std::vector<CPatchInfo> &patchs, const std::vector<CBorderVertex> &borderVertices, uint32 numVertices)
{
	sint	i,j;
	nlassert(!Compiled);

	ZoneId= zoneId;
	BorderVertices= borderVertices;

	// Compute the bbox and the bias/scale.
	//=====================================
	CAABBox		bb;
	if(patchs.size())
		bb.setCenter(patchs[0].Patch.Vertices[0]);
	bb.setHalfSize(CVector::Null);
	for(j=0;j<(sint)patchs.size();j++)
	{
		const CBezierPatch	&p= patchs[j].Patch;
		for(i=0;i<4;i++)
			bb.extend(p.Vertices[i]);
		for(i=0;i<8;i++)
			bb.extend(p.Tangents[i]);
		for(i=0;i<4;i++)
			bb.extend(p.Interiors[i]);
	}
	// Compute BBox, and Patch Scale Bias, according to Noise.
	computeBBScaleBias(bb);


	// Compute/compress Patchs.
	//=========================
	Patchs.resize(patchs.size());
	PatchConnects.resize(patchs.size());
	sint	maxVertex=-1;
	for(j=0;j<(sint)patchs.size();j++)
	{
		const CPatchInfo	&pi= patchs[j];
		const CBezierPatch	&p= pi.Patch;
		CPatch				&pa= Patchs[j];
		CPatchConnect		&pc= PatchConnects[j];

		// Smoothing flags
		pa.Flags&=~NL_PATCH_SMOOTH_FLAG_MASK;
		pa.Flags|=NL_PATCH_SMOOTH_FLAG_MASK&(pi.Flags<<NL_PATCH_SMOOTH_FLAG_SHIFT);


		// Noise Data
		// copy noise rotation.
		pa.NoiseRotation= pi.NoiseRotation;
		// copy all noise smoothing info.
		for(i=0;i<4;i++)
		{
			pa.setCornerSmoothFlag(i, pi.getCornerSmoothFlag(i));
		}


		// Build the patch.
		for(i=0;i<4;i++)
			pa.Vertices[i].pack(p.Vertices[i], PatchBias, PatchScale);
		for(i=0;i<8;i++)
			pa.Tangents[i].pack(p.Tangents[i], PatchBias, PatchScale);
		for(i=0;i<4;i++)
			pa.Interiors[i].pack(p.Interiors[i], PatchBias, PatchScale);
		pa.Tiles= pi.Tiles;
		pa.TileColors= pi.TileColors;

		// Copy order of the patch
		pa.OrderS= pi.OrderS;
		pa.OrderT= pi.OrderT;

		// Number of lumels in this patch
		uint lumelCount=(pi.OrderS*NL_LUMEL_BY_TILE)*(pi.OrderT*NL_LUMEL_BY_TILE);

		// Lumel empty ?
		if (pi.Lumels.size ()==lumelCount)
		{
			// Pack the lumel map
			pa.packShadowMap (&pi.Lumels[0]);
		}
		else
		{
			// Reset lightmap
			pa.resetCompressedLumels ();
		}

		nlassert(pa.Tiles.size()== (uint)pi.OrderS*pi.OrderT);
		nlassert(pa.TileColors.size()== (uint)(pi.OrderS+1)*(pi.OrderT+1));

		// Build the patchConnect.
		pc.ErrorSize= pi.ErrorSize;
		for(i=0;i<4;i++)
		{
			pc.BaseVertices[i]= pi.BaseVertices[i];
			maxVertex= max((sint)pc.BaseVertices[i], maxVertex);
		}
		for(i=0;i<4;i++)
			pc.BindEdges[i]= pi.BindEdges[i];
	}

	NumVertices= maxVertex+1;
	NumVertices= max((uint32)NumVertices, numVertices);
}


// ***************************************************************************
void			CZone::retrieve(std::vector<CPatchInfo> &patchs, std::vector<CBorderVertex> &borderVertices)
{
	sint i,j;

	// uncompress Patchs.
	//=========================
	patchs.resize(Patchs.size());
	for(j=0;j<(sint)patchs.size();j++)
	{
		CPatchInfo			&pi= patchs[j];
		CBezierPatch		&p= pi.Patch;
		CPatch				&pa= Patchs[j];
		CPatchConnect		&pc= PatchConnects[j];


		// Smoothing flags
		pi.Flags= (pa.Flags&NL_PATCH_SMOOTH_FLAG_MASK)>>NL_PATCH_SMOOTH_FLAG_SHIFT;


		// Noise Data
		// copy noise rotation.
		pi.NoiseRotation= pa.NoiseRotation;
		// copy all noise smoothing info.
		for(i=0;i<4;i++)
		{
			pi.setCornerSmoothFlag(i, pa.getCornerSmoothFlag(i));
		}


		// re-Build the uncompressed bezier patch.
		for(i=0;i<4;i++)
			pa.Vertices[i].unpack(p.Vertices[i], PatchBias, PatchScale);
		for(i=0;i<8;i++)
			pa.Tangents[i].unpack(p.Tangents[i], PatchBias, PatchScale);
		for(i=0;i<4;i++)
			pa.Interiors[i].unpack(p.Interiors[i], PatchBias, PatchScale);
		pi.Tiles= pa.Tiles;
		pi.TileColors= pa.TileColors;
		pi.Lumels.resize ((pa.OrderS*4)*(pa.OrderT*4));
		pi.Flags=(pa.Flags&NL_PATCH_SMOOTH_FLAG_MASK)>>NL_PATCH_SMOOTH_FLAG_SHIFT;

		// Unpack the lumel map
		pa.unpackShadowMap (&pi.Lumels[0]);

		// from the patchConnect.
		pi.OrderS= pa.OrderS;
		pi.OrderT= pa.OrderT;
		pi.ErrorSize= pc.ErrorSize;
		for(i=0;i<4;i++)
		{
			pi.BaseVertices[i]= pc.BaseVertices[i];
		}
		for(i=0;i<4;i++)
			pi.BindEdges[i]= pc.BindEdges[i];
	}

	// retrieve bordervertices.
	//=========================
	borderVertices= BorderVertices;
}


// ***************************************************************************
void			CZone::build(const CZone &zone)
{
	nlassert(!Compiled);

	ZoneId= zone.ZoneId;
	BorderVertices= zone.BorderVertices;

	// Compute the bbox and the bias/scale.
	//=====================================
	ZoneBB= zone.ZoneBB;
	PatchScale= zone.PatchScale;
	PatchBias= zone.PatchBias;


	// Compute/compress Patchs.
	//=========================
	Patchs= zone.Patchs;
	PatchConnects= zone.PatchConnects;


	NumVertices= zone.NumVertices;
}



// ***************************************************************************
void			CBorderVertex::serial(NLMISC::IStream &f)
{
	uint	ver= f.serialVersion(0);
	f.serial(CurrentVertex, NeighborZoneId, NeighborVertex);
}
void			CZone::CPatchConnect::serial(NLMISC::IStream &f)
{
	uint	ver= f.serialVersion(1);

	if (ver<1)
		f.serial(OldOrderS, OldOrderT, ErrorSize);
	else
		f.serial(ErrorSize);
	f.serial(BaseVertices[0], BaseVertices[1], BaseVertices[2], BaseVertices[3]);
	f.serial(BindEdges[0], BindEdges[1], BindEdges[2], BindEdges[3]);
}
void			CPatchInfo::CBindInfo::serial(NLMISC::IStream &f)
{
	int		i;
	uint	ver= f.serialVersion(0);
	f.serial(NPatchs);
	nlassert ( (NPatchs==0) | (NPatchs==1) | (NPatchs==2) | (NPatchs==4) | (NPatchs==5) );
	f.serial(ZoneId);
	for(i=0;i<4;i++)
		f.serial(Next[i]);
	for(i=0;i<4;i++)
		f.serial(Edge[i]);
}

// ***************************************************************************
void			CZone::serial(NLMISC::IStream &f)
{
	/*
	Version 3:
		- Lumels compression version 2.
	Version 2:
		- Lumels.
	Version 1:
		- Tile color.
	Version 0:
		- base verison.
	*/
	uint	ver= f.serialVersion(3);

	// No more compatibility before version 3
	if (ver<3)
	{
		throw EOlderStream(f);
	}

	f.serialCheck((uint32)'ENOZ');
	f.serial(ZoneId, ZoneBB, PatchBias, PatchScale, NumVertices);
	f.serialCont(BorderVertices);
	f.serialCont(Patchs);
	f.serialCont(PatchConnects);

	// If read and version 0, must init default TileColors of patchs.
	//===============================================================
	if(f.isReading() && ver<2)
	{
		for(sint j=0;j<(sint)Patchs.size();j++)
		{
			CPatch				&pa= Patchs[j];
			CPatchConnect		&pc= PatchConnects[j];

			// Force Order of the patch
			pa.OrderS=pc.OldOrderS;
			pa.OrderT=pc.OldOrderT;

			// Tile colors exist ?
			if (ver<1)
			{
				// Leave it as default behavior: Must init the color as pure white...
				// We must fo it with help of patchconnects OrderS and OrderT.
				pa.TileColors.resize( (pc.OldOrderS+1)*(pc.OldOrderT+1) );
				for(sint i=0;i<(sint)pa.TileColors.size();i++)
				{
					pa.TileColors[i].Color565= 0xFFFF;
					pa.TileColors[i].LightX= 0xFF;
					pa.TileColors[i].LightY= 0x00;
					pa.TileColors[i].LightZ= 0x00;
				}
			}

			// Lumels compressed exist ?
			if (ver<3)
			{
				// Reset shadows
				pa.resetCompressedLumels ();
			}
		}
	}
}


// ***************************************************************************
void			CZone::compile(CLandscape *landscape, TZoneMap &loadedZones)
{
	sint	i,j;
	TZoneMap		neighborZones;

	//nlinfo("Compile Zone: %d \n", (sint32)getZoneId());

	// Can't compile if compiled.
	nlassert(!Compiled);
	Landscape= landscape;

	// Attach this to loadedZones.
	//============================
	nlassert(loadedZones.find(ZoneId)==loadedZones.end());
	loadedZones[ZoneId]= this;
	
	// Create/link the base vertices according to present neigbor zones.
	//============================
	BaseVertices.clear();
	BaseVertices.resize(NumVertices);
	// First try to link vertices to other.
	for(i=0;i<(sint)BorderVertices.size();i++)
	{
		sint	cur= BorderVertices[i].CurrentVertex;
		sint	vertto= BorderVertices[i].NeighborVertex;
		sint	zoneto= BorderVertices[i].NeighborZoneId;
		nlassert(cur<NumVertices);

		if(loadedZones.find(zoneto)!=loadedZones.end())
		{
			CZone	*zone;
			zone= (*loadedZones.find(zoneto)).second;
			nlassert(zone!=this);
			// insert the zone in the neigborood (if not done...).
			neighborZones[zoneto]= zone;
			// Doesn't matter if BaseVertices is already linked to an other zone... 
			// This should be the same pointer in this case...
			BaseVertices[cur]=  zone->getBaseVertex(vertto);
		}
	}
	// Else, create unbounded vertices.
	for(i=0;i<(sint)BaseVertices.size();i++)
	{
		if(BaseVertices[i]==NULL)
		{
			BaseVertices[i]=  new CTessBaseVertex;
		}
	}


	// compile() the patchs.
	//======================
	for(j=0;j<(sint)Patchs.size();j++)
	{
		CPatch				&pa= Patchs[j];
		CPatchConnect		&pc= PatchConnects[j];
		CTessVertex			*baseVertices[4];

		baseVertices[0]= &(BaseVertices[pc.BaseVertices[0]]->Vert);
		baseVertices[1]= &(BaseVertices[pc.BaseVertices[1]]->Vert);
		baseVertices[2]= &(BaseVertices[pc.BaseVertices[2]]->Vert);
		baseVertices[3]= &(BaseVertices[pc.BaseVertices[3]]->Vert);
		pa.compile(this, j, pa.OrderS, pa.OrderT, baseVertices, pc.ErrorSize);
	};

	// bind() the patchs. (after all compiled).
	//===================
	for(j=0;j<(sint)Patchs.size();j++)
	{
		CPatch				&pa= Patchs[j];
		CPatchConnect		&pc= PatchConnects[j];

		bindPatch(loadedZones, pa, pc);
	}
	
	
	// rebindBorder() on neighbor zones.
	//==================================
	ItZoneMap		zoneIt;
	// Traverse the neighborood.
	for(zoneIt= neighborZones.begin(); zoneIt!=neighborZones.end(); zoneIt++)
	{
		(*zoneIt).second->rebindBorder(loadedZones);
	}

	// End!!
	Compiled= true;
}

// ***************************************************************************
void			CZone::release(TZoneMap &loadedZones)
{
	sint	i,j;

	if(!Compiled)
		return;

	// detach this zone to loadedZones.
	//=================================
	nlassert(loadedZones.find(ZoneId)!=loadedZones.end());
	loadedZones.erase(ZoneId);
	// It doesn't server to unbindPatch(), since patch is not binded to neigbors.


	// unbind() the patchs.
	//=====================
	for(j=0;j<(sint)Patchs.size();j++)
	{
		CPatch				&pa= Patchs[j];
		CPatchConnect		&pc= PatchConnects[j];
		unbindPatch(loadedZones, pa, pc);
		/* 
			This patch may be unbinded with exceptions (multiple patch case), but there is no problem, since in this case,
			his neighbor will (or have precedently) unbind.
			Since only Bind 1/1 are permitted on zone neighborood, there should be no problem:
			patch are unbinded with no exceptions.
		*/
	}


	// rebindBorder() on neighbor zones.
	//==================================
	// Build the nieghborood.
	TZoneMap		neighborZones;
	for(i=0;i<(sint)BorderVertices.size();i++)
	{
		sint	cur= BorderVertices[i].CurrentVertex;
		sint	zoneto= BorderVertices[i].NeighborZoneId;
		nlassert(cur<NumVertices);

		if(loadedZones.find(zoneto)!=loadedZones.end())
		{
			CZone	*zone;
			zone= (*loadedZones.find(zoneto)).second;
			nlassert(zone!=this);
			// insert the zone in the neigborood (if not done...).
			neighborZones[zoneto]= zone;
		}
	}
	// rebind borders.
	ItZoneMap		zoneIt;
	// Traverse the neighborood.
	for(zoneIt= neighborZones.begin(); zoneIt!=neighborZones.end(); zoneIt++)
	{
		// Since 
		(*zoneIt).second->rebindBorder(loadedZones);
	}


	// release() the patchs.
	//======================
	// unbind() need compiled neigbor patchs, so do the release after all unbind (so after rebindBorder() too...).
	for(j=0;j<(sint)Patchs.size();j++)
	{
		CPatch				&pa= Patchs[j];
		pa.release();
	}


	// destroy/unlink the base vertices (internal..), according to present neigbor zones.
	//=================================
	// Just release the smartptrs (easy!!). Do it after patchs released...
	BaseVertices.clear();
	

	// End!!
	Compiled= false;
	Landscape= NULL;
	ClipResult= ClipOut;
}


// ***************************************************************************
// ***************************************************************************
// Private part.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CZone::rebindBorder(TZoneMap &loadedZones)
{
	sint	j;

	// rebind patchs which are on border.
	for(j=0;j<(sint)Patchs.size();j++)
	{
		CPatch				&pa= Patchs[j];
		CPatchConnect		&pc= PatchConnects[j];

		if(patchOnBorder(pc))
			bindPatch(loadedZones, pa, pc);
	}
}

// ***************************************************************************
CPatch		*CZone::getZonePatch(TZoneMap &loadedZones, sint zoneId, sint patch)
{
#ifdef NL3D_DEBUG_DONT_BIND_PATCH
	return NULL;
#endif
	if(loadedZones.find(zoneId)==loadedZones.end())
		return NULL;
	else
		return (loadedZones[zoneId])->getPatch(patch);
}


// ***************************************************************************
void		CZone::buildBindInfo(uint patchId, uint edge, CZone *neighborZone, CPatch::CBindInfo	&paBind)
{
	nlassert(patchId < Patchs.size());
	nlassert(neighborZone);

	CPatchConnect	&pc= PatchConnects[patchId];


	// Get the bind info of this patch to his neighbor on "edge".
	CPatchInfo::CBindInfo	&pcBind= pc.BindEdges[edge];
	nlassert(pcBind.NPatchs==0 || pcBind.NPatchs==1 || pcBind.NPatchs==2 || pcBind.NPatchs==4 || pcBind.NPatchs==5);


	// copy zone ptr.
	paBind.Zone= neighborZone;


	// Special case of a small patch connected to a bigger.
	if(pcBind.NPatchs==5)
	{
		paBind.NPatchs= 1;
		paBind.Next[0]= neighborZone->getPatch(pcBind.Next[0]);
		paBind.Edge[0]= pcBind.Edge[0];
		
		// Get the twin bindInfo of pcBind.
		const CPatchInfo::CBindInfo	&pcBindNeighbor= 
			neighborZone->getPatchConnect(pcBind.Next[0])->BindEdges[pcBind.Edge[0]];
		// must have a multiple bind.	
		nlassert(pcBindNeighbor.NPatchs == 2 || pcBindNeighbor.NPatchs == 4);

		// number of bind is stored on the twin bindInfo.
		paBind.MultipleBindNum= pcBindNeighbor.NPatchs;

		// Search our patchId on neighbor;
		paBind.MultipleBindId= 255;
		for(sint i=0; i<paBind.MultipleBindNum; i++)
		{
			if(pcBindNeighbor.Next[i]==patchId)
				paBind.MultipleBindId= i;
		}
		nlassert(paBind.MultipleBindId!= 255);
	}
	else
	{
		paBind.MultipleBindNum= 0;
		paBind.NPatchs= pcBind.NPatchs;
		for(sint i=0;i<paBind.NPatchs; i++)
		{
			paBind.Next[i]= neighborZone->getPatch(pcBind.Next[i]);
			paBind.Edge[i]= pcBind.Edge[i];
		}
	}


}


// ***************************************************************************
void		CZone::unbindAndMakeBindInfo(TZoneMap &loadedZones, CPatch &pa, CPatchConnect &pc, CPatch::CBindInfo	edges[4])
{
	CPatch	*exceptions[4]= {NULL, NULL, NULL, NULL};

	/*
		Remind: the old version with CPatch::unbindFrom*() doesn't work because of CZone::release(). This function
		first erase the zone from loadedZones...
		Not matter here. We use CPatch::unbind() which should do all the good job correctly (unbind pa from ohters
		, and unbind others from pa at same time).
	*/

	// Fill all edges.
	for(sint i=0;i<4;i++)
	{
		CPatchInfo::CBindInfo	&pcBind= pc.BindEdges[i];
		CPatch::CBindInfo		&paBind= edges[i];

		nlassert(pcBind.NPatchs==0 || pcBind.NPatchs==1 || pcBind.NPatchs==2 || pcBind.NPatchs==4 || pcBind.NPatchs==5);
		paBind.NPatchs= pcBind.NPatchs;


		// Find the zone.
		TZoneMap::iterator	itZoneMap;
		// If no neighbor, or if zone neighbor not loaded.
		if( paBind.NPatchs==0 || (itZoneMap=loadedZones.find(pcBind.ZoneId)) == loadedZones.end() )
			paBind.Zone= NULL;
		else
			paBind.Zone= itZoneMap->second;


		// Special case of a small patch connected to a bigger.
		if(paBind.NPatchs==5)
		{
			// In this case, neither the unbind must not be done, nor the bind.
			exceptions[i]= CZone::getZonePatch(loadedZones, pcBind.ZoneId, pcBind.Next[0]);
			// This code prevent the bind in CPatch::bind() to be done!!
			// The bind must not be done, since exceptions[] prevent the unbind!
			paBind.NPatchs=0;
			continue;
		}


		if(paBind.NPatchs>=1)
		{
			paBind.Edge[0]= pcBind.Edge[0];
			paBind.Next[0]= CZone::getZonePatch(loadedZones, pcBind.ZoneId, pcBind.Next[0]);
			// If not loaded, don't bind to this edge.
			if(!paBind.Next[0])
				paBind.NPatchs=0;
		}
		if(paBind.NPatchs>=2)
		{
			paBind.Edge[1]= pcBind.Edge[1];
			paBind.Next[1]= CZone::getZonePatch(loadedZones, pcBind.ZoneId, pcBind.Next[1]);
			// If not loaded, don't bind to this edge.
			if(!paBind.Next[1])
				paBind.NPatchs=0;
		}
		if(paBind.NPatchs>=4)
		{
			paBind.Edge[2]= pcBind.Edge[2];
			paBind.Edge[3]= pcBind.Edge[3];
			paBind.Next[2]= CZone::getZonePatch(loadedZones, pcBind.ZoneId, pcBind.Next[2]);
			paBind.Next[3]= CZone::getZonePatch(loadedZones, pcBind.ZoneId, pcBind.Next[3]);
			// If not loaded, don't bind to this edge.
			if(!paBind.Next[2] || !paBind.Next[3])
				paBind.NPatchs=0;
		}
	}

	pa.unbind(exceptions);
}

// ***************************************************************************
void		CZone::bindPatch(TZoneMap &loadedZones, CPatch &pa, CPatchConnect &pc)
{
	CPatch::CBindInfo	edges[4];

	unbindAndMakeBindInfo(loadedZones, pa, pc, edges);

	pa.bind(edges);
}


// ***************************************************************************
void		CZone::unbindPatch(TZoneMap &loadedZones, CPatch &pa, CPatchConnect &pc)
{
	CPatch::CBindInfo	edges[4];

	unbindAndMakeBindInfo(loadedZones,  pa, pc, edges);

	// Don't rebind.
}


// ***************************************************************************
bool			CZone::patchOnBorder(const CPatchConnect &pc) const
{
	// If only one of neighbor patch is not of this zone, we are on a border.

	// Test all edges.
	for(sint i=0;i<4;i++)
	{
		const CPatchInfo::CBindInfo	&pcBind= pc.BindEdges[i];

		nlassert(pcBind.NPatchs==0 || pcBind.NPatchs==1 || pcBind.NPatchs==2 || pcBind.NPatchs==4 || pcBind.NPatchs==5);
		if(pcBind.NPatchs>=1)
		{
			if(pcBind.ZoneId != ZoneId)
				return true;
		}
	}

	return false;
}


// ***************************************************************************
// ***************************************************************************
// Render part.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CZone::clip(const std::vector<CPlane>	&pyramid)
{
	nlassert(Compiled);

	// Store current pyramid.
	CurrentPyramid= pyramid;

	// bkup old ClipResult. NB: by default, it is ClipOut (no VB created).
	sint	oldClipResult= ClipResult;

	// Compute ClipResult.
	//-------------------
	ClipResult= ClipIn;
	for(sint i=0;i<(sint)pyramid.size();i++)
	{
		// If entirely out.
		if(!ZoneBB.clipBack(pyramid[i]))
		{
			ClipResult= ClipOut;
			// If out of only one plane, out of all.
			break;
		}
		// If partially IN (ie not entirely out, and not entirely IN)
		else if(ZoneBB.clipFront(pyramid[i]))
		{
			// Force ClipResult to be ClipSide, and not ClipIn.
			ClipResult=ClipSide;
		}
	}

	// Fill computeTileErrorMetric.
	CBSphere		zonesphere(ZoneBB.getCenter(), ZoneBB.getRadius());
	if(zonesphere.intersect(CTessFace::TileFarSphere))
		ComputeTileErrorMetric= true;
	else
		ComputeTileErrorMetric= false;

	// Easy Clip  :)
	if(Patchs.size()==0)
	{
		ClipResult= ClipOut;
	}


	// Clip By Patch Pass.
	//--------------------
	if(ClipResult==ClipOut)
	{
		CPatch		*pPatch= &(*Patchs.begin());
		for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
		{
			// The patch is entirely clipped, and so on for Render.
			pPatch->forceClip();
			pPatch->forceRenderClip();
		}
	}
	else if(ClipResult==ClipIn)
	{
		CPatch		*pPatch= &(*Patchs.begin());
		for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
		{
			// The patch is entirely unclipped, and so on for Render.
			pPatch->forceNoClip();
			pPatch->forceNoRenderClip();
		}
	}
	else
	{
		CPatch		*pPatch= &(*Patchs.begin());
		for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
		{
			pPatch->clip(CurrentPyramid);
		}
	}


	// delete / reallocate / fill VBuffers.
	//-------------------
	// If there is a change in the Clip of the zone, or if patchs may have change (ie ClipSide is undetermined).
	if(oldClipResult!=ClipResult || oldClipResult==ClipSide)
	{
		// Then, we must test by patch.
		CPatch		*pPatch= &(*Patchs.begin());
		for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
		{
			// For all patchs, we may delete or allocate / Fill VBs.
			pPatch->updateClipPatchVB();
		}

	}

}


// ***************************************************************************
// DebugYoyo.
// Code for Debug test Only.. Do not erase it, may be used later :)
/*
static	void	cleanTess(CTessFace *face)
{
	if(!face->isLeaf())
	{
		cleanTess(face->SonLeft);
		cleanTess(face->SonRight);
	}
	// If has father, clean it.
	if(face->Father)
	{
		CTessFace	*face1=face->Father;
		CTessFace	*face2=face->Father->FBase;
		face1->FLeft= face1->SonLeft->FBase;
		face1->FRight= face1->SonRight->FBase;
		if(face2!=NULL)
		{
			face2->FLeft= face2->SonLeft->FBase;
			face2->FRight= face2->SonRight->FBase;
		}
	}
}
static	void	testTess(CTessFace *face)
{
	if(!face->isLeaf())
	{
		testTess(face->SonLeft);
		testTess(face->SonRight);
	}
	// Test validity.
	nlassert(!face->FBase || face->FBase->Patch!=(CPatch*)0xdddddddd);
	nlassert(!face->FLeft || face->FLeft->Patch!=(CPatch*)0xdddddddd);
	nlassert(!face->FRight || face->FRight->Patch!=(CPatch*)0xdddddddd);
}
static	void	checkTess()
{
	// This test should be inserted at begin of CZone::refine().
	// And it needs hacking public/private.
	CPatch		*pPatch;
	sint		n;
	pPatch= &(*Patchs.begin());
	for(n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		cleanTess(pPatch->Son0);
		cleanTess(pPatch->Son1);
	}
	pPatch= &(*Patchs.begin());
	for(n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		testTess(pPatch->Son0);
		testTess(pPatch->Son1);
	}
}
*/


// ***************************************************************************
// DebugYoyo.
//volatile sint pipo1;
//volatile sint pipo2;
void			CZone::refine()
{
	nlassert(Compiled);
	// Must be 2^X-1.
	static const	sint	hideRefineFreq= 15;

	// DebugYoyo.
	// For the monkey bind test.
	/*extern sint numFrames;
	pipo1=(rand()>>12)&1;
	pipo2=(rand()>>12)&1;
	//if(pipo1 && numFrames>1360)
	if(true)
	{
		TZoneMap	pipoMap;
		pipoMap[ZoneId]= this;
		bindPatch(pipoMap, Patchs[0], PatchConnects[0]);
	}*/

	
	// Force refine of invisible zones only every 8 times.
	if(ClipResult==ClipOut && (CTessFace::CurrentDate & hideRefineFreq)!=(ZoneId & hideRefineFreq))
		return;
	// Fuck stlport....
	if(Patchs.size()==0)
		return;

	CPatch		*pPatch= &(*Patchs.begin());
	if(ClipResult==ClipSide)
	{
		// Force refine of invisible patchs only every 16 times.
		// NB: do this only if zone is clipSide
		for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
		{
			// "ZoneId+n", because there is only 70 approx patchs per zone. doing this may stabilize framerate.
			if(pPatch->isClipped() && (CTessFace::CurrentDate & hideRefineFreq)!=((ZoneId+n) & hideRefineFreq))
				continue;
			pPatch->refine();
		}
	}
	else
	{
		// Else refine ALL patchs (even those which may be invisible).
		for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
		{
			pPatch->refine();
		}
	}

}


// ***************************************************************************
void			CZone::excludePatchFromRefineAll(uint patch, bool exclude)
{
	nlassert(Compiled);
	nlassert(patch<Patchs.size());

	if(patch>=Patchs.size())
		return;

	Patchs[patch].ExcludeFromRefineAll= exclude;
}


// ***************************************************************************
void			CZone::refineAll()
{
	nlassert(Compiled);

	// Fuck stlport....
	if(Patchs.size()==0)
		return;

	// Do a dummy clip.
	ComputeTileErrorMetric= true;
	CPatch		*pPatch= &(*Patchs.begin());
	sint n;
	for(n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		pPatch->forceNoClip();
		// DO NOT do a forceNoRenderClip(), to avoid big allocation of Near/Far VB vertices in driver.
	}
	// DO NOT modify ClipResult, to avoid big allocation of Near/Far VB vertices in driver.


	// refine ALL patchs (even those which may be invisible).
	pPatch= &(*Patchs.begin());
	for(n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		// For Pacs construction: may exclude some patch from refineAll (for speed improvement).
		if(!pPatch->ExcludeFromRefineAll)
			pPatch->refine();
	}

}


// ***************************************************************************
void			CZone::averageTesselationVertices()
{
	nlassert(Compiled);

	// Fuck stlport....
	if(Patchs.size()==0)
		return;

	// averageTesselationVertices of ALL patchs.
	CPatch		*pPatch= &(*Patchs.begin());
	for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		pPatch->averageTesselationVertices();
	}
}


// ***************************************************************************
void			CZone::preRender(const std::vector<CPlane>	&pyramid)
{
	nlassert(Compiled);
	if(ClipResult==ClipOut)
		return;

	// PreRender Pass.
	CPatch		*pPatch= &(*Patchs.begin());
	for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		pPatch->preRender(pyramid);
	}
}


// ***************************************************************************
void			CZone::renderFar0()
{
	nlassert(Compiled);
	if(ClipResult==ClipOut)
		return;

	// RenderFar0.
	CPatch		*pPatch= &(*Patchs.begin());
	for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		pPatch->renderFar0();
	}
}
// ***************************************************************************
void			CZone::renderFar1()
{
	nlassert(Compiled);
	if(ClipResult==ClipOut)
		return;

	// RenderFar1.
	CPatch		*pPatch= &(*Patchs.begin());
	for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		pPatch->renderFar1();
	}
}
// ***************************************************************************
void			CZone::renderTile(sint pass)
{
	nlassert(Compiled);
	if(ClipResult==ClipOut)
		return;

	// RenderTile.
	CPatch		*pPatch= &(*Patchs.begin());
	for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		pPatch->renderTile(pass);
	}
}


// ***************************************************************************
void			CZone::resetRenderFar()
{
	CPatch		*pPatch;
	if(Patchs.size()>0)
		pPatch= &(*Patchs.begin());
	for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		pPatch->resetRenderFar();
	}
}


// ***************************************************************************
void			CZone::forceMergeAtTileLevel()
{
	CPatch		*pPatch;
	if(Patchs.size()>0)
		pPatch= &(*Patchs.begin());
	for(sint n=(sint)Patchs.size();n>0;n--, pPatch++)
	{
		pPatch->forceMergeAtTileLevel();
	}
}


// ***************************************************************************
// ***************************************************************************
// Misc part.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CZone::changePatchTextureAndColor (sint numPatch, const std::vector<CTileElement> *tiles, const std::vector<CTileColor> *colors)
{
	nlassert(numPatch>=0);
	nlassert(numPatch<getNumPatchs());
	

	// Update the patch texture.
	if (tiles)
	{
		nlassert( Patchs[numPatch].Tiles.size() == tiles->size() );
		Patchs[numPatch].Tiles = *tiles;
	}

	// Update the patch colors.
	if (colors)
	{
		nlassert( Patchs[numPatch].TileColors.size() == colors->size() );
		Patchs[numPatch].TileColors = *colors;
	}

	if (Compiled)
	{
		// If the patch is visible, then we must LockBuffers, because new VertexVB may be created.
		if(Patchs[numPatch].RenderClipped)
			Landscape->updateGlobalsAndLockBuffers(CVector::Null);

		// Recompute UVs for new setup of Tiles.
		Patchs[numPatch].deleteTileUvs();
		Patchs[numPatch].recreateTileUvs();

		// unlockBuffers() if necessary.
		if(Patchs[numPatch].RenderClipped)
			Landscape->unlockBuffers();
	}
}


// ***************************************************************************
void			CZone::refreshTesselationGeometry(sint numPatch)
{
	nlassert(numPatch>=0);
	nlassert(numPatch<getNumPatchs());
	nlassert(Compiled);

	// At next render, we must re-fill the entire unclipped VB, so change are taken into account.
	Landscape->_RenderMustRefillVB= true;

	Patchs[numPatch].refreshTesselationGeometry();
}


// ***************************************************************************
const std::vector<CTileElement> &CZone::getPatchTexture(sint numPatch) const
{
	nlassert(numPatch>=0);
	nlassert(numPatch<getNumPatchs());

	// Update the patch texture.
	return Patchs[numPatch].Tiles;
}


// ***************************************************************************
const std::vector<CTileColor> &CZone::getPatchColor(sint numPatch) const
{
	nlassert(numPatch>=0);
	nlassert(numPatch<getNumPatchs());

	// Update the patch texture.
	return Patchs[numPatch].TileColors;
}


// ***************************************************************************
void			CZone::debugBinds(FILE *f)
{
	fprintf(f, "*****************************\n");
	fprintf(f, "ZoneId: %d. NPatchs:%d\n", ZoneId, PatchConnects.size());
	sint i;
	for(i=0;i<(sint)PatchConnects.size();i++)
	{
		CPatchConnect	&pc= PatchConnects[i];
		fprintf(f, "patch%d:\n", i);
		for(sint j=0;j<4;j++)
		{
			CPatchInfo::CBindInfo	&bd= pc.BindEdges[j];
			fprintf(f, "    edge%d: Zone:%d. NPatchs:%d. ", j, bd.ZoneId, bd.NPatchs);
			for(sint k=0;k<bd.NPatchs;k++)
			{
				fprintf(f, "p%de%d - ", bd.Next[k], bd.Edge[k]);
			}
			fprintf(f, "\n");
		}
	}

	fprintf(f,"Vertices :\n");
	for(i=0;i<(sint)BorderVertices.size();i++)
	{
		fprintf(f,"current : %d -> (zone %d) vertex %d\n",BorderVertices[i].CurrentVertex,
											BorderVertices[i].NeighborZoneId,
											BorderVertices[i].NeighborVertex);
	}
}


// ***************************************************************************
void			CZone::applyHeightField(const CLandscape &landScape)
{
	sint	i,j;
	vector<CBezierPatch>	patchs;

	// no patch, do nothing.
	if(Patchs.size()==0)
		return;

	// 0. Unpack patchs to Bezier Patchs.
	//===================================
	patchs.resize(Patchs.size());
	for(j=0;j<(sint)patchs.size();j++)
	{
		CBezierPatch		&p= patchs[j];
		CPatch				&pa= Patchs[j];

		// re-Build the uncompressed bezier patch.
		for(i=0;i<4;i++)
			pa.Vertices[i].unpack(p.Vertices[i], PatchBias, PatchScale);
		for(i=0;i<8;i++)
			pa.Tangents[i].unpack(p.Tangents[i], PatchBias, PatchScale);
		for(i=0;i<4;i++)
			pa.Interiors[i].unpack(p.Interiors[i], PatchBias, PatchScale);
	}

	// 1. apply heightfield on bezier patchs.
	//===================================
	for(j=0;j<(sint)patchs.size();j++)
	{
		CBezierPatch		&p= patchs[j];

		// apply delta.
		for(i=0;i<4;i++)
			p.Vertices[i]+= landScape.getHeightFieldDeltaZ(p.Vertices[i].x, p.Vertices[i].y);
		for(i=0;i<8;i++)
			p.Tangents[i]+= landScape.getHeightFieldDeltaZ(p.Tangents[i].x, p.Tangents[i].y);
		for(i=0;i<4;i++)
			p.Interiors[i]+= landScape.getHeightFieldDeltaZ(p.Interiors[i].x, p.Interiors[i].y);
	}


	// 2. Re-compute Patch Scale/Bias, and Zone BBox.
	//===================================
	CAABBox		bb;
	bb.setCenter(patchs[0].Vertices[0]);
	bb.setHalfSize(CVector::Null);
	for(j=0;j<(sint)patchs.size();j++)
	{
		// extend bbox.
		const CBezierPatch	&p= patchs[j];
		for(i=0;i<4;i++)
			bb.extend(p.Vertices[i]);
		for(i=0;i<8;i++)
			bb.extend(p.Tangents[i]);
		for(i=0;i<4;i++)
			bb.extend(p.Interiors[i]);
	}
	// Compute BBox, and Patch Scale Bias, according to Noise.
	computeBBScaleBias(bb);


	// 3. Re-pack patchs.
	//===================================
	for(j=0;j<(sint)patchs.size();j++)
	{
		CBezierPatch		&p= patchs[j];
		CPatch				&pa= Patchs[j];

		// Build the packed patch.
		for(i=0;i<4;i++)
			pa.Vertices[i].pack(p.Vertices[i], PatchBias, PatchScale);
		for(i=0;i<8;i++)
			pa.Tangents[i].pack(p.Tangents[i], PatchBias, PatchScale);
		for(i=0;i<4;i++)
			pa.Interiors[i].pack(p.Interiors[i], PatchBias, PatchScale);
	}
}



} // NL3D
