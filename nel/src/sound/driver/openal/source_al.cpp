/** \file source_al.cpp
 * OpenAL sound source
 *
 * $Id: source_al.cpp,v 1.10 2001/08/27 08:50:56 cado Exp $
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

#include "source_al.h"
#include "sound_driver_al.h"
#include "nel/misc/vector.h"
#include "nel/misc/common.h"

using namespace NLMISC;


namespace NLSOUND {


// Currently, the OpenAL headers are different between Windows and Linux versions !
#ifdef NL_OS_UNIX
#define alGetSourcei alGetSourceiv
#define alGetSourcef alGetSourcefv
#endif


/*
 * Constructor
 */
CSourceAL::CSourceAL( ALuint sourcename ) : ISource(), _SourceName(sourcename)
{}


/*
 * Destructor
 */
CSourceAL::~CSourceAL()
{
	CSoundDriverAL *sdal = CSoundDriverAL::instance();
	sdal->removeSource( this );
}


/* Set the buffer that will be played (no streaming)
 * If the buffer is stereo, the source mode becomes stereo and the source relative mode is on,
 * otherwise the source is considered as a 3D source.
 */
void					CSourceAL::setStaticBuffer( IBuffer *buffer )
{
	ISource::setStaticBuffer( buffer );

	// Stop source
	alSourceStop( _SourceName );
	TestALError();

	// Set buffer
	if ( buffer == NULL )
	{
		alSourcei( _SourceName, AL_BUFFER, AL_NONE );
		TestALError();
	}
	else
	{
		CBufferAL *bufferAL = dynamic_cast<CBufferAL*>(buffer);
		alSourcei( _SourceName, AL_BUFFER, bufferAL->bufferName() );
		TestALError();

		// Set relative mode if the buffer is stereo
		setSourceRelativeMode( bufferAL->isStereo() );
	}
}


/*
 * Set looping on/off for future playbacks (default: off)
 */
void					CSourceAL::setLooping( bool l )
{
	alSourcei( _SourceName, AL_LOOPING, l?AL_TRUE:AL_FALSE );
	TestALError();
}


/*
 * Return the looping state
 */
bool					CSourceAL::getLooping() const
{
	ALint b;
	alGetSourcei( _SourceName, AL_LOOPING, &b );
	TestALError();
	return ( b == AL_TRUE );
}


/*
 * Play the static buffer (or stream in and play)
 */
void					CSourceAL::play()
{
	if ( _Buffer != NULL )
	{
		// Static playing mode
		alSourcePlay( _SourceName );
		TestALError();
	}
	else
	{
		// Streaming mode
		nlwarning( "AM: Cannot play null buffer; streaming not implemented" );
		nlstop;
	}
}


/*
 * Stop playing
 */
void					CSourceAL::stop()
{
	if ( _Buffer != NULL )
	{
		// Static playing mode
		alSourceStop( _SourceName );
		TestALError();
	}
	else
	{
		// Streaming mode
		nlwarning( "AM: Cannot stop null buffer; streaming not implemented" );
		//nlstop;
	}
}


/*
 * Pause. Call play() to resume.
 */
void					CSourceAL::pause()
{
	if ( _Buffer != NULL )
	{
		// Static playing mode
		alSourcePause( _SourceName );
		TestALError();
	}
	else
	{
		// Streaming mode
		nlwarning( "AM: Cannot pause null buffer; streaming not implemented" );
		nlstop;
	}
}


/*
 * Return the playing state
 */
bool					CSourceAL::isPlaying() const
{
	ALint srcstate;
	alGetSourcei( _SourceName, AL_SOURCE_STATE, &srcstate );
	TestALError();
	return (srcstate == AL_PLAYING);
}


/*
 * Update the source (e.g. continue to stream the data in)
 */
void					CSourceAL::update()
{
	// Streaming not implemented
}



/* Set the position vector.
 * 3D mode -> 3D position
 * st mode -> x is the pan value (from left (-1) to right (1)), set y and z to 0
 */
void					CSourceAL::setPos( const NLMISC::CVector& pos )
{
	// Coordinate system: conversion from NeL to OpenAL/GL:
	alSource3f( _SourceName, AL_POSITION, pos.x, pos.z, -pos.y );
	TestALError();
}


/* Get the position vector.
 * See setPos() for details.
 */
void					CSourceAL::getPos( NLMISC::CVector& pos ) const
{
	ALfloat v[3];
	alGetSourcefv( _SourceName, AL_POSITION, v );
	TestALError();
	// Coordsys conversion
	pos.set( v[0], -v[2], v[1] );
}


/*
 * Set the velocity vector (3D mode only)
 */
void					CSourceAL::setVelocity( const NLMISC::CVector& vel )
{
	// Coordsys conversion
	alSource3f( _SourceName, AL_VELOCITY, vel.x, vel.z, -vel.y );
	TestALError();
}


/*
 * Get the velocity vector
 */
