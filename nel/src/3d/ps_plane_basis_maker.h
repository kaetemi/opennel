/** \file plane_basis_maker.h
 * <File description>
 *
 * $Id: ps_plane_basis_maker.h,v 1.4 2001/07/12 15:41:27 vizerie Exp $
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

#ifndef NL_PLANE_BASIS_MAKER_H
#define NL_PLANE_BASIS_MAKER_H

#include "nel/misc/types_nl.h"
#include "3d/ps_attrib_maker_template.h"
#include "3d/ps_attrib_maker_bin_op.h"
#include "3d/ps_plane_basis.h"


namespace NL3D {



/** these are some attribute makers for plane_basis
 * This is a plane basis class. It just blend between 2 plane by linearly interpolating the normal
 * (non sampled version)
 */

class CPSPlaneBasisBlender : public CPSValueBlender<CPlaneBasis>
{
public:
	NLMISC_DECLARE_CLASS(CPSPlaneBasisBlender) ;

	CPSPlaneBasisBlender(const CPlaneBasis &startBasis = CPlaneBasis(NLMISC::CVector::I), const CPlaneBasis &endBasis = CPlaneBasis(NLMISC::CVector::J), float nbCycles = 1.0f) : CPSValueBlender<CPlaneBasis>(nbCycles)
	{
		_F.setValues(startBasis, endBasis) ;
	}
	
	// F is serialized by base classes...

} ;


/// This is a PlaneBasis gradient class
class CPSPlaneBasisGradient : public CPSValueGradient<CPlaneBasis>
{
public:
	NLMISC_DECLARE_CLASS(CPSPlaneBasisGradient) ;

	/**	
	 *	Construct the value gradient blender by passing a pointer to a float table.
	 *  \param nbStages The result is sampled into a table by linearly interpolating values. This give the number of step between each value
	 * \param nbCycles : The nb of time the pattern is repeated during particle life. see ps_attrib_maker.h
	 */

	CPSPlaneBasisGradient(const CPlaneBasis *basisTab = CPSPlaneBasisGradient::DefaultPlaneBasisTab
		, uint32 nbValues = 2, uint32 nbStages = 16, float nbCycles = 1.0f) : CPSValueGradient<CPlaneBasis>(nbCycles)
	{
		_F.setValues(basisTab, nbValues, nbStages) ;
	}

	static CPlaneBasis DefaultPlaneBasisTab[] ;
		
	// F is serialized by base classes...	
} ;



/** this is a 'follow direction' plane basis maker
 * It set the plane basis to have its normal in the same direction than speed of the located
 * The cycle param has no effect o the direction
 */
class CPSPlaneBasisFollowSpeed : public CPSAttribMaker<CPlaneBasis>
{
	public:

		CPSPlaneBasisFollowSpeed() : CPSAttribMaker<CPlaneBasis>(1) {}

		/// compute one value of the attribute for the given index
		virtual CPlaneBasis get(CPSLocated *loc, uint32 index) ;

		/** Fill tab with an attribute by using the given stride. It fills numAttrib attributes.
		 *  \param loc the 'located' that hold the 'located bindable' that need an attribute to be filled
		 *  \param startIndex usually 0, it gives the index of the first element in the located
		 */

		virtual void *make(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib, bool enableNoCopy = false) const ;

		/** The same as make, but it replicate each attribute 4 times, thus filling 4*numAttrib. Useful for facelookat and the like
		 *  \see make()
		 */
		virtual void make4(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib) const ;


		/** the same as make4, but with nbReplicate replication isntead of 4
		 *  \see make4
		 */
		virtual void makeN(CPSLocated *loc, uint32 startIndex, void *tab, uint32 stride, uint32 numAttrib, uint32 nbReplicate) const ;

		NLMISC_DECLARE_CLASS(CPSPlaneBasisFollowSpeed) ;

		/// serialization
		virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			// nothing to save here
			f.serialVersion(1) ;
		}

} ;




/** this memorize value by applying some function on the emitter. For a particle's attribute, each particle has its
  * own value memorized
  *  You MUST called setScheme (from CPSAttribMakerMemory) to tell how the value will be generted
  */
class CPSPlaneBasisMemory : public CPSAttribMakerMemory<CPlaneBasis>
{
public:
	CPSPlaneBasisMemory() { setDefaultValue(CPlaneBasis(NLMISC::CVector::K)) ; }
	NLMISC_DECLARE_CLASS(CPSPlaneBasisMemory) ;
} ;


/** An attribute maker whose output if the result of a binary op on plkane basis
  *
  */
class CPSPlaneBasisBinOp : public CPSAttribMakerBinOp<CPlaneBasis>
{
public:
	NLMISC_DECLARE_CLASS(CPSPlaneBasisBinOp) ;
} ;



} // NL3D


#endif // NL_PLANE_BASIS_MAKER_H

/* End of plane_basis_maker.h */
