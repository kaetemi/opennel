/** \file track_keyframer.h
 * Definition of TrackKeyframer.
 *
 * $Id: track_keyframer.h,v 1.3 2001/05/09 17:08:27 berenguier Exp $
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

#ifndef NL_TRACK_KEYFRAMER_H
#define NL_TRACK_KEYFRAMER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/track.h"
#include "nel/3d/key.h"
#include <map>
#include <memory>
#include "nel/misc/matrix.h"


namespace NL3D
{


// ***************************************************************************
// ***************************************************************************
// Templates for KeyFramer tracks.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
/**
 * ITrack interface for keyframer.
 *
 * The ITrack and animated value types MUST match else assertions will be raised.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class CKeyT>
class ITrackKeyFramer : public ITrack
{
public:
	// Some types
	typedef std::map <CAnimationTime, CKeyT>	TMapTimeCKey;


	/// ctor.
	ITrackKeyFramer ()
	{
		_Dirty= false;
		_RangeLock= true;
		_LoopMode= false;
	}


	/// Destructor
	~ITrackKeyFramer ()
	{
	}

	/**
	  * Add a key in the keyframer.
	  *
	  * The key passed is duplicated in the track.
	  *
	  * \param key is the key value to add in the keyframer.
	  * \param time is the time of the key to add in the keyframer.
	  */
	void addKey (const CKeyT &key, CAnimationTime time)
	{
		// Insert the key in the map
		_MapKey.insert (TMapTimeCKey::value_type (time, key));

		// must precalc at next eval.
		_Dirty= true;
	}

	/// set an explicit animation range. (see getBeginTime() / setEndTime() ).
	void	unlockRange(CAnimationTime begin, CAnimationTime end)
	{
		_RangeLock= false;
		_RangeBegin= begin;
		_RangeEnd= end;
		_Dirty= true;
	}

	/// range is computed from frist and last key time (default).
	void	lockRange()
	{
		_RangeLock= true;
		_Dirty= true;
	}

	/// return true if Range is locked to first/last key. use getBeginTime and getEndTime to get the effective begin/end range times...
	bool	isRangeLocked() const {return _RangeLock;}


	/// rangeDelta is (length of effective Range) - (length of LastKey-FirstKey). NB: if RangeLock, rangeDelta==0.
	CAnimationTime	getRangeDelta() const
	{
		// update track.
		testAndClean();

		return _RangeDelta;
	}


	/// set LoopMode. 2 mode only: "constant" (<=>false), and "loop" (<=> true). same mode for in and out...
	void	setLoopMode(bool loop) {_LoopMode= loop; _Dirty= true;}

	/// get LoopMode.
	bool	getLoopMode() const {return _LoopMode;}


	/// From ITrack. 
	virtual void eval (const CAnimationTime& inDate)
	{
		float	date= inDate;
		const CKeyT *previous=NULL;
		const CKeyT *next=NULL;
		CAnimationTime datePrevious;
		CAnimationTime dateNext;

		// must precalc ??
		testAndClean();

		// No keys?
		if(_MapKey.empty())
			return;


		// Loop gestion.
		if(_LoopMode && _MapKey.size()>1 )
		{
			nlassert(_LoopEnd > _LoopStart);

			// force us to be in interval [_LoopStart, _LoopEnd[.
			if( date<_LoopStart || date>=_LoopEnd )
			{
				double	d= (date-_LoopStart)*_OOTotalRange;

				// floor(d) is the truncated number of loops.
				d= date- floor(d)*_TotalRange;
				date= (float)d;

				// For precision problems, ensure correct range.
				if(date<_LoopStart || date >= _LoopEnd)
					date= _LoopStart;
			}
		}


		// Return upper key
		TMapTimeCKey::iterator ite=_MapKey.upper_bound (date);

		// First next ?
		if (ite!=_MapKey.end())
		{
			// Next
			next= &(ite->second);
			dateNext=ite->first;
		}
		// loop mgt.
		else if	(_LoopMode && _MapKey.size()>1 )
		{
			// loop to first!!
			next= &(_MapKey.begin()->second);
			// must slerp from last to first, 
			dateNext= _LoopEnd;
		}


		// First previous ?
		if ((!_MapKey.empty())&&(ite!=_MapKey.begin()))
		{
			// Previous
			ite--;
			previous= &(ite->second);
			datePrevious=ite->first;
		}

		// Call evalutation fonction
		evalKey (previous, next, datePrevious, dateNext, date);
	}


	virtual CAnimationTime getBeginTime () const
	{
		// must precalc ??
		testAndClean();

		return _RangeBegin;
	}
	virtual CAnimationTime getEndTime () const
	{
		// must precalc ??
		testAndClean();

		return _RangeEnd;
	}


	/// Serial the template
	virtual void serial (NLMISC::IStream& f) throw (NLMISC::EStream)
	{
		// Serial version
		sint version=f.serialVersion (0);

		f.serialCont(_MapKey);
		f.serial(_RangeLock, _RangeBegin, _RangeEnd);
		f.serial(_LoopMode);

		if(f.isReading())
			_Dirty= true;
	}

