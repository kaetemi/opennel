/** \file ps_int.h
 * <File description>
 *
 * $Id: ps_int.h,v 1.3 2001/07/12 15:45:56 vizerie Exp $
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

#ifndef NL_PS_INT_H
#define NL_PS_INT_H

#include "nel/misc/types_nl.h"
#include "3d/ps_attrib_maker_template.h"
#include "3d/ps_attrib_maker_bin_op.h"


namespace NL3D {


/// these are some attribute makers for int

/// This is a int blender class. It just blend between 2 values

class CPSIntBlender : public CPSValueBlender<sint32>
{
public:
	NLMISC_DECLARE_CLASS(CPSIntBlender) ;
	CPSIntBlender(sint32 startInt = 0 , sint32 endInt = 10, float nbCycles = 1.0f) : CPSValueBlender<sint32>(nbCycles)
	{
		_F.setValues(startInt, endInt) ;
	}
	
	// F is serialized by base classes...

} ;

class CPSUIntBlender : public CPSValueBlender<uint32>
{
public:
	NLMISC_DECLARE_CLASS(CPSUIntBlender) ;
	CPSUIntBlender(uint32 startInt = 0 , uint32 endInt = 10, float nbCycles = 1.0f) : CPSValueBlender<uint32>(nbCycles)
	{
		_F.setValues(startInt, endInt) ;
	}	
	// F is serialized by base classes...

} ;







/// This is a int gradient class
class CPSIntGradient : public CPSValueGradient<sint32>
{
public:
	NLMISC_DECLARE_CLASS(CPSIntGradient) ;

	/**	
	 *	Construct the value gradient blender by passing a pointer to a float table.
	 *  \param nbStages The result is sampled into a table by linearly interpolating values. This give the number of step between each value
	 * \param nbCycles : The nb of time the pattern is repeated during particle life. see ps_attrib_maker.h
	 */

	CPSIntGradient(const sint32 *intTab = CPSIntGradient::_DefaultGradient
						, uint32 nbValues = 2, uint32 nbStages = 10, float nbCycles = 1.0f) ;

	static sint32 _DefaultGradient[] ;
	
	// F is serialized by base classes...	

} ;

class CPSUIntGradient : public CPSValueGradient<uint32>
{
public:
	NLMISC_DECLARE_CLASS(CPSUIntGradient) ;

	/**	
	 *	Construct the value gradient blender by passing a pointer to a float table.
	 *  \param nbStages The result is sampled into a table by linearly interpolating values. This give the number of step between each value
	 * \param nbCycles : The nb of time the pattern is repeated during particle life. see ps_attrib_maker.h
	 */

	CPSUIntGradient(const uint32 *intTab = CPSUIntGradient::_DefaultGradient
						, uint32 nbValues = 2, uint32 nbStages = 10, float nbCycles = 1.0f) ;

	static uint32 _DefaultGradient[] ;
	
	// F is serialized by base classes...	

} ;

/** this memorize value by applying some function on the emitter. For a particle's attribute, each particle has its
  * own value memorized
  *  You MUST called setScheme (from CPSAttribMakerMemory) to tell how the value will be generted
  */
class CPSIntMemory : public CPSAttribMakerMemory<sint32>
{
public:
	NLMISC_DECLARE_CLASS(CPSIntMemory) ;
	CPSIntMemory() { setDefaultValue(0) ; }
} ;


/** this memorize value by applying some function on the emitter. For a particle's attribute, each particle has its
  * own value memorized
  *  You MUST called setScheme (from CPSAttribMakerMemory) to tell how the value will be generted
  */
class CPSUIntMemory : public CPSAttribMakerMemory<uint32>
{
public:
	CPSUIntMemory() { setDefaultValue(0) ; }
	NLMISC_DECLARE_CLASS(CPSUIntMemory) ;
} ;


/** An attribute maker whose output if the result of a binary op on uint32
  *
  */
class CPSIntBinOp : public CPSAttribMakerBinOp<sint32>
{
public:
	NLMISC_DECLARE_CLASS(CPSIntBinOp) ;
} ;


/** An attribute maker whose output if the result of a binary op on uint32
  *
  */
class CPSUIntBinOp : public CPSAttribMakerBinOp<uint32>
{
public:
	NLMISC_DECLARE_CLASS(CPSUIntBinOp) ;
} ;






} // NL3D


#endif // NL_PS_INT_H

/* End of ps_int.h */
