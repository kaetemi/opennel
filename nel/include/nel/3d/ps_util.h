/** \file ps_util.h
 * <File description>
 *
 * $Id: ps_util.h,v 1.8 2001/05/31 12:16:11 vizerie Exp $
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

#ifndef NL_PS_UTIL_H
#define NL_PS_UTIL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/vector.h"
#include "nel/misc/rgba.h"



namespace NLMISC
{
	class CMatrix ;
	class CVector ;
} ;

namespace NL3D 
{


	class CFontGenerator ;
	class CFontManager ;
	class IDriver ;


	using NLMISC::CMatrix ;
	using NLMISC::CVector ;
	using NLMISC::CRGBA ;
/**
 * This struct contains utility functions used by the particle system.
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
struct CPSUtil
{
	/// register the classes of the system mus be called when serializing

	static void registerSerialParticleSystem(void) ;

	/// this draw a bounding box		

	static void displayBBox(const NLMISC::CAABBox &box) ;

	/// draw a sphere
	static void displaySphere(IDriver &driver, float radius, const CVector &center, uint nbSubdiv = 4, CRGBA color = CRGBA::White) ;


	/** draw a disc (not filled)
	 *  \param mat : a matrix, whose K vector is normal to the plane containing the disc
	 */
	static void displayDisc(IDriver &driver, float radius, const CVector &center, const CMatrix &mat, uint nbSubdiv = 32, CRGBA color = CRGBA::White) ;


	/** draw a cylinder (not filled)	 
	 *  \param dim dimension of the cylinder along each axis, packed in a vector
	 */
	static void displayCylinder(IDriver &driver, const CVector &center, const CMatrix &mat, const CVector &dim, uint nbSubdiv = 32, CRGBA color = CRGBA::White) ;

	/// display a 3d quad in wireline, by using the 4 gicen corners
	static void display3DQuad(IDriver &driver, const CVector &c1, const CVector &c2
								,const CVector &c3,  const CVector &c4, CRGBA color = CRGBA::White) ;
							

	/// enlarge a bounding box by the specified radius	 
	inline static void addRadiusToAABBox(NLMISC::CAABBox &box, float radius) ;
	 
	/// display a basis using the given matrix. The model matrix must be restored after this call
	static void displayBasis(const CMatrix &modelMat, const NLMISC::CMatrix &m, float size, CFontGenerator &fg, CFontManager &fm) ;


	/// display a string at the given position. The model matrix must be restored after this call
	static void print(const std::string &text, CFontGenerator &fg, CFontManager &fm, const NLMISC::CVector &pos, float size) ;


	
	/**
	* Compute the union of 2 aabboxes, that is the  aabbox that contains the 2.
	* Should end up in NLMISC
	*/


	static NLMISC::CAABBox computeAABBoxUnion(const NLMISC::CAABBox &b1, const NLMISC::CAABBox &b2) ;


	/** Apply a matrix on an aabbox
	 *  \return an aabbox, bigger or equal to parameter, after the matrix multiplication
	 */

	static NLMISC::CAABBox transformAABBox(const NLMISC::CMatrix &mat, const NLMISC::CAABBox &box) ;

	/** build a basis from a vector using Schmidt orthogonalization method
	 *  \param v : K axis in the resulting basis
	 */
	static CMatrix buildSchmidtBasis(const CVector &v) ;	 


	/** get a cosine from the fast cosine table (which must be have initialised with initFastCosNSinTable).
	 *  256 <=> 2 Pi
	 */
	static inline float getCos(sint32 angle) 
	{ 
		nlassert(_CosTableInitialized == true) ;
		return _CosTable[angle & 0xff] ;
	}

	/** get a cosine from the fast cosine table (which must be have initialised with initFastCosNSinTable).
	 *  256 <=> 2 Pi
	 */
	static inline float getSin(sint32 angle) 
	{ 
		nlassert(_CosTableInitialized == true) ;
		return _SinTable[angle & 0xff] ;
	}

	/** Init the table for cosine and sinus lookup
     *	\see getCos(), getSin()
	 */
	static void initFastCosNSinTable(void) ;


	/** compute a perlin noise value, that will range from [0 to 1]
	 *  The first octave has the unit size
	 *  \see initPerlinNoiseTable()
	 */
	static inline float buildPerlinNoise(CVector &pos, uint nbOctaves) ;

	/** init the table used by perlin noise.
	 *  This must be used before any call to  buildPerlinNoise
	 */
	static void initPerlinNoiseTable(void) ;
		

	protected:

		#ifdef NL_DEBUG
			static bool _CosTableInitialized ;
		#endif

		#ifdef NL_DEBUG
			static bool _PerlinNoiseTableInitialized ;
		#endif

		// a table for fast cosine lookup
		static float _CosTable[256] ;
		// a table for fast sinus lookup
		static float _SinTable[256] ;

		static float _PerlinNoiseTab[1024] ;


		// used by perlin noise to compute each octave		
		static float getInterpolatedNoise(const CVector &pos) ;

		// get non interpolated noise 
		static float getPerlinNoise(uint x, uint y, uint z) ;



};

