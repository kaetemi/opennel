/** \file channel_mixer.h
 * class CChannelMixer
 *
 * $Id: channel_mixer.h,v 1.6 2002/05/13 15:53:49 berenguier Exp $
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

#ifndef NL_CHANNEL_MIXER_H
#define NL_CHANNEL_MIXER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/smart_ptr.h"
#include "nel/3d/animation_time.h"
#include "3d/animation_set.h"
#include <map>

namespace NL3D 
{

class CAnimation;
class IAnimatable;
class IAnimatedValue;
class ITrack;
class CAnimationSet;

/**
 * A channel mixer. Animated value are registred in it. 
 * Each animated value create a channel in the mixer. Then, mixer animates
 * all those channels with 1 to CChannelMixer::NumAnimationSlot animations.
 *
 * Animation are referenced in an animation slot (CSlot).
 * 
 * Each slot have an IAnimation pointer, a weight for this animation
 * between [0.f ~ 1.f] and a time for this animation.
 *
 * Each CChannel have a weight on each animation slot between [0.f ~ 1.f].
 *
 * Blending is normalized internaly so, weight sum have not to be == 1.f.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CChannelMixer : public NLMISC::CRefCount
{
public:

	/// \name Const values

	enum 
	{ 
		/// Number of animation slot in the CChannelMixer
		NumAnimationSlot=8 
	};

private:

	/// \name Internal classes

	/// An animation slot.
	class CSlot
	{
		friend class CChannelMixer;

		/// Default Ctor
		CSlot ()
		{
			// Not modified
			_Dirt=false;

			// default is 1.
			_Weight= 1.f;

			// Set it empty
			empty ();
		}

		/// Empty the slot
		void empty ()
		{
			_Animation=NULL;
			_SkeletonWeight=NULL;
			_InvertedSkeletonWeight=false;
		}

		/// Is the slot empty ?
		bool isEmpty ()
		{
			return _Animation==NULL;
		}

	private:
		/// Animation pointer to use by this slot. If NULL, slot is empty.
		const CAnimation*	_Animation;

		/// Skeleton weight pointer
		const CSkeletonWeight*	_SkeletonWeight;

		/// Skeleton weight pointer inverted or not
		bool				_InvertedSkeletonWeight;

		/// Time to use to eval the animation.
		TAnimationTime		_Time;

		/**
		  * Global weight to apply to the animation of this slot.
		  * This weight can be gived in any range because renormalisatio is done in final
		  * weight evaluation. If weight is 0.f, the final mix is not influenced by the animation
		  * of this slot.
		  */
		float				_Weight;

		/// Dirt flag. True if the animation of this slot as been modified
		bool				_Dirt;
	};

	/**
	 * An animated channel of the CChannelMixer. This class is used internaly in the 
	 * CChannelMixer.
	 *
	 * \author Cyril 'Hulud' Corvazier
	 * \author Nevrax France
	 * \date 2001
	 */
	class CChannel
	{
		friend class CChannelMixer;
	public:
		enum	{EnableUserFlag= 1, EnableLodFlag= 2, EnableAllFlag= 3};

	public:
		/// Default ctor
		CChannel ()
		{
			// not in the list
			_InTheList=false;
			// enabled by default.
			_EnableFlags= EnableAllFlag;
		}
	private:
		/// True if this channel is in the list
		bool				_InTheList;

		/// the detail mode.
		bool				_Detail;

		/// enabled Flags. User | Lod
		uint8				_EnableFlags;

		/// Is this Animated Value a CQuat Animated Value???
		bool				_IsQuat;


		/// Name of the channel in the channel mixer. Must be the same than the animated value name.
		std::string			_ChannelName;

		/// A pointer on the IAnimatable object that handles the channel value.
		IAnimatable*		_Object;

		/// A pointer on the IAnimatedValue animated by this channel. If NULL, the channel is empty
		IAnimatedValue*		_Value;

		/// The id of the animated value in the IAnimatable object.
		uint32				_ValueId;

		/// The id of the OwnerBit to touch the IAnimatable object. Usefull for IAnimatable derivation.
		uint32				_OwnerValueId;

		/// The default track pointer used when track are missing in the animation. Can't be NULL.
		const ITrack*		_DefaultTracks;

		/** 
		  * A track pointer on each slot CAnimation. Can't be NULL. If no track found for this 
		  * channel, the pointer is _DefaultTracks.
		  */
		const ITrack*		_Tracks[NumAnimationSlot];

		/**
		  * A weight array for to blend each slot. 
		  * This value must be between 0.f and 1.f. If it is 0.f, the slot is not used. If it is 1.f,
		  * the slot is used at 100%. This weight can be set using a "skeleton template weight".
		  * Default value is 1.f.
		  */
		float				_Weights[NumAnimationSlot];

		/**
		  * Pointer on the next channel selected for the animations selected in the slots
		  *
		  * This list is used to only visit the channels animated by the animations set in the slots
		  * of the mixer
		  */
		CChannel*			_Next;

	};
