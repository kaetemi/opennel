/** \file vector.h
 * CVector class
 *
 * $Id: vector.h,v 1.5 2000/10/24 15:24:33 lecroart Exp $
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

#ifndef NL_VECTOR_H
#define NL_VECTOR_H

#include "nel/misc/types_nl.h"
#include <math.h>
#include "nel/misc/stream.h"

namespace	NLMISC
{

class IStream;

// ======================================================================================================
/**
 * A 3D vector of float.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CVector
{
public:		// Attributes.
	float	x,y,z;

public:		// const.
	/// Null vector (0,0,0).
	static const	CVector		Null;
	/// I vector (1,0,0).
	static const	CVector		I;
	/// J vector (0,1,0).
	static const	CVector		J;
	/// K vector (0,0,1).
	static const	CVector		K;

public:		// Methods.
	/// @name Object.
	//@{
	/// Constructor wich do nothing.
	CVector() {}
	/// Constructor .
	CVector(float	_x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	/// Copy Constructor.
	CVector(const CVector &v) : x(v.x), y(v.y), z(v.z) {}
	//@}

	/// @name Base Maths.
	//@{
	CVector	&operator+=(const CVector &v);
	CVector	&operator-=(const CVector &v);
	CVector	&operator*=(float f);
	CVector	&operator/=(float f);
	CVector	operator+(const CVector &v) const;
	CVector	operator-(const CVector &v) const;
	CVector	operator*(float f) const;
	CVector	operator/(float f) const;
	CVector	operator-() const;
	//@}

	/// @name Advanced Maths.
	//@{
	/// Dot product.
	float	operator*(const CVector &v) const;
	/** Cross product.
	 * compute the cross product *this ^ v.
	 */
	CVector	operator^(const CVector &v) const;
	/// Return the norm of the vector.
	float	norm() const;
	/// Return the square of the norm of the vector.
	float	sqrnorm() const;
	/// Normalize the vector.
	void	normalize();
	/// Return the vector normalized.
	CVector	normed() const;
	//@}

	/// @name Misc.
	//@{
	void	set(float _x, float _y, float _z);
	bool	operator==(const CVector &v) const;
	bool	operator!=(const CVector &v) const;
	bool	isNull() const;
	/** 
	 * Setup the vector with spheric coordinates.
	 * the formula is:  \n
	 * x= r*cos(theta)*cos(phi) \n
	 * y= r*sin(theta)*cos(phi) \n
	 * z= r*sin(phi) \n
	 * \sa cartesianToSpheric()
	 */
	void	sphericToCartesian(float r, float theta,float phi);
	/**
	 * Get the sphreic coordinates of the vector.
	 * See sphericToCartesian() to know coordinates conventions.
	 * \sa sphericToCartesian()
	 */
	void	cartesianToSpheric(float &theta,float &phi, float &r) const;
	void	serial(IStream &f);
	//@}

	// friends.
	friend	CVector	operator*(float f, const CVector &v0);
};


}


#include "nel/misc/vector_inline.h"


#endif // NL_VECTOR_H

/* End of vector.h */
