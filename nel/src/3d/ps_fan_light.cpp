/** \file ps_fan_light.cpp
 * FanLight particles
 *
 * $Id: ps_fan_light.cpp,v 1.9 2004/03/04 14:29:31 vizerie Exp $
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

#include "std3d.h"

#include "3d/ps_fan_light.h"
#include "3d/ps_macro.h"
#include "3d/ps_attrib_maker.h"
#include "3d/ps_iterator.h"
#include "3d/particle_system.h"
#include "3d/driver.h"



namespace NL3D 
{



//////////////////////////////
// fan light implementation //
//////////////////////////////


uint8 CPSFanLight::_RandomPhaseTab[32][128];
bool CPSFanLight::_RandomPhaseTabInitialized = false;

CPSFanLight::TVBMap				CPSFanLight::_VBMap; // fanlight, no texture
CPSFanLight::TVBMap				CPSFanLight::_TexVBMap; // fanlight, textured
CPSFanLight::TVBMap				CPSFanLight::_ColoredVBMap; // fanlight, no texture, varying color
CPSFanLight::TVBMap				CPSFanLight::_ColoredTexVBMap; // fanlight, textured, varying color
CPSFanLight::TIBMap				CPSFanLight::_IBMap;


static const uint FanLightBufSize  = 128; // the size of a buffer of particle to deal with at a time
static const uint NumVertsInBuffer = 4 * FanLightBufSize;


///====================================================================================

/** Well, we could have put a method template in CPSFanLight, but some compilers
  * want the definition of the methods in the header, and some compilers
  * don't want friend with function template, so we use a static method template of a friend class instead,
  * which gives us the same result :)
  */
