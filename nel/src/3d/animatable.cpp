/** \file animatable.cpp
 * Class IAnimatable
 *
 * $Id: animatable.cpp,v 1.7 2001/07/03 09:46:22 corvazier Exp $
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

#include "3d/animatable.h"
#include "3d/channel_mixer.h"


namespace NL3D 
{

// ***************************************************************************

void	IAnimatable::addValue(CChannelMixer *chanMixer, uint valueId, uint ownerValueId, const std::string &prefix, bool detail)
{
	nlassert(chanMixer);
	chanMixer->addChannel(prefix+getValueName(valueId), this, getValue(valueId), getDefaultTrack(valueId), valueId, ownerValueId, detail);
}

// ***************************************************************************

} // NL3D
