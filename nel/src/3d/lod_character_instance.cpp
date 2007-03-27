/** \file lod_character_instance.cpp
 * TODO: File description
 *
 * $Id$
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
#include "lod_character_instance.h"
#include "lod_character_manager.h"


namespace NL3D 
{


// ***************************************************************************
CLodCharacterInstance::~CLodCharacterInstance()
{
	// If the texture was allocated.
	if(_TextureId>=0)
	{
		nlassert(_Owner);
		_Owner->releaseInstance(*this);
	}
}


// ***************************************************************************
const CUV		*CLodCharacterInstance::getUVs() const
{
	if(_UVs.empty())
		return NULL;

	return &_UVs[0];
}



} // NL3D
