/** \file ps_shockwave.cpp
 * Shockwaves particles.
 *
 * $Id: ps_shockwave.cpp,v 1.3 2002/02/28 12:59:51 besson Exp $
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

#include "3d/ps_macro.h"
#include "3d/ps_shockwave.h"
#include "3d/driver.h"
#include "3d/texture_grouped.h"
#include "3d/ps_iterator.h"
#include "3d/particle_system.h"


namespace NL3D 
{							   

///////////////////////////
// constant definition   //
///////////////////////////

// max number of shockwave to be processed at once
static const uint ShockWaveBufSize = 128; 

// the number of vertices we want in a vertex buffer
static const uint NumVertsInBuffer = 8 * ShockWaveBufSize;


CPSShockWave::TPBMap CPSShockWave::_PBMap; // the primitive blocks
CPSShockWave::TVBMap CPSShockWave::_VBMap; // vb ith unanimated texture
CPSShockWave::TVBMap CPSShockWave::_AnimTexVBMap; // vb ith unanimated texture
CPSShockWave::TVBMap CPSShockWave::_ColoredVBMap; // vb ith unanimated texture
CPSShockWave::TVBMap CPSShockWave::_ColoredAnimTexVBMap; // vb ith unanimated texture
/////////////////////////////////
// CPSShockWave implementation //
/////////////////////////////////


/** Well, we could have put a method template in CPSShockWave, but some compilers
  * want the definition of the methods in the header, and some compilers
  * don't want friend with function template, so we use a static method template of a friend class instead,
  * which gives us the same result :)
  */
class CPSShockWaveHelper
{
public:
	template <class T>	
	static void drawShockWave(T posIt, CPSShockWave &s, uint size, uint32 srcStep)
	{
		PARTICLES_CHECK_MEM;
		nlassert(s._Owner);

		// get / build the vertex buffer and the primitive block
		CVertexBuffer *vb;
		CPrimitiveBlock *pb;
		s.getVBnPB(vb, pb);

		const uint32 vSize = vb->getVertexSize();
		IDriver *driver = s.getDriver();
		s._Owner->incrementNbDrawnParticles(size); // for benchmark purpose	
		s.setupDriverModelMatrix();
		const uint numShockWaveToDealWith = std::min(ShockWaveBufSize, s.getNumShockWavesInVB());
		driver->activeVertexBuffer(*vb);	

		static CPlaneBasis planeBasis[ShockWaveBufSize];
		float       sizes[ShockWaveBufSize];
		float       angles[ShockWaveBufSize];
		
		uint leftToDo  = size, toProcess;
		T endIt;
		uint8 *currVertex;
		uint k ;

		const float angleStep = 256.f / s._NbSeg;
		float currAngle;

		CPlaneBasis *ptCurrBasis;
		uint32	ptCurrBasisIncrement = s._PlaneBasisScheme ? 1 : 0;

		float *ptCurrSize;
		uint32 ptCurrSizeIncrement = s._SizeScheme ? 1 : 0;

		float *ptCurrAngle;
		uint32 ptCurrAngleIncrement = s._Angle2DScheme ? 1 : 0;

		CVector radVect, innerVect;
		float radiusRatio;

		do
		{
			currVertex = (uint8 *) vb->getVertexCoordPointer();
			toProcess = leftToDo > numShockWaveToDealWith ? numShockWaveToDealWith : leftToDo;
			endIt = posIt + toProcess;
			if (s._SizeScheme)
			{
				ptCurrSize  = (float *) (s._SizeScheme->make(s._Owner, size - leftToDo, (void *) sizes, sizeof(float), toProcess, true, srcStep));			
			}
			else
			{
				ptCurrSize = &s._ParticleSize;
			}

			if (s._PlaneBasisScheme)
			{
				ptCurrBasis  = (CPlaneBasis *) (s._PlaneBasisScheme->make(s._Owner, size - leftToDo, (void *) planeBasis, sizeof(CPlaneBasis), toProcess, true, srcStep));			
			}
			else
			{
				ptCurrBasis = &s._PlaneBasis;
			}

			if (s._Angle2DScheme)
			{
				ptCurrAngle  = (float *) (s._Angle2DScheme->make(s._Owner, size - leftToDo, (void *) angles, sizeof(float), toProcess, true, srcStep));			
			}
			else
			{
				ptCurrAngle = &s._Angle2D;
			}
			

			s.updateVbColNUVForRender(size - leftToDo, toProcess, srcStep, *vb);
			do
			{			
				currAngle = *ptCurrAngle;
				if (fabsf(*ptCurrSize) > 10E-6)
				{
					radiusRatio = (*ptCurrSize - s._RadiusCut) / *ptCurrSize;
				}
				else
				{
					radiusRatio = 0.f;
				}

				for (k = 0; k <= s._NbSeg; ++k)
				{
					radVect = *ptCurrSize * (CPSUtil::getCos((sint32) currAngle) * ptCurrBasis->X + CPSUtil::getSin((sint32) currAngle) * ptCurrBasis->Y);
					innerVect = radiusRatio * radVect;
					CHECK_VERTEX_BUFFER(*vb, currVertex);
					* (CVector *) currVertex = *posIt + radVect;
					currVertex += vSize;
					CHECK_VERTEX_BUFFER(*vb, currVertex);
					* (CVector *) currVertex = *posIt + innerVect;
					currVertex += vSize;
					currAngle += angleStep;				
				}
				
				++posIt;
				ptCurrBasis +=  ptCurrBasisIncrement;
				ptCurrSize  +=  ptCurrSizeIncrement;
				ptCurrAngle  +=  ptCurrAngleIncrement;
			}
			while (posIt != endIt);			

			pb->setNumQuad(toProcess * s._NbSeg);
			driver->render(*pb, s._Mat);
			leftToDo -= toProcess;		
		}
		while (leftToDo);
		PARTICLES_CHECK_MEM;
	}
};

