/** \file particle_system.cpp
 * <File description>
 *
 * $Id: particle_system.cpp,v 1.39 2001/11/23 18:48:56 vizerie Exp $
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

#include "3d/particle_system.h"
#include "3d/ps_located.h"
#include "3d/driver.h"
#include "3d/vertex_buffer.h"
#include "3d/material.h"
#include "3d/primitive_block.h"
#include "3d/nelu.h"
#include "3d/ps_util.h"
#include "3d/ps_particle.h"
#include "3d/ps_sound.h"
#include "3d/particle_system_shape.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/file.h"
#include "nel/misc/stream.h"




namespace NL3D 
{




uint32 CParticleSystem::NbParticlesDrawn = 0;
UPSSoundServer *		 CParticleSystem::_SoundServer = NULL;





///////////////////////////////////
// CPaticleSystem implementation //
///////////////////////////////////


/// the default max distance of view for particle systems
const float PSDefaultMaxViewDist = 300.f;

/*
 * Constructor
 */
CParticleSystem::CParticleSystem() : _FontGenerator(NULL), _FontManager(NULL)
									, _Date(0), _Scene(NULL), _CurrEditedElementLocated(NULL)
									, _CurrEditedElementIndex(0), _Driver(NULL)
									, _TimeThreshold(0.15f)
									, _MaxNbIntegrations(2)
									, _CanSlowDown(true)
									, _AccurateIntegration(true)
									, _InvMaxViewDist(1.f / PSDefaultMaxViewDist)									
									, _InvCurrentViewDist(1.f / PSDefaultMaxViewDist)									
									, _MaxViewDist(PSDefaultMaxViewDist)
									, _LODRatio(0.5f)
									, _ComputeBBox(true)
									, _BBoxTouched(true)
									, _DieCondition(none)
									, _DelayBeforeDieTest(0.2f) 
									, _DestroyModelWhenOutOfRange(false)
									, _DestroyWhenOutOfFrustum(false)
									, _SystemDate(0.f)
									, _OneMinusCurrentLODRatio(0)									
									, _MaxNumFacesWanted(0)
									, _PerformMotionWhenOutOfFrustum(true)
								
									
{
	for (uint k = 0; k < MaxPSUserParam; ++k) _UserParam[k] = 0;
}



/// immediatly shut down all the sound in this system
void CParticleSystem::stopSound()
{
	for (uint k = 0; k < this->getNbProcess(); ++k)
	{
		CPSLocated *psl = dynamic_cast<NL3D::CPSLocated *>(this->getProcess(k));
		if (psl)
		{
			for (uint l = 0; l < psl->getNbBoundObjects(); ++l)
			{
				if (psl->getBoundObject(l)->getType() == PSSound)
				{
					static_cast<CPSSound *>(psl->getBoundObject(l))->stopSound();

				}
			}
		}
	}	
}


void CParticleSystem::reactivateSound()
{
	for (uint k = 0; k < this->getNbProcess(); ++k)
	{
		CPSLocated *psl = dynamic_cast<NL3D::CPSLocated *>(this->getProcess(k));
		if (psl)
		{
			for (uint l = 0; l < psl->getNbBoundObjects(); ++l)
			{
				if (psl->getBoundObject(l)->getType() == NL3D::PSSound)
				{
					static_cast<CPSSound *>(psl->getBoundObject(l))->reactivateSound();
				}
			}
		}
	}
}


void CParticleSystem::notifyMaxNumFacesChanged(void)
{
	
	_MaxNumFacesWanted = 0;	
	for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{		
		_MaxNumFacesWanted += (*it)->querryMaxWantedNumFaces();
	}
}


float CParticleSystem::getWantedNumTris(float dist)
{
			 	 
	if (dist > _MaxViewDist) return 0;
	else return ((1.f - dist * _InvMaxViewDist) * _MaxNumFacesWanted);	
}


