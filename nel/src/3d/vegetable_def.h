/** \file vegetable_def.h
 * Some global definitions for CVegetable*
 *
 * $Id: vegetable_def.h,v 1.2 2001/11/07 13:11:39 berenguier Exp $
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

#ifndef NL_VEGETABLE_DEF_H
#define NL_VEGETABLE_DEF_H

#include "nel/misc/types_nl.h"


namespace NL3D 
{

// ***************************************************************************
// RdrPass for Vegetables
#define	NL3D_VEGETABLE_NRDRPASS				4
#define	NL3D_VEGETABLE_RDRPASS_LIGHTED		0
#define	NL3D_VEGETABLE_RDRPASS_LIGHTED_2SIDED	1
#define	NL3D_VEGETABLE_RDRPASS_UNLIT		2
#define	NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED		3


// 65000 is a maximum because of GeForce limitations. But allow Vegetable only for GeForce3 and better
#define	NL3D_VEGETABLE_VERTEX_MAX_VERTEX_VBHARD		100000


// 64 LUT entries in constant of VertexProgram.
#define	NL3D_VEGETABLE_VP_LUT_SIZE			64


} // NL3D


#endif // NL_VEGETABLE_DEF_H

/* End of vegetable_def.h */
