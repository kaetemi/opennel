/** \file vegetable.h
 * <File description>
 *
 * $Id: vegetable.h,v 1.7 2001/12/03 09:29:22 berenguier Exp $
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

#ifndef NL_VEGETABLE_H
#define NL_VEGETABLE_H

#include "nel/misc/types_nl.h"
#include "3d/noise_value.h"
#include "nel/misc/vector_2f.h"
#include "nel/misc/matrix.h"


namespace NL3D 
{


using NLMISC::CVector2f;
class	CVegetableManager;
class	CVegetableShape;
class	CVegetableInstanceGroup;


// ***************************************************************************
/**
 * A vegetable descriptor.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CVegetable
{
public:
	/// Name of the shape to use.
	std::string			ShapeName;
	/// Density== Number of instance to create / m².
	CNoiseValue			Density;	// floor(Density*surface)== nbShape to create.
	/// the maximum density computed. if <0, no maximum. -1 by default.
	float				MaxDensity;
	/// Random Scale.
	CNoiseValue			Sxy, Sz;
	/// Random Orientation.
	CNoiseValue			Rx, Ry, Rz;
	/// Random Bend factor.
	CNoiseValue			BendFactor;
	/// Random Bend Phase.
	CNoiseValue			BendPhase;
	/// The color to modulate the instance.
	CNoiseColorGradient	Color;
	/** At which distance this vegetable will be rendered.
	 *	NB: DistType is not managed by CVegetableManager, but by user (eg CLandscape).
	 */
	uint32				DistType;


public:
	CVegetable();

	/// \name Density Angle Factor
	// @{
	enum	TAngleType	{AngleGround=0, AngleCeiling, AngleWall};

	/** set an Angle setup such that max density is when normal== K, and 0 density is when normalAngle= cosAngleMin.
	 *	\param cosAngleMinLimit should be a cosinus angle between -1 and 1.
	 */
	void	setAngleGround(float cosAngleMin);

	/** set an Angle setup such that max density is when normal= -K, and 0 density is when normalAngle= cosAngleMax.
	 *	\param cosAngleMinLimit should be a cosinus angle between -1 and 1.
	 */
	void	setAngleCeiling(float cosAngleMax);

	/** set an Angle setup such that max density is when normal is at (cosAngleMin+cosAngleMax)/2, and 0 density is
	 *	when cosAngle= one of the cosAngleMin or cosAngleMax.
	 */
	void	setAngleWall(float cosAngleMin, float cosAngleMax);

	float		getCosAngleMin() const {return _CosAngleMin;}
	float		getCosAngleMax() const {return _CosAngleMax;}
	TAngleType	getAngleType() const {return _AngleType;}

	// @}


	/** register the vegetable to the vegetable manager. do it first. used by generateInstance().
	 *	Manager will load the shape needed for this vegetable.
	 */
	void	registerToManager(CVegetableManager *manager);


	/**	generate a list of instance to create on a small ground element, according to density and noise.
	 *	result is a list of instance to create, with random 2d position. 2d position are in range [0..1].
	 *	caller should scale, or use it to compute real world position, and then use generateInstance() to really
	 *	compute the instances.
	 *
	 *	Warning! Use OptFastFloor()! So call must be enclosed with a OptFastFloorBegin()/OptFastFloorEnd().
	 *
	 *	\param posInWorld center (approx) of the surface which generate group of vegetable
	 *	\param surfaceNormal UNIT up vector of the surface to compare with (0,0,1), to modulate density
	 *	\param area area of the surface, to know number of elements to generate for this surface
	 *	\param vegetSeed "random" value which Should be different for all vegetables in the same area.
	 *	usefull if lot of different vegetable generated in same area: positions won't be generated at same place. 
	 *	\param instances the generated position of instances.
	 */
	void	generateGroup(const CVector &posInWorld, const CVector &surfaceNormal, float area, uint vegetSeed, std::vector<CVector2f> &instances) const;


	/** same as generateGroup, but with smoother density effects on border with desnity==0
	 *	\name posInWorldAround 4 position on 4 edges around the center in folowing order:
	 *	(0,0.5), (1,0.5), (0.5,0), (0.5,1) (in instance UV coordinates space (as returned in "instances").
	 */
	void	generateGroupBiLinear(const CVector &posInWorld, const CVector posInWorldBorder[4], const CVector &surfaceNormal, float area, uint vegetSeed, std::vector<CVector2f> &instances) const;


	/** posInWorld should be a matrix of position + rotation (typically for surface alignement). 
	 *	FinalPos= posInWorld * noiseMatrix(scale/rot)
	 *	FinalColor= modulateColor * randomColor. Both for Ambient and diffuse.
	 *	If instance is in AlphaBlend/ZSort, blendDistMax is the distance where the instance is invisible (alpha==0).
	 *	instance is added to the manager, under the instance group ig. This one must have been generated by the VegetableManager.
	 */
	void	generateInstance(CVegetableInstanceGroup *ig, const NLMISC::CMatrix &posInWorld, 
		const NLMISC::CRGBAF &modulateAmbientColor, const NLMISC::CRGBAF &modulateDiffuseColor, float blendDistMax) const;


	void	serial(NLMISC::IStream &f);


// *********************
private:
	/** density is multiplied by a function f(cos angle with verticalVector), such that 
	 *	f(CosAngleMiddle)==1, and f(CosAngleMin)==f(CosAngleMax)== 0
	 */
	float				_CosAngleMin, _CosAngleMax, _CosAngleMiddle, _OOCosAngleDist;
	/// angle type setuped with setAngleMin etc...
	TAngleType			_AngleType;


	/// the manager
	CVegetableManager	*_Manager;
	/// shape in the manager
	CVegetableShape		*_VegetableShape;


	/// Do the generateGroup, but take nbInst unmodulated by normal
	void	generateGroupEx(float nbInst, const CVector &posInWorld, const CVector &surfaceNormal, uint vegetSeed, std::vector<CVector2f> &instances) const;


	// easineasout
	static inline float	easeInEaseOut(float x)
	{
		float	y;
		// cubic such that f(0)=0, f'(0)=0, f(1)=1, f'(1)=0.
		float	x2=x*x;
		float	x3=x2*x;
		y= -2*x3 + 3*x2;
		return y;
	}

};


} // NL3D


#endif // NL_VEGETABLE_H

/* End of vegetable.h */
