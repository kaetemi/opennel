/** \file driver_user2.cpp
 * <File description>
 *
 * $Id: driver_user2.cpp,v 1.26 2004/07/08 16:08:44 berenguier Exp $
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

// ***************************************************************************
// THIS FILE IS DIVIDED IN TWO PARTS BECAUSE IT MAKES VISUAL CRASH.
// fatal error C1076: compiler limit : internal heap limit reached; use /Zm to specify a higher limit
// ***************************************************************************

#include "3d/driver_user.h"
#include "3d/scene_user.h"
#include "3d/text_context_user.h"
#include "nel/3d/u_driver.h"
#include "3d/dru.h"
#include "3d/scene.h"
#include "3d/texture_user.h"
#include "nel/3d/u_camera.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/hierarchical_timer.h"


using namespace std;
using namespace NLMISC;


namespace NL3D 
{


// ***************************************************************************
// ***************************************************************************
// Component Management.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
UScene			*CDriverUser::createScene(bool bSmallScene) 
{
	NL3D_MEM_DRIVER

	CSceneUser *pSU = new CSceneUser(this, bSmallScene);

	// set the shape bank
	pSU->getScene().setShapeBank( &_ShapeBank._ShapeBank );
	// set the MeshSkin Vertex Streams
	pSU->getScene().getRenderTrav().setMeshSkinManager(&_MeshSkinManager);
	pSU->getScene().getRenderTrav().setShadowMeshSkinManager(&_ShadowMeshSkinManager);
	// set the AsyncTextureManager
	pSU->getScene().setAsyncTextureManager(&_AsyncTextureManager);
	// set the lodManager
	pSU->getScene().setLodCharacterManager(&_LodCharacterManager);
	return _Scenes.insert(pSU);
}
// ***************************************************************************
void			CDriverUser::deleteScene(UScene	*scene) 
{
	NL3D_MEM_DRIVER

	_Scenes.erase((CSceneUser*)scene, "deleteScene(): Bad scene ptr");
}

// ***************************************************************************
UTextContext	*CDriverUser::createTextContext(const std::string fontFileName, const std::string fontExFileName) 
{
	NL3D_MEM_DRIVER

	return _TextContexts.insert(new CTextContextUser(fontFileName, fontExFileName, this, &_FontManager));
}
// ***************************************************************************
void			CDriverUser::deleteTextContext(UTextContext	*textContext) 
{
	NL3D_MEM_DRIVER

	_TextContexts.erase((CTextContextUser*)textContext, "deleteTextContext: Bad TextContext");
}
// ***************************************************************************
void			CDriverUser::setFontManagerMaxMemory(uint maxMem) 
{
	NL3D_MEM_DRIVER

	_FontManager.setMaxMemory(maxMem);
}
// ***************************************************************************
std::string		CDriverUser::getFontManagerCacheInformation() const
{
	NL3D_MEM_DRIVER

	return _FontManager.getCacheInformation();
}

// ***************************************************************************
UTextureFile	*CDriverUser::createTextureFile(const std::string &file) 
{
	NL3D_MEM_DRIVER

	CTextureFileUser	*text= new CTextureFileUser(file);
	_Textures.insert(text);
	return text;
}
// ***************************************************************************
void			CDriverUser::deleteTextureFile(UTextureFile *textfile) 
{
	NL3D_MEM_DRIVER

	_Textures.erase(dynamic_cast<CTextureFileUser*>(textfile), "deleteTextureFile: Bad textfile");
}
// ***************************************************************************
UTextureMem		*CDriverUser::createTextureMem(uint width, uint height, CBitmap::TType texType)
{
	NL3D_MEM_DRIVER

	CTextureMemUser	*pTx= new CTextureMemUser(width, height, texType);
	_Textures.insert(pTx);
	return pTx;
}
// ***************************************************************************
void			CDriverUser::deleteTextureMem(UTextureMem *pTx) 
{
	NL3D_MEM_DRIVER

	_Textures.erase(dynamic_cast<CTextureMemUser*>(pTx), "deleteTextureMem: Bad pTx");
}
// ***************************************************************************
UMaterial		CDriverUser::createMaterial() 
{
	NL3D_MEM_DRIVER

	return UMaterial(new CMaterial);
}
// ***************************************************************************
void			CDriverUser::deleteMaterial(UMaterial &umat) 
{
	NL3D_MEM_DRIVER

	delete umat.getObjectPtr();
	umat.detach();
}

// ***************************************************************************
UAnimationSet			*CDriverUser::createAnimationSet() 
{
	NL_ALLOC_CONTEXT( 3dAnmSt )
	
	return _AnimationSets.insert(new CAnimationSetUser(this));
}
// ***************************************************************************
UAnimationSet			*CDriverUser::createAnimationSet(const std::string &animationSetFile) 
{
	NL_ALLOC_CONTEXT( 3dAnmSt )
	H_AUTO( NL3D_Load_AnimationSet )
	
	NLMISC::CIFile	f;
	// throw exception if not found.
	std::string	path= CPath::lookup(animationSetFile);
	f.open(path);
	return _AnimationSets.insert(new CAnimationSetUser(this, f));
}
// ***************************************************************************
void			CDriverUser::deleteAnimationSet(UAnimationSet *animationSet) 
{
	NL_ALLOC_CONTEXT( 3dAnmSt )
	
	_AnimationSets.erase((CAnimationSetUser*)animationSet, "deleteAnimationSet(): Bad AnimationSet ptr");
}



// ***************************************************************************
// ***************************************************************************
// Profile.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CDriverUser::profileRenderedPrimitives(CPrimitiveProfile &pIn, CPrimitiveProfile &pOut)
{
	NL3D_MEM_DRIVER

	_Driver->profileRenderedPrimitives(pIn, pOut);
}


// ***************************************************************************
uint32			CDriverUser::profileAllocatedTextureMemory()
{
	NL3D_MEM_DRIVER

	return _Driver->profileAllocatedTextureMemory();
}


// ***************************************************************************
uint32			CDriverUser::profileSetupedMaterials() const
{
	NL3D_MEM_DRIVER

	return _Driver->profileSetupedMaterials();
}


// ***************************************************************************
uint32			CDriverUser::profileSetupedModelMatrix() const
{
	NL3D_MEM_DRIVER
	return _Driver->profileSetupedModelMatrix();
}


// ***************************************************************************
void			CDriverUser::enableUsedTextureMemorySum (bool enable)
{
	NL3D_MEM_DRIVER
	_Driver->enableUsedTextureMemorySum (enable);
}


// ***************************************************************************
uint32			CDriverUser::getUsedTextureMemory () const
{
	NL3D_MEM_DRIVER
	return _Driver->getUsedTextureMemory ();
}


// ***************************************************************************
void			CDriverUser::startProfileVBHardLock()
{
	_Driver->startProfileVBHardLock();
}

// ***************************************************************************
void			CDriverUser::endProfileVBHardLock(std::vector<std::string> &result)
{
	_Driver->endProfileVBHardLock(result);
}

// ***************************************************************************
void			CDriverUser::profileVBHardAllocation(std::vector<std::string> &result)
{
	_Driver->profileVBHardAllocation(result);
}

// ***************************************************************************
void			CDriverUser::profileTextureUsage(std::vector<std::string> &result)
{
	_Driver->profileTextureUsage(result);
}



} // NL3D