void CParticleSystem::setNumTris(uint numFaces)
{
	float modelDist = (_SysMat.getPos() - _InvertedViewMat.getPos()).norm();
	/*uint numFaceWanted = (uint) getWantedNumTris(modelDist);*/

	const float epsilon = 10E-5f;


	uint wantedNumTri = (uint) getWantedNumTris(modelDist);
	if (numFaces >= wantedNumTri || wantedNumTri == 0 || _MaxNumFacesWanted == 0 || modelDist < epsilon)
	{ 
		_InvCurrentViewDist = _InvMaxViewDist;
	}
	else
	{
		
		_InvCurrentViewDist = (_MaxNumFacesWanted - numFaces) / ( _MaxNumFacesWanted * modelDist);
	}
}


/// dtor
CParticleSystem::~CParticleSystem()
{
	for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		delete *it;
	}
}


void CParticleSystem::setViewMat(const NLMISC::CMatrix &m)
{
	_ViewMat = m;
	_InvertedViewMat = m.inverted();
}				




bool CParticleSystem::hasEmitters(void) const
{
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if ((*it)->hasEmitters()) return true;
	}
	return false;
}
	
bool CParticleSystem::hasParticles(void) const
{
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if ((*it)->hasParticles()) return true;
	}
	return false;
}




void CParticleSystem::stepLocated(TPSProcessPass pass, TAnimationTime et)
{
	if (pass == PSSolidRender || pass == PSBlendRender || pass == PSToolRender)
	{
		for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
		{
			(*it)->step(pass, et);
		}
	}
	else
	{
		for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
		{
			if (!(*it)->isParametricMotionEnabled()) (*it)->step(pass, et);
		}
	}
}

void CParticleSystem::step(TPass pass, TAnimationTime ellapsedTime)
{		
	switch (pass)
	{
		case SolidRender:
			++_Date; // update time		 
			stepLocated(PSSolidRender, ellapsedTime);
		break;
		case BlendRender:
			++_Date; // update time
			stepLocated(PSBlendRender, ellapsedTime);
		break;
		case ToolRender:
			stepLocated(PSToolRender, ellapsedTime);
		break;
		case Anim:
		{
		
			_BBoxTouched = true;
			TAnimationTime et = ellapsedTime;
			uint nbPass = 1;
			if (_AccurateIntegration)
			{
				if (et > _TimeThreshold)
				{
					nbPass = (uint32) ceilf(et / _TimeThreshold);
					if (nbPass > _MaxNbIntegrations)
					{ 
						nbPass = _MaxNbIntegrations;
						et = _CanSlowDown ? _TimeThreshold : (ellapsedTime / nbPass);
					}
					else
					{
						et = ellapsedTime / nbPass;
					}
				}			
			}

			// store the view matrix for lod computation, and further rendering pass
			// update current lod ratio
			const CVector d = _SysMat.getPos() - _ViewMat.getPos();		
			_OneMinusCurrentLODRatio = 1.f - (d.norm() * _InvCurrentViewDist);
			if (_OneMinusCurrentLODRatio < 0) _OneMinusCurrentLODRatio = 0.f;								

			// process passes
			do
			{
				stepLocated(PSEmit, et);
				stepLocated(PSCollision, et);
				stepLocated(PSMotion, et);
				stepLocated(PSCollision, et);
				stepLocated(PSDynamic, et);
				stepLocated(PSPostdynamic, et);										
			}
			while (--nbPass);
			
			// perform parametric motion if present
			for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
			{
				if ((*it)->isParametricMotionEnabled()) (*it)->performParametricMotion(_SystemDate, ellapsedTime);
			}
			

			// update system date
			_SystemDate += ellapsedTime;
		}
	}	
		
}

