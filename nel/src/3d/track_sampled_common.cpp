/** \file track_sampled_common.cpp
 * <File description>
 *
 * $Id: track_sampled_common.cpp,v 1.2 2002/08/21 09:39:54 lecroart Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#include "nel/misc/quat.h"
#include "nel/misc/common.h"
#include "nel/misc/algo.h"
#include "3d/track_sampled_common.h"
#include "nel/misc/vectord.h"

using namespace NLMISC;
using namespace std;


namespace NL3D 
{


// ***************************************************************************
// ***************************************************************************
// CTrackSampledCommon
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CTrackSampledCommon::CTrackSampledCommon()
{
	_LoopMode= true;
}

// ***************************************************************************
CTrackSampledCommon::~CTrackSampledCommon()
{
}

// ***************************************************************************
bool					CTrackSampledCommon::getLoopMode() const
{
	return _LoopMode;
}

// ***************************************************************************
TAnimationTime			CTrackSampledCommon::getBeginTime () const
{
	return _BeginTime;
}

// ***************************************************************************
TAnimationTime			CTrackSampledCommon::getEndTime () const
{
	return _EndTime;
}


// ***************************************************************************
void					CTrackSampledCommon::CTimeBlock::serial(NLMISC::IStream &f)
{
	(void)f.serialVersion(0);

	f.serial(TimeOffset);
	f.serial(KeyOffset);
	f.serial(Times);
}

// ***************************************************************************
void					CTrackSampledCommon::serialCommon(NLMISC::IStream &f)
{
	(void)f.serialVersion(0);

	f.serial(_LoopMode);
	f.serial(_BeginTime);
	f.serial(_EndTime) ;
	f.serial(_TotalRange);
	f.serial(_OOTotalRange);
	f.serial(_DeltaTime);
	f.serial(_OODeltaTime);
	f.serial(_TimeBlocks);

}

// ***************************************************************************
void	CTrackSampledCommon::buildCommon(const std::vector<uint16> &timeList, float beginTime, float endTime)
{
	nlassert( endTime>beginTime || (beginTime==endTime && timeList.size()<=1) );
	uint i;

	// reset.
	uint	numKeys= timeList.size();
	_TimeBlocks.clear();

	// Special case of 0 or 1 key.
	//===================
	if(numKeys<=1)
	{
		_BeginTime= beginTime;
		_EndTime= endTime;
		_TotalRange= 0;
		_OOTotalRange= 0;
		_DeltaTime= 0;
		_OODeltaTime= 0;
		if(numKeys==1)
		{
			_TimeBlocks.resize(1);
			_TimeBlocks[0].TimeOffset= 0;
			_TimeBlocks[0].Times.resize(1);
			_TimeBlocks[0].Times[0]= 0;
		}

		return;
	}

	// Compute All Time blocks.
	//===================
	sint32		lastBlockFrame= -1000000;
	nlassert(timeList[0] == 0);
	// Header info for creating timeBlocks
	vector<uint>	timeBlockKeyId;
	vector<uint>	timeBlockNumKeys;

	// compute how many time block we need.
	for(i=0; i<numKeys; i++)
	{
		// verify growing order, and time difference.
		if(i>0)
		{
			nlassert(timeList[i]>timeList[i-1]);
			nlassert(timeList[i]-timeList[i-1] <= 255 );
		}
		// If the current frame is to far from the last TimeBlock frame (or if 1st timeBlock), must create a new timeBlock
		if(timeList[i]-lastBlockFrame>255)
		{
			// create a new timeblock
			timeBlockKeyId.push_back(i);
			// Add this key to this new time Block (numKey == 1).
			timeBlockNumKeys.push_back(1);
			lastBlockFrame= timeList[i];
		}
		else
		{
			// Add this key to the timeBlock.
			timeBlockNumKeys[timeBlockNumKeys.size()-1]++;
		}
	}

	// Build the timeBlocks.
	_TimeBlocks.resize(timeBlockKeyId.size());
	for(i=0; i<timeBlockKeyId.size(); i++)
	{
		CTimeBlock	&timeBlock= _TimeBlocks[i];
		uint		firstKeyId= timeBlockKeyId[i];
		uint		numKeys= timeBlockNumKeys[i];
		// compute the offset time and key
		timeBlock.KeyOffset= firstKeyId;
		timeBlock.TimeOffset= timeList[firstKeyId];
		// create array of key
		timeBlock.Times.resize(numKeys);
		for(uint j=0;j<timeBlock.Times.size(); j++)
		{
			// get the key time and make it local to the timeBlock.
			timeBlock.Times[j]= timeList[firstKeyId+j] - timeBlock.TimeOffset;
		}
	}

	// Compute other params
	//===================
	_BeginTime= beginTime;
	_EndTime= endTime;
	// compute deltatime for a frame to an other
	uint	totalFrameCount= timeList[numKeys-1] - timeList[0];
	nlassert(totalFrameCount>0);
	_DeltaTime= (_EndTime-_BeginTime) / totalFrameCount;
	_OODeltaTime= (float)(1.0 / _DeltaTime);
	// Compute range of anim
	_TotalRange= _EndTime-_BeginTime;
	_OOTotalRange= float(1.0/_TotalRange);

}

// ***************************************************************************
void	CTrackSampledCommon::setLoopMode(bool mode)
{
	_LoopMode= mode;
}


// ***************************************************************************
CTrackSampledCommon::TEvalType	CTrackSampledCommon::evalTime (const TAnimationTime& date, uint numKeys, uint &keyId0, uint &keyId1, float &interpValue)
{
	// Empty? quit
	if(numKeys==0)
		return EvalDiscard;

	// One Key? easy, and quit.
	if(numKeys==1)
	{
		keyId0= 0;
		return EvalKey0;
	}

	// manage Loop
	//=====================
	float	localTime;
	if(_LoopMode)
	{
		nlassert(_TotalRange>0);
		// get relative to BeginTime.
		localTime= date-_BeginTime;

		// force us to be in interval [0, _TotalRange[.
		if( localTime<0 || localTime>=_TotalRange )
		{
			double	d= localTime*_OOTotalRange;

			// floor(d) is the truncated number of loops.
			d= localTime- floor(d)*_TotalRange;
			localTime= (float)d;

			// For precision problems, ensure correct range.
			if(localTime<0 || localTime >= _TotalRange)
				localTime= 0;
		}

	}
	else
	{
		// get relative to BeginTime.
		localTime= date-_BeginTime;
	}


	// Find the first key before localTime
	//=====================
	// get the frame in the track.
	sint	frame= (sint)floor(localTime*_OODeltaTime);
	// clamp to uint16
	clamp(frame, 0, 65535);

	// Search the TimeBlock.
	CTimeBlock	keyTB;
	keyTB.TimeOffset= frame;
	uint		tbId;
	tbId= searchLowerBound(_TimeBlocks.getPtr(), _TimeBlocks.size(), keyTB);

	// get this timeBlock.
	CTimeBlock	&timeBlock= _TimeBlocks[tbId];
	// get frame relative to this timeBlock.
	sint	frameRel= frame-timeBlock.TimeOffset;
	// clamp to uint8
	clamp(frameRel, 0, 255);
	// get the key in this timeBlock.
	uint	keyIdRel;
	keyIdRel= searchLowerBound(timeBlock.Times.getPtr(), timeBlock.Times.size(), (uint8)frameRel);
	
	// Get the Frame and Value of Key0.
	uint		frameKey0= timeBlock.TimeOffset + timeBlock.Times[keyIdRel];
	// this is the key to evaluate
	keyId0= timeBlock.KeyOffset + keyIdRel;


	// Interpolate with next key
	//=====================

	// If not the last Key
	if(keyId0<numKeys-1)
	{
		// Get the next key.
		keyId1= keyId0+1;
		uint		frameKey1;
		// If last key of the timeBlock, get the first time of the next timeBlock.
		if( keyIdRel+1 >= timeBlock.Times.size() )
		{
			nlassert(tbId+1<_TimeBlocks.size());
			frameKey1= _TimeBlocks[tbId+1].TimeOffset;
		}
		else
		{
			frameKey1= timeBlock.TimeOffset + timeBlock.Times[keyIdRel+1];
		}

		// unpack time.
		float	time0= frameKey0*_DeltaTime;
		float	time1= frameKey1*_DeltaTime;

		// interpolate.
		float	t= (localTime-time0);
		// If difference is one frame, optimize.
		if(frameKey1-frameKey0==1)
			t*= _OODeltaTime;
		else
			t/= (time1-time0);
		clamp(t, 0.f, 1.f);

		// store this interp value.
		interpValue= t;

		return EvalInterpolate;
	}
	// else (last key of anim), just eval this key.
	return EvalKey0;
}


} // NL3D
