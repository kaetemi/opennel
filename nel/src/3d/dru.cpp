/** \file dru.cpp
 * Driver Utilities.
 *
 * $Id: dru.cpp,v 1.2 2000/11/07 17:08:07 berenguier Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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


#include <windows.h>
#include "nel/3d/dru.h"
#include "nel/3d/driver.h"


namespace NL3D 
{


typedef IDriver* (*IDRV_CREATE_PROC)(void); 


IDriver		*CDRU::createGlDriver()
{
	// WINDOWS code.
	HINSTANCE			hInst;
	IDRV_CREATE_PROC	createDriver;

	hInst=LoadLibrary("r:\\code\\nel\\driver_opengl.dll");
	if (!hInst)
		return	NULL;

	createDriver=(IDRV_CREATE_PROC)GetProcAddress(hInst,"NL3D_createIDriverInstance");
	if (!createDriver)
		return	NULL;

	return createDriver();
}


} // RK3D