void CParticleSystem::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{		
	sint version =  f.serialVersion(7);	
	//f.serial(_ViewMat);
	f.serial(_SysMat);
	f.serial(_Date);
	if (f.isReading())
	{
		// delete previous multimap
		_LBMap.clear();
		// delete previously attached process
		for (TProcessVect::iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
		{
			delete (*it);
		}

		_ProcessVect.clear();

		f.serialContPolyPtr(_ProcessVect);		
	
		_InvSysMat = _SysMat.inverted();
		_FontGenerator = NULL;
		_FontManager = NULL;
	}
	else
	{
		f.serialContPolyPtr(_ProcessVect);	
	}
	
	if (version > 1) // name of the system
	{
		f.serial(_Name);
	}

	if (version > 2) // infos about integration, and LOD
	{
		f.serial(_AccurateIntegration);
		if (_AccurateIntegration) f.serial(_CanSlowDown, _TimeThreshold, _MaxNbIntegrations);
		f.serial(_InvMaxViewDist, _LODRatio);	
		_MaxViewDist = 1.f / _InvMaxViewDist;
		_InvCurrentViewDist = _InvMaxViewDist;
	}

	if (version > 3) // tell wether the system must compute his bbox, hold a precomputed bbox
	{
		f.serial(_ComputeBBox);
		if (!_ComputeBBox)
		{
			f.serial(_PreComputedBBox);
		}
	}

	if (version > 4) // lifetime informations
	{
		f.serial(_DestroyModelWhenOutOfRange);
		f.serialEnum(_DieCondition);
		if (_DieCondition != none)
		{
			f.serial(_DelayBeforeDieTest);
		}
	}	

	if (version > 5)
	{
		f.serial(_DestroyWhenOutOfFrustum);
	}

	if (version > 6)
	{
		f.serial(_PerformMotionWhenOutOfFrustum);
	}

	if (f.isReading())
	{
		notifyMaxNumFacesChanged();
	}
}


void CParticleSystem::attach(CParticleSystemProcess *ptr)
{
	nlassert(std::find(_ProcessVect.begin(), _ProcessVect.end(), ptr) == _ProcessVect.end() );
	//nlassert(ptr->getOwner() == NULL); // deja attache a un autre systeme
	_ProcessVect.push_back(ptr);
	ptr->setOwner(this);
	notifyMaxNumFacesChanged();
}



void CParticleSystem::remove(CParticleSystemProcess *ptr)
{
	TProcessVect::iterator it = std::find(_ProcessVect.begin(), _ProcessVect.end(), ptr);
	nlassert(it != _ProcessVect.end() );	
	_ProcessVect.erase(it);
	
	

	delete ptr;
}





void CParticleSystem::computeBBox(NLMISC::CAABBox &aabbox)
{
	if (!_ComputeBBox || !_BBoxTouched)
	{
		aabbox = _PreComputedBBox;
		return;
	}

	bool foundOne = false;
	NLMISC::CAABBox tmpBox;
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if ((*it)->computeBBox(tmpBox))
		{
			if (!(*it)->isInSystemBasis())
			{
				// rotate the aabbox so that it is in the correct basis
				tmpBox = CPSUtil::transformAABBox(_InvSysMat, tmpBox);
			}
			if (foundOne)
			{
				aabbox = CPSUtil::computeAABBoxUnion(aabbox, tmpBox);
			}
			else
			{
				aabbox = tmpBox;
				foundOne = true;
			}
		}
	}

	if (!foundOne)
	{
		aabbox.setCenter(NLMISC::CVector::Null);
		aabbox.setHalfSize(NLMISC::CVector::Null);
	}
	
	_BBoxTouched = false;
	_PreComputedBBox = aabbox;
}



void CParticleSystem::setSysMat(const CMatrix &m)
{
	_SysMat = m;
	_InvSysMat = _SysMat.inverted();
}




bool CParticleSystem::hasOpaqueObjects(void) const
{
	/// for each process
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if (dynamic_cast<CPSLocated *>(*it))
		{
			for (uint k = 0; k < ((CPSLocated *) *it)->getNbBoundObjects(); ++k)
			{
				CPSLocatedBindable *lb = ((CPSLocated *) *it)->getBoundObject(k);
				if (lb->getType() == PSParticle)
				{
					if (((CPSParticle *) lb)->hasOpaqueFaces()) return true;
				}
			}
		}
	}
	return false;
}


