/** \file mesh_base_instance.cpp
 * <File description>
 *
 * $Id: mesh_base_instance.cpp,v 1.3 2001/06/27 15:23:53 corvazier Exp $
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

#include "3d/mesh_base_instance.h"
#include "3d/mesh_base.h"


namespace NL3D 
{


// ***************************************************************************
void		CMeshBaseInstance::registerBasic()
{
	CMOT::registerModel(MeshBaseInstanceId, TransformShapeId, CMeshBaseInstance::creator);
	CMOT::registerObs(AnimDetailTravId, MeshBaseInstanceId, CMeshBaseInstanceAnimDetailObs::creator);
}


// ***************************************************************************
void		CMeshBaseInstance::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix)
{
	CTransformShape::registerToChannelMixer(chanMixer, prefix);

	// Add any materials.
	for(uint i=0;i<_AnimatedMaterials.size();i++)
	{
		// append material  matname.*
		_AnimatedMaterials[i].registerToChannelMixer(chanMixer, prefix + _AnimatedMaterials[i].getMaterialName() + ".");
	}
}


// ***************************************************************************
ITrack*		CMeshBaseInstance::getDefaultTrack (uint valueId)
{
	// Pointer on the CMeshBase
	CMeshBase* pMesh=(CMeshBase*)(IShape*)Shape;

	// Switch the value
	switch (valueId)
	{
	case CTransform::PosValue:			
		return pMesh->getDefaultPos();
	case CTransform::RotEulerValue:		
		return pMesh->getDefaultRotEuler();
	case CTransform::RotQuatValue:		
		return pMesh->getDefaultRotQuat();
	case CTransform::ScaleValue:		
		return pMesh->getDefaultScale();
	case CTransform::PivotValue:		
		return pMesh->getDefaultPivot();
	default:
		// Problem, new values ?
		nlstop;
	};
	return NULL;
}


// ***************************************************************************
uint32 CMeshBaseInstance::getNbLightMap()
{
	CMeshBase* pMesh=(CMeshBase*)(IShape*)Shape;
	return pMesh->_LightInfos.size();
}

// ***************************************************************************
void CMeshBaseInstance::getLightMapName( uint32 nLightMapNb, std::string &LightMapName )
{
	CMeshBase* pMesh=(CMeshBase*)(IShape*)Shape;
	if( nLightMapNb >= pMesh->_LightInfos.size() )
		return;
	CMeshBase::TLightInfoMap::iterator itMap = pMesh->_LightInfos.begin();
	for( uint32 i = 0; i < nLightMapNb; ++i ) ++itMap;
	LightMapName = itMap->first;
}

// ***************************************************************************
void CMeshBaseInstance::setLightMapFactor( const std::string &LightMapName, CRGBA Factor )
{
	CMeshBase* pMesh=(CMeshBase*)(IShape*)Shape;
	CMeshBase::TLightInfoMap::iterator itMap = pMesh->_LightInfos.find( LightMapName );
	if( itMap == pMesh->_LightInfos.end() )
		return;
	CMeshBase::CLightInfoMapList::iterator itList = itMap->second.begin();
	uint32 nNbElt = itMap->second.size();
	for( uint32 i = 0; i < nNbElt; ++i )
	{
		Materials[itList->nMatNb].setLightMapFactor( itList->nStageNb, Factor );
		++itList;
	}
}

// ***************************************************************************
// ***************************************************************************

void CMeshBaseInstanceAnimDetailObs::traverse(IObs *caller)
{
	CTransformAnimDetailObs::traverse(caller);

	// update animated materials.
	CMeshBaseInstance	*mi= (CMeshBaseInstance*)Model;

	// test if animated materials must be updated.
	if(mi->IAnimatable::isTouched(CMeshBaseInstance::OwnerBit))
	{
		// must test / update all AnimatedMaterials.
		for(uint i=0;i<mi->_AnimatedMaterials.size();i++)
		{
			// This test and update the pointed material.
			mi->_AnimatedMaterials[i].update();
		}

		mi->IAnimatable::clearFlag(CMeshBaseInstance::OwnerBit);
	}

	// Lightmap automatic animation
	for( uint i = 0; i < mi->_AnimatedLightmap.size(); ++i )
	{
		const char *LightGroupName = strchr( mi->_AnimatedLightmap[i]->getName().c_str(), '.' )+1;
		mi->setLightMapFactor(	LightGroupName,
								mi->_AnimatedLightmap[i]->getFactor() );
	}
}

// ***************************************************************************

} // NL3D
