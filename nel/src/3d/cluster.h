/** \file cluster.h
 * Definition of a cluster/portal visibility
 *
 * $Id: cluster.h,v 1.1 2001/07/30 14:40:14 besson Exp $
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

#ifndef NL_CLUSTER_H
#define NL_CLUSTER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/plane.h"
#include "nel/misc/aabbox.h"

#include "3d/transform.h"

#include <vector>

namespace NLMISC
{
class CRGBA;
class IStream;
struct EStream;
}

namespace NL3D {


class CPortal;
class CClusterClipObs;
class CInstanceGroup;

const NLMISC::CClassId	ClusterId=NLMISC::CClassId(0x13f37e46, 0x3e880780);

/**
 * CCluster
 *	
 * The objects are in the MOT clip traversal
 *
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2001
 */
class CCluster : public CTransform
{

public:

	static void registerBasic ();
	
public:

	CCluster();

	/**
	 * Make the cluster volume from a plane made with 3 points
	 * All the faces passed must be connected with at least one of the others
	 * return false if the plane we want to add make the volume concave
	 */
	bool makeVolume (NLMISC::CVector& p1, NLMISC::CVector& p2, NLMISC::CVector& p3);

	/**
	 * return true if the vertex is in the cluster volume
	 */
	bool isIn (NLMISC::CVector& p);

	/**
	 * return true if the Bounding box intersect the cluster
	 */
	bool isIn (NLMISC::CAABBox& b);

	/**
	 * return true if the Sphere intersect the cluster
	 */
	bool isIn (NLMISC::CVector& center, float size);

	NLMISC::CAABBox& getBBox() { return _BBox; }
	
	/// Linking

	void link (CPortal* portal);
	void unlink (CPortal* portal);

	uint32 getNbPortals() {return _Portals.size();}
	CPortal* getPortal(uint32 pos) {return _Portals[pos];}

	CClusterClipObs *getClipObs();

	/// Serial

	void serial (NLMISC::IStream& f);

	void setWorldMatrix (const CMatrix &WM);

	// Moving cluster
	CQuadGrid<CCluster*>::CIterator AccelIt;

	// Additionnal properties
	bool FatherVisible;
	bool VisibleFromFather;
	CCluster *Father;
	std::vector<CCluster*> Children;
	CInstanceGroup *Group;

	bool isRoot() { return _LocalVolume.size() == 0; }

private:

	static IModel *creator () {return new CCluster;}

private:

	/// Shortcut to the clip observer
	CClusterClipObs *_Obs;

	/// Portals list
	std::vector<CPortal*>	_Portals;

	/// Local bounding box and volume
	/// -----------------------------
	/// The bounding box of the volume
	NLMISC::CAABBox _LocalBBox;
	/// All planes are oriented to the exterior of the volume
	std::vector<NLMISC::CPlane> _LocalVolume;

	/// World bounding box and volume (valid after hrc trav)
	/// -----------------------------
	NLMISC::CAABBox _BBox;
	/// All planes are oriented to the exterior of the volume
	std::vector<NLMISC::CPlane> _Volume;

	/// Friends classes
	friend class CInstanceGroup;
};

/**
 * CClusterHrcObs
 *
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2001
 */
class CClusterHrcObs : public CTransformHrcObs
{

public:

	virtual	void traverse (IObs *caller);

	static IObs	*creator () {return new CClusterHrcObs;}

	/// Friend class
	friend class CCluster;
};

/**
 * CClusterClipObs
 *
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2001
 */
class CClusterClipObs : public IBaseClipObs
{

public:
	CClusterClipObs();
	
	/// don't render.
	virtual	bool isRenderable () const;

	/// Don't clip.
	virtual	bool clip (IBaseClipObs *caller);

	/// just traverseSons().
	virtual	void traverse (IObs *caller);

private:

	bool Visited;

private:

	static IObs *creator () {return new CClusterClipObs;}

	/// Friend class
	friend class CCluster;
};


} // NL3D


#endif // NL_CLUSTER_H

/* End of cluster.h */
