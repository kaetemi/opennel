/** \file vegetable_light_ex.cpp
 * <File description>
 *
 * $Id: vegetable_light_ex.cpp,v 1.3 2003/08/19 14:11:34 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#include "3d/vegetable_light_ex.h"
#include "3d/point_light_named.h"


namespace NL3D 
{



void			CVegetableLightEx::computeCurrentColors()
{
	for(uint i=0;i<NumLights;i++)
	{
		// get the light.
		CPointLightNamed	*pl= PointLight[i];
		// get the attenuation
		uint	att= PointLightFactor[i];
		// modulate the color with it. Use the Unanimated one!!
		Color[i].modulateFromui(pl->getUnAnimatedDiffuse(), att);
	}
}


} // NL3D
