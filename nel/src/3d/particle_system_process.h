/** \file particle_system_process.h
 * <File description>
 *
 * $Id: particle_system_process.h,v 1.6 2001/11/22 15:34:13 corvazier Exp $
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

#ifndef NL_PARTICLE_SYSTEM_PROCESS_H
#define NL_PARTICLE_SYSTEM_PROCESS_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/3d/animation_time.h"

namespace NLMISC
{
	class CAABBox ;
}

namespace NL3D {


class CParticleSystem ;
class CFontGenerator ;
class CFontManager ;


/** rendering and process passes for a particle system.
 *  PSCollision : collisions with collision zones (see CPSZone)
 *  PSMotion    : motion computation
 *  PSSolidRender : render particle that can modify z-buffer
 *  PSBlendRender : render transparency (no z-buffer write)
 *  PSToolRender  : for edition purpose, show representations for forces, emitters...
 */
enum TPSProcessPass 
{ PSEmit, PSCollision, PSMotion, PSDynamic, PSPostdynamic, PSSolidRender, PSBlendRender, PSToolRender } ;



/**
 *	A system particle process; A process is anything that can be called at each update of the system
 */

class CParticleSystemProcess : public NLMISC::IStreamable
{
	public:
	
		/// \name Object
		/// @{
			/// ctor
			CParticleSystemProcess() : _Owner(NULL), _SystemBasisEnabled(false) {}
			
			/// dtor
			virtual ~CParticleSystemProcess()  {}

			/** Serialize this object.
			* Everything is saved, except for the fontManager and the fontGenerator.
			* They must be set again if the PSToolRender pass is used.
			*/
			virtual void			serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;					
		/// @}

		
		/**
		* execute this process, telling how much time ellapsed 
		*/
		virtual void			step(TPSProcessPass pass, TAnimationTime ellapsedTime) = 0 ;


		/** Compute the aabbox of this process, (expressed in world basis).
		*  \return true if there is any aabbox
		*  \param aabbox a ref to the result box
		*/
		virtual bool			computeBBox(NLMISC::CAABBox &aabbox) const = 0 ;

		/// Set the process owner. Called by the particle system during attachment.
		void					setOwner(CParticleSystem *ps) { _Owner = ps ; }

		/// Retrieve the particle system that owns this process
		CParticleSystem			*getOwner(void) { return _Owner ; }

		/// retrieve the particle system that owns this process (const version)
		const CParticleSystem	*getOwner(void) const { return _Owner ; }

		/// \name Useful methods for edition
		//@{
			/// Shortcut to get a font generator if one was set (edition mode)
			CFontGenerator			*getFontGenerator(void) ;

			/// Shortcut to get a font generator if one was set, const version  (edition mode)
			const CFontGenerator	*getFontGenerator(void) const ;

			/// Shortcut to get a font Manager if one was set (edition mode)
			CFontManager			*getFontManager(void) ;

			/// Shortcut to get a font Manager if one was set, const version  (edition mode)
			const CFontManager		*getFontManager(void) const ;
		//@}

		/**	
		* Return true if the process is in the particle system basis, false if it's in the world basis
		*/
		bool					isInSystemBasis(void) const 
		{ 
			return _SystemBasisEnabled ; 
		}

		/** Choose the basis for this process. Warning : This won't change any existing coordinate
		 *  By default, all process are expressed in the world basis
		 *  \param sysBasis truer if particles are in the system basis
		 */
		virtual void			setSystemBasis(bool sysBasis = true) { _SystemBasisEnabled = sysBasis ; }
	
		/// tells wether there are alive entities / particles in the system
		virtual bool			hasParticles(void) const { return false ; }

		/// tells wether there are alive emitters / particles in the system
		virtual bool			hasEmitters(void) const { return false ; }


		/// max number of faces wanted by this process (for load balancing)
		virtual uint			querryMaxWantedNumFaces(void) = 0 ;


		
		/// test wether parametric motion is enabled
		virtual bool		 isParametricMotionEnabled(void) const { return false;}

		/// perform parametric motion if enabled
			virtual void performParametricMotion(TAnimationTime date, TAnimationTime ellapsedTime) { nlassert(0);}
			
	protected:
		CParticleSystem *_Owner ;

		// true if the system basis is used for display and motion
		bool _SystemBasisEnabled ;
	
} ;


} // NL3D


#endif // NL_PARTICLE_SYSTEM_PROCESS_H

/* End of particle_system_process.h */
