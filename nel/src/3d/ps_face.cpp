/** \file ps_face.cpp
 * Face particles.
 *
 * $Id: ps_face.cpp,v 1.1 2002/02/15 17:03:29 vizerie Exp $
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

#include "3d/ps_face.h"
#include "3d/ps_macro.h"
#include "3d/driver.h"
#include "3d/ps_iterator.h"
#include "nel/misc/quat.h"



namespace NL3D 
{

using NLMISC::CQuat;

////////////////////////////
// CPSFace implementation //
////////////////////////////

/** Well, we could have put a method template in CPSFace, but some compilers
  * want the definition of the methods in the header, and some compilers
  * don't want friend with function template, so we use a static method template of a friend class instead,
  * which gives us the same result :)  
  */
class CPSFaceHelper
{
public:
	template <class T, class U>	
	static void drawFaces(T posIt, U indexIt, CPSFace &f, uint size, uint32 srcStep)
	{
		PARTICLES_CHECK_MEM;
		nlassert(f._Owner);		
		IDriver *driver = f.getDriver();

		f.updateMatBeforeRendering(driver);

		CVertexBuffer &vb = f.getNeededVB();
		const uint32 vSize = vb.getVertexSize();	
		uint8 *currVertex; 	

		// number of left faces to draw, number of faces to process at once
		uint32 leftFaces = size, toProcess;
		f._Owner->incrementNbDrawnParticles(size); // for benchmark purpose		
		driver->activeVertexBuffer(vb);
		if (f.isMultiTextureEnabled())
		{
			f.setupMaterial(f._Tex, driver, f._Mat);
		}
		float sizeBuf[CPSQuad::quadBufSize];
		float *ptSize;		
		T endPosIt;

		// if constant size is used, the pointer points always the same float 
		uint32 ptSizeIncrement = f._SizeScheme ? 1 : 0;

		if (f._PrecompBasis.size()) // do we use precomputed basis ?
		{					
			do
			{		
				toProcess = leftFaces > CPSQuad::quadBufSize ? CPSQuad::quadBufSize : leftFaces;
				currVertex = (uint8 *) vb.getVertexCoordPointer() ; 
				if (f._SizeScheme)
				{				
					ptSize = (float *) (f._SizeScheme->make(f._Owner, size - leftFaces, sizeBuf, sizeof(float), toProcess, true, srcStep));								
				}
				else
				{	
					ptSize = &f._ParticleSize;			
				}					
				f.updateVbColNUVForRender(vb, size - leftFaces, toProcess, srcStep);			
				const uint32 stride = vb.getVertexSize(), stride2 = stride << 1, stride3 = stride2 + stride, stride4 = stride2 << 1;
				endPosIt = posIt + toProcess;							
				do		
				{			
					const CPlaneBasis &currBasis = f._PrecompBasis[*indexIt].Basis;
					CHECK_VERTEX_BUFFER(vb, currVertex);
					((CVector *) currVertex)->x = (*posIt).x  + *ptSize * currBasis.X.x;  			
					((CVector *) currVertex)->y = (*posIt).y  + *ptSize * currBasis.X.y;  			
					((CVector *) currVertex)->z = (*posIt).z  + *ptSize * currBasis.X.z;  			
					currVertex += stride;

					CHECK_VERTEX_BUFFER(vb, currVertex);
					((CVector *) currVertex)->x = (*posIt).x  + *ptSize * currBasis.Y.x;  			
					((CVector *) currVertex)->y = (*posIt).y  + *ptSize * currBasis.Y.y;  			
					((CVector *) currVertex)->z = (*posIt).z  + *ptSize * currBasis.Y.z;  			
					currVertex += stride;

					CHECK_VERTEX_BUFFER(vb, currVertex);
					((CVector *) currVertex)->x = (*posIt).x  - *ptSize * currBasis.X.x;  			
					((CVector *) currVertex)->y = (*posIt).y  - *ptSize * currBasis.X.y;  			
					((CVector *) currVertex)->z = (*posIt).z  - *ptSize * currBasis.X.z;  			
					currVertex += stride;

					CHECK_VERTEX_BUFFER(vb, currVertex);
					((CVector *) currVertex)->x = (*posIt).x  - *ptSize * currBasis.Y.x;  			
					((CVector *) currVertex)->y = (*posIt).y  - *ptSize * currBasis.Y.y;  			
					((CVector *) currVertex)->z = (*posIt).z  - *ptSize * currBasis.Y.z;  			
					currVertex += stride;						
					ptSize += ptSizeIncrement;
					++indexIt;
					++posIt;
				}
				while (posIt != endPosIt);
				driver->renderQuads(f._Mat, 0, toProcess);
				leftFaces -= toProcess;
			}
			while (leftFaces);
		}
		else
		{
			// must compute each particle basis at each time
			static CPlaneBasis planeBasis[CPSQuad::quadBufSize]; // buffer to compute each particle basis
			CPlaneBasis *currBasis;
			uint32    ptPlaneBasisIncrement = f._PlaneBasisScheme ? 1 : 0;
			const uint32 vSize = vb.getVertexSize();
			do
			{			
				toProcess = leftFaces > CPSQuad::quadBufSize ? CPSQuad::quadBufSize : leftFaces;
				currVertex = (uint8 *) vb.getVertexCoordPointer() ; 
				if (f._SizeScheme)
				{				
					ptSize  = (float *) (f._SizeScheme->make(f._Owner, size - leftFaces, sizeBuf, sizeof(float), toProcess, true, srcStep));								
				}
				else
				{	
					ptSize = &f._ParticleSize;			
				}

				if (f._PlaneBasisScheme)
				{
					currBasis = (CPlaneBasis *) (f._PlaneBasisScheme->make(f._Owner, size - leftFaces, planeBasis, sizeof(CPlaneBasis), toProcess, true, srcStep));				
				}
				else
				{
					currBasis = &f._PlaneBasis;
				}						
				f.updateVbColNUVForRender(vb, size - leftFaces, toProcess, srcStep);						
				endPosIt = posIt + toProcess;					
				do		
				{			
					// we use this instead of the + operator, because we avoid 4 constructor calls this way
					CHECK_VERTEX_BUFFER(vb, currVertex);
					((CVector *) currVertex)->x = (*posIt).x  + *ptSize * currBasis->X.x;  			
					((CVector *) currVertex)->y = (*posIt).y  + *ptSize * currBasis->X.y;  			
					((CVector *) currVertex)->z = (*posIt).z  + *ptSize * currBasis->X.z;  			
					currVertex += vSize;

					CHECK_VERTEX_BUFFER(vb, currVertex);
					((CVector *) currVertex)->x = (*posIt).x  + *ptSize * currBasis->Y.x;  			
					((CVector *) currVertex)->y = (*posIt).y  + *ptSize * currBasis->Y.y;  			
					((CVector *) currVertex)->z = (*posIt).z  + *ptSize * currBasis->Y.z;  			
					currVertex += vSize;

					CHECK_VERTEX_BUFFER(vb, currVertex);
					((CVector *) currVertex)->x = (*posIt).x  - *ptSize * currBasis->X.x;  			
					((CVector *) currVertex)->y = (*posIt).y  - *ptSize * currBasis->X.y;  			
					((CVector *) currVertex)->z = (*posIt).z  - *ptSize * currBasis->X.z;  			
					currVertex += vSize;

					CHECK_VERTEX_BUFFER(vb, currVertex);
					((CVector *) currVertex)->x = (*posIt).x  - *ptSize * currBasis->Y.x;  			
					((CVector *) currVertex)->y = (*posIt).y  - *ptSize * currBasis->Y.y;  			
					((CVector *) currVertex)->z = (*posIt).z  - *ptSize * currBasis->Y.z;  			
					currVertex += vSize;
					ptSize += ptSizeIncrement;						
					++posIt;
					currBasis += ptPlaneBasisIncrement;
				}
				while (posIt != endPosIt);
				driver->renderQuads(f._Mat, 0, toProcess);
				leftFaces -= toProcess;
			}
			while (leftFaces);
		}
		PARTICLES_CHECK_MEM;	
	}
};

