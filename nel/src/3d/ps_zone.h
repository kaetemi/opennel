/** \file ps_zone.h
 * <File description>
 *
 * $Id: ps_zone.h,v 1.2 2001/06/18 11:18:57 vizerie Exp $
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

#ifndef NL_PS_ZONE_H
#define NL_PS_ZONE_H

#include "nel/misc/types_nl.h"
#include "3d/ps_force.h"
#include "3d/ps_edit.h"
#include "3d/ps_attrib.h"
#include "3d/ps_plane_basis.h"


namespace NL3D {


/** This epsilon is in meter and give a thickness to surfaces for tests. It must be above above 0
* for the system to work correctly
*/
const float PSCollideEpsilon = 10E-3f ;



/**
 * This class hold any entity that has an effect over located : a sink , a bouncing zone etc
 * This is a kind a specialized force, and it has an attached list of the targets
 * Not sharable.
 * 
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSZone : public CPSTargetLocatedBindable
{
public:

	/// behaviour when a collision occurs

	enum TCollisionBehaviour { bounce = 0, destroy = 1 } ;

	/// Constructor
	CPSZone();

	// dtor

	~CPSZone() ;

	/**
	*  Gives the type for this bindable.	
	*/
	virtual uint32 getType(void) const { return PSZone ; }

	/**
	* Get the priority of the bindable
	* The more high it is, the earlier it is dealt with
	*/
	virtual uint32 getPriority(void) const { return 3500 ; }
	
	/**
	 * Process one pass for the zone
	 * The default behaviour call performMotion or show depending on the pass being processed
	 */
	virtual void step(TPSProcessPass pass, CAnimationTime ellapsedTime) ;

	
	/// This is usually called by the step method for the pass dedicated to zone
	virtual void performMotion(CAnimationTime ellapsedTime) = 0 ;

	/// Show the zone (edition mode).
	virtual void show(CAnimationTime ellapsedTime) = 0 ;
		

	/// Add a new type of located for this zone to apply on. nlassert if already present	
	virtual void attachTarget(CPSLocated *ptr) ;


	
	/// serialization, DERIVER must override this, and call the parent version
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) 
	{ 
		CPSTargetLocatedBindable::serial(f) ; 
		f.serialEnum(_CollisionBehaviour) ;
		f.serial(_BounceFactor) ;
		
	}


	/** Inherited from CPSTargetLocatedBindable. It's called when one of the targets has been detroyed or detached
	 *  The default behaviour, release collision infos from the located
	 */
	
	virtual void releaseTargetRsc(CPSLocated *target) ;


	/// set the bounce factor. It has meaning only if the behaviour is set to bounce...	
	void setBounceFactor(float bounceFactor) { _BounceFactor = bounceFactor ; }

	/// get the bounce factor. It has meaning only if the behaviour is set to bounce...	
	float getBounceFactor(void) const { return  _BounceFactor ; }


	void setCollisionBehaviour(TCollisionBehaviour behaviour) { _CollisionBehaviour = behaviour ; }

	TCollisionBehaviour getCollisionBehaviour(void) const { return _CollisionBehaviour ; }


protected:


	// the bounce factor. 1.f mean no energy loss
	float _BounceFactor ;


	TCollisionBehaviour _CollisionBehaviour ;

	/**
	 * This set speed of a located so that it looks like bouncing on a surface
	 * \param locatedIndex the index 
	 * \param bouncePoint the position where the collision occured
	 * \param surfNormal  the normal of the surface at the collision point (this must be a unit vector)
	 * \elasticity  1 = full bounce, 0 = no bounce (contact)
	 * \ellapsedTime the time ellapsed
	 */

//	void bounce(uint32 locatedIndex, const CVector &bouncePoint, const CVector &surfNormal, float elasticity, float ellapsedTime) ;




};


/** A plane over which particles bounce
 * It has an interface to move each plane individually
 */

class CPSZonePlane : public CPSZone, public IPSMover
{
	public:
		virtual void performMotion(CAnimationTime ellapsedTime) ;
		virtual void show(CAnimationTime ellapsedTime)  ;
	

		NLMISC_DECLARE_CLASS(CPSZonePlane) ;


		
		virtual void setMatrix(uint32 index, const CMatrix &m) ;		
		virtual CMatrix getMatrix(uint32 index) const ;

		virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;

	protected:
		TPSAttribVector _Normal ;
		CMatrix buildBasis(uint32 index) const ;

		virtual void resize(uint32 size) ;

		virtual void newElement(void) ;

		virtual void deleteElement(uint32 index) ;
} ;




/// a radius and its suare in the same struct
struct CRadiusPair
{
	// the adius, and the square radius
	float R, R2 ;
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial(R, R2) ;
	}
} ;


typedef CPSAttrib<CRadiusPair> TPSAttribRadiusPair ;

/** A sphere
 */


