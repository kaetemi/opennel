/** \file root_model.cpp
 * <File description>
 *
 * $Id: root_model.cpp,v 1.3 2003/03/26 10:20:55 berenguier Exp $
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

#include "3d/root_model.h"
#include "3d/scene.h"


namespace NL3D {


// ***************************************************************************
void	CRootModel::registerBasic()
{
	CScene::registerModel( RootModelId, 0, CRootModel::creator);
}


// ***************************************************************************
void	CRootModel::traverseHrc(CTransform *caller)
{
	// Traverse the Hrc sons.
	uint	num= hrcGetNumChildren();
	for(uint i=0;i<num;i++)
		hrcGetChild(i)->traverseHrc(this);
}

// ***************************************************************************
void	CRootModel::traverseClip(CTransform *caller)
{
	// Traverse the Clip sons.
	uint	num= clipGetNumChildren();
	for(uint i=0;i<num;i++)
		clipGetChild(i)->traverseClip(this);
}

// ***************************************************************************
void	CRootModel::traverseAnimDetail(CTransform *caller)
{	
	// no-op
}

// ***************************************************************************
void	CRootModel::traverseLoadBalancing(CTransform *caller)
{
	// no-op
}

// ***************************************************************************
void	CRootModel::traverseLight(CTransform *caller)
{
	// no-op
}

// ***************************************************************************
void	CRootModel::traverseRender()
{
	// no-op
}


} // NL3D
