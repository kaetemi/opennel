/** \file ig_surface_light_build.h
 * TODO: File description
 *
 * $Id$
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

#ifndef NL_IG_SURFACE_LIGHT_BUILD_H
#define NL_IG_SURFACE_LIGHT_BUILD_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector_2f.h"
#include "nel/3d/surface_light_grid.h"
#include "nel/3d/mesh.h"


namespace NL3D 
{


// ***************************************************************************
/**
 * Class used by NL3D::CInstanceLighter to build CIGSurfaceLight with PACS surfaces
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CIGSurfaceLightBuild
{
public:

	enum	{MaxOverSamples= 16};

public:

	/** A surface cell corner Information.
	 */
	struct	CCellCorner
	{
		// Is this CellCorner in the surface polygon or not?
		bool				InSurface;
		// Computed by CInstanceLighter
		bool				Dilated;
		// Copy of SunContribution computed by CInstanceLighter.
		uint8				SunContribution;
		// Number of overSamples. At least one if InSurface. NB: Put here for packing. 
		uint8				NumOverSamples;

		// World Position of this corner.
		NLMISC::CVector		CenterPos;

		// OverSamples. NB: there can be any number of overSamples (3, 4, 5,7 ...) because
		// some may be out of surface and thus disabled
		NLMISC::CVector		OverSamples[MaxOverSamples];


		/** This is Temp Light information used during CInstanceLighter::light().
		 *	Yes, void* is ugly, but it is to avoid too much dependencies.
		 */
		void				*LightInfo[CSurfaceLightGrid::NumLightPerCorner];
		void				*LocalAmbientLight;
	};


	/** A surface Lighting Information. Filled by CInstanceLighter
	 */
	struct	CSurface
	{
		// Origin and Size of the grid.
		// NB: The grid must be valid an not empty (ie Width>=2 and Height>=2).
		NLMISC::CVector2f			Origin;
		uint32						Width;
		uint32						Height;
		// The CellCorner info.
		std::vector<CCellCorner>	Cells;
	};


	/** A LocalRetriever Lighting Information.
	 *
	 */
	struct	CRetrieverLightGrid
	{
		// There is localRetriver.getNumSurfaces() Grids.
		std::vector<CSurface>		Grids;
	};
	typedef std::map<uint, CRetrieverLightGrid>		TRetrieverGridMap;
	typedef TRetrieverGridMap::iterator				ItRetrieverGridMap;


	// The requested CellSize (setuped by CInstanceLighter)
	float					CellSize;
	// Array of surfaces retrieved in PACS for an IG
	TRetrieverGridMap		RetrieverGridMap;


public:

	/// Debug: build a colored Grid mesh of SunContribution.
	void			buildSunDebugMesh(CMesh::CMeshBuild &meshBuild, CMeshBase::CMeshBaseBuild &meshBaseBuild, const CVector &deltaPos=CVector::Null);

	/// Debug: build a colored Grid mesh of PointLight. R= pointLight1 id. G= PointLight2 id. B= The multiplier used to show Ids.
	void			buildPLDebugMesh(CMesh::CMeshBuild &meshBuild, CMeshBase::CMeshBaseBuild &meshBaseBuild, const CVector &deltaPos, const CInstanceGroup &igOut);

private:
	void			addDebugMeshFaces(CMesh::CMeshBuild &meshBuild, CSurface &surface, uint vId0, 
		const std::vector<CRGBA>	&colors);

};


} // NL3D


#endif // NL_IG_SURFACE_LIGHT_BUILD_H

/* End of ig_surface_light_build.h */

/* Merge OpenNeL SVN
 */
