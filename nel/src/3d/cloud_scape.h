/** \file cloud.h
 * Cloud definition to work with the cloudscape
 *
 * $Id: cloud_scape.h,v 1.5 2004/04/08 09:05:45 corvazier Exp $
 */

/* Copyright, 2002 Nevrax Ltd.
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

#ifndef NL_CLOUD_SCAPE_H
#define NL_CLOUD_SCAPE_H

// ------------------------------------------------------------------------------------------------

#include "3d/material.h"
#include "3d/texture.h"
#include "3d/texture_mem.h"
#include "3d/vertex_buffer.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/geom_ext.h"
#include "nel/misc/value_smoother.h"
#include "cloud.h"
#include "noise_3d.h"
#include "nel/3d/u_cloud_scape.h"

namespace NL3D
{

// ------------------------------------------------------------------------------------------------
struct SCloudTexture3D
{
	uint32 Width, Height, Depth;
	uint32 NbW, NbH; // Number of slice in width and height (NbW*NbH = Depth)
	uint8									*Mem;
	uint8									*Mem2;
	uint8									*MemBuffer;
	NLMISC::CSmartPtr<NL3D::CTextureMem>	Tex;
	NLMISC::CSmartPtr<NL3D::CTextureMem>	Tex2;
	NLMISC::CSmartPtr<NL3D::CTextureMem>	TexBuffer;
	NL3D::CMaterial							ToLightRGB;
	NL3D::CMaterial							ToLightAlpha;
	NL3D::CMaterial							ToBill;
	NL3D::CMaterial							MatCopy;

	SCloudTexture3D ();
	void init (uint32 nWidth, uint32 nHeight, uint32 nDepth);
};

// ------------------------------------------------------------------------------------------------
struct SCloudTextureClamp
{
	uint32 Width, Height, Depth;
	uint32 NbW, NbH; // Number of slice in width and height (NbW*NbH = Depth)
	uint8									*Mem;
	NLMISC::CSmartPtr<NL3D::CTextureMem>	Tex;
	NL3D::CMaterial							ToClamp;

	SCloudTextureClamp ();
	void init (uint32 nWidth, uint32 nHeight, uint32 nDepth, const std::string &filename);
};

// ------------------------------------------------------------------------------------------------
class CCloudScape
{
public:

	CCloudScape (NL3D::IDriver *pDriver);

	~CCloudScape ();

	void init (SCloudScapeSetup *pCSS = NULL, NL3D::CCamera *pCamera = NULL);

	void set (SCloudScapeSetup &css);

	// Function that make cloud scape (work with screeen as temp buffer)
	void anim (double dt, NL3D::CCamera *pCamera);

	// Render all clouds to the screen
	void render ();

	uint32 getMemSize();

	void setQuality (float threshold) { _LODQualityThreshold = threshold; }

	void setNbCloudToUpdateIn80ms (uint32 n) { _NbHalfCloudToUpdate = n; }

	bool isDebugQuadEnabled () { return _DebugQuad; }
	void setDebugQuad (bool b) { _DebugQuad = b; }

private:

	void makeHalfCloud ();

private:

	uint32 _NbHalfCloudToUpdate; // In 40 ms
	double _GlobalTime;
	double _DeltaTime;
	double _DTRest;
	NLMISC::CValueSmoother _AverageFrameRate;
	bool _Generate; // or light ?
	CCloud *_CurrentCloudInProcess; // Current cloud that is processing 
	double _CurrentCloudInProcessFuturTime;

	std::vector<uint8> _CloudPower;
	std::vector<bool> _ShouldProcessCloud;

	SCloudScapeSetup _CurrentCSS;
	SCloudScapeSetup _NewCSS;
	SCloudScapeSetup _OldCSS;
	double _TimeNewCSS;

	bool				_IsIncomingCSS;
	SCloudScapeSetup	_IncomingCSS;
	
	CNoise3d	_Noise3D;

	std::vector<CCloud>		_AllClouds;

	// Cloud scheduler
	struct SCloudSchedulerEntry
	{
		sint32 CloudIndex;
		sint32 DeltaNextCalc;
		uint32 Frame;
		NLMISC::CRGBA Ambient;
		NLMISC::CRGBA Diffuse;
		uint8 Power;
		SCloudSchedulerEntry()
		{
			CloudIndex = DeltaNextCalc = -1;
		}
	};
	struct SCloudSchedulerAccel
	{
		bool	ValidPos;
		std::list<SCloudSchedulerEntry>::iterator Pos;
//		uint32	Frame;

		SCloudSchedulerAccel()
		{
			ValidPos = false;
		}
	};
//	std::deque<SCloudSchedulerEntry>	_CloudScheduler;
	std::list<SCloudSchedulerEntry>	_CloudScheduler;
	uint32 _CloudSchedulerSize;
	std::vector<SCloudSchedulerAccel>	_CloudSchedulerLastAdded;
	uint32 _FrameCounter;
	std::vector<float> _ExtrapolatedPriorities;


	// Cloud sort
	struct SSortedCloudEntry
	{
		CCloud *Cloud;
		float Distance;
	};
	std::vector<SSortedCloudEntry> _SortedClouds;



	float _LODQualityThreshold;
	bool _DebugQuad;

	NL3D::IDriver			*_Driver;
	NL3D::CVertexBuffer		_VertexBuffer;
	NL3D::CMaterial			_MatClear;
	NL3D::CMaterial			_MatBill;

	NL3D::CCamera			*_ViewerCam;

	SCloudTexture3D			Tex3DTemp;
	SCloudTextureClamp		TexClamp;

	// Driver reset counter initial value
	uint					_ResetCounter;

	friend class CCloud;
};

// ------------------------------------------------------------------------------------------------
} // namespace NL3D

#endif // NL_CLOUD_SCAPE_H

