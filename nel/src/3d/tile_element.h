/** \file tile_element.h
 * <File description>
 *
 * $Id: tile_element.h,v 1.1 2001/06/15 16:24:45 corvazier Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#ifndef NL_TILE_ELEMENT_H
#define NL_TILE_ELEMENT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"


namespace NL3D 
{

#define NL_TILE_ELM_MASK_ROTATE				0x03
#define NL_TILE_ELM_OFFSET_ROTATE			0
#define NL_TILE_ELM_SIZE_ROTATE				2
#define NL_TILE_ELM_MASK_UVINFO				0x07
#define NL_TILE_ELM_OFFSET_UVINFO			6
#define NL_TILE_ELM_SIZE_UVINFO				3

#define NL_TILE_ELM_LAYER_EMPTY				0xffff

// ***************************************************************************
/**
 * An Element for CPatchTexture. Temporary! since CPatchTexture should be compressed...
 * NB: no default ctor => must init all fields.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CTileElement
{
private:
	uint16	Flags;	// Tile Orientation, and Tile 256x256 UV offset.

public:
	/** The three tile ident. NL_TILE_ELM_LAYER_EMPTY means no Tile for this pass. Tile[0] should be !=NL_TILE_ELM_LAYER_EMPTY.
	 * Else cross are drawn...
	 */
	uint16	Tile[3];
	
	/** Set the tile orientation of pass i, to "orient".
	 * orient E [0,3]. The rotation is CCW.
	 */
	void	setTileOrient(sint i, uint8 orient);
	
	/** Get the tile orientation of pass i.
	 * orient E [0,3]. The rotation is CCW.
	 */
	uint8	getTileOrient(sint i) const;


	/** Set the tile 256x256 information of pass 0.
	 *
	 * NB: During UV computing, orient is applied first, then tile256x256 uvOffset (only if the tile is 256x256).
	 * \param is256x256 is this tile a part of a 256x256
	 * \param uvOff the UV offset of tile 256x256. uvOff E [0,3]. Meanings:
	 *	  ---------
	 *	  | 0 | 3 |
	 *	  |___|___|
	 *	  |   |   |
	 *	  | 1 | 2 |
	 *	  ---------
	 *
	 */
	void	setTile256Info(bool is256x256, uint8 uvOff=0);
	
	/** Get the tile 256x256 information.
	 */
	void	getTile256Info(bool &is256x256, uint8 &uvOff) const;


	void	serial(NLMISC::IStream &f);
};



} // NL3D


#endif // NL_TILE_ELEMENT_H

/* End of tile_element.h */
