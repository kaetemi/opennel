/** \file mesh_base_instance.h
 * <File description>
 *
 * $Id: mesh_base_instance.h,v 1.20 2002/11/08 18:41:58 berenguier Exp $
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

#ifndef NL_MESH_BASE_INSTANCE_H
#define NL_MESH_BASE_INSTANCE_H

#include "nel/misc/types_nl.h"
#include "3d/transform_shape.h"
#include "3d/material.h"
#include "3d/animated_material.h"
#include "3d/animated_lightmap.h"
#include "3d/animated_morph.h"
#include "3d/async_texture_block.h"


namespace NL3D
{


class CMeshBase;
class CMesh;
class CMeshMRM;
class CMeshBaseInstanceAnimDetailObs;
class CAnimatedLightmap;
class CAsyncTextureManager;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		MeshBaseInstanceId=NLMISC::CClassId(0xef44331, 0x739f6bcf);


// ***************************************************************************
/**
 * An base class for instance of CMesh and CMeshMRM  (which derive from CMeshBase).
 * NB: this class is a model but is not designed to be instanciated in CMOT.
 * NB: no observers are needed, but AnimDetailObs, since same functionality as CTransformShape.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CMeshBaseInstance : public CTransformShape
{
public:
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();

public:


	/** The list of materials, copied from the mesh.
	 * Each CMeshBaseInstance has those materials, so they can be animated or modified for each instance.
	 * By default, they are copied from the Mesh.
	 */
	std::vector<CMaterial>			Materials;

	/** For Aynsc Texture Loading. This has the same size as Materials.
	 *	User can fill here the name of the texture he want to async load.
	 *	WARNING: once AsyncTextureMode is set, Material's Texture fields should not be modified, else
	 *	undefined results
	 */
	std::vector<CAsyncTextureBlock>	AsyncTextures;


	/// \name IAnimatable Interface (registering only IAnimatable sons).
	// @{
	enum	TAnimValues
	{
		OwnerBit= CTransformShape::AnimValueLast, 

		AnimValueLast,
	};


	virtual	void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix);

	// @}

	/// \name Derived from CTransformShape.
	// @{
	virtual uint		getNumMaterial () const;
	virtual const CMaterial	*getMaterial (uint materialId) const;
	virtual CMaterial	*getMaterial (uint materialId);
	// @}

	/// \name Derived from ITransformable.
	// @{
	/// Default Track Values.
	virtual ITrack* getDefaultTrack (uint valueId);
	// @}

	/// \name LightMap properties
	// @{
	uint32 getNbLightMap();
	void getLightMapName( uint32 nLightMapNb, std::string &LightMapName );
	void setLightMapFactor( const std::string &LightMapName, CRGBA nFactor );
	// @}

	// To build lightmap
	void setAnimatedLightmap (CAnimatedLightmap *alm);

	/// \name BlendShape properties
	// @{
	// Interface
	uint32 getNbBlendShape();
	void getBlendShapeName (uint32 nBlendShapeNb, std::string &BlendShapeName );
	void setBlendShapeFactor (const std::string &BlendShapeName, float rFactor);

	// Internal
	std::vector<CAnimatedMorph>* getBlendShapeFactors()
	{
		return &_AnimatedMorphFactor;
	}
	// @}


	/// Get the scene which owns this instance.
	CScene				*getScene() const {return _OwnerScene;}


	/** Change MRM Distance setup. Only for mesh which support MRM. NB MeshMultiLod apply it only on Lod0.
	 *	NB: This apply to the shape direclty!! ie All instances using same shape will be affected
	 *	NB: no-op if distanceFinest<0, distanceMiddle<=distanceFinest or if distanceCoarsest<=distanceMiddle.
	 *	\param distanceFinest The MRM has its max faces when dist<=distanceFinest.
	 *	\param distanceMiddle The MRM has 50% of its faces at dist==distanceMiddle.
	 *	\param distanceCoarsest The MRM has faces/Divisor (ie near 0) when dist>=distanceCoarsest.
	 */
	virtual void		changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest) {}

	/** If there are selectable texture in this mesh shape, this replace the matching material instances with the right texture	
	 *	If getAsyncTextureMode()==true, then this replace the AsyncTexture fileNames, instead of the Materials file Names.
	 */
	void selectTextureSet(uint id);


	/// \name Async Texture Loading
	// @{
	/** if true, the instance is said in "AsyncTextureMode". Ie user must fill AsyncTextures field with name of the
	 *	textures to load. At each startAsyncTextureLoading(), the system start to load async them.
	 *	Then, isAsyncTextureReady() should be test each frame, to know if loading has completed.
	 *	By default, AsyncTextureMode=false. 
	 *	When it swap from false to true, each texture file in Materials are replaced with 
	 *	"blank.tga", and true fileNames are copied into AsyncTextures.
	 *	When it swap from true to false, the inverse is applied.
	 *	NB: calling enableAsyncTextureMode(true) calls setAsyncTextureDirty(true)
	 */
	void			enableAsyncTextureMode(bool enable);
	bool			getAsyncTextureMode() const {return _AsyncTextureMode;}
	/** Start to load all textures in AsyncTextures array (if needed)
	 *	NB: old setup is kept in Material => instance is still rendered with "coherent" textures, until new textures
	 *	are ready
	 *	no op if not in async texture mode.
	 */
	void			startAsyncTextureLoading();
	/**	return true if all the async textures of the instances are uploaded.
	 *	if was not ready before, this swap the 
	 *	return always true if not in async texture mode, or if startAsyncTextureLoading() has not been called
	 *	since last enableAsyncTextureMode(true)
	 */
	bool			isAsyncTextureReady();

	/** For Lod of texture, and load balancing, set the approximate distance of the instance to the camera.
	 */
	void			setAsyncTextureDistance(float dist) {_AsyncTextureDistance= dist;}
	float			getAsyncTextureDistance() const {return _AsyncTextureDistance;}

	/** User is free to flag this state, to know if startAsyncTextureLoading() should be called. 
	 *	Internal system don't use this flag. 
	 *	Default is false
	 */
	void			setAsyncTextureDirty(bool flag) {_AsyncTextureDirty= flag;}
	/// see dirtAsyncTextureState()
	bool			isAsyncTextureDirty() const {return _AsyncTextureDirty;}

	/** Get an AynscTextureId. ret -1 if not found, or not a textureFile.
	 *	NB: the id returned is the one in _CurrentAsyncTexture it the valid ones (thoses loaded or being loaded)
	 *	Can be used for some (non deleting) request to the AsyncTextureManager
	 */
	sint			getAsyncTextureId(uint matId, uint stage) const;

	// @}


