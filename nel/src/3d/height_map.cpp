/** \file height_map.cpp
 * <File description>
 *
 * $Id: height_map.cpp,v 1.5 2002/10/28 17:32:13 corvazier Exp $
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

#include "nel/3d/height_map.h"


using namespace NLMISC;


namespace NL3D
{


// ***************************************************************************
void			CHeightMap::resize(uint w, uint h)
{
	NL3D_MEM_HMAP
	_Width= w;
	_Height= h;
	Heights.resize(w*h);
}


// ***************************************************************************
void			CHeightMap::buildFromBitmap(const NLMISC::CBitmap &bitmap0)
{
	NL3D_MEM_HMAP
	// copy bitmap.
	CBitmap		bitmap= bitmap0;
	// convert to luminance.
	bitmap.convertToType(CBitmap::Luminance);

	// resize array.
	uint	w, h;
	w= bitmap.getWidth();
	h= bitmap.getHeight();
	resize(w,h);

	// get luminance image.
	std::vector<uint8>	&array= bitmap.getPixels();
	// invert the image in Y.
	for(uint y=0;y<h;y++)
	{
		for(uint x=0;x<w;x++)
		{
			uint8	v= array[(h-1-y)*w+x];
			Heights[y*w+x]= v;
		}
	}
}


// ***************************************************************************
float			CHeightMap::getZ(uint x, uint y) const
{
	NL3D_MEM_HMAP
	nlassert(x<_Width && y<_Height);
	return Heights[y*_Width+x]*MaxZ/255;
}



} // NL3D
