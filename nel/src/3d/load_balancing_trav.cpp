/** \file load_balancing_trav.cpp
 * The LoadBalancing traversal.
 *
 * $Id: load_balancing_trav.cpp,v 1.16 2003/03/27 16:51:45 berenguier Exp $
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

#include "std3d.h"

#include "3d/load_balancing_trav.h"
#include "3d/hrc_trav.h"
#include "3d/clip_trav.h"
#include "nel/misc/common.h"
#include "nel/misc/hierarchical_timer.h"
#include "3d/transform.h"


using namespace std;
using namespace NLMISC;


// ***************************************************************************
#define	NL3D_DEFAULT_LOADBALANCING_VALUE_SMOOTHER	50
#define	NL3D_LOADBALANCING_SMOOTHER_MAX_RATIO		1.1f

namespace NL3D 
{

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CLoadBalancingGroup::CLoadBalancingGroup()
{
	_PrecPolygonBalancingMode= CLoadBalancingGroup::PolygonBalancingOff;
	_NbFaceWanted= 20000;
	_ValueSmoother.init(NL3D_DEFAULT_LOADBALANCING_VALUE_SMOOTHER);
	_DefaultGroup= false;

	_NbFacePass0= 0;
	_FaceRatio= 1;
}


// ***************************************************************************
void			CLoadBalancingGroup::computeRatioAndSmooth(TPolygonBalancingMode polMode)
{
	// If Default group, disable load balancing
	if(_DefaultGroup)
		polMode= PolygonBalancingOff;

	// Compute ratio
	switch(polMode)
	{
	case PolygonBalancingOff:
		_FaceRatio= 1;
		break;
	case PolygonBalancingOn	:
		if(_NbFacePass0!=0)
			_FaceRatio= (float)_NbFaceWanted / _NbFacePass0;
		else
			_FaceRatio= 1;
		break;
	case PolygonBalancingClamp:
		if(_NbFacePass0!=0)
			_FaceRatio= (float)_NbFaceWanted / _NbFacePass0;
		else
			_FaceRatio= 1;
		clamp(_FaceRatio, 0, 1);
		break;
		default: break;
	};

	// smooth the value.
	// if change of PolygonBalancingMode, reset the _ValueSmoother.
	if(polMode!=_PrecPolygonBalancingMode)
	{
		_ValueSmoother.init(NL3D_DEFAULT_LOADBALANCING_VALUE_SMOOTHER);
		_PrecPolygonBalancingMode= polMode;
	}
	// if not PolygonBalancingOff, smooth the ratio.
	if(polMode!=PolygonBalancingOff)
	{
#ifdef NL_OS_WINDOWS
		// FIX: If the _FaceRatio is not a float (NaN or +-oo), don't add it!!
		if(_finite(_FaceRatio))
			_ValueSmoother.addValue(_FaceRatio);
#else
		_ValueSmoother.addValue(_FaceRatio);
#endif
		float	fSmooth= _ValueSmoother.getSmoothValue();

		// If after smoothing, the number of faces is still too big, reduce smooth effect! (frustrum clip effect)
		if(fSmooth*_NbFacePass0 > _NbFaceWanted*NL3D_LOADBALANCING_SMOOTHER_MAX_RATIO)
		{
			// reset the smoother
			_ValueSmoother.reset();
			// reduce smooth effect
			fSmooth= _FaceRatio*NL3D_LOADBALANCING_SMOOTHER_MAX_RATIO;
			_ValueSmoother.addValue(fSmooth);
		}

		// take the smoothed value.
		_FaceRatio= fSmooth;
	}


}



// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CLoadBalancingTrav::CLoadBalancingTrav()
{
	PolygonBalancingMode= CLoadBalancingGroup::PolygonBalancingOff;

	// Add the default group and make it default
	_GroupMap["Default"].Name= "Default";
	_GroupMap["Default"]._DefaultGroup= true;

	// set the DefaultGroup ptr.
	_DefaultGroup= &_GroupMap["Default"];

	// prepare some space
	_VisibleList.reserve(1024);
}


// ***************************************************************************
void				CLoadBalancingTrav::clearVisibleList()
{
	_VisibleList.clear();
}


// ***************************************************************************
void				CLoadBalancingTrav::traverse()
{
	H_AUTO( NL3D_TravLoadBalancing );

	CTravCameraScene::update();

	// Reset each group.
	//================
	ItGroupMap	it= _GroupMap.begin();
	for(;it!=_GroupMap.end();it++)
	{
		// reset _NbFacePass0.
		it->second._NbFacePass0= 0;
	}


	// Traverse the graph 2 times.

	// 1st pass, count NBFaces drawed.
	//================
	_LoadPass= 0;
	// count _NbFacePass0.
	traverseVisibilityList();


	// Reset _SumNbFacePass0
	_SumNbFacePass0= 0;
	// For each group
	it= _GroupMap.begin();
	for(;it!=_GroupMap.end();it++)
	{
		// compute ratio and smooth
		it->second.computeRatioAndSmooth(PolygonBalancingMode);
		// update _SumNbFacePass0
		_SumNbFacePass0+= it->second.getNbFaceAsked();
	}


	// 2nd pass, compute Faces that will be drawed.
	//================
	_LoadPass= 1;
	traverseVisibilityList();

}


// ***************************************************************************
void				CLoadBalancingTrav::traverseVisibilityList()
{
	// Traverse all nodes of the visibility list.
	uint	nModels= _VisibleList.size();
	for(uint i=0; i<nModels; i++)
	{
		CTransform	*model= _VisibleList[i];
		model->traverseLoadBalancing(NULL);
	}
}


// ***************************************************************************
float				CLoadBalancingTrav::getNbFaceAsked () const
{
	return _SumNbFacePass0;
}


// ***************************************************************************
CLoadBalancingGroup	*CLoadBalancingTrav::getOrCreateGroup(const std::string &group)
{
	// find
	ItGroupMap	it;
	it= _GroupMap.find(group);
	// if not exist, create.
	if(it==_GroupMap.end())
	{
		// create and set name.
		it= _GroupMap.insert(make_pair(group, CLoadBalancingGroup())).first;
		it->second.Name= group;
	}

	return &(it->second);
}

// ***************************************************************************
void				CLoadBalancingTrav::setGroupNbFaceWanted(const std::string &group, uint nFaces)
{
	// get/create if needed, and assign.
	getOrCreateGroup(group)->setNbFaceWanted(nFaces);
}

// ***************************************************************************
uint				CLoadBalancingTrav::getGroupNbFaceWanted(const std::string &group)
{
	// get/create if needed, and get.
	return getOrCreateGroup(group)->getNbFaceWanted();
}

// ***************************************************************************
float				CLoadBalancingTrav::getGroupNbFaceAsked (const std::string &group) const
{
	TGroupMap::const_iterator	it;
	it= _GroupMap.find(group);
	if(it==_GroupMap.end())
		return 0;
	else
		return it->second.getNbFaceAsked();
}



} // NL3D