///======================================================================================
CPSFace::CPSFace(CSmartPtr<ITexture> tex) : CPSQuad(tex)
{   
	_Name = std::string("Face");	
}

///======================================================================================
void CPSFace::step(TPSProcessPass pass, TAnimationTime ellapsedTime)
{
	if (pass == PSToolRender) // edition mode only
	{			
		showTool();
		return;
	}
	else if (pass == PSMotion)
	{
		
		if (_PrecompBasis.size()) // do we use precomputed basis ?
		{			
			// rotate all precomputed basis		
			for (std::vector< CPlaneBasisPair >::iterator it = _PrecompBasis.begin(); it != _PrecompBasis.end(); ++it)
			{
				// not optimized at all, but this will apply to very few elements anyway...
				CMatrix mat;
				mat.rotate(CQuat(it->Axis, ellapsedTime * it->AngularVelocity));
				CVector n = mat * it->Basis.getNormal();
				it->Basis = CPlaneBasis(n);
			}
		}
		return;
	}
	else	// check this is the right pass
	if (!
		(	(pass == PSBlendRender && hasTransparentFaces())
			|| (pass == PSSolidRender && hasOpaqueFaces())
		)
	   )
	{
		return;
	}
	
	

	if (!_Owner->getSize()) return;	
	uint32 step;
	uint   numToProcess;
	computeSrcStep(step, numToProcess);	
	if (!numToProcess) return;

	
	if (step == (1 << 16))
	{
		/// build index iterator
		std::vector<uint32>::const_iterator indexIt = _IndexInPrecompBasis.begin();

		/// draw the faces
		CPSFaceHelper::drawFaces(_Owner->getPos().begin(),	
								 indexIt,
								 *this,
								 numToProcess,
								 step
								);
	}
	else
	{		
		/// build index iterator
		CAdvance1616Iterator<std::vector<uint32>::const_iterator, const uint32>			
			indexIt(_IndexInPrecompBasis.begin(), 0, step);
		CPSFaceHelper::drawFaces(TIteratorVectStep1616(_Owner->getPos().begin(), 0, step),
								 indexIt,
								 *this,
								 numToProcess,
								 step				
								);
	}
	
}


