/** \file ps_force.h
 * <File description>
 *
 * $Id: ps_force.h,v 1.5 2001/07/13 17:00:49 vizerie Exp $
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

#ifndef NL_PS_FORCE_H
#define NL_PS_FORCE_H

#include "nel/misc/types_nl.h"
#include "3d/ps_located.h"
#include "3d/ps_util.h"
#include "3d/ps_attrib_maker.h"
#include "3d/ps_edit.h"
#include "3d/ps_direction.h"


namespace NL3D {



/** This struct describe where 
 * 


/**
 * All forces in the system derive from this class
 * It all a list all located on which the force can apply
 * Only the motion and toolRender passes are supported for a force
 * The corresponding virtual calls are done
 * Not sharable.
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSForce : public CPSTargetLocatedBindable
{
public:

	/// Constructor
	CPSForce();
	

	/// return this bindable type
	uint32 getType(void) const { return PSForce ; }


	/// return priority for forces

	virtual uint32 getPriority(void) const { return 4000 ; }

	/// Override of CPSLocatedBindable::doesProduceBBox. forces usually are not part of the bbox
	virtual bool doesProduceBBox(void) const { return false ; }

	/**
	 * process one pass for the force 
	 */
	virtual void step(TPSProcessPass pass, CAnimationTime ellapsedTime) ;

	
	/// Compute the force on the targets
	virtual void performMotion(CAnimationTime ellapsedTime) = 0 ;

	/// Show the force (edition mode)
	virtual void show(CAnimationTime ellapsedTime)  = 0 ;

	/// Serial the force definition. MUST be called by deriver during their serialisation
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;

protected:

	virtual void newElement(CPSLocated *emitterLocated, uint32 emitterIndex) = 0 ;
	
	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable are still accessible for of the index given
	 *  index out of range -> nl_assert
	 */
	virtual void deleteElement(uint32 index) = 0 ;

	/** Resize the bindable attributes containers DERIVERS SHOULD CALL THEIR PARENT VERSION
	 * should not be called directly. Call CPSLOcated::resize instead
	 */
	virtual void resize(uint32 size) = 0 ;

	
};


/// this is a class to set force instensity (acceleration for gravity, k coefficient for springs...)
class CPSForceIntensity
{
public:

	// ctor
	CPSForceIntensity() : _IntensityScheme(NULL)
	{
	}

	~CPSForceIntensity() ;
	

	/// get the constant intensity that was set for the force
	float getIntensity(void) const  { return _K ; }

	/// set a constant intensity for the force. this discrad any previous call to setIntensityScheme
	void setIntensity(float value) ;

	/// set a non-constant intensity
	void setIntensityScheme(CPSAttribMaker<float> *scheme) ;

	// deriver have here the opportunity to setup the functor object. The default does nothing
	virtual void setupFunctor(uint32 indexInLocated) { }

	/// get the attribute maker for a non constant intensity
	CPSAttribMaker<float> *getIntensityScheme(void) { return _IntensityScheme ; }
	const CPSAttribMaker<float> *getIntensityScheme(void) const { return _IntensityScheme ; }
	void serialForceIntensity(NLMISC::IStream &f) throw(NLMISC::EStream) ;

protected:

	/// deriver must return the located that own them here
	virtual CPSLocated *getForceIntensityOwner(void) = 0 ;


	// the intensity ...
	float _K ;
	CPSAttribMaker<float> *_IntensityScheme ;

	void newForceIntensityElement(CPSLocated *emitterLocated, uint32 emitterIndex)
	{
		if (_IntensityScheme && _IntensityScheme->hasMemory()) _IntensityScheme->newElement(emitterLocated, emitterIndex) ;
	}	
	void deleteForceIntensityElement(uint32 index)
	{
		if (_IntensityScheme && _IntensityScheme->hasMemory()) _IntensityScheme->deleteElement(index) ;
	}
	void resizeForceIntensity(uint32 size)
	{
		if (_IntensityScheme && _IntensityScheme->hasMemory()) _IntensityScheme->resize(size, getForceIntensityOwner()->getSize()) ;
	}
} ;


/**
  * this class defines the newElement, deleteElement, and resize method of a class that derives from CPSForceIntensity
  * And that don't add per paerticle attribute
  */
