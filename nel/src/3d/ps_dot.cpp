/** \file ps_dot.cpp
 * Dot particles
 *
 * $Id: ps_dot.cpp,v 1.10 2004/08/13 15:40:43 vizerie Exp $
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

#include "3d/ps_dot.h"
#include "3d/ps_macro.h"
#include "3d/ps_iterator.h"
#include "3d/driver.h"
#include "3d/particle_system.h"
#include "nel/misc/fast_mem.h"

namespace NL3D 
{

static const uint dotBufSize = 1024; // size used for point particles batching


///////////////////////////
// CPSDot implementation //
///////////////////////////

/// static members
CVertexBuffer CPSDot::_DotVb;
CVertexBuffer CPSDot::_DotVbColor;


///===================================================================
template <class T>
inline void DrawDot(T it,
					CVertexBuffer &vb,
					CPSAttribMaker<NLMISC::CRGBA> *colorScheme,
					uint leftToDo,
					CPSLocated *owner,
					CMaterial &mat,
					IDriver *driver,
					uint32 srcStep
				   )
{		
	nlassert(leftToDo != 0);
	const uint total = leftToDo;
	T itEnd;
	if (colorScheme)
	{	
		// compute the colors
		colorScheme->setColorType(driver->getVertexColorFormat());
	}
	do
	{		
		uint toProcess = leftToDo < dotBufSize ? leftToDo : dotBufSize;		
		vb.setNumVertices(toProcess); // because of volatile vb copy, indicate the numebr of vertices to copy
		{
			CVertexBufferReadWrite vba;
			vb.lock (vba);
			if (colorScheme)
			{			
				// compute the colors			
				colorScheme->make(owner,
								  total - leftToDo,
								  vba.getColorPointer(),
								  vb.getVertexSize(),
								  toProcess,
								  false,
								  srcStep
								 );

				itEnd = it + toProcess;			
				uint8    *currPos = (uint8 *) vba.getVertexCoordPointer();	
				uint32 stride = vb.getVertexSize();
				do
				{
					CHECK_VERTEX_BUFFER(vb, currPos);
					*((CVector *) currPos) =  *it;	
					++it ;
					currPos += stride;
				}
				while (it != itEnd);
			}
			else if (srcStep == (1 << 16)) // make sure we haven't got auto-lod and that the step is 1.0
			{
				// there's no color information in the buffer, so we can copy it directly
				NLMISC::CFastMem::memcpy(vba.getVertexCoordPointer(), &(*it), sizeof(NLMISC::CVector) * toProcess);
				it += toProcess;
			}
			else
			{
				itEnd = it + toProcess;			
				uint8    *currPos = (uint8 *) vba.getVertexCoordPointer();				
				do
				{
					CHECK_VERTEX_BUFFER(vb, currPos);
					*((CVector *) currPos) =  *it;				
					++it ;
					currPos += sizeof(float[3]);
				}
				while (it != itEnd);
			}
		}
		driver->activeVertexBuffer(vb);				
		driver->renderRawPoints(mat, 0, toProcess);

		leftToDo -= toProcess;
	}
	while (leftToDo);
}


///===================================================================
void CPSDot::draw(bool opaque)
{	
	PARTICLES_CHECK_MEM;	
	if (!_Owner->getSize()) return;	

	uint32 step;
	uint   numToProcess;
	computeSrcStep(step, numToProcess);	
	if (!numToProcess) return;

	_Owner->incrementNbDrawnParticles(numToProcess); // for benchmark purpose		
	setupDriverModelMatrix();	
	IDriver *driver = getDriver();
	CVertexBuffer &vb = _ColorScheme ? _DotVbColor : _DotVb;
	


	/// update the material if the global color of the system is variable
	CParticleSystem &ps = *(_Owner->getOwner());
	if (_ColorScheme == NULL)
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
		_Mat.setColor(col);		
		forceTexturedMaterialStages(0);
	}
	else
	{			
		forceTexturedMaterialStages(1);
		if (ps.getForceGlobalColorLightingFlag() || usesGlobalColorLighting())
		{
			_Mat.texConstantColor(0, ps.getGlobalColorLighted());
		}
		else
		{
			_Mat.texConstantColor(0, ps.getGlobalColor());
		}
		SetupModulatedStage(_Mat, 0, CMaterial::Diffuse, CMaterial::Constant);
	}
	//////



	// Use the right drawing routine (auto-lod and non auto-lod)
	if (step == (1 << 16))
	{
		DrawDot(_Owner->getPos().begin(),
				vb,
				_ColorScheme,			    
				numToProcess,
			    _Owner,
				_Mat,
				driver,
				step
			   );
	}
	else
	{		
		DrawDot(TIteratorVectStep1616(_Owner->getPos().begin(), 0, step),
				vb,
				_ColorScheme,			    
				numToProcess,
			    _Owner,
				_Mat,
				driver,
				step
			   );
	}
	
	PARTICLES_CHECK_MEM;
}


///===================================================================
/// init the vertex buffers
void CPSDot::initVertexBuffers()
{
	_DotVb.setName("CPSDot::_DotVb");
	_DotVb.setPreferredMemory(CVertexBuffer::AGPVolatile, false);
	_DotVb.setVertexFormat(CVertexBuffer::PositionFlag);
	_DotVb.setNumVertices(dotBufSize);	
	_DotVbColor.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag);
	_DotVbColor.setPreferredMemory(CVertexBuffer::AGPVolatile, true); // keep local mem because of interleaved fill
	_DotVbColor.setNumVertices(dotBufSize);
	_DotVbColor.setName("CPSDot::_DotVbColor");
	
}

///===================================================================
void CPSDot::init(void)
{		
	_Mat.setLighting(false);	
	_Mat.setZFunc(CMaterial::less);
	
	updateMatAndVbForColor();
}

///===================================================================
uint32 CPSDot::getNumWantedTris() const
{
	nlassert(_Owner);
	//return _Owner->getMaxSize();
	return _Owner->getSize();
}

///===================================================================
void CPSDot::newElement(const CPSEmitterInfo &info)
{
	newColorElement(info);
}

///===================================================================
void CPSDot::deleteElement(uint32 index)
{
	deleteColorElement(index);
}

///===================================================================
void CPSDot::updateMatAndVbForColor(void)
{	
}

///===================================================================
bool CPSDot::hasTransparentFaces(void)
{
	return getBlendingMode() != CPSMaterial::alphaTest ;
}

///===================================================================
bool CPSDot::hasOpaqueFaces(void)
{
	return !hasTransparentFaces();
}

///===================================================================
void CPSDot::resize(uint32 size)
{	
	nlassert(size < (1 << 16));
	resizeColor(size);
}

///===================================================================
void CPSDot::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	
	f.serialVersion(1);	


	CPSParticle::serial(f);
	CPSColoredParticle::serialColorScheme(f);
	serialMaterial(f);
	if (f.isReading())
	{
		init();		
	}
}

} // NL3D