public:
	/// Constructor. The default constructor resets the slots and the channels.
	CChannelMixer();

	/// \name Setup the mixer

	/**
	  * Set the animation set used by this channel mixer.
	  * The pointer is hold by the channel mixer until it changes.
	  */
	void setAnimationSet (const CAnimationSet* animationSet);

	/**
	  * Get the animation set used by this channel mixer.
	  * The pointer is hold by the channel mixer until it changes. Return NULL if no animationSet defined.
	  */
	const CAnimationSet* getAnimationSet () const;

	/** 
	  * Launch evaluation of channels.
	  *
	  * This is the main method. It evals animations selected in the slots for listed
	  * channels.
	  *
	  * Only the channels that are animated by animations selected in the slots are evaluated.
	  * They are stored in a linked list managed by the channel array.
	  *
	  * Others are initialized with the default channel value.
	  *
	  * \param detail true if eval the detail part of animation. (done after clipping).
	  * \param evalDetailDate chann mixer store the last date of anim detail evaluated. if same, do nothing. ingored if detail is false.
	  */
	void eval (bool detail, uint64 evalDetailDate=0);

	/// \name Channel access

	/** 
	  * Add a channel for a specific value of an IAnimatable object.
	  * Warning: this method will assign the default value in the animated value.
	  * 
	  * \param channelName is the name of the channel.
	  * \param animatable is a pointer on the IAnimatable object in which the value is stored. It will be kept
	  * by the CChannelMixer until it is removed from the channel.
	  * \param value is a pointer on the value the channel works with. It will be kept
	  * by the CChannelMixer until it is removed from the channel.
      * \param defaultValue is a track used by default if a track is not presents in the animation for this channel. 
	  * It will be kept by the CChannelMixer until it is removed from the channel.
	  * \param valueId is the value ID in the IAnimatable object.
	  * \param ownerId is the owner Bit of the animated vlaue, in the IAnimatable object. touched when the animatedvalue is touched.
	  * \param detail true if this channel must be evaluated in detail mode (see eval()).
	  * \return -1 if the track was not found in the animationSet, else it return the channelId 
	  *	as if returned by CAnimationSet::getChannelIdByName(channelName).
	  */
	sint addChannel (const std::string& channelName, IAnimatable* animatable, IAnimatedValue* value, ITrack* defaultValue, uint32 valueId, uint32 ownerValueId, bool detail);

	/// Reset the channel list if the mixer. All channels are removed from the mixer.
	void resetChannels ();


	/** disabling a channel means it is no more modified during animation. Default is enabled. 
	 *	NB: this channel must have been added (via addChannel()....).
	 *	\param channelId channelId get from CAnimationSet::getChannelIdByName() or addChannel()
	 */
	void enableChannel (uint channelId, bool enable);

	/** see enableChannel(). return false if channel do not exist...
	 *	\param channelId channelId get from CAnimationSet::getChannelIdByName() or addChannel()
	 */
	bool isChannelEnabled (uint channelId) const;

	/** Same as enableChannel but for Animation Lod system. The channel is animated only if both
	 *	enableChannel() and lodEnableChannel() are true. Default is enabled.
	 *	NB: this channel must have been added (via addChannel()....).
	 *	\param channelId channelId get from CAnimationSet::getChannelIdByName() or addChannel()
	 */
	void lodEnableChannel (uint channelId, bool enable);

	/** see enableChannel(). return false if channel do not exist...
	 *	\param channelId channelId get from CAnimationSet::getChannelIdByName() or addChannel()
	 */
	bool isChannelLodEnabled (uint channelId) const;


	/// \name Slots acces

	/** 
	  * Set slot animation.
	  *
	  * You must set an animationSet in the channel mixer before calling this.
	  *
	  * Calling this method will dirt the mixer, ie, all the mixer's channels will 
	  * be visited to check if they are used by the new animation. If they are, they
	  * will be linked in the internal CChannel list.
	  * 
	  * \param slot is the slot number to change the animation. Must be >= 0 and < NumAnimationSlot.
	  * \param animation is the new animation index in the animationSet use by this slot.
	  * \see CAnimationSet, CAnimation
	  */
	void setSlotAnimation (uint slot, uint animation);

	/// Get the animation used by a given slot
	const CAnimation	*getSlotAnimation(uint slot) const;

	/** 
	  * Set time of a slot.
	  *
	  * This time will be used to eval the animation set in this slot.
	  * Each slot can have different time.
	  *
	  * Calling this method won't dirt the mixer.
	  * 
	  * \param slot is the slot number to change the time. Must be >= 0 and < NumAnimationSlot.
	  * \param time is the new time to use in the slot.
	  * \see TAnimationTime
	  */
	void setSlotTime (uint slot, TAnimationTime time)
	{
		// Check alot arg
		nlassert (slot<NumAnimationSlot);

		// Set the time
		_SlotArray[slot]._Time=time;
	}

	/** 
	  * Set slot weight.
	  *
	  * This weight will be used to eval the animation set in this slot.
	  * Each slot can have different weight. Calling this method won't dirt the mixer.
	  *
	  *	By default the weight of the slot is 1.0f.
	  *
	  * \param slot is the slot number to change the weight.
	  * \param weight is the new weight to use in the slot. No range for this weight. If the weight == 0.f, 
	  * the slot have no effect on the final mix.
	  */
	void setSlotWeight (uint slot, float weight)
	{
		// Check alot arg
		nlassert (slot<NumAnimationSlot);

		// Set the time
		_SlotArray[slot]._Weight=weight;
	}

	/** 
	  * Empty a slot.
	  * 
	  * Calling this method will dirt the mixer, ie, all the mixer's channels will 
	  * be visited to check if they are used by the old animation. If they are, they
	  * will be linked in the internal CChannel list.
	  *
	  * Warning: this method will assign the default value in the animated value that are removed from the active channel queue.
	  * 
	  * \param slot is the slot number to empty. Must be >= 0 and < NumAnimationSlot.
	  */
	void emptySlot (uint slot);

	/**
	  * Reset the slot of the mixer. All slot will be empty.
	  * 
	  * Calling this method will dirt the mixer, ie, all the mixer's channels will 
	  * be visited to check if they are used by the old animation. If they are, they
	  * will be linked in the internal CChannel list.
	  * 
	  */
	void resetSlots ();

	/**
	  * Apply a skeleton template weight on a specific slot.
	  *
	  * This method apply the weight of each node contains in skelWeight to the channel's slot weight.
	  *
	  * \param slot is the slot number to empty. Must be >= 0 and < NumAnimationSlot.
	  * \param skeleton is the index of a skeleton in the animationSet.
	  * \param invert is true if the weights to attach to the channels are the weights of the skeleton template. 
	  * false if the weights to attach to the channels are the 1.f-weights of the skeleton template.
	  */
	void applySkeletonWeight (uint slot, uint skeleton, bool invert=false);

	/**
	  * Reset the skeleton weight for a specific slot.
	  *
	  * This method apply set each channel's slot weight to 1.f.
	  *
	  * \param slot is the slot number to empty. Must be >= 0 and < NumAnimationSlot.
	  */
	void resetSkeletonWeight (uint slot);

private:
	
	/// /name Iternal methods
	
	/// Clean the mixer
	void cleanAll ();
	
	/// Dirt all slots
	void dirtAll ();

	/// Reshresh channel list
	void refreshList ();

	// The slot array
	CSlot							_SlotArray[NumAnimationSlot];

	// The animation set
	const CAnimationSet*			_AnimationSet;

	// The set of CChannel infos. Only channels added by addChannel are present.
	std::map<uint, CChannel>		_Channels;

	// The first Global channel. If NULL, no channel to animate.  (animed in eval(false))
	CChannel*						_FirstChannelGlobal;

	// The first dertail channel. If NULL, no channel to animate.  (animed in eval(true))
	CChannel*						_FirstChannelDetail;

	// last date of evalDetail().
	sint64							_LastEvalDetailDate;

	// The channels list is dirty if true.
	bool							_Dirt;
};


} // NL3D


#endif // NL_CHANNEL_MIXER_H

/* End of channel_mixer.h */
