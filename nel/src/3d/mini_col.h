/** \file mini_col.h
 * TODO: File description
 *
 * $Id: mini_col.h,v 1.3 2004/11/15 10:24:44 lecroart Exp $
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

#ifndef NL_MINI_COL_H
#define NL_MINI_COL_H


#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include "nel/misc/plane.h"
#include "nel/misc/triangle.h"
#include "3d/quad_grid.h"
#include "3d/landscape.h"
#include <set>


namespace NL3D 
{


using	NLMISC::CVector;
using	NLMISC::CPlane;


// ***************************************************************************
/**
 * This is a TEMPORARY collision "system".
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CMiniCol
{
public:

	/// Constructor
	CMiniCol();


	/// Init the size of the collision system, and init it with the landscape.
	void			init(CLandscape *land, float radMin=100, float radDelta=50);

	/// Add a zone to the collision system. Zone must be loaded into the landscape before.
	void			addZone(uint16 zoneId);

	/// Remove a zone from the collision system. Zone do not have to be loaded into the landscape.
	void			removeZone(uint16 zoneId);

	/// Reset the center of interset of the collision zone.
	void			setCenter(const CVector& center);


	/** This function test if a move is OK, by snaping it to ground, and test if angle is<45deg.
	 * If !OK, cur is set to prec, and false is returned.
	 */
	bool			testMove(const CVector &prec, CVector &cur);

	/** This function snap a position on the current set of faces.
	 * hbot and hup are the margin where pos.z can't change. (the pos can't move higher than +hup and lower than -hbot)
	 */
	bool			snapToGround(CVector &pos, float hup=0.5, float hbot=1000);


	/** This function get the ground normal under a position.
	 * hbot and hup are the margin where face can be taken.
	 */
	bool			getGroundNormal(const CVector &pos, CVector &normal, float hup=0.5, float hbot=1000);


	/** This function get the faces which intersect a bbox..
	 */
	void			getFaces(std::vector<CTriangle>	&triresult, const CAABBox &bbox);



// *****************************
private:
	struct	CFace
	{
		CTriangle	Face;
		CPlane		Plane;
		uint16		ZoneId;		// From which zone this face come from...
		uint16		PatchId;	// From which patch this face come from...

		bool		isFromPatch(sint zoneId, sint patchId) const
		{
			return ZoneId==zoneId && PatchId==patchId;
		}
	};

	struct	CPatchIdent
	{
		CBSphere	Sphere;
		bool		Inserted;

		CPatchIdent() {Inserted= false;}
	};

	struct	CZoneIdent
	{
		sint		ZoneId;
		CBSphere	Sphere;
		sint		NPatchInserted;		// number of patch inserted.
		std::vector<CPatchIdent>		Patchs;

		CZoneIdent() {NPatchInserted= 0;}
		bool		operator<(const CZoneIdent &z) const {return ZoneId<z.ZoneId;}
	};



	typedef	CQuadGrid<CFace>		TGrid;
	typedef	std::set<CZoneIdent>	TZoneSet;

private:
	CRefPtr<CLandscape>	_Landscape;
	float				_RadMin, _RadMax;
	TGrid				_Grid;
	TZoneSet			_Zones;

	//bool				_Inited;

private:
	void			addFaces(const std::vector<CTriangle> &faces, uint16 zoneId, uint16 patchId);
	void			addLandscapePart(uint16 zoneId, uint16 patchId);
	void			removeLandScapePart(uint16 zoneId, uint16 patchId, const CBSphere &sphere);


};


} // NL3D


#endif // NL_MINI_COL_H

/* End of mini_col.h */
