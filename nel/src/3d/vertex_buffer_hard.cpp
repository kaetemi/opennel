/** \file vertex_buffer_hard.cpp
 * <File description>
 *
 * $Id: vertex_buffer_hard.cpp,v 1.3 2001/09/06 07:25:37 corvazier Exp $
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

#include "3d/vertex_buffer_hard.h"


namespace NL3D 
{

// ***************************************************************************

void	IVertexBufferHard::initFormat (uint16 vertexFormat, const uint8 *typeArray, uint32 numVertices)
{
	// _NbVerts.
	_NbVerts= numVertices;

	// Compute format: flags / offsets, for each component.
	_VertexSize=0;
	_Flags=0;

	// For each values
	for (uint value=0; value<CVertexBuffer::NumValue; value++)
	{
		// Flag for this value
		uint flag=1<<value;

		// Value used ?
		if (vertexFormat & flag)
		{
			// Use it
			_Flags|=flag;

			// Value offset
			_Offset[value]=_VertexSize;

			// New vertex size
			_VertexSize+=CVertexBuffer::SizeType[typeArray[value]];

			// Setup the type
			_Type[value]=typeArray[value];
		}
	}
}

// ***************************************************************************

uint8		IVertexBufferHard::getNumWeight () const
{
	// Num weight
	switch (_Type[CVertexBuffer::Weight])
	{
	case CVertexBuffer::Float1:
		return 1;
	case CVertexBuffer::Float2:
		return 2;
	case CVertexBuffer::Float3:
		return 3;
	case CVertexBuffer::Float4:
		return 4;
	}

	// No weight
	return 0;
}

// ***************************************************************************

} // NL3D
