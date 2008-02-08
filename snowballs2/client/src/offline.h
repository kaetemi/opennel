/**
 * \file offline.h
 * \brief COffline
 * \date 2008-02-06 14:43GMT
 * \author Jan Boon (Kaetemi)
 * COffline
 * 
 * $Id$
 */

/* 
 * Copyright (C) 2008  Jan Boon (Kaetemi)
 * Based on NEVRAX SNOWBALLS, Copyright (C) 2001  Nevrax Ltd.
 * 
 * This file is part of OpenNeL Snowballs.
 * OpenNeL Snowballs is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 * 
 * OpenNeL Snowballs is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with OpenNeL Snowballs; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef SBCLIENT_OFFLINE_H
#define SBCLIENT_OFFLINE_H
#include <nel/misc/types_nl.h>

// Project includes
#include "config_proxy.h"
#include "login.h"

// NeL includes

// STL includes

namespace SBCLIENT {
	class CLoading;
	class CLoadingScreen;
	class CLandscape;
	class CEntitiesOld;
	class CTime;

/**
 * \brief COffline
 * \date 2008-02-06 14:43GMT
 * \author Jan Boon (Kaetemi)
 * COffline
 */
class COffline
{
protected:
	// pointers
	CLoadingScreen *_LoadingScreen; // p
	CLogin::CLoginData *_LoginData; // p
	CLoading *_Loading; // p
	CLandscape *_Landscape; // p
	CEntitiesOld *_Entities; // p
	CTime *_Time; // p
	
	// instances
	CConfigProxy _Config;
public:
	COffline(CLoadingScreen *loadingScreen, const std::string &id, CLogin::CLoginData *loginData, CLoading *loading, CLandscape *landscape, CEntitiesOld *entities, CTime *time);
	virtual ~COffline();
}; /* class COffline */

} /* namespace SBCLIENT */

#endif /* #ifndef SBCLIENT_OFFLINE_H */

/* end of file */