/** \file u_animation.h
 * User interface to access animations fonctionnalities
 *
 * $Id: u_animation.h,v 1.1 2001/07/03 09:46:22 corvazier Exp $
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

#ifndef NL_U_ANIMATION_H
#define NL_U_ANIMATION_H

#include "nel/misc/types_nl.h"


namespace NL3D 
{

class UTrack;

/**
 * Give manual access to the animation functions.
 *
 * By this interface, you can load animations, get tracks from this animation 
 * with there names, and interpolate manually value from those tracks.
 *
 * This interface is not made to work with UAnimationSet or UPlayList.
 * UAnimation has its own methods to load / release animations.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class UAnimation
{
public:

	/// Constructor
	UAnimation();

	/// \name Create / delete animations
	// @{
	
	/**
	  * Load an animation. This method will use CPath to find the good
	  * animation file. (*.anim). You should call releaseAnimation
	  * to delete the animation.
	  *
	  * \param sPath is the animation file path.
	  * \return NULL if the file is not found. Else return the pointer on the 
	  * animation interface.
	  */
	static UAnimation*	createAnimation (const char* sPath);

	/**
	  * Release an animation. This animation must have been created will createAnimation().
	  *
	  * \param animation is the animation to release.
	  */
	static void			releaseAnimation (UAnimation* animation);

	// @}

	/**
	  * Get an animation track with its name. The track interface should be released
	  * with releaseTrack.
	  *
	  * \param name is the name of the track to get.
	  * \return NULL if the track is not found, else a pointer on the track interface.
	  */
	virtual UTrack*		getTrackByName (const char* name) =0;

	/**
	  * Relase a track interface.
	  *
	  * \param track is a pointer on the track interface to release.
	  */
	virtual void		releaseTrack (UTrack* track) =0;
};


} // NL3D


#endif // NL_U_ANIMATION_H

/* End of u_animation.h */
