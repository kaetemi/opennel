/** \file landscape_def.h
 * Global Definitions for Landscape
 *
 * $Id: landscape_def.h,v 1.4 2001/11/07 16:41:53 berenguier Exp $
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

#ifndef NL_LANDSCAPE_DEF_H
#define NL_LANDSCAPE_DEF_H

#include "nel/misc/types_nl.h"
#include "nel/misc/bsphere.h"
#include "3d/landscapevb_info.h"


namespace NL3D 
{

using	NLMISC::CVector;


class	CLandscapeVBAllocator;
class	IDriver;


// ***************************************************************************
// 4th pass is always the Lightmapped one (Lightmap*clouds).
#define	NL3D_MAX_TILE_PASS 5
// There is no Face for lightmap, since lightmap pass share the RGB0 face.
#define	NL3D_MAX_TILE_FACE	NL3D_MAX_TILE_PASS-1

#define	NL3D_TILE_PASS_RGB0		0
#define	NL3D_TILE_PASS_RGB1		1
#define	NL3D_TILE_PASS_RGB2		2
#define	NL3D_TILE_PASS_ADD		3
#define	NL3D_TILE_PASS_LIGHTMAP	4
// NB: RENDER ORDER: CLOUD*LIGHTMAP is done BEFORE ADDITIVE.


// ***************************************************************************
// see CTessFace::updateRefineMerge()
#define	NL3D_REFINE_MERGE_THRESHOLD		2.0f


// ***************************************************************************
/// For Landscape Vegetable: Distance Types are: 10m, 20m, 30m, 40m, 50m.
#define	NL3D_LANDSCAPE_VEGETABLE_BLOCK_NUMDIST	5
#define	NL3D_LANDSCAPE_VEGETABLE_BLOCK_ELTDIST	10.0f


// ***************************************************************************
class	CLandscapeGlobals
{
public:
	// LANDSCAPE RENDERING CONTEXT.  Landscape must setup it at the begining at refine()/render().
	// The current date of LandScape for refine only.
	static	sint	CurrentDate;
	// The current date of LandScape for render only.
	static	sint	CurrentRenderDate;
	// The center view for refinement.
	static	CVector RefineCenter;
	// What is the treshold for tessellation.
	static	float	RefineThreshold;
	// Guess.
	static	float	OORefineThreshold;


	// Tile Global Info.
	// What are the limit distances for Tile tesselation transition.
	static	float	TileDistNear, TileDistFar;
	// System, computed from prec.
	static	float	TileDistNearSqr, TileDistFarSqr;
	// System, computed from prec.
	static	float	OOTileDistDeltaSqr;
	// The tiles are not subdivided above this limit (but because of enforced splits). Default: 4 => 50cm.
	static	sint	TileMaxSubdivision;
	// The sphere for TileFar test.
	static	NLMISC::CBSphere	TileFarSphere;
	// The sphere for TileNear test.
	static	NLMISC::CBSphere	TileNearSphere;
	// The size of a 128x128 tile, in pixel. UseFull for HalfPixel Scale/Bias.
	static	float		TilePixelSize;


	// Render Global info. Used by Patch.
	// The distance transition for Far0 and Far1 (200m / 400m).
	static	float	Far0Dist, Far1Dist;
	// Distance for Alpha blend transition
	static	float	FarTransition;


	// This Tells if VertexProgram is activated for the current landscape.
	static	bool					VertexProgramEnabled;
	// The current VertexBuffer for Far0
	static	CFarVertexBufferInfo	CurrentFar0VBInfo;
	// The current VertexBuffer for Far1.
	static	CFarVertexBufferInfo	CurrentFar1VBInfo;
	// The current VertexBuffer for Tile.
	static	CNearVertexBufferInfo	CurrentTileVBInfo;

	// The current VertexBuffer Allocator for Far0
	static	CLandscapeVBAllocator	*CurrentFar0VBAllocator;
	// The current VertexBuffer Allocator for Far1.
	static	CLandscapeVBAllocator	*CurrentFar1VBAllocator;
	// The current VertexBuffer Allocator for Tile.
	static	CLandscapeVBAllocator	*CurrentTileVBAllocator;


	// PATCH GLOBAL INTERFACE.  patch must setup them at the begining at refine()/render().
	// NO!!! REMIND: can't have any patch global, since a propagated split()/updateErrorMetric()
	// can arise. must use Patch pointer.

	// Render:
	// Globals for speed render.
	static IDriver				*PatchCurrentDriver;
	// The triangles array for the current pass rendered.
	static std::vector<uint32>	PassTriArray;
	static uint					PassNTri;


};



} // NL3D


#endif // NL_LANDSCAPE_DEF_H

/* End of landscape_def.h */
