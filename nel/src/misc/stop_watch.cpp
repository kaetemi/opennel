/** \file stop_watch.cpp
 * Performance stopwatch
 *
 * $Id: stop_watch.cpp,v 1.1 2001/11/27 14:03:26 cado Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "nel/misc/stop_watch.h"
#include <numeric>

using namespace std;

namespace NLMISC {


/*
 * Constructor
 */
CStopWatch::CStopWatch( uint queueLength ) :
	_BeginTime( 0 ),
	_ElapsedTicks( 0 ),
	_SumTicks( 0 ),
	_MeasurementNumber( 0 ),
	_Queue(),
	_QLength( queueLength )
{}


/*
 * Begin measurement
 */
void	CStopWatch::start()
{
	_BeginTime = CTime::getPerformanceTime();
	_ElapsedTicks = 0;
}


/*
 * Pause
 */
void	CStopWatch::pause()
{
	_ElapsedTicks += (TTickDuration)(CTime::getPerformanceTime() - _BeginTime);
}


/*
 * Resume
 */
void	CStopWatch::resume()
{
	_BeginTime = CTime::getPerformanceTime();
}


/*
 * Add time (in TTicks unit) to the current measurement
 */
void	CStopWatch::addTime( TTickDuration t )
{
	_ElapsedTicks += t;
}


/*
 * End measurement
 */
void	CStopWatch::stop()
{
	_ElapsedTicks += (TTickDuration)(CTime::getPerformanceTime() - _BeginTime);

	// Setup average
	_SumTicks += _ElapsedTicks;
	++_MeasurementNumber;

	// Setup partial average
	if ( _QLength != 0 )
	{
		_Queue.push_back( _ElapsedTicks );
		if ( _Queue.size() > _QLength )
		{
			_Queue.pop_front();
		}
	}
}


/*
 * Add an external duration (in TTicks unit) to the average queue
 */
void	CStopWatch::addMeasurement( TTickDuration t )
{
	// Setup average
	_SumTicks += t;
	++_MeasurementNumber;

	// Setup partial average
	if ( _QLength != 0 )
	{
		_Queue.push_back( t );
		if ( _Queue.size() > _QLength )
		{
			_Queue.pop_front();
		}
	}

}


/*
 * Elapsed time in millisecond (call it after stop())
 */
TMsDuration	CStopWatch::getDuration() const
{
	return (TMsDuration)(CTime::ticksToSecond( _ElapsedTicks ) * 1000.0);
}


/*
 * Average of the queueLength last durations (using the queueLength argument specified in the constructor)
 */
TMsDuration	CStopWatch::getPartialAverage() const
{
	return (TMsDuration)(CTime::ticksToSecond( accumulate( _Queue.begin(), _Queue.end(), 0 ) / _Queue.size() ) * 1000.0);
}


/*
 * Average of the duration
 */
TMsDuration	CStopWatch::getAverageDuration() const
{
	return (TMsDuration)(CTime::ticksToSecond( _SumTicks / _MeasurementNumber ) * 1000.0);
}



} // NLMISC