private:
	mutable	bool		_Dirty;
	bool				_LoopMode;
	bool				_RangeLock;
	float				_RangeBegin;	// if RangeLock==true, valid only when track cleaned.
	float				_RangeEnd;		// if RangeLock==true, valid only when track cleaned.
	// Valid only when cleaned.
	float				_RangeDelta;
	float				_LoopStart;
	float				_LoopEnd;
	float				_TotalRange;
	float				_OOTotalRange;


	// update track if necessary.
	void		testAndClean() const
	{
		if(_Dirty)
		{
			ITrackKeyFramer<CKeyT>	*self= const_cast<ITrackKeyFramer<CKeyT>*>(this);
			self->compile();
			_Dirty= false;
		}
	}


protected:
	TMapTimeCKey		_MapKey;


	/// This is for Deriver compile(), because _RangeDelta (getRangeDelta()) is himself computed in compile().
	float	getCompiledRangeDelta()
	{
		return _RangeDelta;
	}


	/**
	  * Precalc keyframe runtime infos for interpolation (OODTime...). All keys should be processed.
	  * This is called by eval when necessary. Deriver should call ITrackKeyFramer::compile() first, to compile basic
	  * Key runtime info.
	  */
	virtual void compile   ()
	{
		float	timeFirstKey;
		float	timeLastKey;

		// Compute time of first/last key.
		if( !_MapKey.empty() )
		{
			TMapTimeCKey::const_iterator ite;

			// Get first key
			ite=_MapKey.begin ();
			timeFirstKey= ite->first;

			// Get last key
			ite=_MapKey.end ();
			ite--;
			timeLastKey= ite->first;
		}
		else
		{
			timeFirstKey= 0.0f;
			timeLastKey= 0.0f;
		}


		// Compute RangeBegin / RangeEnd. (if not user provided).
		if(_RangeLock)
		{
			_RangeBegin= timeFirstKey;
			_RangeEnd= timeLastKey;
		}


		// Compute _RangeDelta.
		if(_RangeLock)
		{
			_RangeDelta= 0;
		}
		else
		{
			_RangeDelta= (_RangeEnd - _RangeBegin) - (timeLastKey - timeFirstKey);
		}

		// Misc range.
		_TotalRange= _RangeEnd - _RangeBegin;
		if(_TotalRange>0.0f)
			_OOTotalRange= 1.0f/_TotalRange;
		// start of loop / ned.
		_LoopStart= timeFirstKey;
		_LoopEnd= timeFirstKey + _TotalRange;


		// After _RangeDelta computed, compute OO delta times.
		TMapTimeCKey::iterator	it= _MapKey.begin();
		for(;it!=_MapKey.end();it++)
		{
			TMapTimeCKey::iterator	next= it;
			next++;
			if(next!=_MapKey.end())
				it->second.OODeltaTime= 1.0f/(next->first - it->first);
			else if(_RangeDelta>0.0f)
				// after last key, must slerp to first key.
				it->second.OODeltaTime= 1.0f/_RangeDelta;
			else
				it->second.OODeltaTime= 0.0f;
		}

	}

	/**
	  * Evaluate the keyframe interpolation. 
	  *
	  * i is the keyframe with the bigger time value that is inferior or equal than date.
	  *
	  * \param previous is the i key in the keyframe. NULL if no key.
	  * \param next is the i+1 key in the keyframe. NULL if no key.
	  */
	virtual void evalKey   (const CKeyT* previous, const CKeyT* next,
							CAnimationTime datePrevious, CAnimationTime dateNext, 
							CAnimationTime date ) =0;

};


// ***************************************************************************
// Key Tools.
// separated to just change this in RGBA and sint32 special implementation.

// just copy the content of a value issued from key interpolation, into a value.
template<class T, class TKeyVal> inline void	copyToValue(T &value, const TKeyVal &keyval)
{
	value = keyval;
}


