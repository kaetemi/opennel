/** \file point_light_model.h
 * <File description>
 *
 * $Id: point_light_model.h,v 1.1 2002/02/06 16:54:56 berenguier Exp $
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

#ifndef NL_POINT_LIGHT_MODEL_H
#define NL_POINT_LIGHT_MODEL_H

#include "nel/misc/types_nl.h"
#include "3d/transform.h"


namespace NL3D {


class	CPointLightModelLightObs;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		PointLightModelId=NLMISC::CClassId(0x7e842eba, 0x140b6c69);


// ***************************************************************************
/**
 * This model is a dynamic light. It handles a PointLight, where Pos is the worldPos updated by CScene
 *	at each render(). CPointLightModel are linked to the LightModelRoot in the LightTrav.
 *
 *	Hrc: Lights herit CTransform so they can be put in hierarchy, even sticked to a skeleton. They can be hide,
 *	moved etc... (default CTransform).
 *	Clip: Lights are always in frustum, not renderable (default CTransform).
 *	Light: lightModels are not lightables (ie they can't be lighted). (default CTransform).
 *		the observer is specialised.
 * 
 *	PERFORMANCE WARNING: big lights (disabled attenuation and big attenuationEnd) slow down
 *	performances. (by experience, with a factor of 2).
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CPointLightModel : public CTransform
{
public:
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();


public:

	/** The pointLight setup (color/attenuation). Do not use PointLight.Pos to setup the position.
	 *	Use the CTransform interface to set the position.
	 */
	CPointLight		PointLight;


	/**	setup the deltaPosToSkeletonWhenOutOfFrustum
	 *	When a light is sticked to a skeleton, and if this skeleton is clipped, then the position of the light 
	 *	can't be computed correctly without animating the skeleton. To allow good position of the light,
	 *	and to avoid recomputing the skeleton even if it is clipped, the light position is set to 
	 *	skeletonMatrix * this "deltaPosToSkeletonWhenOutOfFrustum".
	 *
	 *	Default is (0, 0, 1.5).
	 *	You may change this according to the approximate size of the skeleton (dwarf or giant), and you must
	 *	take into account any mount (horse etc...). eg for a man on a elephant, a good value would be (0,0,5) :)
	 */
	void			setDeltaPosToSkeletonWhenOutOfFrustum(const CVector &deltaPos);
	/// see setDeltaPosToSkeletonWhenOutOfFrustum()
	const CVector	&getDeltaPosToSkeletonWhenOutOfFrustum() const;


protected:
	/// Constructor
	CPointLightModel();
	/// Destructor
	virtual ~CPointLightModel();

	/// Implement the initModel method: link to the LightModelRoot.
	virtual void	initModel();


// *********************
private:
	friend class	CPointLightModelLightObs;

	static IModel	*creator() {return new CPointLightModel;}


	/** tells if the pointLightModel is not hidden by user
	 *	actually, it is the result of hrc Visibility.
	 */
	bool	isHrcVisible() const
	{
		return _HrcObs->WorldVis;
	}

	/** tells if the pointLightModel has been clipped in the clip traversal. 
	 *	actually, it is the result of hrc Visibility * Skeleton cliping.
	 */
	bool	isClipVisible() const
	{
		return _ClipObs->Visible;
	}

	/** get the Hrc obs.
	 */
	CTransformHrcObs	*getHrcObs() const
	{
		return _HrcObs;
	}

	/// see setDeltaPosToSkeletonWhenOutOfFrustum()
	CVector			_DeltaPosToSkeletonWhenOutOfFrustum;
};


// ***************************************************************************
/**
 * \sa CTransformLightObs
 *	This observer only implements traverse() method and is different from normal CTransform (ie lightedModels)
 *
 *	The traverse() method is called to update the worldPosition of the light, resetLightedModels(), and 
 *	re-insert the light in the lightingManager.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CPointLightModelLightObs : public CTransformLightObs
{
public:

	/** 
	 * The base light method. This do all the good thing and should not be derived.
	 * traverse() is called only on visible objects with no _AncestorSkeletonModel, 
	 * It test if transform->_NeedUpdateLighting==true.
	 *
	 * The observers should not traverseSons(), for speed improvement.
	 */
	virtual	void	traverse(IObs *caller);


	static IObs	*creator() {return new CPointLightModelLightObs;}

};


} // NL3D


#endif // NL_POINT_LIGHT_MODEL_H

/* End of point_light_model.h */
