/** \file listener_al.h
 * OpenAL sound listener
 *
 * $Id: listener_al.h,v 1.1 2001/06/26 15:28:56 cado Exp $
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

#ifndef NL_LISTENER_AL_H
#define NL_LISTENER_AL_H

#include "nel/misc/types_nl.h"
#include "sound/driver/listener.h"


namespace NLSOUND {


/**
 * OpenAL sound listener
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CListenerAL : public IListener
{
public:

	/// Constructor
	CListenerAL();

	/// \name Listener properties
	//@{
	/// Set the position vector (default: (0,0,0)) (3D mode only)
	virtual void			setPosition( const NLMISC::CVector& pos );
	/** Get the position vector.
	 * See setPosition() for details.
	 */
	virtual void			getPosition( NLMISC::CVector& pos ) const;
	/// Set the velocity vector (3D mode only) (default: (0,0,0))
	virtual void			setVelocity( const NLMISC::CVector& vel );
	/// Get the velocity vector
	virtual void			getVelocity( NLMISC::CVector& vel ) const;
	/// Set the orientation vectors (3D mode only) (default: (0,1,0), (0,0,-1))
	virtual void			setOrientation( const NLMISC::CVector& front, const NLMISC::CVector& up );
	/// Get the orientation vectors
	virtual void			getOrientation( NLMISC::CVector& front, NLMISC::CVector& up ) const;
	//@}

	/// \name Global properties
	//@{
	/// Set the doppler factor (default: 1) to exaggerate or not the doppler effect
	virtual void			setDopplerFactor( float f );
	/// Set the rolloff factor (default: 1) to scale the distance attenuation effect
	virtual void			setRolloffFactor( float f );
	/// Set DSPROPERTY_EAXLISTENER_ENVIRONMENT and DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE if EAX available (see EAX listener properties)
	virtual void			setEnvironment( uint env, float size=7.5f );
	/// Set any EAX listener property if EAX available
	virtual void			setEAXProperty( uint prop, void *value, uint valuesize );
	//@}

	/// Destructor
	virtual					~CListenerAL() {}

};


} // NLSOUND


#endif // NL_LISTENER_AL_H

/* End of listener_al.h */
