/** \file animation.h
 * <File description>
 *
 * $Id: animation.h,v 1.1 2001/06/15 16:24:42 corvazier Exp $
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

#ifndef NL_ANIMATION_H
#define NL_ANIMATION_H

#include "nel/misc/types_nl.h"
#include "nel/3d/animation_time.h"
#include <memory>
#include <map>
#include <set>
#include <vector>

namespace NLMISC
{
class IStream;
struct EStream;
}

namespace NL3D 
{

class ITrack;

/**
 * This class describes animations for several tracks. Each track works
 * with a IChannel for the IAnimatable objects.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CAnimation
{
public:

	/// Destructor
	~CAnimation ();

	/// \name Public interface.

	enum { NotFound=0xffffffff };

	/** Get track with its name.
	  *
	  * \param name is the name of the desired track.
	  * \return CAnimation::NotFound if the track doesn't exist else the the id of the track.
	  */
	uint getIdTrackByName (const std::string& name) const
	{
		// Find an entry in the name/id map
		TMapStringUInt::const_iterator ite=_IdByName.find (name);

		// Not found ?
		if (ite==_IdByName.end ())
			// yes, error
			return NotFound;
		else
			// no, return track ID
			return (uint)ite->second;
	}

	/** 
	  * Fill the set of string with the name of the channels.
	  *
	  * \return the count of track in this animation
	  */
	void getTrackNames (std::set<std::string>& setString) const;

	/** Get a track pointer
	  *
	  * \param channelId is the id of the desired channel.
	  */
	const ITrack* getTrack (uint trackId) const
	{
		// Get the trackId-th track pointer
		return _TrackVector[trackId];
	}

	/** Add a track at the end of the track list.
	  * 
	  * This method is used to insert tracks in the animation.
	  * Tracks must be allocated with new. The pointer is then handeled
	  * by the CAnimation.
	  */
	void addTrack (const std::string& name, ITrack* pChannel);

	/**
	  * Get begin time of the track.
	  *
	  * \return the begin time.
	  */
	CAnimationTime getBeginTime () const;

	/**
	  * Get end time of the track.
	  *
	  * \return the end time.
	  */
	CAnimationTime getEndTime () const;

	/// Serial the template
	void serial (NLMISC::IStream& f);

private:
	/// \name Members
	typedef std::map<std::string, uint32> TMapStringUInt;
	typedef std::vector<ITrack* > TVectAPtrTrack;

	// Animation name
	std::string			_Name;

	// Map to get a channel id with a name.
	TMapStringUInt		_IdByName;

	// Vector of channel pointer.
	TVectAPtrTrack		_TrackVector;
};


} // NL3D


#endif // NL_ANIMATION_H

/* End of animation.h */
