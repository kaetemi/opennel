/** \file light_user.h
 * TODO: File description
 *
 * $Id: light_user.h,v 1.3 2004/11/15 10:24:40 lecroart Exp $
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

#ifndef NL_LIGHT_USER_H
#define NL_LIGHT_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_light.h"
#include "3d/light.h"

#define NL3D_MEM_LIGHT						NL_ALLOC_CONTEXT( 3dLight )

namespace NL3D 
{

class	CDriverUser;


// ***************************************************************************
/**
 * ULight implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CLightUser : public ULight
{
protected:
	CLight			_Light;
	friend class	CDriverUser;

public:

	/// \name Quick setup.
	//@{

	/// Quick setup a directional light
	void setupDirectional (const NLMISC::CRGBA& ambiant, const NLMISC::CRGBA& diffuse, const NLMISC::CRGBA& specular, const NLMISC::CVector& direction,
							float constant=1, float linear=0, float quadratic=0)
	{
		NL3D_MEM_LIGHT
		_Light.setupDirectional (ambiant, diffuse, specular, direction, constant, linear, quadratic);
	}
	
	/// Quick setup a point light
	void setupPointLight (const NLMISC::CRGBA& ambiant, const NLMISC::CRGBA& diffuse, const NLMISC::CRGBA& specular, const NLMISC::CVector& position, 
						const NLMISC::CVector& direction, float constant=1, float linear=0, float quadratic=0)
	{
		NL3D_MEM_LIGHT
		_Light.setupPointLight (ambiant, diffuse, specular, position, direction, constant, linear, quadratic);
	}

	/// Quick setup a spotlight
	void setupSpotLight (const NLMISC::CRGBA& ambiant, const NLMISC::CRGBA& diffuse, const NLMISC::CRGBA& specular, const NLMISC::CVector& position, 
						const NLMISC::CVector& direction, float exponent, float cutoff, float constant=1, float linear=0, float quadratic=0)
	{
		NL3D_MEM_LIGHT
		_Light.setupSpotLight (ambiant, diffuse, specular, position, direction, exponent, cutoff, constant, linear, quadratic);
	}

	/** 
	  * Setup attenuation with begin and end attenuation distance.
	  *
	  * \param farAttenuationBegin is the distance of the begin of the attenuation (attenuation == 0.9f)
	  * \param farAttenuationEnd is the distance of the end of the attenuation (attenuation == 0.1f)
	  */
	void setupAttenuation (float farAttenuationBegin, float farAttenuationEnd)
	{
		NL3D_MEM_LIGHT
		_Light.setupAttenuation (farAttenuationBegin, farAttenuationEnd);
	}

	/** 
	  * Set no attenuation.
	  *
	  * The light will not use attenuation.
	  *
	  */
	void setNoAttenuation ()
	{
		NL3D_MEM_LIGHT
		_Light.setNoAttenuation ();
	}

	/** 
	  * Setup spot exponent with angle of the hotspot.
	  *
	  * \param hotSpotAngle is the angle in radian between the axis of the spot and the vector from light 
	  * where attenuation is == 0.9.
	  */
	void setupSpotExponent (float hotSpotAngle)
	{
		NL3D_MEM_LIGHT
		_Light.setupSpotExponent (hotSpotAngle);
	}

	//@}


	/// \name Set methods.
	//@{
	
	/**
	  * Set the light mode.
	  */
	void setMode (ULight::TLightMode mode)
	{
		NL3D_MEM_LIGHT
		_Light.setMode ((CLight::TLightMode)(uint32)mode);
	}
	
	/**
	  * Set the ambiant color of the light.
	  */
	void setAmbiant (const NLMISC::CRGBA& ambiant)
	{
		NL3D_MEM_LIGHT
		_Light.setAmbiant (ambiant);
	}

	/**
	  * Set the diffuse color of the light.
	  */
	void setDiffuse (const NLMISC::CRGBA& diffuse)
	{
		NL3D_MEM_LIGHT
		_Light.setDiffuse (diffuse);
	}

	/**
	  * Set the specular color of the light.
	  */
	void setSpecular (const NLMISC::CRGBA& specular)
	{
		NL3D_MEM_LIGHT
		_Light.setSpecular (specular);
	}

	/**
	  * Set the position of the light. Used only for SpotLight and PointLight.
	  */
	void setPosition (const NLMISC::CVector& position)
	{
		NL3D_MEM_LIGHT
		_Light.setPosition (position);
	}

	/**
	  * Set the direction of the light. Used only for DirectionalLight and SpotLight.
	  */
	void setDirection (const NLMISC::CVector& direction)
	{
		NL3D_MEM_LIGHT
		_Light.setDirection (direction);
	}

	/**
	  * Set the Intensity distribution of the light. Should be between [0, 1]. Used only for SpotLight.
	  */
	void setExponent (float exponent)
	{
		NL3D_MEM_LIGHT
		_Light.setExponent (exponent);
	}

	/**
	  * Set the cutoff of the light in radian. Should be between [0, Pi/2]. Used only for SpotLight.
	  */
	void setCutoff (float cutoff)
	{
		NL3D_MEM_LIGHT
		_Light.setCutoff (cutoff);
	}

	/**
	  * Set constant attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION + 
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	void setConstantAttenuation (float constant)
	{
		NL3D_MEM_LIGHT
		_Light.setConstantAttenuation (constant);
	}

	/**
	  * Set linear attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION + 
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	void setLinearAttenuation (float linear)
	{
		NL3D_MEM_LIGHT
		_Light.setLinearAttenuation (linear);
	}

	/**
	  * Set quadratic attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION + 
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	void setQuadraticAttenuation (float quadratic)
	{
		NL3D_MEM_LIGHT
		_Light.setQuadraticAttenuation (quadratic);
	}

	//@}

	/// \name Get methods.
	//@{
		
	/**
	  * Get the light mode.
	  */
	ULight::TLightMode getMode () const
	{
		NL3D_MEM_LIGHT
		return (ULight::TLightMode)(uint32)_Light.getMode ();
	}

	/**
	  * Get the ambiant color of the light.
	  */
	NLMISC::CRGBA getAmbiant () const
	{
		NL3D_MEM_LIGHT
		return _Light.getAmbiant ();
	}

	/**
	  * Get the diffuse color of the light.
	  */
	NLMISC::CRGBA getDiffuse () const
	{
		NL3D_MEM_LIGHT
		return _Light.getDiffuse ();
	}

	/**
	  * Get the specular color of the light.
	  */
	NLMISC::CRGBA getSpecular () const
	{
		NL3D_MEM_LIGHT
		return _Light.getSpecular ();
	}

	/**
	  * Get the position of the light. Used only for SpotLight and PointLight.
	  */
	NLMISC::CVector getPosition () const
	{
		NL3D_MEM_LIGHT
		return _Light.getPosition ();
	}

	/**
	  * Get the direction of the light. Used only for DirectionalLight and SpotLight.
	  */
	NLMISC::CVector getDirection () const
	{
		NL3D_MEM_LIGHT
		return _Light.getDirection ();
	}

	/**
	  * Get the exponent of the light. Used only for SpotLight.
	  */
	float getExponent () const
	{
		NL3D_MEM_LIGHT
		return _Light.getExponent ();
	}

	/**
	  * Get the cutoff of the light in radian. Should be between [0, Pi/2]. Used only for SpotLight.
	  */
	float getCutoff () const
	{
		NL3D_MEM_LIGHT
		return _Light.getCutoff ();
	}

	/**
	  * Get constant attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION + 
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	float getConstantAttenuation () const
	{
		NL3D_MEM_LIGHT
		return _Light.getConstantAttenuation ();
	}

	/**
	  * Get linear attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION + 
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	float getLinearAttenuation () const
	{
		NL3D_MEM_LIGHT
		return _Light.getLinearAttenuation ();
	}

	/**
	  * Get quadratic attenuation.
	  *
	  * The intensity of the light is attenuated this way:
	  * light_intensity = light_intensity / ( CONSTANT_ATTENUATION + vertex_light_distance * LINEAR_ATTENUATION + 
	  *	vertex_light_distance * vertex_light_distance * QUADRATIC_ATTENUATION );
	  */
	float getQuadraticAttenuation () const
	{
		NL3D_MEM_LIGHT
		return _Light.getQuadraticAttenuation ();
	}

	//@}
};


} // NL3D


#endif // NL_LIGHT_USER_H

/* End of light_user.h */
