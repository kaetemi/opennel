/** \file mesh_base.cpp
 * <File description>
 *
 * $Id: mesh_base.cpp,v 1.12 2001/09/10 13:21:47 berenguier Exp $
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

#include "3d/mesh_base.h"
#include "3d/mesh_base_instance.h"


namespace NL3D 
{



// ***************************************************************************
CMeshBase::CMeshBase()
{
	// To have same functionnality than previous version, init to identity.
	_DefaultPos.setValue(CVector(0,0,0));
	_DefaultPivot.setValue(CVector(0,0,0));
	_DefaultRotEuler.setValue(CVector(0,0,0));
	_DefaultRotQuat.setValue(CQuat::Identity);
	_DefaultScale.setValue(CVector(1,1,1));
	_DefaultLMFactor.setValue(CRGBA(255,255,255,255));
}


// ***************************************************************************
// ***************************************************************************
// Animated material.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CMeshBase::setAnimatedMaterial(uint id, const std::string &matName)
{
	if(id<_Materials.size())
	{
		// add / replace animated material.
		_AnimatedMaterials[id].Name= matName;
		// copy Material default.
		_AnimatedMaterials[id].copyFromMaterial(&_Materials[id]);
	}
}

// ***************************************************************************
CMaterialBase	*CMeshBase::getAnimatedMaterial(uint id)
{
	TAnimatedMaterialMap::iterator	it;
	it= _AnimatedMaterials.find(id);
	if(it!=_AnimatedMaterials.end())
		return &it->second;
	else
		return NULL;
}


// ***************************************************************************
// ***************************************************************************
// Serial - buildBase.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CMeshBase::CMeshBaseBuild::CMeshBaseBuild()
{
	DefaultPos.set(0,0,0);
	DefaultPivot.set(0,0,0);
	DefaultRotEuler.set(0,0,0);
	DefaultScale.set(1,1,1);

	bCastShadows= false;
	bRcvShadows= false;
}

// ***************************************************************************
#if 0
void	CMeshBase::CMeshBaseBuild::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/*
	Version 1:
		- Cut in version because of badly coded ITexture* serialisation. throw an exception if 
			find a version < 1.
	Version 0:
		- 1st version.
	*/
	sint	ver= f.serialVersion(1);

	if(ver<1)
		throw NLMISC::EStream(f, "MeshBuild in Stream is too old (MeshBaseBuild version < 1)");

	f.serial( DefaultPos );
	f.serial( DefaultPivot );
	f.serial( DefaultRotEuler );
	f.serial( DefaultRotQuat );
	f.serial( DefaultScale );

	f.serialCont( Materials );
}
#endif


// ***************************************************************************
void	CMeshBase::serialMeshBase(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/*
	Version 1:
		- Cut in version because of badly coded ITexture* serialisation. throw an exception if 
			find a version < 1.
	Version 0:
		- 1st version.
	*/
	sint	ver= f.serialVersion(1);

	if(ver<1)
		throw NLMISC::EStream(f, "Mesh in Stream is too old (MeshBase version < 1)");

	f.serial (_DefaultPos);
	f.serial (_DefaultPivot);
	f.serial (_DefaultRotEuler);
	f.serial (_DefaultRotQuat);
	f.serial (_DefaultScale);

	f.serialCont(_Materials);
	f.serialCont(_AnimatedMaterials);
	f.serialCont(_LightInfos);
}


// ***************************************************************************
void	CMeshBase::buildMeshBase(CMeshBaseBuild &m)
{
	// Copy light information
	_LightInfos = m.LightInfoMap;

	// copy the materials.
	_Materials= m.Materials;

	// clear the animated materials.
	_AnimatedMaterials.clear();

	/// Copy default position values
	_DefaultPos.setValue (m.DefaultPos);
	_DefaultPivot.setValue (m.DefaultPivot);
	_DefaultRotEuler.setValue (m.DefaultRotEuler);
	_DefaultRotQuat.setValue (m.DefaultRotQuat);
	_DefaultScale.setValue (m.DefaultScale);

}



// ***************************************************************************
void	CMeshBase::instanciateMeshBase(CMeshBaseInstance *mi)
{
	// setup materials.
	//=================
	mi->Materials= _Materials;

	// setup animated materials.
	//==========================
	TAnimatedMaterialMap::iterator	it;
	mi->_AnimatedMaterials.reserve(_AnimatedMaterials.size());
	for(it= _AnimatedMaterials.begin(); it!= _AnimatedMaterials.end(); it++)
	{
		CAnimatedMaterial	aniMat(&it->second);

		// set the target instance material.
		nlassert(it->first < mi->Materials.size());
		aniMat.setMaterial(&mi->Materials[it->first]);

		// Must set the Animatable father of the animated material (the mesh_base_instance!).
		aniMat.setFather(mi, CMeshBaseInstance::OwnerBit);

		// Append this animated material.
		mi->_AnimatedMaterials.push_back(aniMat);
	}
	
	// Setup position with the default value
	mi->ITransformable::setPos( ((CAnimatedValueVector&)_DefaultPos.getValue()).Value  );
	mi->ITransformable::setRotQuat( ((CAnimatedValueQuat&)_DefaultRotQuat.getValue()).Value  );
	mi->ITransformable::setScale( ((CAnimatedValueVector&)_DefaultScale.getValue()).Value  );
	mi->ITransformable::setPivot( ((CAnimatedValueVector&)_DefaultPivot.getValue()).Value  );

	// Check materials for transparency
	uint32 i;

	mi->setTransparency( false );
	mi->setOpacity( false );
	for( i = 0; i < mi->Materials.size(); ++i )
	if( mi->Materials[i].getBlend() )
	{
		mi->setTransparency( true );
	}
	else
	{
		mi->setOpacity( true );
	}
}

// ***************************************************************************
void	CMeshBase::flushTextures(IDriver &driver)
{
	// Mat count
	uint matCount=_Materials.size();

	// Flush each material textures
	for (uint mat=0; mat<matCount; mat++)
	{
		/// Flush material textures
		_Materials[mat].flushTextures (driver);
	}
}


} // NL3D
