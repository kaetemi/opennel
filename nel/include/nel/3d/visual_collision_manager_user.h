/** \file visual_collision_manager_user.h
 * UVisualCollisionManager implementation.
 *
 * $Id: visual_collision_manager_user.h,v 1.1 2001/06/12 11:49:39 berenguier Exp $
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

#ifndef NL_VISUAL_COLLISION_MANAGER_USER_H
#define NL_VISUAL_COLLISION_MANAGER_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/tmp/u_visual_collision_manager.h"
#include "nel/3d/visual_collision_manager.h"
#include "nel/3d/landscape_user.h"
#include "nel/3d/visual_collision_entity_user.h"
#include "nel/3d/ptr_set.h"


namespace NL3D {


/**
 * UVisualCollisionManager implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVisualCollisionManagerUser : public UVisualCollisionManager
{
public:

	/// Constructor
	CVisualCollisionManagerUser() {}


	virtual void					setLandscape(ULandscape *landscape)
	{
		_Manager.setLandscape(&(dynamic_cast<CLandscapeUser*>(landscape)->getLandscape()->Landscape));
	}

	virtual UVisualCollisionEntity	*createEntity()
	{
		return _Entities.insert(new CVisualCollisionEntityUser(&_Manager));
	}

	virtual void					deleteEntity(UVisualCollisionEntity	*entity)
	{
		_Entities.erase(dynamic_cast<CVisualCollisionEntityUser*>(entity));
	}


protected:
	CVisualCollisionManager		_Manager;
	typedef	CPtrSet<CVisualCollisionEntityUser>		TVisualCollisionEntitySet;
	TVisualCollisionEntitySet	_Entities;

};


} // NL3D


#endif // NL_VISUAL_COLLISION_MANAGER_USER_H

/* End of visual_collision_manager_user.h */