class CPSForceIntensityHelper : public CPSForce, public CPSForceIntensity
{
public:
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) 
	{
		f.serialVersion(1) ;
		CPSForce::serial(f) ;
		serialForceIntensity(f) ;
	}
protected:
	virtual CPSLocated *getForceIntensityOwner(void) { return _Owner ; }
	virtual void newElement(CPSLocated *emitterLocated, uint32 emitterIndex) { newForceIntensityElement(emitterLocated, emitterIndex) ; }
	virtual void deleteElement(uint32 index) { deleteForceIntensityElement(index) ; }
	virtual void resize(uint32 size) { resizeForceIntensity(size) ; }
	
} ;




/** a helper class to create isotropic force : they are independant of the basis, and have no position 
 *  (fluid friction for example)
 *  To use this class you should provide to it a functor class that define the () operator with 3 parameters
 *  param1 = a const reference to the position of the particle
 *  param2 = a reference to the position, that must be updated
 *  param3 =  a float giving the inverse of the mass
 *  param4 = the ellapsed time, in second (has the CAnimationTime type).
 *  Example of use :
 *  class MyForceFunctor
 *  {
 *    public:
 *      /// it is strongly recommended to have your operator inlined
 *      void operator() (const NLMISC::CVector &pos, NLMISC::CVector &speed, float invMass , CanimationTime ellapsedTime)
 *      {
 *			// perform the speed update there
 *		}
 *		
 *      // you can provide a serialization method. Note that that if the functor parameters are set before each use,
 *      // it useless to serial something ...
 *		void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
 *
 *    protected:
 *      ...
 *  } ;
 *
 *
 * because of the serialization process, you must proceed like the following. (but you don't need to redefine serial, which
 * will serilize the functor object you passed for you
 *
 *	class MyForce : public CHomogenousForceT<MyForceFunctor>
 *  {
 *		public:
 *         MyForce() ;
 *		   NLMISC_DECLARE_CLASS(Myforce) ;
 *
 *      protected:
 *			...
 *
 *  } ;
 *
 *
 *  not that each functor may have its own parameter. the setupFunctor method will be called each time 
 */

template <class T> class CIsotropicForceT : public CPSForce
{
public: 

	/// Compute the force on the targets
	virtual void performMotion(CAnimationTime ellapsedTime) ;


	/// serialization
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialVersion(1) ;
		CPSForce::serial(f) ;
		f.serial(_F) ; // serial the functor object 5does nothing most of the time ...)
	}

	
	/** Show the force (edition mode). The default does nothing
	 *  TODO later
	 */

	 void show(CAnimationTime ellapsedTime)  {}


	 /// setup the functor object. The default does nothing

	 virtual void setupFunctor(uint32 index) {} ;

protected:
	
	/// the functor object
	T _F ;

	
		
	virtual void newElement(CPSLocated *emitterLocated, uint32 emitterIndex) { } ;		
	virtual void deleteElement(uint32 index) {} ;	
	virtual void resize(uint32 size) {} ;


} ;

//////////////////////////////////////////////////////////////////////
// implementation of method of the template class  CHomogenousForceT //
//////////////////////////////////////////////////////////////////////


template <class T> void CIsotropicForceT<T>::performMotion(CAnimationTime ellapsedTime)
{
	for (uint32 k = 0 ; k < _Owner->getSize() ; ++k)
	{	
		setupFunctor(k) ;
		for (TTargetCont::iterator it = _Targets.begin() ; it != _Targets.end() ; ++it)
		{			
			
			TPSAttribVector::iterator speedIt = (*it)->getSpeed().begin(), endSpeedIt = (*it)->getSpeed().end() ;
			TPSAttribVector::const_iterator posIt = (*it)->getPos().begin() ;
			TPSAttribFloat::const_iterator invMassIt = (*it)->getInvMass().begin() ;
			
			for (; speedIt != endSpeedIt ; ++speedIt, ++posIt, ++invMassIt)
			{
				_F(*posIt, *speedIt, *invMassIt, ellapsedTime) ;				
			}
		}
	}
}


/**
 *  a force that has the same direction everywhere. Mass is also taken in account (which is not the case for gravity)
 */

class CPSDirectionnalForce : public CPSForceIntensityHelper, public CPSDirection
{
	public:
	/// Compute the force on the targets
	virtual void performMotion(CAnimationTime ellapsedTime)  ;

