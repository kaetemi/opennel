/** \file particle_system_sound_user.cpp
 * <File description>
 *
 * $Id: particle_system_sound_user.cpp,v 1.2 2001/08/30 08:07:30 vizerie Exp $
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



//#include "nel/3d/u_particle_system_sound.h" we don't include this to avoid a link with NLSOUND
#include "3d/particle_system.h"
#include "nel/3d/u_ps_sound_interface.h"

namespace NL3D 
{

void assignSoundServerToPS(UPSSoundServer *soundServer)	
{
	CParticleSystem::registerSoundServer(soundServer);
} // NL3D


}

/* End of particle_system_sound_user.cpp */
