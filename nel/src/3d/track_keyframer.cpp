/** \file track_keyframer.cpp
 * <File description>
 *
 * $Id: track_keyframer.cpp,v 1.7 2003/02/03 16:51:33 coutelas Exp $
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

#include "3d/track_keyframer.h"


namespace NL3D
{

template <class T>
void ITrackKeyFramer<T>::getKeysInRange(TAnimationTime t1, TAnimationTime t2, std::vector<TAnimationTime> &result)
{
	nlassert(t1 <= t2);
	typename TMapTimeCKey::const_iterator it = _MapKey.lower_bound(t1);
	typename TMapTimeCKey::const_iterator ub = _MapKey.upper_bound(t2);
	result.clear();		
	while (it != ub)
	{			
		result.push_back(it->first);
		++it;
	}
}





// Some compilation check: force Visual to compile to template
CTrackKeyFramerTCBFloat ttoto0;
CTrackKeyFramerTCBVector ttoto1;
CTrackKeyFramerTCBQuat ttoto2;
CTrackKeyFramerTCBInt ttoto3;
CTrackKeyFramerTCBRGBA ttoto8;


CTrackKeyFramerBezierFloat ttoto4;
CTrackKeyFramerBezierVector ttoto5;
CTrackKeyFramerBezierQuat ttoto6;
CTrackKeyFramerBezierInt ttoto7;
CTrackKeyFramerBezierRGBA ttoto9;


CTrackKeyFramerLinearFloat	lattoto10;
CTrackKeyFramerLinearVector	lattoto11;
CTrackKeyFramerLinearQuat	lattoto12;
CTrackKeyFramerLinearInt	lattoto13;
CTrackKeyFramerLinearRGBA	lattoto14;


CTrackKeyFramerConstFloat	attoto10;
CTrackKeyFramerConstVector	attoto11;
CTrackKeyFramerConstQuat	attoto12;
CTrackKeyFramerConstInt		attoto13;
CTrackKeyFramerConstRGBA	attoto16;
CTrackKeyFramerConstString	attoto14;
CTrackKeyFramerConstBool	attoto15;


} // NL3D
