/** \file point_light_user.h
 * <File description>
 *
 * $Id: point_light_user.h,v 1.2 2002/02/18 13:21:55 berenguier Exp $
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

#ifndef NL_POINT_LIGHT_USER_H
#define NL_POINT_LIGHT_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_point_light.h"
#include "3d/transform_user.h"


namespace NL3D 
{


class	CPointLightModel;


// ***************************************************************************
/**
 * UPointLight implementation
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CPointLightUser : virtual public UPointLight, public CTransformUser
{
public:
	/// This is the SAME pointer than _Transform, but correctly casted.
	CPointLightModel			*_PointLightModel;

public:

	/// \name Object
	// @{
	/// This model should have been created with Scene::createInstance().
	CPointLightUser(CScene *scene, IModel *trans);
	virtual	~CPointLightUser()
	{
		// deleted in CTransformUser.
		_PointLightModel= NULL;
	}
	// @}


	virtual void			setAmbient (NLMISC::CRGBA ambient);
	virtual void			setDiffuse (NLMISC::CRGBA diffuse);
	virtual void			setSpecular (NLMISC::CRGBA specular);
	virtual void			setColor (NLMISC::CRGBA color);

	virtual NLMISC::CRGBA	getAmbient () const ;
	virtual NLMISC::CRGBA	getDiffuse () const ;
	virtual NLMISC::CRGBA	getSpecular () const ;

	virtual void			setupAttenuation(float attenuationBegin, float attenuationEnd);
	virtual float			getAttenuationBegin() const;
	virtual float			getAttenuationEnd() const;

	virtual void			enableSpotlight(bool enable) ;
	virtual bool			isSpotlight() const ;
	virtual void			setupSpotAngle(float spotAngleBegin, float spotAngleEnd) ;
	virtual float			getSpotAngleBegin() const ;
	virtual float			getSpotAngleEnd() const ;


	virtual void			setDeltaPosToSkeletonWhenOutOfFrustum(const CVector &deltaPos) ;
	virtual const CVector	&getDeltaPosToSkeletonWhenOutOfFrustum() const ;


};


} // NL3D


#endif // NL_POINT_LIGHT_USER_H

/* End of point_light_user.h */
