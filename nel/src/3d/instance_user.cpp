/** \file instance_user.cpp
 * <File description>
 *
 * $Id: instance_user.cpp,v 1.7 2001/12/27 15:12:25 besson Exp $
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

#include "3d/instance_user.h"
#include "nel/misc/debug.h"


using	namespace NLMISC;

namespace NL3D 
{



// ***************************************************************************
CInstanceUser::CInstanceUser(CScene *scene, IModel *trans) : 
  CTransformUser(scene, trans)
{
	_Instance= safe_cast<CTransformShape*>(_Transform);
	CMeshBaseInstance	*mi= dynamic_cast<CMeshBaseInstance*>(_Instance);

	// If It it is a CMeshBaseInstance (with materials).
	if(mi)
	{
		// create user mats.
		uint	numMat= mi->Materials.size();
		_Materials.reserve(numMat);
		for(uint i=0;i<numMat;i++)
		{
			_Materials.push_back(&mi->Materials[i]);
		}
	}
}


// ***************************************************************************
void				CInstanceUser::getShapeAABBox(NLMISC::CAABBox &bbox) const
{
	_Instance->getAABBox(bbox);
}

// ***************************************************************************
void CInstanceUser::setBlendShapeFactor (const std::string &blendShapeName, float factor, bool dynamic)
{
	CMeshBaseInstance	*mi= dynamic_cast<CMeshBaseInstance*>(_Instance);

	if (mi)
	{
		mi->setBlendShapeFactor (blendShapeName, factor);
		// \todo trap takes care of the dynamic flag
	}
}

} // NL3D
