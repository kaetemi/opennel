/** \file particle_system_process.cpp
 * <File description>
 *
 * $Id: particle_system_process.cpp,v 1.7 2004/07/21 13:28:27 vizerie Exp $
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

#include "3d/particle_system_process.h"
#include "3d/particle_system.h"

namespace NL3D {

/////////////////////////////////////////////
// CParticleSystemProcess implementation   //
/////////////////////////////////////////////

//***********************************************************************************************************
void CParticleSystemProcess::setOwner(CParticleSystem *ps)
{
	if (ps == _Owner) return;
	if (ps == NULL)
	{
		releaseAllRef();		
	}	
	if (_Owner) _Owner->releaseRefForUserSysCoordInfo(getUserMatrixUsageCount());		
	_Owner = ps; 
	if (_Owner) _Owner->addRefForUserSysCoordInfo(getUserMatrixUsageCount());
}


//***********************************************************************************************************
uint CParticleSystemProcess::getUserMatrixUsageCount() const
{
	return _MatrixMode == PSUserMatrix;
}

//***********************************************************************************************************
void CParticleSystemProcess::setMatrixMode(TPSMatrixMode matrixMode)
{
	nlassert((uint) matrixMode <= PSMatrixModeCount);
	if (matrixMode == _MatrixMode) return;
	if (_Owner) // notify the system that matrix mode has changed for that object
	{
		_Owner->matrixModeChanged(this, _MatrixMode, matrixMode);
	}
	_MatrixMode = matrixMode;
}


//***********************************************************************************************************
CFontGenerator *CParticleSystemProcess::getFontGenerator(void)
{
			nlassert(_Owner);
			return _Owner->getFontGenerator();
}

//***********************************************************************************************************
const CFontGenerator *CParticleSystemProcess::getFontGenerator(void) const 
{
			nlassert(_Owner);
			return _Owner->getFontGenerator();
}

//***********************************************************************************************************
CFontManager *CParticleSystemProcess::getFontManager(void)
{
			nlassert(_Owner);
			return _Owner->getFontManager();
}

//***********************************************************************************************************
const CFontManager *CParticleSystemProcess::getFontManager(void) const 
{
			nlassert(_Owner);
			return _Owner->getFontManager();
}



//***********************************************************************************************************
void CParticleSystemProcess::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{	
	// version 2 : added matrix mode (just not fx world matrix or identity)
	// version 1 : base version
	sint ver = f.serialVersion(2);
	f.serialPtr(_Owner);	
	if (ver == 1)
	{	
		nlassert(f.isReading());
		bool usesFXWorldMatrix;
		f.serial(usesFXWorldMatrix);
		_MatrixMode = usesFXWorldMatrix ? PSFXWorldMatrix : PSIdentityMatrix;
	}
	if (ver >= 2)
	{
		f.serialEnum(_MatrixMode);
	}
}


} // NL3D
