/** \file surface_light_grid.h
 * <File description>
 *
 * $Id: surface_light_grid.h,v 1.1 2002/02/06 16:54:57 berenguier Exp $
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

#ifndef NL_SURFACE_LIGHT_GRID_H
#define NL_SURFACE_LIGHT_GRID_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/object_vector.h"
#include "nel/misc/vector_2f.h"
#include "nel/misc/vector.h"
#include "nel/3d/point_light_influence.h"


namespace NL3D 
{


using	NLMISC::CVector;


class	CIGSurfaceLight;


// ***************************************************************************
/**
 * Dynamic objects lighted by IG's Static Lights lookup into this grid.
 *	Builded by CInstanceLighter.
 *	Used by CIGSurfaceLight
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CSurfaceLightGrid
{
public:

	/// We support only 2 light per corner. Should never be changed.
	enum	{NumLightPerCorner= 2};


	struct	CCellCorner
	{
		/// Contribution of Sun.
		uint8	SunContribution;
		/// Light that influence this point. 0xFF if none. if Light[i]==0xFF, then Light[j] with j>i is disabled too
		uint8	Light[NumLightPerCorner];

		void		serial(NLMISC::IStream &f)
		{
			uint	ver= f.serialVersion(0);
			nlassert(NumLightPerCorner==2);
			f.serial(SunContribution);
			f.serial(Light[0]);
			f.serial(Light[1]);
		}
	};


public:
	// Origin of the grid. Size is (Width-1) * CIGSurfaceLight.getCellSize().
	NLMISC::CVector2f					Origin;
	uint32								Width;
	uint32								Height;
	NLMISC::CObjectVector<CCellCorner>	Cells;


public:

	/// Constructor
	CSurfaceLightGrid();

	// Get Infos from the grid
	void		getStaticLightSetup(const CVector &localPos, std::vector<CPointLightInfluence> &pointLightList, uint8 &sunContribution, CIGSurfaceLight &igsl) const;

	// serial
	void		serial(NLMISC::IStream &f);

};


} // NL3D


#endif // NL_SURFACE_LIGHT_GRID_H

/* End of surface_light_grid.h */
