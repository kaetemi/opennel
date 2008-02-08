/**
 * \file time.h
 * \brief CTime
 * \date 2008-02-07 17:21GMT
 * \author Jan Boon (Kaetemi)
 * CTime
 * 
 * $Id$
 */

/* 
 * Copyright (C) 2008  Jan Boon (Kaetemi)
 * 
 * This file is part of OpenNeL Snowballs.
 * OpenNeL Snowballs is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 * 
 * OpenNeL Snowballs is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with OpenNeL Snowballs; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef SBCLIENT_TIME_H
#define SBCLIENT_TIME_H
#include <nel/misc/types_nl.h>

// Project includes
#include "member_callback_decl.h"

// NeL includes
#include <nel/misc/time_nl.h>

// component specific
#include <nel/3d/animation_time.h>

// STL includes

namespace NLMISC {
	class IProgressCallback;
}

namespace SBCLIENT {

/**
 * \brief CTime
 * \date 2008-02-07 17:21GMT
 * \author Jan Boon (Kaetemi)
 * Synchronize time.
 */
class CTime
{
protected:
public:
	NLMISC::TLocalTime LocalTime; // use for differences only
	NLMISC::TLocalTime ServerTime; // use for differences only
	NLMISC::TGameTime GameTime;
	NLMISC::TGameCycle GameCycle;
	NLMISC::TLocalTime LocalTimeDelta;
	NLMISC::TLocalTime ServerTimeDelta;
	NLMISC::TGameTime GameTimeDelta;
	float FramesPerSecond;

	// component specific
	NL3D::TGlobalAnimationTime AnimationTime;
	NL3D::TGlobalAnimationTime AnimationDelta;
protected:
	// pointers
	// ...
	
	// instances
	NLMISC::TTime _Time;
	NLMISC::TLocalTime _SecondsPerCycle; // changes with update
	NLMISC::TGameTime _GameTimePerCycle; // stays the same
	NLMISC::TGameTime _GameTimePerSecond; // stays the same
	NLMISC::TGameCycle _CyclesPerUpdate; // stays the same
	NLMISC::TLocalTime _LastCycleUpdate; // updated when update detects cycle up
	double _GameCycle; // goes up as long as it's not too high
	NLMISC::TGameCycle _NewGameCycle; // set by updateGameCycle

	// component specific
	bool _AnimateServer;
public:
	CTime(NLMISC::IProgressCallback &progressCallback);
	virtual ~CTime();

	/// Set the game cycle configuration.
	/// This is done by the online or offline component.
	void setGameCycle(NLMISC::TGameCycle gameCycle, NLMISC::TLocalTime secondsPerCycle = 0.1, NLMISC::TGameTime gameTimePerSecond = 60.0, NLMISC::TGameCycle cyclesPerUpdate = 2);
	
	/// Update the game cycle. This needs to be called every time it goes up.
	/// New cycle is sent by server trough network or generated by offline.
	void updateGameCycle(NLMISC::TGameCycle gameCycle);

	/// Update the time
	SBCLIENT_CALLBACK_DECL(updateTime);
}; /* class CTime */

} /* namespace SBCLIENT */

#endif /* #ifndef SBCLIENT_TIME_H */

/* end of file */