/** \file driver_direct3d_profile.cpp
 * Direct 3d driver implementation
 *
 * $Id: driver_direct3d_profile.cpp,v 1.3 2004/08/09 14:45:18 vizerie Exp $
 *
 * \todo manage better the init/release system (if a throw occurs in the init, we must release correctly the driver)
 */

/* Copyright, 2000 Nevrax Ltd.
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

#include "stddirect3d.h"

#include "driver_direct3d.h"
#include "nel/misc/hierarchical_timer.h"

using namespace std;
using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************

void CDriverD3D::profileRenderedPrimitives(CPrimitiveProfile &pIn, CPrimitiveProfile &pOut) 
{
	pIn= _PrimitiveProfileIn;
	pOut= _PrimitiveProfileOut;
}

// ***************************************************************************

uint32 CDriverD3D::profileAllocatedTextureMemory() 
{
	return _AllocatedTextureMemory;
}

// ***************************************************************************

uint32 CDriverD3D::profileSetupedMaterials() const 
{
	return _NbSetupMaterialCall;
}

// ***************************************************************************

uint32 CDriverD3D::profileSetupedModelMatrix() const 
{
	return _NbSetupModelMatrixCall;
}

// ***************************************************************************

void CDriverD3D::enableUsedTextureMemorySum (bool enable) 
{
	if (enable)
		nlinfo ("PERFORMANCE INFO: enableUsedTextureMemorySum has been set to true in CDriverD3D\n");
	_SumTextureMemoryUsed=enable;
}

// ***************************************************************************

uint32 CDriverD3D::getUsedTextureMemory() const 
{
	// Sum memory used
	uint32 memory=0;

	// For each texture used
	set<CTextureDrvInfosD3D*>::iterator ite=_TextureUsed.begin();
	while (ite!=_TextureUsed.end())
	{
		// Get the d3d texture
		CTextureDrvInfosD3D*	d3dtext;
		d3dtext= (*ite);

		// Sum the memory used by this texture
		memory+=d3dtext->TextureMemory;

		// Next texture
		ite++;
	}

	// Return the count
	return memory;
}

// ***************************************************************************
void CDriverD3D::startProfileVBHardLock() 
{
	if(_VBHardProfiling)
		return;

	// start
	_VBHardProfiles.clear();
	_VBHardProfiles.reserve(50);
	_VBHardProfiling= true;
	_CurVBHardLockCount= 0;
	_NumVBHardProfileFrame= 0;
}

// ***************************************************************************

void CDriverD3D::endProfileVBHardLock(std::vector<std::string> &result) 
{
	if(!_VBHardProfiling)
		return;

	// Fill infos.
	result.clear();
	result.resize(_VBHardProfiles.size() + 1);
	float	total= 0;
	for(uint i=0;i<_VBHardProfiles.size();i++)
	{
		const	uint tmpSize= 256;
		char	tmp[tmpSize];
		CVBHardProfile	&vbProf= _VBHardProfiles[i];
		const char	*vbName;
		if(vbProf.VBHard && !vbProf.VBHard->getName().empty())
		{
			vbName= vbProf.VBHard->getName().c_str();
		}
		else
		{
			vbName= "????";
		}
		// Display in ms.
		float	timeLock= (float)CTime::ticksToSecond(vbProf.AccumTime)*1000 / max(_NumVBHardProfileFrame,1U);
		smprintf(tmp, tmpSize, "%16s%c: %2.3f ms", vbName, vbProf.Change?'*':' ', timeLock );
		total+= timeLock;

		result[i]= tmp;
	}
	result[_VBHardProfiles.size()]= toString("Total: %2.3f", total);
	
	// clear.
	_VBHardProfiling= false;
	contReset(_VBHardProfiles);
}

// ***************************************************************************

void CDriverD3D::appendVBHardLockProfile(NLMISC::TTicks time, CVertexBuffer *vb)
{
	// must allocate a new place?
	if(_CurVBHardLockCount>=_VBHardProfiles.size())
	{
		_VBHardProfiles.resize(_VBHardProfiles.size()+1);
		// set the original VBHard
		_VBHardProfiles[_CurVBHardLockCount].VBHard= vb;
	}

	// Accumulate.
	_VBHardProfiles[_CurVBHardLockCount].AccumTime+= time;
	// if change of VBHard for this chrono place
	if(_VBHardProfiles[_CurVBHardLockCount].VBHard != vb)
	{
		// flag, and set new
		_VBHardProfiles[_CurVBHardLockCount].VBHard= vb;
		_VBHardProfiles[_CurVBHardLockCount].Change= true;
	}

	// next!
	_CurVBHardLockCount++;
}

// ***************************************************************************
void CDriverD3D::startProfileIBLock() 
{
	/*
	if(_IBProfiling)
		return;
	
	// start
	_IBProfiles.clear();
	_IBProfiles.reserve(50);
	_IBProfiling= true;
	_CurIBLockCount= 0;
	_NumIBProfileFrame= 0;
	*/
}

// ***************************************************************************

void CDriverD3D::endProfileIBLock(std::vector<std::string> &result) 
{
	/*
	if(!_IBProfiling)
		return;
	
	// Fill infos.
	result.clear();
	result.resize(_IBProfiles.size() + 1);
	float	total= 0;
	for(uint i=0;i<_IBProfiles.size();i++)
	{
		const	uint tmpSize= 256;
		char	tmp[tmpSize];
		CIBProfile	&ibProf= _IBProfiles[i];
		const char	*ibName;
		if(ibProf.IB && !ibProf.IB->getName().empty())
		{
			ibName= ibProf.IB->getName().c_str();
		}
		else
		{
			ibName= "????";
		}
		// Display in ms.
		float	timeLock= (float)CTime::ticksToSecond(ibProf.AccumTime)*1000 / max(_NumIBProfileFrame,1U);
		smprintf(tmp, tmpSize, "%16s%c: %2.3f ms", ibName, ibProf.Change?'*':' ', timeLock );
		total+= timeLock;
		
		result[i]= tmp;
	}
	result[_IBProfiles.size()]= toString("Total: %2.3f", total);
	nlwarning("IB lock time = %2.3f", total);
	
	// clear.
	_IBProfiling= false;
	contReset(_IBProfiles);
	*/
}