	/// Show the force (edition mode)
	virtual void show(CAnimationTime ellapsedTime)  ;

	

	CPSDirectionnalForce(float i = 1.f) 
	{ 
		_Name = std::string("DirectionnalForce") ; 
		setIntensity(i); 
		_Dir = NLMISC::CVector(0, 0, -1) ;
	}

	/// serialization
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;


	NLMISC_DECLARE_CLASS(CPSDirectionnalForce) ; 

	/// set the direction of the force
	virtual void setDir(const NLMISC::CVector &dir) { _Dir = dir ; }

	/// get the direction of the force
	virtual NLMISC::CVector getDir(void) const  { return _Dir ; }

protected:

	NLMISC::CVector _Dir ;	
} ;







/// a gravity class. Mass isn't taken in account (true with a uniform gravity model, near earth )
class CPSGravity : public CPSForceIntensityHelper
{
public:
	/// Compute the force on the targets
	virtual void performMotion(CAnimationTime ellapsedTime)  ;

	/// Show the force (edition mode)
	virtual void show(CAnimationTime ellapsedTime)  ;

	

	CPSGravity(float g = 9.8f) 
	{ 
		_Name = std::string("Gravity") ; 
		setIntensity(g); 
	}

	/// serialization
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;


	NLMISC_DECLARE_CLASS(CPSGravity) ; 
} ;


/// a central gravity class. Mass is taken in account here
class CPSCentralGravity : public CPSForceIntensityHelper
{
public:
	/// Compute the force on the targets
	virtual void performMotion(CAnimationTime ellapsedTime)  ;

	/// Show the force (edition mode)
	virtual void show(CAnimationTime ellapsedTime)  ;

	

	CPSCentralGravity(float i = 1.f) 
	{ 
		_Name = std::string("CentralGravity") ; 
		setIntensity(i); 
	}

	/// serialization
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;


	NLMISC_DECLARE_CLASS(CPSCentralGravity) ; 
} ;


/// a spring class
class CPSSpring : public CPSForceIntensityHelper
{
public:
	
	/// ctor : k is the coefficient of the spring
	CPSSpring(float k = 1.0f)
	{ 
		_Name = std::string("Spring") ; 
		setIntensity(k) ;
	}


	/// serialization
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;


	/// Compute the force on the targets
	virtual void performMotion(CAnimationTime ellapsedTime)  ;

	/// Show the force (edition mode)
	virtual void show(CAnimationTime ellapsedTime)  ;


	NLMISC_DECLARE_CLASS(CPSSpring) ; 

} ;



/// a fluid friction functor, it is used by the fluid friction class
class CPSFluidFrictionFunctor
{
public:
	CPSFluidFrictionFunctor() : _K(1.f) 
	{		
	}

	#ifdef NL_OS_WINDOWS
		__forceinline
	#endif
	 void operator() (const NLMISC::CVector &pos, NLMISC::CVector &speed, float invMass , CAnimationTime ellapsedTime)
	 {
		speed -= (ellapsedTime * _K * invMass * speed)  ;
	 }

	 virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	 {
		 f.serialVersion(1) ;
		 // we don't save intensity info : it is saved by the owning object (and set before each use of this functor)
	 }

	 // get the friction coefficient
	 float getK(void) const { return _K ; }

	 // set the friction coefficient
	 void setK(float coeff) { _K = coeff ; }
protected:
	// the friction coeff
	float _K ;
} ;


/** the fluid friction force. We don't derive from CPSForceIntensityHelper (which derives from CPSForce
  * , because CIsotropicForceT also derives from CPSForce, and we don't want to use virtual inheritance
  */  


class CPSFluidFriction : public CIsotropicForceT<CPSFluidFrictionFunctor>, public CPSForceIntensity
{
public:
	// create the force with a friction coefficient
	CPSFluidFriction(float frictionCoeff = 1.f)
	{
		setIntensity(frictionCoeff) ;
		_Name = std::string("FluidFriction") ;
	}

	// inherited from CIsotropicForceT
	virtual void setupFunctor(uint32 index)
	{
		_F.setK(_IntensityScheme ? _IntensityScheme->get(_Owner, index) : _K) ;
	}
	