class CPSFanLightHelper
{
public:
	template <class T, class U>	
	static void drawFanLight(T posIt, U timeIt, CPSFanLight &f, uint size, uint32 srcStep)
	{
		PARTICLES_CHECK_MEM;
		nlassert(f._RandomPhaseTabInitialized);
		//
		f.setupDriverModelMatrix();
		const CVector I = f.computeI();
		const CVector K = f.computeK();		
		//		
		CVertexBuffer *vb;
		CPSFanLight::TIndexBuffer  *ib;
		// get (and build if necessary) the vb and the ib
		f.getVBnIB(vb, ib);
		IDriver *driver = f.getDriver();
		driver->activeVertexBuffer(*vb);
		const uint maxNumFanLightToDealWith = std::min(FanLightBufSize, f.getNumFanlightsInVB());	
		uint8 *randomPhaseTab = &f._RandomPhaseTab[f._PhaseSmoothness][0];
		f._Owner->incrementNbDrawnParticles(size); // for benchmark purpose			
		float pSizes[FanLightBufSize];
		float pAngles[FanLightBufSize];
		T endPosIt;		

		sint32 k; // helps to count the fans

		
		 // if so, we need to deal process separatly group of particles			
		const uint32 stride = vb->getVertexSize();

		float currentAngle;
		const float angleStep = 256.0f / f._NbFans;	

		
		float *currentSizePt; // it points either the particle constant size, or a size in a table
		float *currentAnglePt; // it points either the particle constant angle, or an angle in a table

		
		const uint32 currentSizePtIncrement = f._SizeScheme ? 1 : 0; // increment to get the next size for the size pointer. It is 0 if the size is constant
		const uint32 currentAnglePtIncrement = f._Angle2DScheme ? 1 : 0; // increment to get the next angle for the angle pointer. It is 0 if the size is constant
				

		uint leftToDo = size;
		do
		{				
			uint8 *ptVect = (uint8 *) vb->getVertexCoordPointer();
			uint toProcess = std::min(leftToDo, maxNumFanLightToDealWith);							
			// compute individual colors if needed
			if (f._ColorScheme)
			{
				// we change the color at each fan light center
				f._ColorScheme->make(f._Owner, size - leftToDo, vb->getColorPointer(), vb->getVertexSize() * (f._NbFans + 2), toProcess, false, srcStep);
			}
			if (f._SizeScheme)
			{
				currentSizePt  = (float *) (f._SizeScheme->make(f._Owner, size - leftToDo, pSizes, sizeof(float), toProcess, true, srcStep));
				currentSizePt = pSizes;
			}
			else
			{
				currentSizePt = &f._ParticleSize;
			}
			if (f._Angle2DScheme)
			{
				currentAnglePt = (float *) (f._Angle2DScheme->make(f._Owner, size - leftToDo, pAngles, sizeof(float), toProcess, true, srcStep));					
			}
			else
			{
				currentAnglePt = &f._Angle2D;
			}			
			//								
			float fSize, firstSize, sizeStepBase=0.0, sizeStep;
			if (f._PhaseSmoothness)
			{
				sizeStepBase = 1.f / f._PhaseSmoothness;
			}
			endPosIt = posIt + toProcess;
			for (;posIt != endPosIt; ++posIt, ++timeIt)
			{	
				
				CHECK_VERTEX_BUFFER(*vb, ptVect);
				*(CVector *) ptVect = *posIt;				
				// the start angle
				currentAngle = *currentAnglePt;
				const uint8 phaseAdd = (uint8) (f._PhaseSpeed * (*timeIt));
				ptVect += stride;
				const float fanSize = *currentSizePt * 0.5f;
				const float moveIntensity = f._MoveIntensity * fanSize;				
				// compute radius & vect for first fan
				firstSize  = fanSize + (moveIntensity * CPSUtil::getCos(randomPhaseTab[0] + phaseAdd));
				*(CVector *) ptVect = (*posIt) + I * firstSize * (CPSUtil::getCos((sint32) currentAngle))
									  + K * firstSize * (CPSUtil::getSin((sint32) currentAngle));
				currentAngle += angleStep;
				ptVect += stride;
				fSize = firstSize;
				// computes other fans
				const sint32 upperBound = (sint32) (f._NbFans - f._PhaseSmoothness - 1);
				for (k = 1; k <= upperBound; ++k)
				{
					fSize  = fanSize + (moveIntensity * CPSUtil::getCos(randomPhaseTab[k] + phaseAdd));
					*(CVector *) ptVect = (*posIt) + I * fSize * (CPSUtil::getCos((sint32) currentAngle))
										  + K * fSize * (CPSUtil::getSin((sint32) currentAngle));
					currentAngle += angleStep;
					ptVect += stride;
				}

				// interpolate radius, so that the fanlight loops correctly
				sizeStep = sizeStepBase * (firstSize - fSize);
				for (; k <= (sint32) (f._NbFans - 1); ++k)
				{				
					*(CVector *) ptVect = (*posIt) + I * fSize * (CPSUtil::getCos((sint32) currentAngle))
										  + K * fSize * (CPSUtil::getSin((sint32) currentAngle));
					currentAngle += angleStep;
					ptVect += stride;
					fSize  += sizeStep;
				}
				// last fan
				*(CVector *) ptVect = (*posIt) + I * firstSize * (CPSUtil::getCos((sint32) *currentAnglePt))
										  + K * firstSize * (CPSUtil::getSin((sint32) *currentAnglePt));
				ptVect += stride;
				currentSizePt += currentSizePtIncrement;
				currentAnglePt += currentAnglePtIncrement;
			}			
			driver->renderTriangles(f._Mat, &((*ib)[0]), toProcess * f._NbFans);
			leftToDo -= toProcess;
		}		
		while (leftToDo != 0);
		PARTICLES_CHECK_MEM;
	}
};

	
///====================================================================================
// this blur a tab of bytes once
static void BlurBytesTab(const uint8 *src, uint8 *dest, uint size)
{
	std::vector<uint8> b(src, src + size);
	for (sint k = 1 ; k < (sint) (size - 1); ++k)
	{
		dest[k] = (uint8) (((uint16) b[k - 1] + (uint16) b[k + 1])>>1);
	}
}

///====================================================================================
void CPSFanLight::initFanLightPrecalc(void)
{
	// build several random tab, and linearly interpolate between l values
	float currPhase, nextPhase, phaseStep;
	for (uint l = 0; l < 32 ; l++)
	{
		nextPhase = (float) (uint8) (rand()&0xFF);
		uint32 k = 0;
		while (k < 128)
		{
			currPhase = nextPhase;
			nextPhase = (float) (uint8) (rand()&0xFF);
			phaseStep = (nextPhase - currPhase) / (l + 1);

			for (uint32 m = 0; m <= l; ++m)
			{
				_RandomPhaseTab[l][k] = (uint8) currPhase;
				currPhase += phaseStep;
				++k;
				if (k >= 128) break;
			}
		}
		for (uint m = 0; m < 2 * l; ++m)
			BlurBytesTab(&_RandomPhaseTab[l][0], &_RandomPhaseTab[l][0], 128);
	}	
	//#ifdef NL_DEBUG
		_RandomPhaseTabInitialized = true;
	//#endif
}

