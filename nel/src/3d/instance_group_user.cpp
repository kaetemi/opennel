/** \file instance_group_user.cpp
 * Implementation of the user interface managing instance groups.
 *
 * $Id: instance_group_user.cpp,v 1.11 2001/08/30 10:07:12 corvazier Exp $
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
#include "3d/instance_group_user.h"
#include "3d/scene_user.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"

using namespace NLMISC;
using namespace std;

namespace NL3D 
{

// ***************************************************************************

UInstanceGroup	*UInstanceGroup::createInstanceGroup (const std::string &instanceGroup)
{
	// Create the instance group
	CInstanceGroupUser *user=new CInstanceGroupUser;

	// Init the class
	if (!user->init (instanceGroup))
	{
		// Prb, erase it
		delete user;

		// Return error code
		return NULL;
	}

	// return the good value
	return user;
}

// ***************************************************************************

/*bool CInstanceGroupUser::init (const std::string &instanceGroup, CScene& scene)
{
	// Create a file
	CIFile file;
	if (file.open (instanceGroup))
	{
		// Serialize this class
		try
		{
			// Read the class
			_InstanceGroup.serial (file);

			// Driver pointer
			CDriver *driver = ;

			

			// Add to the scene
			addToScene (scene);
		}
		catch (EStream& e)
		{
			// Avoid visual warning
			EStream ee=e;

			// Serial problem
			return false;
		}
	}
	else
	{
		// Failed.
		return false;
	}

	// Ok
	return true;
}*/

// ***************************************************************************
bool CInstanceGroupUser::init (const std::string &instanceGroup)
{
	// Create a file
	CIFile file;
	if (file.open (CPath::lookup (instanceGroup) ))
	{
		// Serialize this class
		try
		{
			// Read the class
			_InstanceGroup.serial (file);
		}
		catch (EStream& e)
		{
			// Avoid visual warning
			EStream ee=e;

			// Serial problem
			return false;
		}
	}
	else
	{
		// Failed.
		return false;
	}

	// Ok
	return true;
}

// ***************************************************************************
void CInstanceGroupUser::addToScene (class UScene& scene, UDriver *driver)
{
	// Get driver pointer
	IDriver *cDriver= driver ? safe_cast<CDriverUser*>(driver)->getDriver() : NULL;

	// Add to the scene
	addToScene (((CSceneUser*)&scene)->getScene(), cDriver);
}

// ***************************************************************************
void CInstanceGroupUser::addToScene (class CScene& scene, IDriver *driver)
{
	_InstanceGroup.addToScene (scene, driver);
	// Fill in the map accelerating search of instance by names
	for( uint32 i = 0; i < _InstanceGroup._Instances.size(); ++i)
	{
		CInstanceUser *pIU = new CInstanceUser (&scene,_InstanceGroup._Instances[i]);
		string stmp = _InstanceGroup.getInstanceName (i);
		_Instances.insert (map<string,CInstanceUser*>::value_type(stmp, pIU));
	}		
}

// ***************************************************************************
void CInstanceGroupUser::removeFromScene (class UScene& scene)
{
	_InstanceGroup.removeFromScene (((CSceneUser*)&scene)->getScene());
	// Remove all instance in the map
	map<string,CInstanceUser*>::iterator it = _Instances.begin();
	while (it != _Instances.end())
	{
		map<string,CInstanceUser*>::iterator itDel = it;
		++it;
		_Instances.erase (itDel);
	}
}

// ***************************************************************************
uint CInstanceGroupUser::getNumInstance () const
{
	return _InstanceGroup.getNumInstance ();
}

// ***************************************************************************

const std::string& CInstanceGroupUser::getShapeName (uint instanceNb) const
{
	// Check args
	if (instanceNb>=_InstanceGroup.getNumInstance ())
		nlerror("getShapeName*(): bad instance Id");
	
	return _InstanceGroup.getShapeName (instanceNb);
}

