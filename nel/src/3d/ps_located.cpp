/** \file particle_system_located.cpp
 * <File description>
 *
 * $Id: ps_located.cpp,v 1.24 2001/07/24 15:16:00 vizerie Exp $
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



#include <algorithm>
#include "nel/misc/aabbox.h"
#include "nel/misc/matrix.h"
#include "3d/ps_util.h"
#include "3d/particle_system.h"
#include "3d/ps_zone.h"
#include "3d/driver.h"
#include "3d/material.h"
#include "3d/dru.h"
#include "3d/ps_located.h"
#include "3d/ps_particle.h"

#include "nel/misc/line.h"
#include "nel/misc/cpu_info.h"

namespace NL3D {




/**
 * Constructor
 */
	CPSLocated::CPSLocated() : _LastForever(true)
						 , _InitialLife(1.f), _LifeScheme(NULL)
						 , _InitialMass(1.f), _MassScheme(NULL)
						 , _Size(0), _MaxSize(DefaultMaxLocatedInstance)
						 , _UpdateLock(false)
						 , _CollisionInfo(NULL), _CollisionInfoNbRef(0)						 
						 , _NbFramesToSkip(0)
						 , _Name(std::string("located"))
						 , _LODDegradation(false)
						 , _MaxNumFaces(0)
{		
}



void CPSLocated::notifyMaxNumFacesChanged(void)
{

	// we examine wether we have particle attached to us, and ask for the max number of faces they may want
	_MaxNumFaces  = 0 ;
	for (TLocatedBoundCont::const_iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		if ((*it)->getType() == PSParticle)
		{
			_MaxNumFaces += ((CPSParticle *) (*it))->getMaxNumFaces() ;
		}
	}	


	if (_Owner)
	{
		_Owner->notifyMaxNumFacesChanged() ;
	}
}


uint CPSLocated::querryMaxWantedNumFaces(void)
{
	return _MaxNumFaces ;
}


/// tells wether there are alive entities / particles in the system
bool CPSLocated::hasParticles(void) const
{
	for (TLocatedBoundCont::const_iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		if ((*it)->getType() == PSParticle && (*it)->hasParticles()) return true ;
	}
	return false ;
}

/// tells wether there are alive emitters
bool CPSLocated::hasEmitters(void) const
{
	for (TLocatedBoundCont::const_iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		if ((*it)->getType() == PSEmitter && (*it)->hasEmitters()) return true ;
	}
	return false ;
}


void CPSLocated::getLODVect(NLMISC::CVector &v, float &offset, bool systemBasis)
{
	nlassert(_Owner) ;
	_Owner->getLODVect(v, offset, systemBasis) ;
}



float CPSLocated::getUserParam(uint numParam) const
{
	nlassert(_Owner) ;
	return _Owner->getUserParam(numParam) ;
}

CScene *CPSLocated::getScene(void)
{
	nlassert(_Owner) ;
	return _Owner->getScene() ;
}


void CPSLocated::incrementNbDrawnParticles(uint num)
{
	CParticleSystem::_NbParticlesDrawn += num ; // for benchmark purpose	
}

void CPSLocated::setInitialLife(CAnimationTime lifeTime)
{
	_LastForever = false ;
	_InitialLife = lifeTime ;
	delete _LifeScheme ;
	_LifeScheme = NULL ;	

}
void CPSLocated::setLifeScheme(CPSAttribMaker<float> *scheme)
{
	nlassert(scheme) ;
	nlassert(!scheme->hasMemory()) ; // scheme with memory is invalid there !!
	_LastForever = false ;
	delete _LifeScheme ;
	_LifeScheme = scheme ;
}
void CPSLocated::setInitialMass(float mass)
{
	_InitialMass = mass ;
	delete _MassScheme ;
	_MassScheme = NULL ;	
}
void CPSLocated::setMassScheme(CPSAttribMaker<float> *scheme)
{
	nlassert(scheme) ;
	nlassert(!scheme->hasMemory()) ; // scheme with memory is invalid there !!
	delete _MassScheme ;
	_MassScheme = scheme ;	
}
	