protected:
	/// Constructor
	CMeshBaseInstance();
	/// Destructor
	virtual ~CMeshBaseInstance();


private:
	static IModel	*creator() {return new CMeshBaseInstance;}
	friend	class CMeshBase;
	friend	class CMeshBaseInstanceAnimDetailObs;


	/** The list of animated materials, instanciated from the mesh.
	 */
	std::vector<CAnimatedMaterial>	_AnimatedMaterials;

	std::vector<CAnimatedLightmap*> _AnimatedLightmap;

	std::vector<CAnimatedMorph> _AnimatedMorphFactor; 

	/// The Scene where the instance is created.
	CScene		*_OwnerScene;

	/// \name Async Texture Loading
	// @{
	/// 0 if all the texture are async loaded. Setup by the CAsyncTextureManager
	friend	class	CAsyncTextureManager;
	sint							_AsyncTextureToLoadRefCount;
	bool							_AsyncTextureDirty;
	bool							_AsyncTextureMode;
	bool							_AsyncTextureReady;
	// A copy of AsyncTextures done at each startAsyncTextureLoading().
	std::vector<CAsyncTextureBlock>	_CurrentAsyncTextures;
	// distance for texture load balancing
	float							_AsyncTextureDistance;

	void			releaseCurrentAsyncTextures();

	// @}

/// public only for IMeshVertexProgram classes.
public:

	/// CMeshVPWindTree instance specific part.
	float		_VPWindTreePhase;		// Phase time of the wind animation. 0-1

};


// ***************************************************************************
/**
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - extend the traverse method.
 *
 * \sa CAnimDetailTrav IBaseAnimDetailObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CMeshBaseInstanceAnimDetailObs : public CTransformAnimDetailObs
{
public:

	/** this do :
	 *  - call CTransformAnimDetailObs::traverse()
	 *  - update animated materials.
	 */
	virtual	void	traverse(IObs *caller);


public:
	static IObs	*creator() {return new CMeshBaseInstanceAnimDetailObs;}
};



} // NL3D


#endif // NL_MESH_BASE_INSTANCE_H

/* End of mesh_base_instance.h */