class CPSZoneSphere : public CPSZone, public IPSMover
{
	public:
		virtual void performMotion(CAnimationTime ellapsedTime) ;
		virtual void show(CAnimationTime ellapsedTime)  ;
	

		NLMISC_DECLARE_CLASS(CPSZoneSphere) ;

		CPSZoneSphere()
		{
			_Name = std::string("ZoneSphere") ;
		}
	 
	

		virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;


		// inherited from IPSMover
		virtual bool supportUniformScaling(void) const { return true ; }
		virtual void setMatrix(uint32 index, const CMatrix &m) ;	
		virtual CMatrix getMatrix(uint32 index) const ;
		virtual void setScale(uint32 k, float scale) ;
		virtual CVector getScale(uint32 k) const ;


	protected:		

		
	
		TPSAttribRadiusPair _Radius ;	

		CMatrix buildBasis(uint32 index) const ;

		virtual void resize(uint32 size) ;

		virtual void newElement(void) ;

		virtual void deleteElement(uint32 index) ;
} ;

/// a disc

class CPSZoneDisc : public CPSZone, public IPSMover
{
	public:
		virtual void performMotion(CAnimationTime ellapsedTime) ;
		virtual void show(CAnimationTime ellapsedTime)  ;
	
		CPSZoneDisc()
		{
			_Name = std::string("ZoneDisc") ;
		}

		NLMISC_DECLARE_CLASS(CPSZoneDisc) ;


		// inherited from IPSMover
		virtual bool supportUniformScaling(void) const { return true ; }
		virtual void setMatrix(uint32 index, const CMatrix &m) ;		
		virtual CMatrix getMatrix(uint32 index) const ;
		virtual void setScale(uint32 k, float scale) ;
		virtual CVector getScale(uint32 k) const ;

		virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;

		

	protected:
		TPSAttribVector _Normal ;
		TPSAttribRadiusPair _Radius ;

		CMatrix buildBasis(uint32 index) const ;

		virtual void resize(uint32 size) ;

		virtual void newElement(void) ;

		virtual void deleteElement(uint32 index) ;

} ;



/// a caped cylinder


class CPSZoneCylinder : public CPSZone, public IPSMover
{
	public:
		virtual void performMotion(CAnimationTime ellapsedTime) ;
		virtual void show(CAnimationTime ellapsedTime)  ;
	
		CPSZoneCylinder()
		{
			_Name = std::string("ZoneCylinder") ;
		}

		NLMISC_DECLARE_CLASS(CPSZoneCylinder) ;

		// inherited from IPSMover
		virtual bool supportUniformScaling(void) const { return true ; }
		virtual bool supportNonUniformScaling(void) const { return true ; }	
		virtual void setMatrix(uint32 index, const CMatrix &m) ;	
		virtual CMatrix getMatrix(uint32 index) const ;
		virtual void setScale(uint32 k, float scale) ;
		virtual void setScale(uint32 k, const CVector &s) ;
		virtual CVector getScale(uint32 k) const ;

		// serialization
		virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;

	

		

	protected:

		// the I and J vector of the cylinder
		CPSAttrib<CPlaneBasis> _Basis ;

		// dimension of cylinder in each direction, encoded in a vector
		TPSAttribVector _Dim ;

		CMatrix buildBasis(uint32 index) const ;

		virtual void resize(uint32 size) ;

		virtual void newElement(void) ;

		virtual void deleteElement(uint32 index) ;
} ;



/**
 *	The same as a plane, but with a rectangle. We don't encode the plane by its normal, however...
 */ 

class CPSZoneRectangle : public CPSZone, public IPSMover
{
	public:
		virtual void performMotion(CAnimationTime ellapsedTime) ;
		virtual void show(CAnimationTime ellapsedTime)  ;
	
		CPSZoneRectangle()
		{
			_Name = std::string("CPSZoneRectangle")  ;
		}

		NLMISC_DECLARE_CLASS(CPSZoneRectangle) ;


	
	

		// serialization
		virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;

		// inherited from IPSMover
		virtual bool supportUniformScaling(void) const { return true ; }		
		virtual bool supportScaling(void) const { return true ; }
		virtual void setMatrix(uint32 index, const CMatrix &m) ;	
		virtual CMatrix getMatrix(uint32 index) const ;
		virtual void setScale(uint32 index, float scale) ;	
		virtual void setScale(uint32 index, const CVector &s) ;		
		virtual CVector getScale(uint32 index) const ;

	protected:

		
		CPSAttrib<CPlaneBasis> _Basis ;				

		//  width
		TPSAttribFloat _Width ;

		//  Height
		TPSAttribFloat _Height ;


		CMatrix buildBasis(uint32 index) const ;

		virtual void resize(uint32 size) ;

		virtual void newElement(void) ;

		virtual void deleteElement(uint32 index) ;

} ;

 


} // NL3D


#endif // NL_PS_ZONE_H

/* End of ps_zone.h */