/// get a matrix that helps to express located B coordinate in located A basis
const NLMISC::CMatrix &CPSLocated::getConversionMatrix(const CPSLocated *A, const CPSLocated *B)
{
	nlassert(A->_Owner == B->_Owner) ; // conversion must be made between entity of the same system
	if (A->_SystemBasisEnabled == B->_SystemBasisEnabled)
	{
		return NLMISC::CMatrix::Identity ;
	}
	else
	{
		if (B->_SystemBasisEnabled)
		{
			return B->_Owner->getSysMat() ;
		}
		else
		{
			return A->_Owner->getInvertedSysMat() ;
		}


	}
}


NLMISC::CVector CPSLocated::computeI(void) const 
{
	if (!_SystemBasisEnabled)
	{
		return _Owner->getInvertedViewMat().getI() ;
	}
	else
	{
		// we must express the I vector in the system basis, so we need to multiply it by the inverted matrix of the system
		return _Owner->getInvertedSysMat().mulVector(_Owner->getInvertedViewMat().getI()) ;
	}
}


NLMISC::CVector CPSLocated::computeJ(void) const 
{
	if (!_SystemBasisEnabled)
	{
		return _Owner->getInvertedViewMat().getJ() ;
	}
	else
	{
		// we must express the J vector in the system basis, so we need to multiply it by the inverted matrix of the system
		return _Owner->getInvertedSysMat().mulVector(_Owner->getInvertedViewMat().getJ()) ;
	}
}



NLMISC::CVector CPSLocated::computeK(void) const
{
	if (!_SystemBasisEnabled)
	{
		return _Owner->getInvertedViewMat().getK() ;
	}
	else
	{
		// we must express the K vector in the system basis, so we need to multiply it by the inverted matrix of the system
		return _Owner->getInvertedSysMat().mulVector(_Owner->getInvertedViewMat().getK()) ;
	}
}



IDriver *CPSLocated::getDriver() const 
{ 
	nlassert(_Owner) ;
	nlassert (_Owner->getDriver() ) ; // you haven't called setDriver on the system
	return _Owner->getDriver() ;
}

/// dtor

CPSLocated::~CPSLocated()
{
	// we must do a copy, because the subsequent call can modify this vector
	TDtorObserversVect copyVect(_DtorObserversVect.begin(), _DtorObserversVect.end()) ;
	// call all the dtor observers
	for (TDtorObserversVect::iterator it = copyVect.begin() ; it != copyVect.end() ; ++it)
	{
		(*it)->notifyTargetRemoved(this) ;
	}

	nlassert(_CollisionInfoNbRef == 0) ; //If this is not = 0, then someone didnt call releaseCollisionInfo
										 // If this happen, you can register with the registerDTorObserver
										 // (observer pattern)
										 // and override notifyTargetRemove to call releaseCollisionInfo
	nlassert(!_CollisionInfo) ;

	// delete all bindable

	for (TLocatedBoundCont::iterator it2 = _LocatedBoundCont.begin() ; it2 != _LocatedBoundCont.end() ; ++it2)
	{
		delete *it2 ;
	}


	delete _LifeScheme ;
	delete _MassScheme ;
}



/**
* sorted insertion  (by decreasing priority order) of a bindable (particle e.g an aspect, emitter) in a located
*/
void CPSLocated::bind(CPSLocatedBindable *lb)
{
	nlassert(std::find(_LocatedBoundCont.begin(), _LocatedBoundCont.end(), lb) == _LocatedBoundCont.end()) ;	
	TLocatedBoundCont::iterator it = _LocatedBoundCont.begin() ;
	while (it != _LocatedBoundCont.end() && **it < *lb) // the "<" operator sort them correctly
	{
		++it ;
	}

	_LocatedBoundCont.insert(it, lb) ;
	lb->setOwner(this) ;
	lb->resize(_MaxSize) ;

	// any located bindable that is bound to us should have no element in it for now !!
	// we reisze the boundable, so that it has ne same number of elements as us

	uint32 initialSize  = _Size ;
	for (uint32 k = 0 ; k < initialSize ; ++k)
	{
		_Size = k ;
		lb->newElement(NULL, 0) ;
	}
	_Size = initialSize ;

	/// the max number of shapes may have changed
	notifyMaxNumFacesChanged() ;
}