///=================================================================================
CPSShockWave::CPSShockWave(uint nbSeg, float radiusCut, CSmartPtr<ITexture> tex) 
		:  _NbSeg(nbSeg)
		   , _RadiusCut(radiusCut)
		   , _UFactor(1.f)

{
	nlassert(nbSeg > 2 && nbSeg <= 64);
	setTexture(tex);
	init();
	_Name = std::string("ShockWave");
}

///=================================================================================
uint32 CPSShockWave::getMaxNumFaces(void) const
{
	nlassert(_Owner);
	return (_Owner->getMaxSize() * _NbSeg) << 1 ;
}

///=================================================================================
bool CPSShockWave::hasTransparentFaces(void)
{
	return getBlendingMode() != CPSMaterial::alphaTest ;
}

///=================================================================================
bool CPSShockWave::hasOpaqueFaces(void)
{
	return !hasTransparentFaces();
}

///=================================================================================
void CPSShockWave::setNbSegs(uint nbSeg)
{
	nlassert(nbSeg > 2 && nbSeg <= 64);
	_NbSeg = nbSeg;
	if (_Owner)
	{
		resize(_Owner->getMaxSize());
		notifyOwnerMaxNumFacesChanged();
	}
}

///=================================================================================
void CPSShockWave::setRadiusCut(float radiusCut)
{
	_RadiusCut = radiusCut;	
	if (_Owner)
	{
		resize(_Owner->getMaxSize());
	}
}

///=================================================================================
void	CPSShockWave::setUFactor(float value)
{
	nlassert(_Owner); // must be attached to an owner before to call this method
	_UFactor = value;
	resize(_Owner->getSize()); // resize also recomputes the UVs..
}

///=================================================================================
void CPSShockWave::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{	
	sint ver  = f.serialVersion(2);
	CPSParticle::serial(f);
	CPSColoredParticle::serialColorScheme(f);
	CPSSizedParticle::serialSizeScheme(f);
	CPSTexturedParticle::serialTextureScheme(f);
	CPSRotated3DPlaneParticle::serialPlaneBasisScheme(f);
	CPSRotated2DParticle::serialAngle2DScheme(f);
	serialMaterial(f);
	f.serial(_NbSeg, _RadiusCut);
	if (ver > 1)
	{
		f.serial(_UFactor);
	}
	init();	
}

