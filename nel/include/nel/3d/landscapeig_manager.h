/** \file landscapeig_manager.h
 * <File description>
 *
 * $Id: landscapeig_manager.h,v 1.3 2002/01/03 17:22:35 besson Exp $
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

#ifndef NL_LANDSCAPEIG_MANAGER_H
#define NL_LANDSCAPEIG_MANAGER_H

#include "nel/misc/types_nl.h"
#include <vector>
#include <map>


namespace NL3D 
{


class	UInstanceGroup;
class	UScene;


// ***************************************************************************
/**
 * This class is used to load and unload IG of landscape. Used in conjunction with ULandscape.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CLandscapeIGManager
{
public:

	/// Constructor
	CLandscapeIGManager();
	~CLandscapeIGManager();

	/** Initialize and load Instances Group from a text file. no-op if not found.
	 *	the file is just a list of filename eg: "150_EM.ig", separated with return.
	 *	At this time, All UInstanceGroup are loaded, but none are addToScene() ed.
	 */
	void	initIG(UScene *scene, const std::string &igDesc);

	/** load of an instanceGroup of a zone. name is like "150_EM". no-op if "".
	 *	If exist (see initIG), The instanceGroup is added to the scene.
	 *	call after ULandscape::refreshZonesAround()
	 */
	void	loadZoneIG(const std::string &name);

	/** same as prec loadZoneIG, but with an array of name. no-op if "".
	 *	call after ULandscape::loadAllZonesAround().
	 */
	void	loadArrayZoneIG(const std::vector<std::string> &names);

	/** unload of an instanceGroup of a zone. name is like "150_EM". no-op if "".
	 *	If exist (see initIG), The instanceGroup is removed from the scene.
	 *	call after ULandscape::refreshZonesAround()
	 */
	void	unloadZoneIG(const std::string &name);

	/// is the Ig added to scene? name is like "150_EM". false if not found.
	bool	isIGAddedToScene(const std::string &name) const;

	/// get the Ig. name is like "150_EM". NULL if not found.
	UInstanceGroup	*getIG(const std::string &name) const;

	/// unload and delete all the zoneIgs. Call before deletion of the scene (else exception).
	void	reset();


	/// reload all the Igs and re-add to scene.
	void	reloadAllIgs();


private:
	UScene			*_Scene;
	struct			CInstanceGroupElement
	{
		UInstanceGroup	*Ig;
		bool			AddedToScene;
		std::string		FileName;

		CInstanceGroupElement(UInstanceGroup	*ig= NULL, const char *fileName= NULL);
		
		// delete the ig.
		void	release();
	};
	typedef	std::map<std::string, CInstanceGroupElement>	TZoneInstanceGroupMap;
	typedef	TZoneInstanceGroupMap::iterator					ItZoneInstanceGroupMap;
	typedef	TZoneInstanceGroupMap::const_iterator			ConstItZoneInstanceGroupMap;
	TZoneInstanceGroupMap			_ZoneInstanceGroupMap;

	std::string		translateName(const std::string &name) const;

};


} // NL3D


#endif // NL_LANDSCAPEIG_MANAGER_H

/* End of landscapeig_manager.h */