void CPSLocated::remove(const CPSLocatedBindable *p)
{
	TLocatedBoundCont::iterator it = std::find(_LocatedBoundCont.begin(), _LocatedBoundCont.end(), p) ;
	nlassert(it != _LocatedBoundCont.end()) ;	
	delete *it ;
	_LocatedBoundCont.erase(it) ;
}


void CPSLocated::registerDtorObserver(CPSLocatedBindable *anObserver)
{
	// check wether the observer wasn't registered twice
	nlassert(std::find(_DtorObserversVect.begin(), _DtorObserversVect.end(), anObserver) == _DtorObserversVect.end()) ;
	_DtorObserversVect.push_back(anObserver) ;
}

void CPSLocated::unregisterDtorObserver(CPSLocatedBindable *anObserver)
{
	// check that it was registered
	TDtorObserversVect::iterator it = std::find(_DtorObserversVect.begin(), _DtorObserversVect.end(), anObserver) ;
	nlassert(it != _DtorObserversVect.end()) ;
	_DtorObserversVect.erase(it) ;
}





/**
 * new element generation
 */

sint32 CPSLocated::newElement(const CVector &pos, const CVector &speed, CPSLocated *emitter, uint32 indexInEmitter)
{	
	if (_UpdateLock)
	{
		postNewElement(pos, speed) ;
	}
	

	if (_CollisionInfo)
	{
		_CollisionInfo->insert() ;
	}

	sint32 creationIndex ;

	// get the convertion matrix  from the emitter basis to the emittee basis
	// if the emitter is null, we assume that the coordinate are given in the chosen basis for this particle type

	
	
	if (_MaxSize == _Size) return -1 ;

	const CMatrix &convMat = emitter ? CPSLocated::getConversionMatrix(this, emitter) 
							:  CMatrix::Identity ;
	

	creationIndex  =_Pos.insert(convMat * pos) ;
	nlassert(creationIndex != -1) ; // all attributs must contains the same number of elements
	_Speed.insert(convMat.mulVector(speed)) ;
			
	_InvMass.insert(1.f / ((_MassScheme && emitter) ? _MassScheme->get(emitter, indexInEmitter) : _InitialMass ) ) ;
	_Time.insert(0.0f) ;	
	_TimeIncrement.insert( 1.f / ((_LifeScheme && emitter) ?  _LifeScheme->get(emitter, indexInEmitter) : _InitialLife ) ) ;

	// generate datas for all bound objects
	
	
	_UpdateLock = true ;	

	
	for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin() ; it != _LocatedBoundCont.end() ; ++it)
	{
		(*it)->newElement(emitter, indexInEmitter) ;
	}

	
	_UpdateLock = false ;
	

	++_Size ;	// if this is modified, you must also modify the getNewElementIndex in this class
				// because that method give the index of the element being created for overrider of the newElement method
				// of the CPSLocatedClass (which is called just above)

	return creationIndex ;
}

void CPSLocated::postNewElement(const CVector &pos, const CVector &speed)
{
	_RequestStack.push(CPostNewElementRequestInfo(pos, speed)) ;
}


/**
 * delete an element
 */