	NLMISC_DECLARE_CLASS(CPSFluidFriction)


	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialVersion(1) ;
		CIsotropicForceT<CPSFluidFrictionFunctor>::serial(f) ;
		serialForceIntensity(f) ;
	}
	

protected:
	virtual CPSLocated *getForceIntensityOwner(void) { return _Owner ; }
	virtual void newElement(CPSLocated *emitterLocated, uint32 emitterIndex) { newForceIntensityElement(emitterLocated, emitterIndex) ; }
	virtual void deleteElement(uint32 index) { deleteForceIntensityElement(index) ; }
	virtual void resize(uint32 size) { resizeForceIntensity(size) ; }
} ;


// a brownian force functor
class CPSBrownianFunctor
{
public:
	CPSBrownianFunctor() : _K(1.f) 
	{		
	}

	#ifdef NL_OS_WINDOWS
		__forceinline
	#endif
	 void operator() (const NLMISC::CVector &pos, NLMISC::CVector &speed, float invMass , CAnimationTime ellapsedTime)
	 {
		static double divRand = (2.f / RAND_MAX) ;
		NLMISC::CVector dir(float(rand() * divRand - 1), float(rand() * divRand - 1) , float(rand() * divRand - 1) ) ;
		speed += ellapsedTime * _K * invMass * dir ;
	 }

	 virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	 {
		 f.serialVersion(1) ;
		 // we don't save intensity info : it is saved by the owning object (and set before each use of this functor)
	 }

	 float getK(void) const { return _K ; }	 
	 void setK(float coeff) { _K = coeff ; }
protected:
	// the friction coeff
	float _K ;
} ;


/** the fluid friction force. We don't derive from CPSForceIntensityHelper (which derives from CPSForce
  * , because CIsotropicForceT also derives from CPSForce, and we don't want to use virtual inheritance
  */  


class CPSBrownianForce : public CIsotropicForceT<CPSBrownianFunctor>, public CPSForceIntensity
{
public:
	// create the force with a friction coefficient
	CPSBrownianForce(float intensity = 1.f)
	{
		setIntensity(intensity) ;
		_Name = std::string("BrownianForce") ;
	}

	// inherited from CIsotropicForceT
	virtual void setupFunctor(uint32 index)
	{
		_F.setK(_IntensityScheme ? _IntensityScheme->get(_Owner, index) : _K) ;
	}
	
	NLMISC_DECLARE_CLASS(CPSBrownianForce)

	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialVersion(1) ;
		CIsotropicForceT<CPSBrownianFunctor>::serial(f) ;
		serialForceIntensity(f) ;
	}

protected:
	virtual CPSLocated *getForceIntensityOwner(void) { return _Owner ; }
	virtual void newElement(CPSLocated *emitterLocated, uint32 emitterIndex) { newForceIntensityElement(emitterLocated, emitterIndex) ; }
	virtual void deleteElement(uint32 index) { deleteForceIntensityElement(index) ; }
	virtual void resize(uint32 size) { resizeForceIntensity(size) ; }
} ;


/// a turbulence force functor

struct CPSTurbulForceFunc
{	
	#ifdef NL_OS_WINDOWS
		__forceinline
	#endif
	void operator() (const NLMISC::CVector &pos, NLMISC::CVector &speed, float invMass , CAnimationTime ellapsedTime)
	{
		nlassert(0) ;

		// TODO : complete that

	/*	static const NLMISC::CVector v1(1.235f, - 45.32f, 157.5f) ;
		static const NLMISC::CVector v2(-0.35f, 7.77f, 220.77f) ;


		speed += ellapsedTime * _Intensity 
			   * NLMISC::CVector(2.f * (-0.5f + CPSUtil::buildPerlinNoise(_Scale * pos, _NumOctaves))
						 , 2.f * (-0.5f +  CPSUtil::buildPerlinNoise(_Scale * (pos +  v1) , _NumOctaves))
						 , 2.f * (-0.5f +  CPSUtil::buildPerlinNoise(_Scale * (pos +  v2) , _NumOctaves))
						 ) ;
						 */
	}

	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	 {
		f.serialVersion(1) ;
		f.serial(_Scale, _NumOctaves) ;
	 }

	float _Scale ;
	float _Intensity ;
	uint32 _NumOctaves ;
} ;



// the turbulence force

