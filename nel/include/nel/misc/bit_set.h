/** \file bit_set.h
 * CBitSet class
 *
 * $Id: bit_set.h,v 1.4 2001/04/24 14:55:08 corvazier Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#ifndef NL_BIT_SET_H
#define NL_BIT_SET_H


#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"


namespace	NLMISC
{


// ***************************************************************************
/**
 * A BitSet, to test / set flags quickly.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CBitSet
{
public:
	/// \name Object.
	//@{
	CBitSet();
	CBitSet(uint numBits);
	CBitSet(const CBitSet &bs);
	~CBitSet();
	CBitSet	&operator=(const CBitSet &bs);
	//@}

	/// \name Basics.
	//@{
	/// Resize the bit array. All Bits are reseted.
	void	resize (uint numBits);
	/// Resize the bit array. Bits are not reseted. New bits are set with value.
	void	resizeNoReset (uint numBits, bool value=false);
	/// Clear the bitarray so size() return 0.
	void	clear();
	/// Return size of the bit array.
	uint	size() const;
	/// Set a bit to 0 or 1.
	void	set(sint bitNumber, bool value);
	/// Get the value of a bit.
	bool	get(sint bitNumber) const;
	/// Get the value of a bit.
	bool	operator[](sint bitNumber) const;
	/// Set a bit to 1.
	void	set(sint bitNumber) {set(bitNumber, true);}
	/// Set a bit to 0.
	void	clear(sint bitNumber) {set(bitNumber, false);}
	/// Set all bits to 1.
	void	setAll();
	/// Set all bits to 0.
	void	clearAll();
	//@}


	/// \name Bit operations.
	//@{
	/// Return The bitarray NOTed.
	CBitSet	operator~() const;
	/**
	 * Return this ANDed with bs.
	 * The result BitSet is of size of \c *this. Any missing bits into bs will be considered as 0.
	 */
	CBitSet	operator&(const CBitSet &bs) const;
	/**
	 * Return this ORed with bs.
	 * The result BitSet is of size of \c *this. Any missing bits into bs will be considered as 0.
	 */
	CBitSet	operator|(const CBitSet &bs) const;
	/**
	 * Return this XORed with bs.
	 * The result BitSet is of size of \c *this. Any missing bits into bs will be considered as 0.
	 */
	CBitSet	operator^(const CBitSet &bs) const;

	/// NOT the BitArray.
	void	flip();
	/**
	 * AND the bitArray with bs.
	 * The bitset size is not changed. Any missing bits into bs will be considered as 0.
	 */
	CBitSet	&operator&=(const CBitSet &bs);
	/**
	 * OR the bitArray with bs.
	 * The bitset size is not changed. Any missing bits into bs will be considered as 0.
	 */
	CBitSet	&operator|=(const CBitSet &bs);
	/**
	 * XOR the bitArray with bs.
	 * The bitset size is not changed. Any missing bits into bs will be considered as 0.
	 */
	CBitSet	&operator^=(const CBitSet &bs);
	//@}


	/// \name Bit comparisons.
	//@{
	/** 
	 * Compare two BitSet not necessarely of same size. The comparison is done on N bits, where N=min(this->size(), bs.size())
	 * \return true if the N common bits of this and bs are the same. false otherwise.
	 */
	bool	compareRestrict(const CBitSet &bs) const;
	/// Compare two BitSet. If not of same size, return false.
	bool	operator==(const CBitSet &bs) const;
	/// operator!=.
	bool	operator!=(const CBitSet &bs) const;
	/// Return true if all bits are set. false if size()==0.
	bool	allSet();
	/// Return true if all bits are cleared. false if size()==0.
	bool	allCleared();
	//@}


	void	serial(NLMISC::IStream &f);

private:
	std::vector<uint>	Array;
	sint			NumBits;
	uint			MaskLast;	// Mask for the last uint.
};


}



#endif // NL_BIT_SET_H

/* End of bit_set.h */