// Vector to RGBA version.
inline void	copyToValue(NLMISC::CRGBA &col, const CVector &v)
{
	sint	i;

	i= (sint)(v.x*255); NLMISC::clamp(i,0,255); col.R= (uint8) i;
	i= (sint)(v.y*255); NLMISC::clamp(i,0,255); col.G= (uint8) i;
	i= (sint)(v.z*255); NLMISC::clamp(i,0,255); col.B= (uint8) i;
	col.A=255;
}


// float to sint32 version.
inline void	copyToValue(sint32 &value, const float &f)
{
	value= (sint32)floor(f+0.5f);
}


// ***************************************************************************
// ***************************************************************************
// Constant Keyframer.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
/**
 * ITrack implementation for Constant keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class CKeyT, class T>
class CTrackKeyFramerConstNotBlendable : public ITrackKeyFramer<CKeyT>
{
public:

	/// From ITrack
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}
	
	/// From ITrackKeyFramer
	virtual void evalKey (	const CKeyT* previous, const CKeyT* next,
							CAnimationTime datePrevious, CAnimationTime dateNext,
							CAnimationTime date )
	{
		// Const key.
		if (previous)
			copyToValue(_Value.Value, previous->Value);
		else
			if (next)
				copyToValue(_Value.Value, next->Value);
	}

private:
	CAnimatedValueNotBlendable<T>		_Value;
};


// ***************************************************************************
/**
 * ITrack implementation for Constant keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class CKeyT, class T>
class CTrackKeyFramerConstBlendable : public ITrackKeyFramer<CKeyT>
{
public:

	/// From ITrack
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}
	
	/// From ITrackKeyFramer
	virtual void evalKey (	const CKeyT* previous, const CKeyT* next,
							CAnimationTime datePrevious, CAnimationTime dateNext,
							CAnimationTime date )
	{
		// Const key.
		if (previous)
			copyToValue(_Value.Value, previous->Value);
		else
			if (next)
				copyToValue(_Value.Value, next->Value);
	}

private:
	CAnimatedValueBlendable<T>		_Value;
};


// ***************************************************************************
// ***************************************************************************
// Linear Keyframer.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
/**
 * ITrack implementation for linear keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class CKeyT, class T>
class CTrackKeyFramerLinear : public ITrackKeyFramer<CKeyT>
{
public:

	/// From ITrack
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}
	
	/// From ITrackKeyFramer
	virtual void evalKey (	const CKeyT* previous, const CKeyT* next,
							CAnimationTime datePrevious, CAnimationTime dateNext,
							CAnimationTime date )
	{
		if(previous && next)
		{
			// lerp from previous to cur.
			date-= datePrevious;
			date*= previous->OODeltaTime;
			NLMISC::clamp(date, 0,1);
			
			// NB: in case of <CKeyInt,sint32> important that second terme is a float, so copyToValue(sint32, float) is used.
			copyToValue(_Value.Value, previous->Value*(1.f-(float)date) + next->Value*(float)date);
		}
		else
		{
			if (previous)
				copyToValue(_Value.Value, previous->Value);
			else
				if (next)
					copyToValue(_Value.Value, next->Value);
		}

	}

private:
	CAnimatedValueBlendable<T>	_Value;
};



// ***************************************************************************
/**
 * Quaternions special implementation for linear keyframer.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CTrackKeyFramerLinear<CKeyQuat, CQuat> : public ITrackKeyFramer<CKeyQuat>
{
public:

	/// From ITrack
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}
	
	/// From ITrackKeyFramer
	virtual void evalKey (	const CKeyQuat* previous, const CKeyQuat* next, 
							CAnimationTime datePrevious, CAnimationTime dateNext,
							CAnimationTime date )
	{
		if(previous && next)
		{
			// slerp from previous to cur.
			date-= datePrevious;
			date*= previous->OODeltaTime;
			NLMISC::clamp(date, 0,1);
			_Value.Value= CQuat::slerp(previous->Value, next->Value, date);
		}
		else
		{
			if (previous)
				_Value.Value=previous->Value;
			else
				if (next)
					_Value.Value=next->Value;
		}
	}

private:
	CAnimatedValueBlendable<CQuat>	_Value;
};


// ***************************************************************************
/**
 * ITrack implementation for linear CRGBA keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CTrackKeyFramerLinear<CKeyRGBA, NLMISC::CRGBA>: public ITrackKeyFramer<CKeyRGBA>
{
public:

	/// From ITrack
	virtual const IAnimatedValue& getValue () const
	{
		return _Value;
	}
	
	/// From ITrackKeyFramer
	virtual void evalKey (	const CKeyRGBA* previous, const CKeyRGBA* next,
							CAnimationTime datePrevious, CAnimationTime dateNext,
							CAnimationTime date )
	{
		if(previous && next)
		{
			// lerp from previous to cur.
			date-= datePrevious;
			date*= previous->OODeltaTime;
			NLMISC::clamp(date, 0,1);
			
			// blend.
			_Value.Value.blendFromui(previous->Value, next->Value, (uint)(date*255));
		}
		else
		{
			if (previous)
				_Value.Value= previous->Value;
			else
				if (next)
					_Value.Value= next->Value;
		}
	}

private:
	CAnimatedValueBlendable<NLMISC::CRGBA>	_Value;
};



// ***************************************************************************
// ***************************************************************************
// TCB / Bezier Keyframer.
// ***************************************************************************
// ***************************************************************************


// Template implementation for TCB and Bezier.
#include "track_tcb.h"
#include "track_bezier.h"



// ***************************************************************************
// ***************************************************************************
// Predefined types for KeyFramer tracks.
// ***************************************************************************
// ***************************************************************************


// Const tracks.
class CTrackKeyFramerConstFloat : public CTrackKeyFramerConstBlendable<CKeyFloat,float>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstFloat);
};
class CTrackKeyFramerConstVector : public CTrackKeyFramerConstBlendable<CKeyVector, CVector>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstVector);
};
class CTrackKeyFramerConstQuat : public CTrackKeyFramerConstBlendable<CKeyQuat, CQuat>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstQuat);
};
class CTrackKeyFramerConstInt : public CTrackKeyFramerConstBlendable<CKeyInt, sint32>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstInt);
};
class CTrackKeyFramerConstRGBA : public CTrackKeyFramerConstBlendable<CKeyRGBA, NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstRGBA);
};

class CTrackKeyFramerConstString : public CTrackKeyFramerConstNotBlendable<CKeyString, std::string>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstString);
};
class CTrackKeyFramerConstBool : public CTrackKeyFramerConstNotBlendable<CKeyBool, bool>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstBool);
};


// Linear tracks.
class CTrackKeyFramerLinearFloat : public CTrackKeyFramerLinear<CKeyFloat, float>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerLinearFloat);
};
class CTrackKeyFramerLinearVector : public CTrackKeyFramerLinear<CKeyVector, CVector>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerLinearVector);
};
class CTrackKeyFramerLinearQuat : public CTrackKeyFramerLinear<CKeyQuat, CQuat>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerLinearQuat);
};
class CTrackKeyFramerLinearInt : public CTrackKeyFramerLinear<CKeyInt, sint32>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerLinearInt);
};
class CTrackKeyFramerLinearRGBA : public CTrackKeyFramerLinear<CKeyRGBA, NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerLinearRGBA);
};


// TCB tracks.
class CTrackKeyFramerTCBFloat : public CTrackKeyFramerTCB<CKeyTCBFloat, float>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerTCBFloat);
};
class CTrackKeyFramerTCBVector : public CTrackKeyFramerTCB<CKeyTCBVector, CVector>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerTCBVector);
};
class CTrackKeyFramerTCBQuat : public CTrackKeyFramerTCB<CKeyTCBQuat, NLMISC::CAngleAxis>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerTCBQuat);
};
class CTrackKeyFramerTCBInt : public CTrackKeyFramerTCB<CKeyTCBFloat, sint32>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerTCBInt);
};
class CTrackKeyFramerTCBRGBA : public CTrackKeyFramerTCB<CKeyTCBVector, NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerTCBRGBA);
};


// Bezier tracks.
class CTrackKeyFramerBezierFloat : public CTrackKeyFramerBezier<CKeyBezierFloat, float>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerBezierFloat);
};
class CTrackKeyFramerBezierVector : public CTrackKeyFramerBezier<CKeyBezierVector, CVector>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerBezierVector);
};
class CTrackKeyFramerBezierQuat : public CTrackKeyFramerBezier<CKeyBezierQuat, CQuat>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerBezierQuat);
};
class CTrackKeyFramerBezierInt : public CTrackKeyFramerBezier<CKeyBezierFloat, sint32>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerBezierInt);
};
class CTrackKeyFramerBezierRGBA : public CTrackKeyFramerBezier<CKeyBezierVector, NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerBezierRGBA);
};





} // NL3D


#endif // NL_TRACK_KEYFRAMER_H

/* End of track_keyframer.h */
