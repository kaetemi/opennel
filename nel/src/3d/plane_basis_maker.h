/** \file plane_basis_maker.h
 * <File description>
 *
 * $Id: plane_basis_maker.h,v 1.1 2001/06/15 16:24:43 corvazier Exp $
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

#ifndef NL_PLANE_BASIS_MAKER_H
#define NL_PLANE_BASIS_MAKER_H

#include "nel/misc/types_nl.h"
#include "ps_attrib_maker_template.h"
#include "


namespace NL3D {


/**
 * <Class description>
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPlaneBasisMaker : public CPSValueBlender<CPlaneBasis>
{
public:

	/// Constructor
	CPlaneBasisMaker();

};


} // NL3D


#endif // NL_PLANE_BASIS_MAKER_H

/* End of plane_basis_maker.h */
