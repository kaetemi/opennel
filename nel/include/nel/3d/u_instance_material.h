/** \file u_instance_material.h
 * <File description>
 *
 * $Id: u_instance_material.h,v 1.1 2001/06/15 16:24:42 corvazier Exp $
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

#ifndef NL_U_INSTANCE_MATERIAL_H
#define NL_U_INSTANCE_MATERIAL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"


namespace NL3D
{


using NLMISC::CRGBA;


// ***************************************************************************
/**
 * Base interface for manipulating Material retrieved from UInstance.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UInstanceMaterial
{
protected:

	/// \name Object
	// @{
	UInstanceMaterial() {}
	virtual	~UInstanceMaterial() {}
	// @}

public:

	/// \name Modes.
	// @{
	virtual	bool				isLighted() const =0;
	virtual	bool				isUserColor() const =0;
	// @}


	/// \name Lighted material mgt. Has effect only if isLighted().
	// @{

	/// Set the emissive part ot material. Usefull only if setLighting(true, false) has been done.
	virtual	void				setEmissive( CRGBA emissive=CRGBA(0,0,0) )=0;
	/// Set the Ambient part ot material. Usefull only if setLighting(true, false) has been done.
	virtual	void				setAmbient( CRGBA ambient=CRGBA(0,0,0) )=0;
	/// Set the Diffuse part ot material. Usefull only if setLighting(true, false) has been done. NB: opacity is NOT copied from diffuse.A.
	virtual	void				setDiffuse( CRGBA diffuse=CRGBA(0,0,0) )=0;
	/// Set the Opacity part ot material. Usefull only if setLighting(true, false) has been done.
	virtual	void				setOpacity( uint8	opa )=0;
	/// Set the specular part ot material. Usefull only if setLighting(true, false) has been done.
	virtual	void				setSpecular( CRGBA specular=CRGBA(0,0,0) )=0;
	/// Set the shininess part ot material. Usefull only if setLighting(true, false) has been done.
	virtual	void				setShininess( float shininess )=0;

	virtual	CRGBA				getEmissive() const =0;
	virtual	CRGBA				getAmbient() const =0;
	/// return diffuse part. NB: A==opacity.
	virtual	CRGBA				getDiffuse() const =0;
	virtual	uint8				getOpacity() const =0;
	virtual	CRGBA				getSpecular() const =0;
	virtual	float				getShininess() const =0;

	// @}


	/// \name UnLighted material mgt. Has effect only if !isLighted().
	// @{
	virtual	void				setColor(CRGBA rgba) =0;
	virtual	CRGBA				getColor(void) const =0;
	// @}


	/// \name Texture UserColor. No effect if !isUserColor(). (getUserColor() return CRGBA(0,0,0,0))
	// @{
	virtual	void				setUserColor(CRGBA userColor)=0;
	virtual	CRGBA				getUserColor() const =0;
	// @}


};


} // NL3D


#endif // NL_U_INSTANCE_MATERIAL_H

/* End of u_instance_material.h */