void CPSLocated::deleteElement(uint32 index)
{
	nlassert(index < _Size) ;
	
	// delete all bindable before : they may need our coordinate
	// to perform a destruction task
	
	_UpdateLock = true ;
	

	for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin() ; it != _LocatedBoundCont.end() ; ++it)
	{
		(*it)->deleteElement(index) ;
	}	

	
	_UpdateLock = false ;	

	// remove common located's attributes

	_InvMass.remove(index) ;
	_Pos.remove(index) ;
	_Speed.remove(index) ;
	_Time.remove(index) ;
	_TimeIncrement.remove(index) ;

	if (_CollisionInfo)
	{
		_CollisionInfo->remove(index) ;
	}

	--_Size ;
}


/// Resize the located container

void CPSLocated::resize(uint32 newSize)
{

	if (newSize < _Size)
	{
		for (uint32 k = _Size - 1 ; k >= newSize ; --k)
		{
			deleteElement(k) ;
			
			if (k == 0) break ; // we're dealing with unsigned quantities
		}
		_Size = newSize ;
	}


	_MaxSize = newSize ;
	_InvMass.resize(newSize) ;
	_Pos.resize(newSize) ;
	_Speed.resize(newSize) ;
	_Time.resize(newSize) ;
	_TimeIncrement.resize(newSize) ;

	if (_CollisionInfo)
	{
		_CollisionInfo->resizeNFill(newSize) ;
	}


	

	// resize attributes for all bound objects
	for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin() ; it != _LocatedBoundCont.end() ; ++it)
	{
		(*it)->resize(newSize) ;
	}


	/// compute the new max number of faces
	notifyMaxNumFacesChanged() ;
}


void CPSLocated::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion(2) ;
	CParticleSystemProcess::serial(f) ;
	
	f.serial(_Name) ;

	f.serial(_InvMass) ;
	f.serial(_Pos) ;
	f.serial(_Speed) ;
	f.serial(_Time) ;
	f.serial(_TimeIncrement) ;

	f.serial(_Size) ; 
	f.serial(_MaxSize) ;

	f.serial(_LastForever) ;

	f.serialPtr(_CollisionInfo) ;
	f.serial(_CollisionInfoNbRef) ;


	if (f.isReading())
	{
		delete _LifeScheme ;
		delete _MassScheme ;

		bool useScheme ;
		f.serial(useScheme) ;
		if (useScheme)
		{
			f.serialPolyPtr(_LifeScheme) ;
		}
		else
		{
			f.serial(_InitialLife) ;
			_LifeScheme = NULL ;
		}

		f.serial(useScheme) ;
		if (useScheme)
		{
			f.serialPolyPtr(_MassScheme) ;
		}
		else
		{
			f.serial(_InitialMass) ;
			_MassScheme = NULL ;
		}
	}
	else
	{
		bool bFalse = false, bTrue = true ;
		if (_LifeScheme)
		{
			f.serial(bTrue) ;
			f.serialPolyPtr(_LifeScheme) ;
		}
		else
		{
			f.serial(bFalse) ;
			f.serial(_InitialLife) ;
		}
		if (_MassScheme)
		{
			f.serial(bTrue) ;
			f.serialPolyPtr(_MassScheme) ;
		}
		else
		{
			f.serial(bFalse) ;
			f.serial(_InitialMass) ;
		}
	}

	f.serial(_NbFramesToSkip) ;

	f.serialContPolyPtr(_DtorObserversVect) ;

	if (f.isReading())
	{
		while(!_RequestStack.empty())
		{
			_RequestStack.pop() ;
		}
		uint32 size ;
		f.serial(size) ;
		for (uint32 k = 0 ; k < size ; ++k)
		{
			TNewElementRequestStack::value_type t ;
			f.serial(t) ;
			_RequestStack.push(t) ;
		}
	}
	else
	{
		// when writing the stack, we must make a copy because we can't access elements by their index
		// so the stack must be destroyed
		TNewElementRequestStack r2 ;
		uint32 size = (uint32) _RequestStack.size() ;
		f.serial(size) ;

		while(!_RequestStack.empty())
		{
			r2.push(_RequestStack.top()) ;
			_RequestStack.pop() ;
		}
		// now rebuild the stack while serializing it ;
		while (!r2.empty())
		{			
			f.serial(r2.top()) ;
			_RequestStack.push(r2.top()) ;
			r2.pop() ;
		}

	}

	
	f.serial(_UpdateLock) ;	
	f.serialContPolyPtr(_LocatedBoundCont) ;	

	if (ver > 1)
	{
		f.serial(_LODDegradation) ;
	}



	if (f.isReading())
	{
		// evaluate our max number of faces
		notifyMaxNumFacesChanged() ;
	}
}


