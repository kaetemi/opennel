/** \file sound_anim_marker.cpp
 * A sound event marker on a sound track
 *
 * $Id$
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

#include "stdsound.h"
#include "nel/misc/common.h"
#include "nel/misc/string_mapper.h"
#include "nel/sound/sound_anim_marker.h"
#include "nel/sound/u_audio_mixer.h"
#include "nel/sound/u_source.h"

using namespace std;
using namespace NLSOUND;
using namespace NLMISC;

namespace NLSOUND {

// ********************************************************

CSoundAnimMarker::~CSoundAnimMarker()
{
}

// ********************************************************

void CSoundAnimMarker::play(UAudioMixer* mixer, NL3D::CCluster *cluster, CSoundContext &context)
{
	TMarkerSoundSet::iterator first(_Sounds.begin()), last(_Sounds.end());

	for (; first != last; ++first)
	{
		USource* source = mixer->createSource((*first), true, NULL, NULL, cluster, &context);
		if (source != NULL)
		{
			source->setRelativeGain(context.RelativeGain);
			source->setPos(context.Position);
			source->play();
		}
	}
}

// ********************************************************

void CSoundAnimMarker::addSound(const NLMISC::TStringId& soundName)
{
	pair<TMarkerSoundSet::iterator, bool> inserted;
	inserted = _Sounds.insert(soundName);
	if (inserted.second == false)
	{
		nlwarning("Duplicate sound (%s)", CStringMapper::unmap(soundName).c_str());
	}
}

// ********************************************************

void CSoundAnimMarker::removeSound(const NLMISC::TStringId &soundName)
{
	TMarkerSoundSet::iterator iter = _Sounds.find(soundName);
    if (iter != _Sounds.end())
	{
		_Sounds.erase(iter);
	}
	else
	{
		nlwarning("No sound was removed (%s)", CStringMapper::unmap(soundName).c_str());		
	}
}

// ********************************************************

void CSoundAnimMarker::getSounds(vector<NLMISC::TStringId> &sounds)
{
	sounds.insert(sounds.end(), _Sounds.begin(), _Sounds.end());

/*	TMarkerSoundSet::iterator first(_Sounds.begin()), last(_Sounds.end());
	for (; first != last; ++first)
	{
		sounds.push_back((*first).c_str());
	}
*/
}



} // namespace NLSOUND
