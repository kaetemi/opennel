/** \file camera.h
 * Animation interface between the game and NeL
 *
 * $Id$
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef CAMERA_H
#define CAMERA_H

//
// Includes
//

#include <nel/3d/u_camera.h>

//
// External definitions
//

namespace NL3D
{
	class UVisualCollisionEntity;
};


//
// External variables
//

extern NL3D::UCamera				Camera;
extern NL3D::UVisualCollisionEntity	*CamCollisionEntity;

//
// External functions
//

void	initCamera();
void	updateCamera();
void	releaseCamera();


void	initSky ();
void releaseSky();

// Update the sky for this frame, and render it.
// Must be called before ANY rendering
void	updateSky ();

void	animateSky (NLMISC::TTime dt);


#endif // CAMERA_H

/* End of camera.h */

/* Merge OpenNeL SVN
 */