// integrate speed of particles. Makes eventually use of SSE instructions when present
static void IntegrateSpeed(uint count, float *src1, const float *src2 ,float ellapsedTime)
{
	#if 0
		#ifdef NL_OS_WINDOWS



		if (NLMISC::CCpuInfo::hasSSE()
			&& ((uint) src1 & 15) == ((uint) src2 & 15)
			&& ! ((uint) src1 & 3)
			&& ! ((uint) src2 & 3)				
		   )   // must must be sure that memory alignment is identical	   
		{

			// compute first datas in order to align to 16 byte boudary

			uint alignCount =  ((uint) src1 >> 2) & 3 ; // number of float to processed

			while (alignCount --)
			{
				*src1++ += ellapsedTime * *src2 ++ ;
			}



			count -= alignCount ;
			if (count > 3)
			{
				float et[4] = { ellapsedTime, ellapsedTime, ellapsedTime, ellapsedTime} ;
				// sse part of computation
				__asm
				{
						mov  ecx, count
						shr  ecx, 2
						

						xor   edx, edx

						mov    eax, src1			
						mov    ebx, src2			
						movups  xmm0, et[0]
					myLoop:
						movaps xmm2, [ebx+8*edx]
						movaps xmm1, [eax+8*edx]
						mulps  xmm2, xmm0			
						addps  xmm1, xmm2
						movaps [eax+8*edx], xmm1			
						add edx, 2					
						dec ecx
						jne myLoop				
				}
			}
			// proceed with left float
			count &= 3 ;

			if (count)
			{
				src1 += alignCount ;	
				src2 += alignCount ;
				do
				{
					*src1 += ellapsedTime * *src2 ;

					++src1 ;
					++src2 ;
				}
				while (--count) ;
			}

		}
		else
		#endif
	#endif
	{
		// standard version	
		
		for (float *src1End = src1 + count; src1 != src1End ; ++src1, ++src2)
		{				
			*src1 += ellapsedTime * *src2 ;			
		} 


	}
}


