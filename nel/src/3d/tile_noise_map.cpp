/** \file tile_noise_map.cpp
 * A Noise Map for landscape.
 *
 * $Id: tile_noise_map.cpp,v 1.3 2002/08/21 09:39:54 lecroart Exp $
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

#include "3d/tile_noise_map.h"


namespace NL3D 
{


// ***************************************************************************
void		CTileNoiseMap::serial(NLMISC::IStream &f)
{
	(void)f.serialVersion(0);

	f.serialBuffer((uint8*)Pixels, NL3D_TILE_NOISE_MAP_SIZE * NL3D_TILE_NOISE_MAP_SIZE);
}


} // NL3D
