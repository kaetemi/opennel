/** \file scene_user.h
 * <File description>
 *
 * $Id: scene_user.h,v 1.13 2001/08/30 09:19:46 besson Exp $
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

#ifndef NL_SCENE_USER_H
#define NL_SCENE_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_scene.h"
#include "3d/driver_user.h"
#include "3d/camera_user.h"
#include "3d/instance_user.h"
#include "3d/particle_system_instance_user.h"
#include "3d/landscape_user.h"
#include "3d/instance_group_user.h"
#include "3d/skeleton_user.h"
#include "3d/visual_collision_manager_user.h"
#include "3d/animation_set_user.h"
#include "3d/play_list_manager_user.h"


namespace NL3D {


/**
 * <Class description>
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CSceneUser : public UScene
{
protected:
	/// The driver which owns this scene.
	CDriverUser			*_DriverUser;
	/// The Scene.
	CScene				_Scene;
	/// The Current camera.
	CCameraUser			*_CurrentCamera;


	// Components List.
	typedef	CPtrSet<CTransformUser>		TTransformSet;
	typedef	CPtrSet<CLandscapeUser>		TLandscapeSet;
	typedef	CPtrSet<CInstanceGroupUser>	TInstanceGroupSet;
	typedef	CPtrSet<CVisualCollisionManagerUser>	TVisualCollisionManagerSet;
	typedef	CPtrSet<CAnimationSetUser>		TAnimationSetSet;
	typedef	CPtrSet<CPlayListManagerUser>	TPlayListManagerSet;
	TTransformSet				_Transforms;
	TLandscapeSet				_Landscapes;
	TInstanceGroupSet			_InstanceGroups;
	TVisualCollisionManagerSet	_VisualCollisionManagers;
	TAnimationSetSet			_AnimationSets;
	TPlayListManagerSet			_PlayListManagers;

	std::map<UInstance**,CTransformShape*> _WaitingInstances;

public:

	/// \name Object
	// @{
	CSceneUser(CDriverUser *drv)
	{
		nlassert(drv);
		_DriverUser= drv;
		_CurrentCamera = NULL;
		// Init Scene.
		_Scene.initDefaultTravs();
		// Don't add any user trav.
		// init default Roots.
		_Scene.initDefaultRoots();
		// Set driver.
		_Scene.setDriver(_DriverUser->getDriver());
		// Set viewport
		_Scene.setViewport (CViewport());
		// Init the world instance group
		_Scene.initGlobalnstanceGroup();
		// Init coarse mesh manager
		_Scene.initCoarseMeshManager ();

		// Create default camera, and active!!
		setCam(createCamera());
	}
	virtual	~CSceneUser()
	{
		_VisualCollisionManagers.clear();
		_Transforms.clear();
		_Landscapes.clear();
		_Scene.setDriver(NULL);
		_Scene.setCam(NULL);
		_Scene.release();
		_CurrentCamera= NULL;
		_DriverUser= NULL;
	}
	// @}

public:

	/// \name Render
	//@{
	virtual	void			render()
	{
		if(_CurrentCamera==NULL)
			nlerror("render(): try to render with no camera linked (may have been deleted)");
		_Scene.render();

		// Update waiting instances
		// Done after the _Scene.render because in this method the instance are checked for creation
		std::map<UInstance**,CTransformShape*>::iterator it = _WaitingInstances.begin();
		while( it != _WaitingInstances.end() )
		{
			if( it->second != NULL )
			{
				*(it->first) = dynamic_cast<UInstance*>( _Transforms.insert(new CInstanceUser(&_Scene, it->second)) );
				std::map<UInstance**,CTransformShape*>::iterator delIt = it;
				++it;
				_WaitingInstances.erase(delIt);
			}
			else
			{
				++it;
			}
		}

		// Must restore the matrix context, so 2D/3D interface not disturbed.
		_DriverUser->restoreMatrixContext();
	}

	virtual	void			animate(CAnimationTime time)
	{
		_Scene.animate(time);
	}

	//@}


	/// \name Camera/Viewport.
	//@{
	virtual	void			setCam(UCamera *cam)
	{
		if(!cam)
			nlerror("setCam(): cannot set a NULL camera");
		CCameraUser		*newCam= dynamic_cast<CCameraUser*>(cam);
		if( newCam->getScene() != &_Scene)
			nlerror("setCam(): try to set a current camera not created from this scene");

		_CurrentCamera= newCam;
		_Scene.setCam(newCam->getCamera());
	}
	virtual	UCamera			*getCam()
	{
		return dynamic_cast<UCamera*>(_CurrentCamera);
	}
	virtual	void			setViewport(const class CViewport& viewport)
	{
		_Scene.setViewport(viewport);
	}
	virtual	CViewport		getViewport()
	{
		return _Scene.getViewport();
	}
	//@}


	/// \name Component Mgt.
	//@{

	virtual	UCamera			*createCamera()
	{
		// The component is auto added/deleted to _Scene in ctor/dtor.
		return dynamic_cast<UCamera*>( _Transforms.insert(new CCameraUser(&_Scene)) );
	}
	virtual	void			deleteCamera(UCamera *cam)
	{
		CCameraUser		*oldCam= dynamic_cast<CCameraUser*>(cam);
		// Is this the current camera??
		if(oldCam==_CurrentCamera)
			_CurrentCamera=NULL;

		// The component is auto added/deleted to _Scene in ctor/dtor.
		_Transforms.erase(oldCam);
	}

	virtual	UInstance		*createInstance(const std::string &shapeName)
	{
		IModel	*model= _Scene.createInstance(shapeName);
		// If not found, return NULL.
		if(model==NULL)
			return NULL;

		// The component is auto added/deleted to _Scene in ctor/dtor.
		if (dynamic_cast<CParticleSystemModel *>(model))
		{
			/// particle system
			return dynamic_cast<UInstance*>( _Transforms.insert(new CParticleSystemInstanceUser(&_Scene, model)) );
		}
		else
		{
			/// mesh
			return dynamic_cast<UInstance*>( _Transforms.insert(new CInstanceUser(&_Scene, model)) );
		}
	}

	virtual	void createInstanceAsync(const std::string &shapeName, UInstance**ppInstance)
	{
		_WaitingInstances[ppInstance] = NULL;
		_Scene.createInstanceAsync(shapeName,&_WaitingInstances[ppInstance]);
//		IModel	*model= _Scene.createInstance(shapeName);
		// If not found, return NULL.
//		if(model==NULL)
//			return NULL;

//		if( dynamic_cast<CMeshInstance*>(model)==NULL )
//			nlerror("UScene::createInstance(): shape is not a mesh");

		// The component is auto added/deleted to _Scene in ctor/dtor.
//		return dynamic_cast<UInstance*>( _Transforms.insert(new CInstanceUser(&_Scene, model)) );
	}

	virtual UTransform *createTransform()
	{
		IModel	*model= _Scene.createModel(TransformId);
		// If not found, return NULL.
		if(model==NULL)
			return NULL;

		// The component is auto added/deleted to _Scene in ctor/dtor.
		return dynamic_cast<UTransform*>( _Transforms.insert(new CTransformUser(&_Scene, model)) );
	}

	virtual	void			deleteInstance(UInstance *inst)
	{
	//	CTransformShape *pTSInstance = dynamic_cast<CTransformShape*>(inst->);
	//	_Scene.deleteInstance(pTSInstance);
		// The component is auto added/deleted to _Scene in ctor/dtor.
		_Transforms.erase(dynamic_cast<CTransformUser*>(inst));
	}


	virtual	USkeleton		*createSkeleton(const std::string &shapeName)
	{
		IModel	*model= _Scene.createInstance(shapeName);
		// If not found, return NULL.
		if(model==NULL)
			return NULL;

		if( dynamic_cast<CSkeletonModel*>(model)==NULL )
			nlerror("UScene::createSkeleton(): shape is not a skeletonShape");

		// The component is auto added/deleted to _Scene in ctor/dtor.
		return dynamic_cast<USkeleton*>( _Transforms.insert(new CSkeletonUser(&_Scene, model)) );
	}
	virtual	void			deleteSkeleton(USkeleton *skel)
	{
		// The component is auto added/deleted to _Scene in ctor/dtor.
		_Transforms.erase(dynamic_cast<CTransformUser*>(skel));
	}


	virtual	ULandscape		*createLandscape()
	{
		// The component is auto added/deleted to _Scene in ctor/dtor.
		return _Landscapes.insert(new CLandscapeUser(&_Scene));
	}
	virtual	void			deleteLandscape(ULandscape *land)
	{
		// The component is auto added/deleted to _Scene in ctor/dtor.
		_Landscapes.erase((CLandscapeUser*) land);
	}
/*

	virtual	UInstanceGroup	*createInstanceGroup (const std::string &instanceGroup)
	{
		// Create the instance group
		CInstanceGroupUser *user=new CInstanceGroupUser;

		// Init the class
		if (!user->load (instanceGroup))
		{
			// Prb, erase it
			delete user;

			// Return error code
			return NULL;
		}

		// Insert the pointer in the pointer list
		_InstanceGroups.insert (user);

		// return the good value
		return user;
	}

	virtual	void			deleteInstanceGroup (UInstanceGroup	*group)
	{
		// The component is auto added/deleted to _Scene in ctor/dtor.
		_InstanceGroups.erase (dynamic_cast<CInstanceGroupUser*>(group));
	}
*/

	virtual void setToGlobalInstanceGroup(UInstanceGroup *pIG)
	{
		CInstanceGroupUser *pIGU = (CInstanceGroupUser*)pIG;
		pIGU->_InstanceGroup.setClusterSystem (_Scene.getGlobalInstanceGroup());
	}

	//@}

	/// \name Animation.
	//@{
	//@}
	/// \name Animation gestion.
	// @{
	/// Create an empty AnimationSet.
	virtual	UAnimationSet	*createAnimationSet();
	/// Create a new AnimationSet, load it from a file. Use CPath to search the animation set. exception EPathNotFound if not found.
	virtual	UAnimationSet	*createAnimationSet(const std::string &animationSetFile) ;
	/// Delete a AnimationSet.
	virtual	void			deleteAnimationSet(UAnimationSet *animationSet) ;
	/// Create a new PlayListManager.
	virtual	UPlayListManager	*createPlayListManager() ;
	/// Delete a PlayListManager.
	virtual	void				deletePlayListManager(UPlayListManager *playListManager) ;
	// @}

	/// \name Visual Collision manager.
	//@{
	virtual	UVisualCollisionManager		*createVisualCollisionManager()
	{
		return _VisualCollisionManagers.insert(new CVisualCollisionManagerUser);
	}
	virtual	void						deleteVisualCollisionManager(UVisualCollisionManager *mgr)
	{
		_VisualCollisionManagers.erase(dynamic_cast<CVisualCollisionManagerUser*>(mgr));
	}
	//@}


	/// \name LoadBalancing mgt.
	//@{
	virtual	void				setPolygonBalancingMode(TPolygonBalancingMode polBalMode);
	virtual	TPolygonBalancingMode	getPolygonBalancingMode() const;

	virtual	void				setLoadMaxPolygon(uint nFaces);
	virtual	uint				getLoadMaxPolygon() const;
	//@}

	/// \name Coarse meshes mgt.
	//@{
	virtual void setStaticCoarseMeshManagerTexture (const char *sPath);
	virtual void setDynamicCoarseMeshManagerTexture (const char *sPath);
	virtual void setStaticCoarseMeshManagerColor (const CRGBA& color);
	virtual void setDynamicCoarseMeshManagerColor (const CRGBA& color);
	//@}

public:
	/// \name Accessor for CSeneUser.
	// @{
	CScene		&getScene()
	{
		return _Scene;
	}
	// @}

};


} // NL3D


#endif // NL_SCENE_USER_H

/* End of scene_user.h */
