/** \file mesh_instance.h
 * <File description>
 *
 * $Id: mesh_instance.h,v 1.4 2001/06/27 15:23:53 corvazier Exp $
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

#ifndef NL_MESH_INSTANCE_H
#define NL_MESH_INSTANCE_H

#include "nel/misc/types_nl.h"
#include "3d/mesh_base_instance.h"
#include "3d/material.h"
#include "3d/animated_material.h"


namespace NL3D
{


class CMesh;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		MeshInstanceId=NLMISC::CClassId(0x6bfe0a34, 0x23b26dc9);


// ***************************************************************************
/**
 * An instance of CMesh.
 * NB: no observers are needed, since same functionnality as CMeshBaseInstance.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CMeshInstance : public CMeshBaseInstance
{
public:
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();

public:

protected:
	/// Constructor
	CMeshInstance()
	{
	}
	/// Destructor
	virtual ~CMeshInstance() {}


private:
	static IModel	*creator() {return new CMeshInstance;}
	friend	class CMesh;

};




} // NL3D


#endif // NL_MESH_INSTANCE_H

/* End of mesh_instance.h */