///=================================================================================
inline void CPSShockWave::setupUFactor()
{	
	if (_UFactor != 1.f)
	{
		_Mat.enableUserTexMat(0);
		CMatrix texMat;
		texMat.setRot(_UFactor  * NLMISC::CVector::I,
					  NLMISC::CVector::J,
					  NLMISC::CVector::K
					 );
		_Mat.setUserTexMat(0, texMat);
	}
	else
	{
		_Mat.enableUserTexMat(0, false);
	}	
}

///=================================================================================
void CPSShockWave::draw(bool opaque)
{
	PARTICLES_CHECK_MEM;	
	if (!_Owner->getSize()) return;	

	uint32 step;
	uint   numToProcess;
	computeSrcStep(step, numToProcess);	
	if (!numToProcess) return;


	
	/// update the material if the global color of the system is variable
	CParticleSystem &ps = *(_Owner->getOwner());
	/// update the material if the global color of the system is variable		
	if (_ColorScheme != NULL && ps.getColorAttenuationScheme() != NULL)
	{		
		CPSMaterial::forceModulateConstantColor(true, ps.getGlobalColor());		
	}
	else
	{
		forceModulateConstantColor(false);
		if (!ps.getColorAttenuationScheme())
		{
			_Mat.setColor(_Color);
		}
		else
		{
			NLMISC::CRGBA col;
			col.modulateFromColor(ps.getGlobalColor(), _Color);
			_Mat.setColor(col);
		}
	}
	//////

	setupUFactor();
	
	if (step == (1 << 16))
	{
		CPSShockWaveHelper::drawShockWave(_Owner->getPos().begin(),				   
										  *this,
										  numToProcess,
										  step
										 );
	}
	else
	{		
		CPSShockWaveHelper::drawShockWave(TIteratorVectStep1616(_Owner->getPos().begin(), 0, step),				   
										  *this,
										  numToProcess,
										  step
										 );
	}
	
	PARTICLES_CHECK_MEM;
}

///=================================================================================
bool CPSShockWave::completeBBox(NLMISC::CAABBox &box) const
{
	// TODO : implement this
	return false;
}

///=================================================================================
void CPSShockWave::init(void)
{
	_Mat.setLighting(false);	
	_Mat.setZFunc(CMaterial::less);
	_Mat.setDoubleSided(true);
	updateMatAndVbForColor();
	updateMatAndVbForTexture();
}

///=================================================================================
void CPSShockWave::updateVbColNUVForRender(uint32 startIndex, uint32 size, uint32 srcStep, CVertexBuffer &vb)
{
	nlassert(_Owner);
	if (!size) return;
	if (_ColorScheme)
	{
		// compute the colors, each color is replicated n times...
		_ColorScheme->makeN(_Owner, startIndex, vb.getColorPointer(), vb.getVertexSize(), size, (_NbSeg + 1) << 1, srcStep);
	}

	if (_TexGroup) // if it has a constant texture we are sure it has been setupped before...
	{	
		sint32 textureIndex[ShockWaveBufSize];
		const uint32 stride = vb.getVertexSize(), stride2 = stride << 1;
		uint8 *currUV = (uint8 *) vb.getTexCoordPointer();				
		uint k;		

		uint32 currIndexIncr;
		const sint32 *currIndex;		

		if (_TextureIndexScheme)
		{
			currIndex  = (sint32 *) (_TextureIndexScheme->make(_Owner, startIndex, textureIndex, sizeof(sint32), size, true, srcStep));			
			currIndexIncr = 1;
		}
		else
		{	
			currIndex = &_TextureIndex;
			currIndexIncr = 0;
		}
		
		while (size--)
		{
			// for now, we don't make texture index wrapping
			const CTextureGrouped::TFourUV &uvGroup = _TexGroup->getUVQuad((uint32) *currIndex);

			k = _NbSeg + 1;

			for (k = 0; k <= _NbSeg; ++k)
			{
				
				*(CUV *) currUV = uvGroup.uv0 + CUV(k * _UFactor, 0);
				*(CUV *) (currUV + stride) = uvGroup.uv3 + CUV(k * _UFactor, 0);				
				// point the next quad
				currUV += stride2;
			}
			while (--k);

			currIndex += currIndexIncr;
		}		
	}	
}