// ***************************************************************************
const std::string& CInstanceGroupUser::getInstanceName (uint instanceNb) const
{
	// Check args
	if (instanceNb>=_InstanceGroup.getNumInstance ())
		nlerror("getInstanceName*(): bad instance Id");
	
	return _InstanceGroup.getInstanceName (instanceNb);
}

// ***************************************************************************
const NLMISC::CVector& CInstanceGroupUser::getInstancePos (uint instanceNb) const
{
	// Check args
	if (instanceNb>=_InstanceGroup.getNumInstance ())
		nlerror("getInstancePos*(): bad instance Id");
	
	return _InstanceGroup.getInstancePos (instanceNb);
}

// ***************************************************************************
const NLMISC::CQuat& CInstanceGroupUser::getInstanceRot (uint instanceNb) const
{
	// Check args
	if (instanceNb>=_InstanceGroup.getNumInstance ())
		nlerror("getInstanceRot*(): bad instance Id");
	
	return _InstanceGroup.getInstanceRot (instanceNb);
}

// ***************************************************************************
const NLMISC::CVector& CInstanceGroupUser::getInstanceScale (uint instanceNb) const
{
	// Check args
	if (instanceNb>=_InstanceGroup.getNumInstance ())
		nlerror("getInstanceScale*(): bad instance Id");
	
	return _InstanceGroup.getInstanceScale (instanceNb);
}



// ***************************************************************************
UInstance *CInstanceGroupUser::getByName (std::string &name)
{
	map<string,CInstanceUser*>::iterator it = _Instances.find (name);
	if (it != _Instances.end())
		return it->second;
	else
		return NULL;
}

// ***************************************************************************

const UInstance *CInstanceGroupUser::getByName (std::string &name) const
{
	map<string,CInstanceUser*>::const_iterator it = _Instances.find (name);
	if (it != _Instances.end())
		return it->second;
	else
		return NULL;
}

// ***************************************************************************

void CInstanceGroupUser::setLightFactor (const std::string &LightName, CRGBA nFactor)
{
	_InstanceGroup.setLightFactor (LightName, nFactor);
}

// ***************************************************************************

void CInstanceGroupUser::createRoot (UScene &scene)
{
	_InstanceGroup.createRoot (((CSceneUser*)&scene)->getScene());
}

// ***************************************************************************
void CInstanceGroupUser::setClusterSystem (UInstanceGroup *pClusterSystem)
{
	_InstanceGroup.setClusterSystem (&((CInstanceGroupUser*)pClusterSystem)->_InstanceGroup);
}

// ***************************************************************************
void CInstanceGroupUser::getDynamicPortals (std::vector<std::string> &names)
{
	_InstanceGroup.getDynamicPortals (names);
}

// ***************************************************************************
void CInstanceGroupUser::setDynamicPortal (std::string& name, bool opened)
{
	_InstanceGroup.setDynamicPortal (name, opened);
}

// ***************************************************************************
bool CInstanceGroupUser::getDynamicPortal (std::string& name)
{
	return _InstanceGroup.getDynamicPortal (name);
}

// ***************************************************************************
void CInstanceGroupUser::setPos (const NLMISC::CVector &pos)
{
	_InstanceGroup.setPos (pos);
}

// ***************************************************************************
void CInstanceGroupUser::setRotQuat (const NLMISC::CQuat &q)
{
	_InstanceGroup.setRotQuat (q);
}

// ***************************************************************************
CVector CInstanceGroupUser::getPos ()
{
	return _InstanceGroup.getPos ();
}

// ***************************************************************************
CQuat CInstanceGroupUser::getRotQuat ()
{
	return _InstanceGroup.getRotQuat();
}


// ***************************************************************************
void			CInstanceGroupUser::freezeHRC()
{
	_InstanceGroup.freezeHRC();
}

// ***************************************************************************
void			CInstanceGroupUser::unfreezeHRC()
{
	_InstanceGroup.unfreezeHRC();
}


} // NL3D
