/** \file camera.h
 * 
 *
 * $Id: camera.h,v 1.4 2001/07/17 12:27:42 legros Exp $
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

namespace NL3D
{
	class UCamera;
	class UVisualCollisionEntity;
};

void	initCamera();
void	updateCamera();
void	releaseCamera();

extern NL3D::UCamera				*Camera;
extern NL3D::UVisualCollisionEntity	*CamCollisionEntity;

#endif // CAMERA_H

/* End of camera.h */