// ***************************************************************************

void CDriverD3D::appendIBLockProfile(NLMISC::TTicks time, CIndexBuffer *ib)
{
	/*
	// must allocate a new place?
	if(_CurIBLockCount>=_IBProfiles.size())
	{
		_IBProfiles.resize(_IBProfiles.size()+1);		
		_IBProfiles[_CurIBLockCount].IB= ib;
	}
	
	// Accumulate.
	_IBProfiles[_CurIBLockCount].AccumTime+= time;
	// if change of VBHard for this chrono place
	if(_IBProfiles[_CurIBLockCount].IB != ib)
	{
		// flag, and set new
		_IBProfiles[_CurIBLockCount].IB= ib;
		_IBProfiles[_CurIBLockCount].Change= true;
	}
	
	// next!
	_CurIBLockCount++;
	*/
}

// ***************************************************************************
void CDriverD3D::profileVBHardAllocation(std::vector<std::string> &result)
{
	result.clear();
	result.reserve(1000);
	result.push_back(toString("Memory Allocated: %4d Ko in AGP / %4d Ko in VRAM", 
		getAvailableVertexAGPMemory()/1000, getAvailableVertexVRAMMemory()/1000 ));
	result.push_back(toString("Num VBHard: %d", _VertexBufferHardSet.size()));

	uint	totalMemUsed= 0;
	set<CVBDrvInfosD3D*>::iterator	it;
	for(it= _VertexBufferHardSet.begin(); it!=_VertexBufferHardSet.end(); it++)
	{
		CVBDrvInfosD3D	*vbHard= *it;
		if(vbHard)
		{
			uint	vSize= vbHard->VertexBufferPtr->getVertexSize();
			uint	numVerts= vbHard->VertexBufferPtr->getNumVertices();
			totalMemUsed+= vSize*numVerts;
		}
	}
	result.push_back(toString("Mem Used: %4d Ko", totalMemUsed/1000) );
	
	for(it= _VertexBufferHardSet.begin(); it!=_VertexBufferHardSet.end(); it++)
	{
		CVBDrvInfosD3D	*vbHard= *it;
		if(vbHard)
		{
			uint	vSize= vbHard->VertexBufferPtr->getVertexSize();
			uint	numVerts= vbHard->VertexBufferPtr->getNumVertices();
			result.push_back(toString("  %16s: %4d ko (format: %d / numVerts: %d)", 
				vbHard->VertexBufferPtr->getName().c_str(), vSize*numVerts/1000, vSize, numVerts ));
		}
	}
}

// ***************************************************************************
void CDriverD3D::profileIBAllocation(std::vector<std::string> &result)
{
	result.clear();
	result.reserve(1000);
	result.push_back(toString("Memory Allocated: %4d Ko in AGP / %4d Ko in VRAM", 
		getAvailableVertexAGPMemory()/1000, getAvailableVertexVRAMMemory()/1000 ));
	result.push_back(toString("Num Index buffers : %d", _IBDrvInfos.size()));
	
	uint	totalMemUsed= 0;
	for(TIBDrvInfoPtrList::iterator it = _IBDrvInfos.begin(); it != _IBDrvInfos.end(); ++it)
	{
		CIBDrvInfosD3D	*ib =  NLMISC::safe_cast<CIBDrvInfosD3D	*>(*it);
		if(ib)
		{			
			uint	numIndex= ib->IndexBufferPtr->getNumIndexes();
			totalMemUsed+= sizeof(uint32)*numIndex;
		}
	}
	result.push_back(toString("Mem Used: %4d Ko", totalMemUsed/1000) );
	
	for(TIBDrvInfoPtrList::iterator it = _IBDrvInfos.begin(); it != _IBDrvInfos.end(); ++it)
	{		
		CIBDrvInfosD3D	*ib =  NLMISC::safe_cast<CIBDrvInfosD3D	*>(*it);
		if(ib)		
		{			
			uint	numIndex= ib->IndexBufferPtr->getNumIndexes();			
			result.push_back(toString("  %16s: %4d ko ", 
				ib->IndexBufferPtr->getName().c_str(), sizeof(uint32) * numIndex));
		}
	}
}

// ***************************************************************************

void CDriverD3D::startBench (bool wantStandardDeviation, bool quick, bool reset)
{
	CHTimer::startBench (wantStandardDeviation, quick, reset);
}

// ***************************************************************************

void CDriverD3D::endBench ()
{
	CHTimer::endBench ();
}

// ***************************************************************************

void CDriverD3D::displayBench (class NLMISC::CLog *log)
{
	// diplay
	CHTimer::displayHierarchicalByExecutionPathSorted(log, CHTimer::TotalTime, true, 48, 2);
	CHTimer::displayHierarchical(log, true, 48, 2);
	CHTimer::displayByExecutionPath(log, CHTimer::TotalTime);
	CHTimer::display(log, CHTimer::TotalTime);
	CHTimer::display(log, CHTimer::TotalTimeWithoutSons);
}

// ***************************************************************************

} // NL3D
