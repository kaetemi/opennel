/** \file u_play_list.h
 * <File description>
 *
 * $Id: u_play_list.h,v 1.1 2001/06/15 16:24:42 corvazier Exp $
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

#ifndef NL_U_PLAY_LIST_H
#define NL_U_PLAY_LIST_H

#include "nel/misc/types_nl.h"
#include "nel/3d/animation_time.h"


namespace NL3D
{


class	UTransform;


// ***************************************************************************
/**
 * A playlist/channelmixer, from which you setup your mix of animations (blend, skeleton template weight...).
 * Once this UPlayList is created from the UPlayListManager (with an UAnimationSet), you just need to registerTransform()
 * your object(s)  (eg: an UInstance). Then you can use all the Animation setup function to blend, mix, setup time etc...
 *
 * NB: a playlist is actually played through UPlayListManager::animate().
 * NB: all AnimationTime are in second.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UPlayList
{
protected:

	/// Constructor
	UPlayList() {}
	/// Constructor
	virtual	~UPlayList() {}

public:
	enum 
	{ 
		// 
		empty=0xffffffff
	};

	/// Wrap mode for the play list
	enum TWrapMode
	{ 
		/// Clamp the animation time. (default)
		Clamp=0,

		/// Repeat the animation.
		Repeat,
		
		/// Disable the animation.
		Disable,

		WrapModeCount
	};


public:


	/// \name Animatable Link.
	// @{

	/** register an Animatable object (UTransform, UCamera, UInstance...) to the playlist.
	 * Before deleting this Animatable object, you SHOULD UPlayList::resetAllChannels() or delete 
	 * the playlist from the playlistmanager.
	 *
	 * \param object is the object which will be affected by this animation playlist.
	 */
	virtual	void	registerTransform(UTransform *object) =0;

	/** Reset all channels so that no more Animatable object (Transforms etc...) are linked to this PLayList.
	 * Hence, the playlist has no effect on anything.
	 */
	virtual	void	resetAllChannels() =0;

	// @}



	/// \name Animation Setup.
	// @{
	/**
	  * Empty the playlist. Each slot is set to its default value.
	  */
	virtual	void emptyPlayList ()=0;

	/**
	  * Set the animation of a slot. Default value is empty.
	  *
	  * \param slot is the id of the slot to set.
	  * \param animation is the animation number to use in this slot. To empty the slot, use UPlayList::empty.
	  */
	virtual	void setAnimation (uint8 slot, uint animation)=0;

	/**
	  * Get the animation of a slot. Default value is empty.
	  *
	  * \param slot is the id of the slot to set.
	  * \return the animation number in use in this slot. Return UPlayList::empty if the slot is empty.
	  */
	virtual	uint getAnimation (uint8 slot) const=0;
	// @}


	/// \name Animation Time Setup.
	// @{
	/**
	  * Set animation time origin, ie, the time in the playlist for which slot time is the startTime of the slot animation.
	  * Default value is 0.f.
	  *
	  * \param slot is the id of the slot to set.
	  * \param timeOrigin time origin to use in the slot.
	  */
	virtual	void setTimeOrigin (uint8 slot, CAnimationTime timeOrigin)=0;

	/**
	  * Get animation time origin, ie, the time in the playlist for which slot time is the startTime of the slot animation.
	  * Default value is 0.f.
	  *
	  * \param slot is the id to get the slot.
	  * \return time origin used in the slot.
	  */
	virtual	CAnimationTime getTimeOrigin (uint8 slot) const=0;

	/**
	  * Set animation speed factor.
	  * Default value is 1.f.
	  *
	  * \param slot is the id of the slot to set.
	  * \param speedFactor is the factor to use in this slot. The animation will go speedFactor* faster.
	  */
	virtual	void setSpeedFactor (uint8 slot, float speedFactor)=0;

	/**
	  * Get animation speed factor.
	  * Default value is 1.f.
	  *
	  * \param slot is the id to get the slot.
	  * \return the speed factor used in the slot.
	  */
	virtual	float getSpeedFactor (uint8 slot) const=0;

	/**
	  * Set the wrap mode to use in the play list.
	  *
	  * Wrap mode tells the play list how to use an animation when current time is not in the animation.
	  * \param wrapMode is the mode to use.
	  */
	virtual	void setWrapMode (uint8 slot, TWrapMode wrapMode)=0;

	/**
	  * Get the wrap mode in use in the play list.
	  *
	  * Wrap mode tells the play list how to use an animation when current time is not in the animation.
	  * \return the wrap mode in use.
	  */
	virtual	TWrapMode getWrapMode (uint8 slot) const=0;

	// @}


	/// \name Animation Weight Setup.
	// @{
	/**
	  * Set animation start weight. This is the weight for this animation use at the beginning of the animation slot.
	  * Default value is 1.f.
	  *
	  * \param slot is the id of the slot to set.
	  * \param startWeight is the factor to use in this slot.
	  * \param time is the playlist time for which the start weight is set. Default is 0.f.
	  */
	virtual	void setStartWeight (uint8 slot, float startWeight, CAnimationTime time)=0;

	/**
	  * Get animation start weight.
	  * Default value is 1.f.
	  *
	  * \param slot is the id to get the slot.
	  * \param time will get the playlist time for which the start weight is set. Default is 0.f.
	  * \return the start weight used in the slot.
	  */
	virtual	float getStartWeight (uint8 slot, CAnimationTime& time) const=0;

	/**
	  * Set animation end weight. This is the weight for this animation use at the end of the animation slot.
	  * Default value is 1.f.
	  *
	  * \param slot is the id of the slot to set.
	  * \param time is the playlist time for which the start weight is set. Default is 1.f.
	  * \param endWeight is the factor to use in this slot.
	  */
	virtual	void setEndWeight (uint8 slot, float endWeight, CAnimationTime time)=0;

	/**
	  * Get animation end weight.
	  * Default value is 1.f.
	  *
	  * \param slot is the id to get the slot.
	  * \param time will get the playlist time for which the start weight is set. Default is 0.f.
	  * \return the end weight used in the slot.
	  */
	virtual	float getEndWeight (uint8 slot, CAnimationTime& time) const=0;

	/**
	  * Set weight smoothness. 
	  * This is the smoothness of the weight interpolation. 
	  * Must be between 0.f and 1.f. 0.f is a sharp interpolation (linear), 1.f is a smooth interpolation (quadratic).
	  * Default value is 0.f.
	  *
	  * \param slot is the id of the slot to set.
	  * \param smoothness is the smoothness value.
	  */
	virtual	void setWeightSmoothness (uint8 slot, float smoothness)=0;

	/**
	  * Get weight smoothness. 
	  * This is the smoothness of the weight interpolation. 
	  * Must be between 0.f and 1.f. 0.f is a sharp interpolation (linear), 1.f is a smooth interpolation (quadratic).
	  * Default value is 0.f.
	  *
	  * \param slot is the id of the slot to set.
	  * \return the smoothness value.
	  */
	virtual	float getWeightSmoothness (uint8 slot) const=0;

	/**
	  * Setup a constant animation weight. 
	  * NB: this is equivalent as following code:
	  *		- setStartWeight(slot, weight, 0);
	  *		- setEndWeight(slot, weight, 0);
	  *
	  * \param slot is the id of the slot to set.
	  * \param weight is the factor to use in this slot.
	  */
	virtual	void setWeight (uint8 slot, float weight)=0;

	// @}



	/// \name Skeleton Weight Setup.
	// @{
	/**
	  * Set the skeleton weight animation of a slot. Default value is empty.
	  *
	  * \param slot is the id of the slot to set.
	  * \param skeletonId is the skeleton weight number to use in this slot. To empty the slot, use UPlayList::empty.
	  * \param inverted is false if the weights must be used as they are, true if the weights to used are 1.f - weight.
	  */
	virtual	void setSkeletonWeight (uint8 slot, uint skeletonId, bool inverted=false)=0;

	/**
	  * Get the skeleton weight of a slot. Default value is empty.
	  *
	  * \param slot is the id of the slot to set.
	  * \param inverted will receive the invert flag.
	  * \return the skeleton weight number in use in this slot. Return UPlayList::empty if the slot is empty.
	  */
	virtual	uint getSkeletonWeight (uint8 slot, bool &inverted) const=0;
	// @}


};


} // NL3D


#endif // NL_U_PLAY_LIST_H

/* End of u_play_list.h */