class CPSTurbul : public CIsotropicForceT<CPSTurbulForceFunc>, public CPSForceIntensity
{
public:
	// create the force with a friction coefficient
	CPSTurbul(float scale = 1.f , uint numOctaves = 4)
	{
		nlassert(numOctaves > 0) ;
		setScale(scale) ;	
		setNumOctaves(numOctaves) ;
		_Name = std::string("Turbulence") ;
	}

	
	float getScale(void) const { return _F._Scale ; }
	void setScale(float scale) { _F._Scale = scale ; } 

	
	uint getNumOctaves(void) const { return _F._NumOctaves ; }
	void setNumOctaves(uint numOctaves) { _F._NumOctaves = numOctaves ; } 


	NLMISC_DECLARE_CLASS(CPSTurbul)

	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialVersion(1) ;
		CIsotropicForceT<CPSTurbulForceFunc>::serial(f) ;
		serialForceIntensity(f) ;
	}

	// inherited from CIsotropicForceT
	virtual void setupFunctor(uint32 index)
	{
		_F._Intensity = (_IntensityScheme ? _IntensityScheme->get(_Owner, index) : _K) ;
	}

protected:
	virtual CPSLocated *getForceIntensityOwner(void) { return _Owner ; }
	virtual void newElement(CPSLocated *emitterLocated, uint32 emitterIndex) { newForceIntensityElement(emitterLocated, emitterIndex) ; }
	virtual void deleteElement(uint32 index) { deleteForceIntensityElement(index) ; }
	virtual void resize(uint32 size) { resizeForceIntensity(size) ; }
} ;




/** a cylindric vortex. It has a limited extend 
  * It has unlimited extension in the z direction
  * The model is aimed at tunability rather than realism
  */

class CPSCylindricVortex : public CPSForceIntensityHelper, public IPSMover
{
public:
	/// Compute the force on the targets
	virtual void performMotion(CAnimationTime ellapsedTime)  ;

	/// Show the force (edition mode)
	virtual void show(CAnimationTime ellapsedTime)  ;

	
	CPSCylindricVortex(float intensity = 1.f) : _RadialViscosity(.1f), _TangentialViscosity(.1f) 
	{
		setIntensity(intensity) ;
		_Name = std::string("Fluid friction") ; 
	}

	// inherited from IPSMover
	virtual bool supportUniformScaling(void) const { return true ; }
	virtual bool supportNonUniformScaling(void) const { return false ; }		
	virtual void setScale(uint32 k, float scale) { _Radius[k] = scale ; }
	virtual NLMISC::CVector getScale(uint32 k) const { return NLMISC::CVector(_Radius[k], _Radius[k], _Radius[k]) ; }
	virtual bool onlyStoreNormal(void) const { return true ; }	
	virtual NLMISC::CVector getNormal(uint32 index) { return _Normal[index] ; }	
	virtual void setNormal(uint32 index, NLMISC::CVector n) { _Normal[index] = n ; }

	virtual void setMatrix(uint32 index, const NLMISC::CMatrix &m) ;
	virtual NLMISC::CMatrix getMatrix(uint32 index) const ;
	
	
	void setRadialViscosity(float v) { _RadialViscosity = v ; }
	float getRadialViscosity(void) const { return _RadialViscosity ; }

	void setTangentialViscosity(float v) { _TangentialViscosity = v ; }
	float getTangentialViscosity(void) const { return _TangentialViscosity ; }

	NLMISC_DECLARE_CLASS(CPSCylindricVortex) ;



	// serialization
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;



	

protected:

	/// inherited from CPSForceIntensity
	virtual CPSLocated *getForceIntensityOwner(void) { return _Owner ; }

	// the normal of the vortex
	CPSAttrib<NLMISC::CVector> _Normal ;
	// radius of the vortex
	TPSAttribFloat _Radius ;

	// radial viscosity : when it is near of 1, if tends to set the radial componenent of speed to 0
	float _RadialViscosity ;

	// tangential viscosity : when set to 1, the tangential speed immediatly reach what it would be in a real vortex (w = 1 / r2)
	float _TangentialViscosity ;
	
	virtual void newElement(CPSLocated *emitterLocated, uint32 emitterIndex) ;
	virtual void deleteElement(uint32 index) ;
	virtual void resize(uint32 size) ;

} ;





} // NL3D


#endif // NL_PS_FORCE_H

/* End of ps_force.h */