void CPSLocated::step(TPSProcessPass pass, CAnimationTime ellapsedTime)
{
	if (!_Size) return ;	

	if (pass == PSMotion)
	{		
		
		// check wether we must perform LOD degradation
		if (_LODDegradation)
		{
			if (ellapsedTime > 0)
			{
				nlassert(_Owner) ;
				// compute the number of particles to show
				const uint maxToHave = (uint) (_MaxSize * _Owner->getOneMinusCurrentLODRatio()) ;
				if (_Size > maxToHave) // too much instances ?
				{
					// choose a random element to start at, and a random step
					// this will avoid a pulse effect when the system is far away

					
					uint pos = maxToHave ? rand() % maxToHave : 0 ;
					uint step  = maxToHave ? rand() % maxToHave : 0 ;

					do
					{
						deleteElement(pos) ;
						pos += step ;
						if (pos >= maxToHave) pos -= maxToHave ;
					}
					while (_Size !=maxToHave) ;				
				}
			}
		}

		// check if we must skip frames
		if (!_NbFramesToSkip || !( (uint32) _Owner->getDate() % (_NbFramesToSkip + 1)))
		{

			// update the located creation requests that may have been posted
			updateNewElementRequestStack() ;

		

			// there are 2 integration steps : with and without collisions

			if (!_CollisionInfo) // no collisionner are used
			{		
				IntegrateSpeed(_Size * 3, &_Pos[0].x, &_Speed[0].x, ellapsedTime) ;
			}
			else
			{
				// integration with collisions

				nlassert(_CollisionInfo) ;
				TPSAttribCollisionInfo::const_iterator itc = _CollisionInfo->begin() ;
				TPSAttribVector::iterator itSpeed = _Speed.begin() ;		
				TPSAttribVector::iterator itPos = _Pos.begin() ;		

				for (uint k = 0 ; k < _Size ;)
				{
					if (itc->dist != -1)
					{
						(*itPos) = itc->newPos ;
						(*itSpeed) = itc->newSpeed ;

						// notify each located bindable that a bounce occured ...
						for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
						{	
							(*it)->bounceOccured(k) ;
						}

						switch(itc->collisionZone->getCollisionBehaviour())
						{
							case CPSZone::bounce:
								++k, ++itPos, ++itSpeed, ++itc ;
							break ;
							case CPSZone::destroy:
								deleteElement(k) ;
							break ;
						}


					}
					else
					{
						(*itPos) += ellapsedTime * (*itSpeed) ;
						++k, ++itPos, ++itSpeed, ++itc ;
					}
				}

				
				// reset collision info for the next time

				resetCollisionInfo() ;
				
			}


			if (! _LastForever)
			{
				TPSAttribTime::iterator itTime = _Time.begin(), itTimeInc = _TimeIncrement.begin() ;
				for (uint32 k = 0 ; k < _Size ;)
				{
					*itTime += ellapsedTime * *itTimeInc ;
					if (*itTime >= 1.0f)
					{
						deleteElement(k) ;
					}
					else
					{
						++k ;
						++itTime ;
						++itTimeInc ;
					}
				}
			}
			else
			{
				// the time attribute gives the life in seconds
				TPSAttribTime::iterator itTime = _Time.begin(), endItTime = _Time.end() ;
				for (; itTime != endItTime ; ++itTime)
				{
					*itTime += ellapsedTime ;
				}
			}
		}
		else
		{
			return ; // we skip the frame...
		}
	}

	// apply the pass to all bound objects
	for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		if ((*it)->getLOD() == PSLod1n2 || _Owner->getLOD() == (*it)->getLOD()) // has this object the right LOD ?
		{
			(*it)->step(pass, ellapsedTime) ;
		}
	}
}




void CPSLocated::updateNewElementRequestStack(void)
{
	while (!_RequestStack.empty())
	{
		newElement(_RequestStack.top()._Pos, _RequestStack.top()._Speed) ;
		_RequestStack.pop() ;
	}
}


bool CPSLocated::computeBBox(NLMISC::CAABBox &box) const
{
	if (!_Size) return false ; // something to compute ?


	TLocatedBoundCont::const_iterator it ;
	TPSAttribVector::const_iterator it2 ;

	// check whether any object bound to us need a bbox

	for (it = _LocatedBoundCont.begin() ; it != _LocatedBoundCont.end() ; ++it)
	{
		if ((*it)->doesProduceBBox()) 
		{
			break ;
		}
	}

	if (it == _LocatedBoundCont.end()) 
	{
		return false ;
	}

	CVector min = _Pos[0], max = _Pos[0] ;

	for (it2 = _Pos.begin() ; it2 != _Pos.end() ; ++ it2)
	{
		const CVector &v = (*it2) ;
		min.minof(min, v) ;	
		max.maxof(max, v) ;		
	}

	box.setMinMax(min, max) ;	

	// we've considered that located had no extent in space
	// now, we must call each objects that are bound to the located in order
	// to complete the bbox if they have no null extent

	NLMISC::CAABBox tmpBox, startBox = box  ;

	for (it = _LocatedBoundCont.begin() ; it != _LocatedBoundCont.end() ; ++it)
	{
		if ((*it)->doesProduceBBox())
		{
			tmpBox = startBox ;
			if ((*it)->completeBBox(tmpBox))
			{
				box = CPSUtil::computeAABBoxUnion(tmpBox, box) ;
			}
		}
	}

	return true ;
}