bool CParticleSystem::hasTransparentObjects(void) const
{
	/// for each process
	for (TProcessVect::const_iterator it = _ProcessVect.begin(); it != _ProcessVect.end(); ++it)
	{
		if (dynamic_cast<CPSLocated *>(*it))
		{
			for (uint k = 0; k < ((CPSLocated *) *it)->getNbBoundObjects(); ++k)
			{
				CPSLocatedBindable *lb = ((CPSLocated *) *it)->getBoundObject(k);
				if (lb->getType() == PSParticle)
				{
					if (((CPSParticle *) lb)->hasTransparentFaces()) return true;
				}
			}
		}
	}
	return false;
}




void CParticleSystem::getLODVect(NLMISC::CVector &v, float &offset,  bool systemBasis)
{
	if (!systemBasis)
	{
		v = _InvCurrentViewDist * _InvertedViewMat.getJ();
		offset = - _InvertedViewMat.getPos() * v;
	}
	else
	{
		const CVector tv = _InvSysMat.mulVector(_InvertedViewMat.getJ());
		const CVector org = _InvSysMat * _InvertedViewMat.getPos();
		v = _InvCurrentViewDist * tv;
		offset = - org * v;
	}
}


TPSLod CParticleSystem::getLOD(void) const
{
	const float dist = fabsf(_InvCurrentViewDist * (_SysMat.getPos() - _InvertedViewMat.getPos()) * _InvertedViewMat.getJ());
	return dist > _LODRatio ? PSLod2 : PSLod1;
}


void CParticleSystem::registerLocatedBindableExternID(uint32 id, CPSLocatedBindable *lb)
{
	nlassert(lb);
	nlassert(lb->getOwner() && lb->getOwner()->getOwner() == this); // the located bindable must belong to that system
	#ifdef NL_DEBUG		
		// check that this lb hasn't been inserted yet
		TLBMap::iterator lbd = _LBMap.lower_bound(id), ubd = _LBMap.upper_bound(id);
		nlassert(std::find(lbd, ubd, TLBMap::value_type (id, lb)) == ubd);
		nlassert(std::find(lbd, ubd, TLBMap::value_type (id, lb)) == ubd );

	#endif
		_LBMap.insert(TLBMap::value_type (id, lb) );
}

void CParticleSystem::unregisterLocatedBindableExternID(CPSLocatedBindable *lb)
{
	nlassert(lb);	
	nlassert(lb->getOwner() && lb->getOwner()->getOwner() == this); // the located bindable must belong to that system
	uint32 id = lb->getExternID();
	if (!id) return;
	TLBMap::iterator lbd = _LBMap.lower_bound(id), ubd = _LBMap.upper_bound(id);
	TLBMap::iterator el = std::find(lbd, ubd, TLBMap::value_type (id, lb));
	nlassert(el != ubd); 
	_LBMap.erase(el);
}

uint CParticleSystem::getNumLocatedBindableByExternID(uint32 id) const
{
	return _LBMap.count(id);
}

CPSLocatedBindable *CParticleSystem::getLocatedBindableByExternID(uint32 id, uint index)
{
	nlassert(index < _LBMap.count(id));
	TLBMap::const_iterator el = _LBMap.lower_bound(id);
	uint left = index;
	while (left--) ++el;
	return  el->second;

}

const CPSLocatedBindable *CParticleSystem::getLocatedBindableByExternID(uint32 id, uint index) const
{
	nlassert(index < _LBMap.count(id));
	TLBMap::const_iterator el = _LBMap.lower_bound(id);
	uint left = index;
	while (left--) ++el;
	return  el->second;
}



void CParticleSystem::merge(CParticleSystemShape *pss)
{
	nlassert(pss);	
	CParticleSystem *duplicate = pss->instanciatePS(*this->_Scene); // duplicate the p.s. to merge
	// now we transfer the located of the duplicated ps to this object...
	for (TProcessVect::iterator it = duplicate->_ProcessVect.begin(); it != duplicate->_ProcessVect.end(); ++it)
	{		
		attach(*it);		
	}
	duplicate->_ProcessVect.clear();
	delete duplicate;
}


} // NL3D
