/** \file track_bezier.h
 * ITrack Bezier implementation
 *
 * $Id: track_bezier.h,v 1.3 2004/04/07 09:51:56 berenguier Exp $
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
#error "internal file included from track_keyframer.h"
#endif



// ***************************************************************************
// ***************************************************************************
// Bezier Keyframer.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
/**
 * ITrack implementation for Bezier keyframer.
 *
 * \author Lionel Berneguier
 * \author Nevrax France
 * \date 2001
 */
template<class CKeyT, class T>
class CTrackKeyFramerBezier : public ITrackKeyFramer<CKeyT>
{
public:

protected:

	typedef typename CKeyT::TValueType		TKeyValueType;


	/// \name From ITrackKeyFramer
	// @{

	/// evalKey (runtime).
	virtual void evalKey (	const CKeyT* previous, const CKeyT* next,
							TAnimationTime datePrevious, TAnimationTime dateNext,
							TAnimationTime date, IAnimatedValue &result )
	{
		CAnimatedValueBlendable<T>	&resultVal= static_cast<CAnimatedValueBlendable<T>&>(result);
		
		if(previous && next && !previous->Step)
		{
			// lerp from previous to cur.
			date-= datePrevious;
			date*= previous->OODeltaTime;
			NLMISC::clamp(date, 0,1);

			// Bezier interpolation.
			float s= date;
			float s2 = s * s;
			float s3 = s2 * s;
			float u = 1.0f - s;
			float u2 = u * u;
			float u3 = u2 * u;

			// compute Bezier control points from tangents.
			TKeyValueType	cp0, cp1;

			// NB: loop case: dateNext is always > datePrevious....
			cp0 =	previous->Value + previous->OutTan * (dateNext-datePrevious) / 3.0f;
			cp1 =	next->Value + next->InTan * (dateNext-datePrevious) / 3.0f;
			
			copyToValue(resultVal.Value, previous->Value*u3 + cp0*3.0f*u2*s
		 		+ cp1*3.0f*u*s2 + next->Value*s3);
		}
		else
		{
			if (previous)
				copyToValue(resultVal.Value, previous->Value);
			else
				if (next)
					copyToValue(resultVal.Value, next->Value);
		}
	}

	/// compile (precalc).
	virtual void compile()
	{
		ITrackKeyFramer<CKeyT>::compile();

		// Nothing else to do!! Tangents are given from user.
	}

	// @}

};



// ***************************************************************************
/**
 * ITrack implementation for Bezier Quaternion keyframer.
 *
 * \author Lionel Berneguier
 * \author Nevrax France
 * \date 2001
 */
class CTrackKeyFramerBezier<CKeyBezierQuat, CQuat> : public ITrackKeyFramer<CKeyBezierQuat>
{
public:

protected:

	/// \name From ITrackKeyFramer
	// @{

	/// evalKey (runtime).
	virtual void evalKey (	const CKeyBezierQuat* previous, const CKeyBezierQuat* next,
							TAnimationTime datePrevious, TAnimationTime dateNext,
							TAnimationTime date, IAnimatedValue &result )
	{
		CAnimatedValueQuat	&resultVal= static_cast<CAnimatedValueQuat&>(result);
		
		if(previous && next)
		{
			// lerp from previous to cur.
			date-= datePrevious;
			date*= previous->OODeltaTime;
			NLMISC::clamp(date, 0,1);

			// quad slerp.
			resultVal.Value = CQuat::squad(previous->Value, previous->A, next->A, next->Value, date);	
		}
		else
		{
			if (previous)
				copyToValue(resultVal.Value, previous->Value);
			else
				if (next)
					copyToValue(resultVal.Value, next->Value);
		}
	}

	/// compile (precalc).
	virtual void compile()
	{
		ITrackKeyFramer<CKeyBezierQuat>::compile();

		// makeclosest quaternions, Tangents Precompute.
		sint	nKeys= _MapKey.size();
		if(nKeys<=1)
			return;

		TMapTimeCKey::iterator	it;
		TMapTimeCKey::iterator	itNext;
		TMapTimeCKey::iterator	itPrev;

		it= _MapKey.begin();				// first key.
		itNext= it;	itNext++;				// second key.
		itPrev= _MapKey.end();				// end key.

		// Compute all keys.
		for(;it!=_MapKey.end();)
		{

			CKeyBezierQuat	&key= it->second;
			CQuat	&cur= key.Value;

			if(itPrev!= _MapKey.end())
			{
				cur.makeClosest(itPrev->second.Value);
			}

			CQuat	prev, next;

			// compute prev / next.
			if(itPrev!= _MapKey.end())
				prev= itPrev->second.Value;
			else
				prev= itNext->second.Value;
			if(itNext!= _MapKey.end())
				next= itNext->second.Value;
			else
				next= itPrev->second.Value;

			// Compute A.
			CQuat qm,qp,r;

			qm = CQuat::lnDif(cur, prev);
			qp = CQuat::lnDif(cur, next);
			r = -.25f*(qm+qp);
			key.A= cur*(r.exp());

			// Next key!!
			itPrev= it;
			it++;

			if(itNext!= _MapKey.end())
				itNext++;
		}

	}

	// @}

};