/// Setup the driver model matrix. It is set accordingly to the basis used for rendering	
void CPSLocated::setupDriverModelMatrix(void) 
{
	if (_SystemBasisEnabled)
	{
		getDriver()->setupModelMatrix(_Owner->getSysMat()) ;		
	}
	else
	{
		getDriver()->setupModelMatrix(CMatrix::Identity) ;
	}
}




void CPSLocated::queryCollisionInfo(void)
{
	if (_CollisionInfoNbRef)
	{
		++ _CollisionInfoNbRef ;
	}
	else
	{
		_CollisionInfo = new TPSAttribCollisionInfo ;
		_CollisionInfoNbRef = 1 ;
		_CollisionInfo->resizeNFill(_MaxSize) ;
		resetCollisionInfo() ;
	}
}

void CPSLocated::releaseCollisionInfo(void)
{
	nlassert(_CollisionInfoNbRef) ; // check whether queryCollisionInfo was called
									// so the number of refs must not = 0									
    --_CollisionInfoNbRef ;
	if (_CollisionInfoNbRef == 0)
	{
		delete _CollisionInfo ;
		_CollisionInfo = NULL ;
	}
}


void CPSLocated::resetCollisionInfo(void)
{
	nlassert(_CollisionInfo) ;

	TPSAttribCollisionInfo::iterator it = _CollisionInfo->begin(), endIt = _CollisionInfo->end() ;

	for (; it != endIt ; ++it)
	{
		it->reset() ;
	}
}

///////////////////////////////////////
// CPSLocatedBindable implementation //
///////////////////////////////////////


CPSLocatedBindable::CPSLocatedBindable() : _Owner(NULL), _LOD(PSLod1n2)
{
	_Owner = NULL ;
}

const NLMISC::CMatrix &CPSLocatedBindable::getLocatedMat(void) const
{
	nlassert(_Owner) ;
	if (_Owner->isInSystemBasis())
	{
		return _Owner->getOwner()->getSysMat() ;
	}
	else
	{
		return NLMISC::CMatrix::Identity ;
	}
}


void CPSLocatedBindable::notifyTargetRemoved(CPSLocated *ptr)
{
	ptr->unregisterDtorObserver(this) ;
}

void CPSLocatedBindable::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion(2) ;
	f.serialPtr(_Owner) ;
	if (ver > 1) f.serialEnum(_LOD) ;
}

void CPSLocatedBindable::displayIcon2d(const CVector tab[], uint nbSegs, float scale)
{
	uint32 size = _Owner->getSize() ;
	if (!size) return ;		
	setupDriverModelMatrix() ;	

	const CVector I = computeI() ;
	const CVector K = computeK() ;

	static std::vector<NLMISC::CLine> lines ;
	
	lines.clear() ;

	// ugly slow code, but not for runtime
	for (uint  k = 0 ; k < size ; ++k)
	{
		// center of the current particle
		const CVector p = _Owner->getPos()[k]  ;
		
		

		for (uint l = 0 ; l < nbSegs ; ++l)
		{
			NLMISC::CLine li ;
			li.V0 = p + scale * (tab[l << 1].x * I + tab[l << 1].y * K) ;
			li.V1 = p + scale * (tab[(l << 1) + 1].x * I + tab[(l << 1) + 1].y * K) ;
			lines.push_back(li) ;
		}
	
		CMaterial mat ;

		mat.setBlendFunc(CMaterial::one, CMaterial::one) ;
		mat.setZWrite(false) ;
		mat.setLighting(false) ;
		mat.setBlend(true) ;
		mat.setZFunc(CMaterial::less) ;
		
	

		CPSLocated *loc ;
		uint32 index ;		
		CPSLocatedBindable *lb ;
		_Owner->getOwner()->getCurrentEditedElement(loc, index, lb) ;
	
		mat.setColor((lb == NULL || this == lb) && loc == _Owner && index == k  ? CRGBA::Red : CRGBA(127, 127, 127)) ;
		

		CDRU::drawLinesUnlit(lines, mat, *getDriver() ) ;
	}

}

