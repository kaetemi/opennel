/** \file quad_grid_clip_manager.h
 * TODO: File description
 *
 * $Id: quad_grid_clip_manager.h,v 1.8 2005/02/22 10:19:11 besson Exp $
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

#ifndef NL_QUAD_GRID_CLIP_MANAGER_H
#define NL_QUAD_GRID_CLIP_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/plane.h"
#include "quad_grid_clip_cluster.h"
#include <vector>
#include "fast_ptr_list.h"
#include "transform.h"


namespace NL3D 
{

using	NLMISC::CVector;
using	NLMISC::CPlane;


class CScene;
class CTransformShape;
class CClipTrav;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		QuadGridClipManagerId=NLMISC::CClassId(0x1ffb079a, 0x6c536a96);


// ***************************************************************************
/**
 * A quadgrix of QuadGridCluster
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CQuadGridClipManager : public CTransform
{
public:
	/// Call at the begining of the program, to register the model
	static	void	registerBasic();

public:

	/// Constructor
	CQuadGridClipManager();
	~CQuadGridClipManager();
	/** Init the manager.
	 *	\param clusterSize is the size of a cluster. eg: 160mx160m.
	 *	\param maxDists eg (100, 500). Intervalls of distance for fast DistMax clip optimisation.
	 *	there is maxDists.size()+1  clusters created by case of clusterSize*clusterSize. And as so many test.
	 *	\param radiusMax a square of radiusMax*2 x radiusMax*2  of CQuadGridCluster are ensured to be created.
	 */
	void				init(float clusterSize, uint numDist, float maxDist, float radiusMax );

	/// delete clusters from scene, and reset the manager.
	void				reset();

	/** create / delete QuadGridClusters around us.
	 */
	void				updateClustersFromCamera(const CVector &camPos);


	/** link a model to the best cluster possible, and update the bbox of this cluster.
	 *	if out of range (no cluster found), don't link to any and return false.
	 *	NB: pTfmShp->getFirstParent() should be NULL.
	 */
	bool				linkModel(CTransformShape *pTfmShp);

	/**	output (nlinfo) Stats for Usage of the QuadClip
	 */
	void				profile() const;

	/// \name CTransform traverse specialisation. Only clip is special
	// @{
	virtual void	traverseHrc() {}
	virtual void	traverseClip();
	virtual void	traverseAnimDetail() {}
	virtual void	traverseLoadBalancing() {}
	virtual void	traverseLight() {}
	virtual void	traverseRender() {}
	virtual	void	profileRender() {}
	// @}

private:
	static CTransform	*creator() {return new CQuadGridClipManager;}

private:
	float							_ClusterSize;
	float							_RadiusMax;
	float							_MaxDist;
	uint							_NumDist;
	sint							_X, _Y;
	sint							_Width, _Height;
	std::vector<CQuadGridClipCluster*>	_QuadGridClusterCases;

	// List of not empty QuadGridClusters
	typedef CFastPtrList<CQuadGridClipCluster>	TClusterList;
	TClusterList					_NotEmptyQuadGridClipClusters;


	void				deleteCaseModels(CClipTrav *pClipTrav, sint x, sint y);
	void				newCaseModels(CQuadGridClipCluster	*&clusterCase, const NLMISC::CAABBox &pivotBbox);

};


} // NL3D


#endif // NL_QUAD_GRID_CLIP_MANAGER_H

/* End of quad_grid_clip_manager.h */