///======================================================================================
void CPSFace::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);
	CPSQuad::serial(f);
	CPSRotated3DPlaneParticle::serialPlaneBasisScheme(f);

	if (f.isReading())
	{
		uint32 nbConfigurations;
		f.serial(nbConfigurations);
		if (nbConfigurations)
		{
			f.serial(_MinAngularVelocity, _MaxAngularVelocity);		
		}
		hintRotateTheSame(nbConfigurations, _MinAngularVelocity, _MaxAngularVelocity);

		init();		
	}
	else	
	{				
		uint32 nbConfigurations = _PrecompBasis.size();
		f.serial(nbConfigurations);
		if (nbConfigurations)
		{
			f.serial(_MinAngularVelocity, _MaxAngularVelocity);		
		}
	}
}
	
	
///======================================================================================
/// this produce a random unit vector
static CVector MakeRandomUnitVect(void)	
{
	CVector v((float) ((rand() % 20000) - 10000)
			  ,(float) ((rand() % 20000) - 10000)
			  ,(float) ((rand() % 20000) - 10000)
			  );
	v.normalize();
	return v;
}

///======================================================================================
void CPSFace::hintRotateTheSame(uint32 nbConfiguration
						, float minAngularVelocity
						, float maxAngularVelocity
					  )
{
	_MinAngularVelocity = minAngularVelocity;
	_MaxAngularVelocity = maxAngularVelocity;
	_PrecompBasis.resize(nbConfiguration);
	if (nbConfiguration)
	{
		// each precomp basis is created randomly;
		for (uint k = 0; k < nbConfiguration; ++k)
		{
			 CVector v = MakeRandomUnitVect();
			_PrecompBasis[k].Basis = CPlaneBasis(v);
			_PrecompBasis[k].Axis = MakeRandomUnitVect();
			_PrecompBasis[k].AngularVelocity = minAngularVelocity 
											   + (rand() % 20000) / 20000.f * (maxAngularVelocity - minAngularVelocity);

		}	
		// we need to do this because nbConfs may have changed
		fillIndexesInPrecompBasis();
	}
}

///======================================================================================
void CPSFace::fillIndexesInPrecompBasis(void)
{
	const uint32 nbConf = _PrecompBasis.size();
	if (_Owner)
	{
		_IndexInPrecompBasis.resize( _Owner->getMaxSize() );
	}	
	for (std::vector<uint32>::iterator it = _IndexInPrecompBasis.begin(); it != _IndexInPrecompBasis.end(); ++it)
	{
		*it = rand() % nbConf;
	}
}

///======================================================================================
void CPSFace::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	CPSQuad::newElement(emitterLocated, emitterIndex);
	newPlaneBasisElement(emitterLocated, emitterIndex);
	const uint32 nbConf = _PrecompBasis.size();
	if (nbConf) // do we use precomputed basis ?
	{
		_IndexInPrecompBasis[_Owner->getNewElementIndex()] = rand() % nbConf;
	}	
}
	
///======================================================================================
	void CPSFace::deleteElement(uint32 index)
{
	CPSQuad::deleteElement(index);
	deletePlaneBasisElement(index);
	if (_PrecompBasis.size()) // do we use precomputed basis ?
	{
		// replace ourself by the last element...
		_IndexInPrecompBasis[index] = _IndexInPrecompBasis[_Owner->getSize() - 1];
	}	
}

///======================================================================================
void CPSFace::resize(uint32 size)
{
	nlassert(size < (1 << 16));
	resizePlaneBasis(size);
	if (_PrecompBasis.size()) // do we use precomputed basis ?
	{
		_IndexInPrecompBasis.resize(size);
	}
	CPSQuad::resize(size);
}

} // NL3D