///=================================================================================
void CPSShockWave::updateMatAndVbForColor(void)
{	

	if (_Owner)
	{
		resize(_Owner->getMaxSize());
	}	
}

///=================================================================================
void CPSShockWave::updateMatAndVbForTexture(void)
{
	_Mat.setTexture(0, _TexGroup ? (ITexture *) _TexGroup : (ITexture *) _Tex);	
}

///=================================================================================
void CPSShockWave::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	newColorElement(emitterLocated, emitterIndex);
	newTextureIndexElement(emitterLocated, emitterIndex);
	newSizeElement(emitterLocated, emitterIndex);
	newAngle2DElement(emitterLocated, emitterIndex);
}

///=================================================================================
void CPSShockWave::deleteElement(uint32 index)
{
	deleteColorElement(index);
	deleteTextureIndexElement(index);
	deleteSizeElement(index);
	deleteAngle2DElement(index);
}

///=================================================================================
void CPSShockWave::resize(uint32 aSize)
{
	nlassert(aSize < (1 << 16));
	resizeColor(aSize);
	resizeTextureIndex(aSize);
	resizeSize(aSize);
	resizeAngle2D(aSize);	
}

///=================================================================================
void CPSShockWave::getVBnPB(CVertexBuffer *&retVb, CPrimitiveBlock *&retPb)
{
	TVBMap &vbMap = _ColorScheme == NULL  ? (_TexGroup == NULL ?  _VBMap : _AnimTexVBMap)
										  : (_TexGroup == NULL ?  _ColoredVBMap : _ColoredAnimTexVBMap);


	TVBMap::iterator vbIt = vbMap.find(_NbSeg);
	if (vbIt != vbMap.end())
	{
		retVb = &(vbIt->second);
		TPBMap::iterator pbIt = _PBMap.find(_NbSeg);
		nlassert(pbIt != _PBMap.end());
		retPb = &(pbIt->second);
	}
	else // we need to create the vb
	{		
		// create an entry (we setup the primitive block at the same time, this could be avoided, but doesn't make much difference)		
		CVertexBuffer &vb = vbMap[_NbSeg]; // create a vb
		CPrimitiveBlock &pb = _PBMap[_NbSeg]; // eventually create a pb
		const uint32 size = getNumShockWavesInVB();
		vb.setVertexFormat(CVertexBuffer::PositionFlag |
						   CVertexBuffer::TexCoord0Flag |
						   (_ColorScheme != NULL ?  CVertexBuffer::PrimaryColorFlag : 0) 
						  );	
		vb.setNumVertices((size * (_NbSeg + 1)) << 1 );		
		pb.reserveQuad(size * _NbSeg);
		for (uint32 k = 0; k < size; ++k)
		{
			for (uint32 l = 0; l < _NbSeg; ++l)
			{	
				const uint32 index = ((k * (_NbSeg + 1)) + l) << 1;						
				pb.setQuad(l + (k * _NbSeg) , index , index + 2, index + 3, index + 1);			
				vb.setTexCoord(index, 0, CUV((float) l, 0));
				vb.setTexCoord(index + 1, 0, CUV((float) l, 1));			
			}
			const uint32 index = ((k * (_NbSeg + 1)) + _NbSeg) << 1;
			vb.setTexCoord(index, 0, CUV((float) _NbSeg, 0));
			vb.setTexCoord(index + 1, 0, CUV((float) _NbSeg, 1));			
		}
		retVb = &vb;
		retPb = &pb;
	}
}

///=================================================================================
uint CPSShockWave::getNumShockWavesInVB() const
{
	const uint numRib = NumVertsInBuffer / ((_NbSeg + 1) << 1);
	return std::max(1u, numRib);
}

} // NL3D
