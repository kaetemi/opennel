/** \file play_list_user.cpp
 * <File description>
 *
 * $Id: play_list_user.cpp,v 1.6 2002/06/10 09:30:08 berenguier Exp $
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

#include "std3d.h"

#include "3d/play_list_user.h"
#include "3d/animation_set_user.h"
#include "nel/misc/hierarchical_timer.h"

#include "nel/misc/debug.h"

using namespace NLMISC;

namespace NL3D 
{

H_AUTO_DECL( NL3D_UI_PlayList )

#define	NL3D_HAUTO_UI_PLAY_LIST					H_AUTO_USE( NL3D_UI_PlayList )


// ***************************************************************************

TAnimationTime CPlayListUser::getLocalTime (uint8 slot, TGlobalAnimationTime globalTime, const UAnimationSet& animSet) const
{
	NL3D_HAUTO_UI_PLAY_LIST;

	// Cast
	const CAnimationSetUser *cAnimSetUser=safe_cast<const CAnimationSetUser*>(&animSet);

	// Get the animationset pointer
	const CAnimationSet* cAnimSet=cAnimSetUser->getAnimationSet ();
	nlassert (cAnimSet);

	return _PlayList.getLocalTime (slot, globalTime, *cAnimSet);
}

// ***************************************************************************

float CPlayListUser::getLocalWeight (uint8 slot, TGlobalAnimationTime globalTime) const
{
	NL3D_HAUTO_UI_PLAY_LIST;

	return _PlayList.getLocalWeight (slot, globalTime);
}

// ***************************************************************************

void	CPlayListUser::registerTransform(UTransform *object, const char* prefix)
{
	NL3D_HAUTO_UI_PLAY_LIST;

	CTransformUser	*obj= dynamic_cast<CTransformUser*>(object);
	nlassert(obj);

	// Register the transform to the channel mixer.
	obj->getTransform()->registerToChannelMixer(&_ChannelMixer, prefix);
}
void	CPlayListUser::resetAllChannels()
{
	NL3D_HAUTO_UI_PLAY_LIST;

	_ChannelMixer.resetChannels();
}

// ***************************************************************************

void CPlayListUser::emptyPlayList ()
{
	NL3D_HAUTO_UI_PLAY_LIST;

	_PlayList.emptyPlayList ();
}

void CPlayListUser::setAnimation (uint8 slot, uint animation)
{
	NL3D_HAUTO_UI_PLAY_LIST;

	_PlayList.setAnimation (slot, animation);
}

uint CPlayListUser::getAnimation (uint8 slot) const
{
	NL3D_HAUTO_UI_PLAY_LIST;

	return _PlayList.getAnimation (slot);
}

// ***************************************************************************

void CPlayListUser::setTimeOrigin (uint8 slot, TGlobalAnimationTime timeOrigin)
{
	NL3D_HAUTO_UI_PLAY_LIST;

	_PlayList.setTimeOrigin (slot, timeOrigin);
}

TGlobalAnimationTime CPlayListUser::getTimeOrigin (uint8 slot) const
{
	NL3D_HAUTO_UI_PLAY_LIST;

	return _PlayList.getTimeOrigin (slot);
}

void CPlayListUser::setSpeedFactor (uint8 slot, float speedFactor)
{
	NL3D_HAUTO_UI_PLAY_LIST;

	_PlayList.setSpeedFactor (slot, speedFactor);
}

float CPlayListUser::getSpeedFactor (uint8 slot) const
{
	NL3D_HAUTO_UI_PLAY_LIST;

	return _PlayList.getSpeedFactor (slot);
}

void CPlayListUser::setWrapMode (uint8 slot, TWrapMode wrapMode)
{
	NL3D_HAUTO_UI_PLAY_LIST;

	_PlayList.setWrapMode (slot, (CAnimationPlaylist::TWrapMode)(uint)wrapMode);
}

UPlayList::TWrapMode CPlayListUser::getWrapMode (uint8 slot) const
{
	NL3D_HAUTO_UI_PLAY_LIST;

	return (UPlayList::TWrapMode)(uint)_PlayList.getWrapMode (slot);
}

// ***************************************************************************

void CPlayListUser::setStartWeight (uint8 slot, float startWeight, TGlobalAnimationTime time)
{
	NL3D_HAUTO_UI_PLAY_LIST;

	_PlayList.setStartWeight (slot, startWeight, time);
}

float CPlayListUser::getStartWeight (uint8 slot, TGlobalAnimationTime& time) const
{
	NL3D_HAUTO_UI_PLAY_LIST;

	return _PlayList.getStartWeight(slot, time);
}

void CPlayListUser::setEndWeight (uint8 slot, float endWeight, TGlobalAnimationTime time)
{
	NL3D_HAUTO_UI_PLAY_LIST;

	_PlayList.setEndWeight (slot, endWeight, time);
}

float CPlayListUser::getEndWeight (uint8 slot, TGlobalAnimationTime& time) const
{
	NL3D_HAUTO_UI_PLAY_LIST;

	return _PlayList.getEndWeight (slot, time);
}

void CPlayListUser::setWeightSmoothness (uint8 slot, float smoothness)
{
	NL3D_HAUTO_UI_PLAY_LIST;

	_PlayList.setWeightSmoothness (slot, smoothness);
}

float CPlayListUser::getWeightSmoothness (uint8 slot) const
{
	NL3D_HAUTO_UI_PLAY_LIST;

	return _PlayList.getWeightSmoothness (slot);
}

void CPlayListUser::setWeight (uint8 slot, float weight)
{
	NL3D_HAUTO_UI_PLAY_LIST;

	_PlayList.setStartWeight (slot, weight, 0);
	_PlayList.setEndWeight (slot, weight, 0);
}

// ***************************************************************************

void CPlayListUser::setSkeletonWeight (uint8 slot, uint skeletonId, bool inverted)
{
	NL3D_HAUTO_UI_PLAY_LIST;

	_PlayList.setSkeletonWeight (slot, skeletonId, inverted);
}

uint CPlayListUser::getSkeletonWeight (uint8 slot, bool &inverted) const
{
	NL3D_HAUTO_UI_PLAY_LIST;

	return _PlayList.getSkeletonWeight (slot, inverted);
}

// ***************************************************************************

void CPlayListUser::enableChannel (uint channelId, bool enable)
{
	NL3D_HAUTO_UI_PLAY_LIST;

	_ChannelMixer.enableChannel(channelId, enable);
}

bool CPlayListUser::isChannelEnabled (uint channelId) const
{
	NL3D_HAUTO_UI_PLAY_LIST;

	return _ChannelMixer.isChannelEnabled (channelId) ;
}


} // NL3D
