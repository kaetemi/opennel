/** \file raw_skinned.cpp
 * Packed struct used for faster Software skinning
 *
 * $Id: raw_skinned.cpp,v 1.2 2005/02/22 10:19:11 besson Exp $
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
#include "raw_skinned.h"


using namespace NLMISC;

namespace NL3D 
{


// ***************************************************************************
void		CRawSkinnedNormalCache::clearArrays()
{
	Vertices1.clear();
	Vertices2.clear();
	Vertices3.clear();
	Vertices4.clear();
	contReset(Geomorphs);
	contReset(RdrPass);
}

} // NL3D