///====================================================================================
uint32 CPSFanLight::getMaxNumFaces(void) const
{
	nlassert(_Owner);
	return _Owner->getMaxSize() * _NbFans;
}

///====================================================================================
bool CPSFanLight::hasTransparentFaces(void)
{
	return getBlendingMode() != CPSMaterial::alphaTest ;
}

///====================================================================================
bool CPSFanLight::hasOpaqueFaces(void)
{
	return !hasTransparentFaces();
}

///====================================================================================
void CPSFanLight::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	newColorElement(emitterLocated, emitterIndex);
	newSizeElement(emitterLocated, emitterIndex);
	newAngle2DElement(emitterLocated, emitterIndex);
}

///====================================================================================
void CPSFanLight::deleteElement(uint32 index)
{
	deleteColorElement(index);
	deleteSizeElement(index);
	deleteAngle2DElement(index);
}

///====================================================================================
void CPSFanLight::setPhaseSpeed(float multiplier)
{
	_PhaseSpeed = 256.0f * multiplier;
}

///====================================================================================
inline void CPSFanLight::setupMaterial()
{
	CParticleSystem &ps = *(_Owner->getOwner());
	/// update material color		
	if (_Tex == NULL)
	{				
		forceTexturedMaterialStages(1);
		SetupModulatedStage(_Mat, 0, CMaterial::Diffuse, CMaterial::Constant);
	}
	else
	{
		_Mat.setTexture(0, _Tex);
		forceTexturedMaterialStages(2);				
		SetupModulatedStage(_Mat, 0, CMaterial::Texture, CMaterial::Constant);
		SetupModulatedStage(_Mat, 1, CMaterial::Diffuse, CMaterial::Previous);		
	}
	
	// always setup global colors 					
	if (_ColorScheme)
	{
		if (ps.getForceGlobalColorLightingFlag() || usesGlobalColorLighting())
		{		
			_Mat.texConstantColor(0, ps.getGlobalColorLighted());
		}
		else
		{
			_Mat.texConstantColor(0, ps.getGlobalColor());
		}		
	}
	else
	{
		NLMISC::CRGBA col;		
		if (ps.getForceGlobalColorLightingFlag() || usesGlobalColorLighting())
		{		
			col.modulateFromColor(ps.getGlobalColorLighted(), _Color);
		}
		else if (ps.getColorAttenuationScheme() != NULL || ps.isUserColorUsed())
		{
			col.modulateFromColor(ps.getGlobalColor(), _Color);			
		}
		else
		{
			col = _Color;			
		}
		_Mat.texConstantColor(0, col);
	}	
}

///====================================================================================
void CPSFanLight::draw(bool opaque)
{
	PARTICLES_CHECK_MEM;	
	if (!_Owner->getSize()) return;	

	uint32 step;
	uint   numToProcess;
	computeSrcStep(step, numToProcess);	
	if (!numToProcess) return;
	
	setupMaterial();	
	
	if (step == (1 << 16))
	{
		CPSFanLightHelper::drawFanLight(_Owner->getPos().begin(),
									    _Owner->getTime().begin(),
									   *this,
										numToProcess,
										step
									   );
	}
	else
	{		
		CPSFanLightHelper::drawFanLight(TIteratorVectStep1616(_Owner->getPos().begin(), 0, step),
										TIteratorTimeStep1616(_Owner->getTime().begin(), 0, step),
									    *this,
									    numToProcess,
									    step				
									   );
	}
	
	PARTICLES_CHECK_MEM;
}

///====================================================================================
void CPSFanLight::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion(2);
	CPSParticle::serial(f);
	CPSColoredParticle::serialColorScheme(f);	
	CPSSizedParticle::serialSizeScheme(f);	
	CPSRotated2DParticle::serialAngle2DScheme(f);
	f.serial(_NbFans);
	serialMaterial(f);
	if (ver > 1)
	{
		f.serial(_PhaseSmoothness, _MoveIntensity);
		ITexture *tex = _Tex;
		f.serialPolyPtr(tex);
		if (f.isReading()) _Tex = tex ;
	}
	if (f.isReading())
	{
		init();		
	}	
}

///====================================================================================
bool CPSFanLight::completeBBox(NLMISC::CAABBox &box) const
{
	// TODO

	return false;
}

///====================================================================================
CPSFanLight::CPSFanLight(uint32 nbFans) : _NbFans(nbFans),
										  _PhaseSmoothness(0),
										  _MoveIntensity(1.5f),
										  _Tex(NULL),
										  _PhaseSpeed(256)
{
	nlassert(nbFans >= 3);


	init();
	if (CParticleSystem::getSerializeIdentifierFlag()) _Name = std::string("FanLight");
}