CFontManager *CPSLocatedBindable::getFontManager(void)
{
	nlassert(_Owner) ;
	return _Owner->getFontManager() ;
}

 /// Shortcut to get the font manager if one was set (const version)
const CFontManager *CPSLocatedBindable::getFontManager(void) const
{
	nlassert(_Owner) ;
	return _Owner->getFontManager() ;
}


// Shortcut to get the matrix of the system	
 const NLMISC::CMatrix &CPSLocatedBindable::getSysMat(void) const 
{
	nlassert(_Owner) ;		
	return _Owner->getOwner()->getSysMat() ;
}

/// shortcut to get the inverted matrix of the system

const NLMISC::CMatrix &CPSLocatedBindable::getInvertedSysMat(void) const 
{
	nlassert(_Owner) ;
		return _Owner->getOwner()->getInvertedSysMat() ;

}

const NLMISC::CMatrix &CPSLocatedBindable::getInvertedLocatedMat(void) const
{
	nlassert(_Owner) ;
	if (_Owner->isInSystemBasis())
	{
		return _Owner->getOwner()->getInvertedSysMat() ;
	}
	else
	{
		return NLMISC::CMatrix::Identity ;
	}
}


/// shortcut to get the view matrix
const NLMISC::CMatrix &CPSLocatedBindable::getViewMat(void) const 
{
	nlassert(_Owner) ;
	return _Owner->getOwner()->getViewMat() ;	
}	

/// shortcut to get the inverted view matrix
const NLMISC::CMatrix &CPSLocatedBindable::getInvertedViewMat(void) const 
{
	nlassert(_Owner) ;
	return _Owner->getOwner()->getInvertedViewMat() ;	
}	

/// shortcut to setup the model matrix (system basis or world basis)
void CPSLocatedBindable::setupDriverModelMatrix(void)  
{
	nlassert(_Owner) ;
	_Owner->setupDriverModelMatrix() ;
}


/////////////////////////////////////////////
// CPSTargetLocatedBindable implementation //
/////////////////////////////////////////////

void CPSTargetLocatedBindable::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1) ;	
	f.serialPtr(_Owner) ;
	f.serial(_Name) ;
	if (f.isReading())
	{
		// delete previous attached bindables...
		for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
		{
			delete (*it) ;
		}
		_Targets.clear() ;		
	}
	f.serialContPolyPtr(_Targets) ;
}


void CPSTargetLocatedBindable::attachTarget(CPSLocated *ptr)
{

	// a target can't be shared between different particle systems
	#ifdef NL_DEBUG
	if (_Owner)
	{
		nlassert(_Owner->getOwner() == ptr->getOwner()) ;
	}
	#endif

	// see wether this target has not been registered before 
	nlassert(std::find(_Targets.begin(), _Targets.end(), ptr) == _Targets.end()) ;
	_Targets.push_back(ptr) ;

	// we register us to be notified when the target disappear
	ptr->registerDtorObserver(this) ;
}


void CPSTargetLocatedBindable::notifyTargetRemoved(CPSLocated *ptr) 
{	
	TTargetCont::iterator it = std::find(_Targets.begin(), _Targets.end(), ptr) ;
	nlassert(it != _Targets.end()) ;
	releaseTargetRsc(*it) ;	
	_Targets.erase(it) ;

	CPSLocatedBindable::notifyTargetRemoved(ptr) ;	
}


CPSTargetLocatedBindable::~CPSTargetLocatedBindable()
{
	// we unregister to all the targets
	for (TTargetCont::iterator it = _Targets.begin() ; it != _Targets.end() ; ++it)
	{		
		(*it)->unregisterDtorObserver(this) ;
	}
}




} // NL3D
