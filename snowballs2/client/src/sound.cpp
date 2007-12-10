/** \file sound.cpp
 * Sound interface between the game and NeL
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

//
// Includes
//

#include <cmath>
#include <vector>

#include <nel/misc/vector.h>
#include <nel/misc/command.h>

#include <nel/sound/u_audio_mixer.h>
#include <nel/sound/u_listener.h>
#include <nel/sound/u_source.h>

#include "sound.h"
#include "entities.h"

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLSOUND;

//
// Variables
//

UAudioMixer *AudioMixer = NULL;
TSoundId SoundId;
const vector<TSoundId> *SoundIdArray;
#ifdef NL_OS_WINDOWS
SBCLIENT::CPlaylistManager *PlaylistManager = NULL;
#endif

//
// Functions
//

void cbConfigFileSoundMaxTracks(NLMISC::CConfigFile::CVar &var)
{
#ifdef NL_OS_WINDOWS
	AudioMixer->changeMaxTrack(var.asInt());
#endif
}

void cbConfigFileFail(NLMISC::CConfigFile::CVar &var)
{
	nlwarning("You can't modify the config variable '%s' at runtime for now, please restart the game", var.asString().c_str());
}

void initSound()
{
#ifdef NL_OS_WINDOWS
	AudioMixer = UAudioMixer::createAudioMixer ();
	std::string driverName;
	NLSOUND::UAudioMixer::TDriver driverType;
	if (!ConfigFile.exists("SoundDriver")) 
#ifdef NL_OS_WINDOWS
		driverType = NLSOUND::UAudioMixer::DriverFMod;
#elif defined (NL_OS_UNIX)
		driverType = NLSOUND::UAudioMixer::DriverOpenAl;
#else
		driverType = NLSOUND::UAudioMixer::DriverAuto;
#endif
	else 
	{
		driverName = ConfigFile.getVar("SoundDriver").asString();
		if (driverName == "Auto") driverType = NLSOUND::UAudioMixer::DriverAuto;
		else if (driverName == "FMod") driverType = NLSOUND::UAudioMixer::DriverFMod;
		else if (driverName == "DSound") driverType = NLSOUND::UAudioMixer::DriverDSound;
		else if (driverName == "OpenAl") driverType = NLSOUND::UAudioMixer::DriverOpenAl;
		else nlerror("SoundDriver value '%s' is invalid.", driverName.c_str());
	}

	AudioMixer->init(
		ConfigFile.exists("SoundMaxTracks")
		? ConfigFile.getVar("SoundMaxTracks").asInt() : 32,
		ConfigFile.exists("SoundUseEax")
		? ConfigFile.getVar("SoundUseEax").asBool() : true,
		ConfigFile.exists("SoundUseADPCM")
		? ConfigFile.getVar("SoundUseADPCM").asBool() : true,
		NULL, false, driverType,
		ConfigFile.exists("SoundForceSoftware")
		? ConfigFile.getVar("SoundForceSoftware").asBool() : true);

	ConfigFile.setCallback("SoundMaxTracks", cbConfigFileSoundMaxTracks);
	ConfigFile.setCallback("SoundUseEax", cbConfigFileFail);
	ConfigFile.setCallback("SoundUseADPCM", cbConfigFileFail);
	ConfigFile.setCallback("SoundForceSoftware", cbConfigFileFail);
	ConfigFile.setCallback("SoundDriver", cbConfigFileFail);

	PlaylistManager = new SBCLIENT::CPlaylistManager(AudioMixer, &ConfigFile, "SoundPlaylist");

	/* AudioMixer->loadSoundBuffers ("sounds.nss", &SoundIdArray);
	nlassert( SoundIdArray->size() == 2 );
	SoundId = (*SoundIdArray)[0];
	// StSoundId = (*SoundIdArray)[1]; */
#endif
}

void playSound (CEntity &entity, TSoundId id)
{
/*	entity.Source = AudioMixer->createSource (id);
	entity.Source->setLooping (true);
	entity.Source->play ();
*/}

void deleteSound (CEntity &entity)
{
/*	if (entity.Source != NULL)
	{
		if (entity.Source->isPlaying ())
			entity.Source->stop ();

		AudioMixer->removeSource (entity.Source);
		entity.Source = NULL;
	}
*/}

void updateSound()
{
#ifdef NL_OS_WINDOWS
	PlaylistManager->update(DiffTime);
	AudioMixer->update();
#endif
}

void releaseSound()
{
#ifdef NL_OS_WINDOWS
	delete PlaylistManager;
	PlaylistManager = NULL;
	delete AudioMixer;
	AudioMixer = NULL;
#endif
}

#ifdef NL_OS_WINDOWS

NLMISC_COMMAND(music_bg,"background music","")
{
	// check args, if there s not the right number of parameter, return bad
	if (args.size() != 0) return false;
	//playMusic("xtarsia_evil-snowballs_game.ogg", 2000, true, true);
	PlaylistManager->playMusic(SBCLIENT_MUSIC_BACKGROUND);
	return true;
}

NLMISC_COMMAND(music_bg_beat,"background music with beat","")
{
	// check args, if there s not the right number of parameter, return bad
	if (args.size() != 0) return false;
	//playMusic("xtarsia_evil-snowballs_beat.ogg", 2000, true, true);
	PlaylistManager->playMusic(SBCLIENT_MUSIC_BACKGROUND_BEAT);
	return true;
}

NLMISC_COMMAND(music_wait,"loading music","")
{
	// check args, if there s not the right number of parameter, return bad
	if (args.size() != 0) return false;
	//playMusic("xtarsia_evil-snowballs_beat.ogg", 2000, true, true);
	PlaylistManager->playMusic(SBCLIENT_MUSIC_WAIT);
	return true;
}

NLMISC_COMMAND(music_login,"login screen music","")
{
	// check args, if there s not the right number of parameter, return bad
	if (args.size() != 0) return false;
	//playMusic("xtarsia_evil-snowballs_beat.ogg", 2000, true, true);
	PlaylistManager->playMusic(SBCLIENT_MUSIC_LOGIN);
	return true;
}

#endif