///====================================================================================
CPSFanLight::~CPSFanLight()
{
}


///====================================================================================
void CPSFanLight::setNbFans(uint32 nbFans)
{
	_NbFans = nbFans;

	resize(_Owner->getMaxSize());

	notifyOwnerMaxNumFacesChanged();
}

///====================================================================================
void CPSFanLight::resize(uint32 size)
{	
	nlassert(size < (1 << 16));
	resizeColor(size);
	resizeAngle2D(size);
	resizeSize(size);
	
}

///====================================================================================
void CPSFanLight::init(void)
{
	_Mat.setLighting(false);	
	_Mat.setZFunc(CMaterial::less);
	_Mat.setDoubleSided(true);
	_Mat.setColor(NLMISC::CRGBA::White);

	updateMatAndVbForColor();
}

///====================================================================================
void CPSFanLight::updateMatAndVbForColor(void)
{	
	//touch();
}

///====================================================================================
void CPSFanLight::getVBnIB(CVertexBuffer *&retVb, CPSFanLight::TIndexBuffer *&retIb)
{
	TVBMap &vbMap = _ColorScheme ? (_Tex == NULL  ? _ColoredVBMap : _ColoredTexVBMap)
								 : (_Tex == NULL  ? _VBMap : _TexVBMap);
	TVBMap::iterator vbIt = vbMap.find(_NbFans);
	if (vbIt != vbMap.end())
	{
		retVb = &(vbIt->second);
		TIBMap::iterator pbIt = _IBMap.find(_NbFans);
		nlassert(pbIt != _IBMap.end());
		retIb = &(pbIt->second);
	}
	else // we need to create the vb
	{		
		// create an entry (we setup the primitive block at the same time, this could be avoided, but doesn't make much difference)		
		CVertexBuffer &vb = vbMap[_NbFans]; // create a vb
		TIndexBuffer &ib = _IBMap[_NbFans]; // eventually create a pb
		const uint32 size = getNumFanlightsInVB();
		vb.setVertexFormat(CVertexBuffer::PositionFlag |
						   CVertexBuffer::PrimaryColorFlag |
						   (_Tex != NULL ?  CVertexBuffer::TexCoord0Flag : 0) 
						  );
		vb.setNumVertices(size * (2 + _NbFans));			
		ib.resize(size * _NbFans * 3);		
		// pointer on the current index to fill
		TIndexBuffer::iterator ptIndex = ib.begin();	
		
		// index of the first vertex of the current fanFilght
		uint currVertFan = 0;

		uint l; // the current fan in the current fanlight
		uint k; // the current fan light

		for (k = 0; k < size; ++k)
		{
			for (l = 0; l < _NbFans; ++l)
			{
				*ptIndex++ = currVertFan;
				*ptIndex++ = currVertFan + (l + 1);
				*ptIndex++ = currVertFan + (l + 2);
			}					
			currVertFan += 2 + _NbFans;
		}
			
		for (k = 0; k < size; ++k)
		{			
			if (_Tex)
			{
				vb.setTexCoord(k * (_NbFans + 2), 0, NLMISC::CUV(0, 0));
			}
			if (!_ColorScheme)
			{
				vb.setColor(k * (_NbFans + 2), CRGBA::White);			
			}		
			if (!_Tex)
			{
				for(l = 1; l <= _NbFans + 1; ++l)
				{
					vb.setColor(l + k * (_NbFans + 2), CRGBA(0, 0, 0));
				}
			}
			else
			{
				for(l = 1; l <= _NbFans + 1; ++l)
				{
					vb.setColor(l + k * (_NbFans + 2), CRGBA(0, 0, 0));
					vb.setTexCoord(l + k * (_NbFans + 2), 0, NLMISC::CUV((l - 1) / (float) _NbFans, 1));
				}
			}
		}
		
		retVb = &vb;
		retIb = &ib;
	}	
}

///====================================================================================
uint CPSFanLight::getNumFanlightsInVB() const
{
	const uint numRib = NumVertsInBuffer / (2 + _NbFans);
	return std::max(1u, numRib);
}

///====================================================================================
void CPSFanLight::enumTexs(std::vector<NLMISC::CSmartPtr<ITexture> > &dest, IDriver &drv)
{
	if (_Tex) 
	{	
		dest.push_back(_Tex);
	}
}

} // NL3D