///////////////////////////
// inline implementation //
///////////////////////////

inline void CPSUtil::addRadiusToAABBox(NLMISC::CAABBox &box, float radius)
{
	box.setHalfSize(box.getHalfSize() + NLMISC::CVector(radius, radius, radius) ) ;
}


// get non interpolated noise 
inline float CPSUtil::getPerlinNoise(uint x, uint y, uint z)
{
	return _PerlinNoiseTab[(x ^ y ^ z) & 1023] ;
}


inline float CPSUtil::getInterpolatedNoise(const CVector &pos)
{
	uint x = (uint) pos.x
		, y = (uint) pos.y
		, z = (uint) pos.z ;

	// we want to avoid costly ctor call there...
	float fx = pos.x - x
		  , fy = pos.y - y
		  , fz = pos.z - z ;

	// we use the following topology to get the value :
	//
	//
	//  z
	//  | 7-----6
	//   /     /
	//  4-----5 |
	//  |     | |
	//  |  3  | |2
	//  |     | /
	//  0_____1/__x

	
	const float v0 = getPerlinNoise(x, y, z)
		 ,v1 = getPerlinNoise(x + 1, y, z)
		 ,v2 = getPerlinNoise(x + 1, y + 1, z)
 		 ,v3 = getPerlinNoise(x, y + 1, z)
		 ,v4 = getPerlinNoise(x, y, z + 1)
		 ,v5 = getPerlinNoise(x + 1, y, z + 1)
		 ,v6 = getPerlinNoise(x + 1, y + 1, z + 1)
 		 ,v7 = getPerlinNoise(x, y + 1, z + 1) ;

	
	const float h1  = fx * v1 + (1.f - fx) * v0
				,h2 = fx * v3 + (1.f - fx) * v2 
				,h3  = fx * v5 + (1.f - fx) * v4
				,h4 = fx * v7 + (1.f - fx )* v6 ; 

    const float c1  = fy * h2 + (1.f - fy) * h1
		       ,c2  = fy * h4 + (1.f - fy) * h3 ;

	return fz * c2 + (1.f - fz) * c1 ;
}


inline float CPSUtil::buildPerlinNoise(CVector &pos, uint numOctaves)
{
	nlassert(_PerlinNoiseTableInitialized) ;

	float result = 0 ;
	float fact = .5f ;
	float scale = 1.f ;

	for (uint k = 0 ; k < numOctaves ; k++)
	{
		result += fact * getInterpolatedNoise(scale * pos) ;
		fact *= .5f ;
		scale *= 1.2537f ;
	}	
	return result ;
}






} // NL3D


#endif // NL_PS_UTIL_H

/* End of ps_util.h */