void					CSourceAL::getVelocity( NLMISC::CVector& vel ) const
{
	ALfloat v[3];
	alGetSourcefv( _SourceName, AL_VELOCITY, v );
	TestALError();
	// Coordsys conversion
	vel.set( v[0], -v[2], v[1] );
}


/*
 * Set the direction vector (3D mode only)
 */
void					CSourceAL::setDirection( const NLMISC::CVector& dir )
{
	// Coordsys conversion
	alSource3f( _SourceName, AL_DIRECTION, dir.x, dir.z, -dir.y );
	TestALError();
}


/*
 * Get the direction vector
 */
void					CSourceAL::getDirection( NLMISC::CVector& dir ) const
{
	ALfloat v[3];
	alGetSourcefv( _SourceName, AL_DIRECTION, v );
	TestALError();
	// Coordsys conversion
	dir.set( v[0], -v[2], v[1] );
}


/* Set the gain (volume value inside [0 , 1]).
 * 0.0 -> silence
 * 0.5 -> -6dB
 * 1.0 -> no attenuation
 * values > 1 (amplification) not supported by most drivers
 */
void					CSourceAL::setGain( float gain )
{
	nlassert( (gain >= 0.0f) && (gain <= 1.0f ) );
	alSourcef( _SourceName, AL_GAIN, gain );
	TestALError();
}


/*
 * Get the gain
 */
float					CSourceAL::getGain() const
{
	ALfloat gain;
	alGetSourcef( _SourceName, AL_GAIN, &gain );
	TestALError();
	return gain;
}


/* Shift the frequency. 1.0f equals identity, each reduction of 50% equals a pitch shift
 * of one octave. 0 is not a legal value.
 */
void					CSourceAL::setPitch( float pitch )
{
	nlassert( (pitch > 0) && (pitch <= 1.0f ) );
	alSourcef( _SourceName, AL_PITCH, pitch );
	TestALError();
}


/*
 * Get the pitch
 */
float					CSourceAL::getPitch() const
{
	ALfloat pitch;
	alGetSourcef( _SourceName, AL_PITCH, &pitch );
	TestALError();
	return pitch;
}


/*
 * Set the source relative mode. If true, positions are interpreted relative to the listener position.
 */
void					CSourceAL::setSourceRelativeMode( bool mode )
{
	alSourcei( _SourceName, AL_SOURCE_RELATIVE, mode?AL_TRUE:AL_FALSE );
	TestALError();
}


/*
 * Get the source relative mode (3D mode only)
 */
bool					CSourceAL::getSourceRelativeMode() const
{
	ALint b;
	alGetSourcei( _SourceName, AL_SOURCE_RELATIVE, &b );
	TestALError();
	return (b==AL_TRUE);
}


/*
 * Set the min and max distances (3D mode only)
 */
void					CSourceAL::setMinMaxDistances( float mindist, float maxdist )
{
	nlassert( (mindist >= 0.0f) && (maxdist >= 0.0f) );
	alSourcef( _SourceName, AL_REFERENCE_DISTANCE, mindist );
	alSourcef( _SourceName, AL_MAX_DISTANCE, maxdist );
	TestALError();
}


/*
 * Get the min and max distances
 */
void					CSourceAL::getMinMaxDistances( float& mindist, float& maxdist ) const
{
	alGetSourcef( _SourceName, AL_REFERENCE_DISTANCE, &mindist );
	alGetSourcef( _SourceName, AL_MAX_DISTANCE, &maxdist );
	TestALError();
}


/*
 * Set the cone angles (in radian) and gain (in [0 , 1]) (3D mode only)
 */
void					CSourceAL::setCone( float innerAngle, float outerAngle, float outerGain )
{
	nlassert( (outerGain >= 0.0f) && (outerGain <= 1.0f ) );
	alSourcef( _SourceName, AL_CONE_INNER_ANGLE, radToDeg(innerAngle) );
	alSourcef( _SourceName, AL_CONE_OUTER_ANGLE, radToDeg(outerAngle) );
	alSourcef( _SourceName, AL_CONE_OUTER_GAIN, outerGain );
	TestALError();
}


/*
 * Get the cone angles (in radian)
 */
void					CSourceAL::getCone( float& innerAngle, float& outerAngle, float& outerGain ) const
{
	float ina, outa;
	alGetSourcef( _SourceName, AL_CONE_INNER_ANGLE, &ina );
	innerAngle = degToRad(ina);
	alGetSourcef( _SourceName, AL_CONE_OUTER_ANGLE, &outa );
	outerAngle = degToRad(outa);
	alGetSourcef( _SourceName, AL_CONE_OUTER_GAIN, &outerGain );
	TestALError();
}


/*
 * Set any EAX source property if EAX available
 */
void					CSourceAL::setEAXProperty( uint prop, void *value, uint valuesize )
{
#ifdef EAX_AVAILABLE
	if ( EAXSetProp != NULL )
	{
		EAXSetProp( &DSPROPSETID_EAX_SourceProperties, prop, _SourceName, value, valuesize );
	}
#endif
}


} // NLSOUND
