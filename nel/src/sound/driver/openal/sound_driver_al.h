/** \file sound_driver_al.h
 * OpenAL sound driver
 *
 * $Id: sound_driver_al.h,v 1.4 2001/07/19 12:47:07 cado Exp $
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

#ifndef NL_SOUND_DRIVER_AL_H
#define NL_SOUND_DRIVER_AL_H

#include "nel/misc/types_nl.h"
#include "sound/driver/sound_driver.h"
#include "sound/driver/openal/source_al.h"
#include "sound/driver/openal/buffer_al.h"
#include <al/al.h>
#ifdef NL_OS_WINDOWS
#include <objbase.h> // needed before eax.h
#endif
#include <eax.h>
#include <vector>


namespace NLSOUND {


// alGenBuffers, alGenSources
//typedef ALAPI ALvoid ALAPIENTRY (*TGenFunctionAL) ( ALsizei, ALuint* );
typedef ALvoid (*TGenFunctionAL) ( ALsizei, ALuint* );

// alIsBuffer, alIsSource
//typedef ALAPI ALboolean ALAPIENTRY (*TTestFunctionAL) ( ALuint );
typedef ALboolean (*TTestFunctionAL) ( ALuint );


// EAXSet global function
extern EAXSet	EAXSetProp;

// EAXGet global function
extern EAXGet	EAXGetProp;


#ifdef NL_DEBUG
void TestALError();
#else
#define TestALError() NULL
#endif



/**
 * OpenAL sound driver
 *
 * The caller of the create methods is responsible for the deletion of the created objects
 * These objects must be deleted before deleting the ISoundDriver instance.
 *
 * 
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CSoundDriverAL : public ISoundDriver
{
public:

	/// Constructor
	CSoundDriverAL();

	/// Return the instance of the singleton
	static CSoundDriverAL	*instance() { return _Instance; }

	/// Initialization
	virtual bool			init();

	/// Create a sound buffer
	virtual	IBuffer			*createBuffer();

	/// Create the listener instance
	virtual	IListener		*createListener();

	/// Create a source
	virtual	ISource			*createSource();

	/// Change the rolloff factor and apply to all sources
	void					applyRolloffFactor( float f );

	/// Temp
	virtual bool			loadWavFile( IBuffer *destbuffer, const char *filename );

	// Does not create a sound loader

public:
	
	/// Destructor
	virtual					~CSoundDriverAL();

protected:

	friend CBufferAL::~CBufferAL();
	friend CSourceAL::~CSourceAL();

	/// Remove a buffer
	void					removeBuffer( IBuffer *buffer );

	/// Remove a source
	void					removeSource( ISource *source );

protected:

	/// Allocate nb new buffers or sources
	void					allocateNewItems( TGenFunctionAL algenfunc, TTestFunctionAL altestfunc,
											  std::vector<ALuint>& names, uint index, uint nb );

	/// Generate nb buffers
	void					generateItems( TGenFunctionAL algenfunc, TTestFunctionAL altestfunc, uint nb, ALuint *array );

	/// Remove names of deleted items and return the number of valid items
	uint					compactAliveNames( std::vector<ALuint>& names, TTestFunctionAL testfunc );

	/// Create a sound buffer or a sound source
	ALuint					createItem( TGenFunctionAL algenfunc, TTestFunctionAL altestfunc,
										std::vector<ALuint>& names, uint& index, uint allocrate );

	/// Delete a buffer or a source
	bool					deleteItem( ALuint name, TGenFunctionAL aldeletefunc, std::vector<ALuint>& names );
	
private:

	// The instance of the singleton
	static CSoundDriverAL	*_Instance;

	// Allocated buffers
	std::vector<ALuint>		_Buffers;

	// Allocated sources
	std::vector<ALuint>		_Sources;

	// Number of exported buffers (including any deleted buffers)
	uint					_NbExpBuffers;

	// Number of exported sources (including any deleted buffers)
	uint					_NbExpSources;

	// Rolloff factor (not in the listener in OpenAL, but relative to the sources)
	float					_RolloffFactor;

};


} // NLSOUND


#endif // NL_SOUND_DRIVER_AL_H

/* End of sound_driver_al.h */
