/** \file point_light_named.h
 * <File description>
 *
 * $Id: point_light_named.h,v 1.1 2002/02/06 16:54:56 berenguier Exp $
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

#ifndef NL_POINT_LIGHT_NAMED_H
#define NL_POINT_LIGHT_NAMED_H

#include "nel/misc/types_nl.h"
#include "3d/point_light.h"
#include "nel/misc/stream.h"


namespace NL3D {


class	CLightInfluenceInterpolator;


// ***************************************************************************
/**
 * A pointLight with a name, and a default color setup. The current setup is the setup in CPointLight.
 * A CPointLightNamed influence can also be interpolated with a CLightInfluenceInterpolator.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CPointLightNamed : public CPointLight
{
public:
	/// Identifier of this PointLight. used for UInstanceGroup::setLightFactor(), and ULandscape::setLightFactor();
	std::string		LightGroupName;


public:

	/// Constructor
	CPointLightNamed();


	/// Set the Default ambient color of the light. Default to Black.
	void			setDefaultAmbient (NLMISC::CRGBA ambient)	{_DefaultAmbient=ambient;}
	/// Set the Default diffuse color of the light. Default to White
	void			setDefaultDiffuse (NLMISC::CRGBA diffuse)	{_DefaultDiffuse=diffuse;}
	/// Set the Default specular color of the light. Default to White
	void			setDefaultSpecular (NLMISC::CRGBA specular)	{_DefaultSpecular=specular;}

	/// Get the Default ambient color of the light.
	NLMISC::CRGBA	getDefaultAmbient () const	{return _DefaultAmbient;}
	/// Get the Default diffuse color of the light.
	NLMISC::CRGBA	getDefaultDiffuse () const	{return _DefaultDiffuse;}
	/// Get the Default specular color of the light.
	NLMISC::CRGBA	getDefaultSpecular () const	{return _DefaultSpecular;}


	/// modulate the default color setup with nFactor, and set to the current setup.
	void			setLightFactor(NLMISC::CRGBA nFactor);


	void			serial(NLMISC::IStream &f);

	
// ******************
private:

	// For CLightInfluenceInterpolator to work. Only used by CLightInfluenceInterpolator.
	friend class		CLightInfluenceInterpolator;
	sint				_IdInInfluenceList;

	// The Default light color.
	NLMISC::CRGBA		_DefaultAmbient;
	NLMISC::CRGBA		_DefaultDiffuse;
	NLMISC::CRGBA		_DefaultSpecular;


};


} // NL3D


#endif // NL_POINT_LIGHT_NAMED_H

/* End of point_light_named.h */
