/** \file u_water.cpp
 * User interface for water manipulation
 *
 * $Id$
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "nel/3d/u_water.h"
#include "water_pool_manager.h"
#include "water_height_map.h"
#include "water_model.h"
#include "driver_user.h"

#define NL3D_MEM_WATER						NL_ALLOC_CONTEXT( 3dWater )

namespace NL3D 
{

//===========================================================================
UWaterHeightMap &UWaterHeightMapManager::getWaterHeightMapFromID(uint32 ID)
{
	NL3D_MEM_WATER
	nlassert(GetWaterPoolManager().hasPool(ID)); // unknown pool ID!
	return  GetWaterPoolManager().getPoolByID(ID);
}


//===========================================================================
void	UWaterHeightMapManager::setBlendFactor(UDriver *drv, float value)
{
	NL3D_MEM_WATER
	NLMISC::clamp(value, 0.f, 1.f);
	GetWaterPoolManager().setBlendFactor(NLMISC::safe_cast<CDriverUser *>(drv)->getDriver(), value);
}

//===========================================================================
void UWaterHeightMapManager::releaseBlendTextures()
{
	GetWaterPoolManager().releaseBlendTextures();
}

//===========================================================================
uint32	UWaterInstance::getWaterHeightMapID() const
{
	NL3D_MEM_WATER
	CWaterModel	*object = getObjectPtr();
	return object ->getWaterHeightMapID();
}


//===========================================================================
float	UWaterInstance::getHeightFactor() const
{
	NL3D_MEM_WATER
	CWaterModel	*object = getObjectPtr();
	return object->getHeightFactor();
}

//===========================================================================
float   UWaterInstance::getHeight(const NLMISC::CVector2f &pos)
{
	NL3D_MEM_WATER
	CWaterModel	*object = getObjectPtr();
	return object->getHeight(pos);
}

//===========================================================================
float   UWaterInstance::getAttenuatedHeight(const NLMISC::CVector2f &pos, const NLMISC::CVector &viewer)
{
	NL3D_MEM_WATER
	CWaterModel	*object = getObjectPtr();
	return object->getAttenuatedHeight(pos, viewer);
}

} // NL3D
