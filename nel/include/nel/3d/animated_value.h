/** \file animated_value.h
 * Class IAnimatedValue
 *
 * $Id: animated_value.h,v 1.7 2001/03/13 17:02:20 corvazier Exp $
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

#ifndef NL_ANIMATED_VALUE_H
#define NL_ANIMATED_VALUE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/quat.h"
#include "nel/misc/rgba.h"


namespace NL3D 
{


/**
 * A value handled by the animation system.
 * This value must be managed by a IAnimatable object.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class IAnimatedValue
{
public:
	/** 
	  * The blend method. This method blend two the animated values and store the result
	  * in the object. The two first args can be reference on the object itself.
	  * Idealy, it performs the operation this->value=(this->value*blendFactor + value*(1.f-blendFactor))
	  *
	  * \param value is the first value in the blend operation.
	  * \param blendFactor must be in the range [0..1].
	  */
	virtual void blend (const IAnimatedValue& value, float blendFactor) =0;

	/** 
	  * An assignation method. This method assign a values in the object. 
	  *
	  * \param value is the new value.
	  */
	virtual void affect (const IAnimatedValue& value) =0;
};


/**
 * A template implementation of IAnimatedValue.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CAnimatedValueBlendable : public IAnimatedValue
{
public:
	// NOT TESTED, JUST COMPILED. FOR PURPOSE ONLY.
	/// A default blend method. Doesn't work for all type.
	virtual void blend (const IAnimatedValue& value, float blendFactor)
	{
		// Check types of value
		nlassert (typeid (value)==typeid(*this));

		// Cast
		CAnimatedValueBlendable<T>	*pValue=(CAnimatedValueBlendable<T>*)&value;

		// Blend
		Value=(T) (Value*blendFactor+pValue->Value*(1.f-blendFactor));
	}
	
	/** 
	  * An assignation method. This method assign a values in the object. 
	  *
	  * \param value is the new value.
	  */
	virtual void affect (const IAnimatedValue& value)
	{
		// Check types of value
		nlassert (typeid (value)==typeid(*this));

		// Cast
		CAnimatedValueBlendable<T>	*pValue=(CAnimatedValueBlendable<T>*)&value;

		// Blend
		Value=pValue->Value;
	}

	// The value read and write
	T	Value;
};



/**
 * A QUATERNION implementation of IAnimatedValue.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CAnimatedValueBlendable<NLMISC::CQuat> : public IAnimatedValue
{
public:
	/// A quat blend method.
	virtual void blend (const IAnimatedValue& value, float blendFactor)
	{
		// Check types of value
		nlassert (typeid (value)==typeid(*this));

		// Cast.
		CAnimatedValueBlendable<NLMISC::CQuat>	*pValue=(CAnimatedValueBlendable<NLMISC::CQuat>*)&value;

		// blend.
		// Yoyo: no makeClosest is done, because the result seems to be better when done
		// before: for all blend values, and not one after one.
		Value= NLMISC::CQuat::slerp(Value, pValue->Value, 1-blendFactor);

	}

	/** 
	  * An assignation method. This method assign a values in the object. 
	  *
	  * \param value is the new value.
	  */
	virtual void affect (const IAnimatedValue& value)
	{
		// Check types of value
		nlassert (typeid (value)==typeid(*this));

		// Cast
		CAnimatedValueBlendable<NLMISC::CQuat>	*pValue=(CAnimatedValueBlendable<NLMISC::CQuat>*)&value;

		// Blend
		Value=pValue->Value;
	}

	// The value
	NLMISC::CQuat	Value;
};


/**
 * A CRGBA implementation of IAnimatedValue.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CAnimatedValueBlendable<NLMISC::CRGBA> : public IAnimatedValue
{
public:
	/// A quat blend method.
	virtual void blend (const IAnimatedValue& value, float blendFactor)
	{
		// Check types of value
		nlassert (typeid (value)==typeid(*this));

		// Cast.
		CAnimatedValueBlendable<NLMISC::CRGBA>	*pValue=(CAnimatedValueBlendable<NLMISC::CRGBA>*)&value;

		// blend.
		Value.blendFromui (pValue->Value, this->Value, (uint)(255.f*blendFactor));
	}

	/** 
	  * An assignation method. This method assign a values in the object. 
	  *
	  * \param value is the new value.
	  */
	virtual void affect (const IAnimatedValue& value)
	{
		// Check types of value
		nlassert (typeid (value)==typeid(*this));

		// Cast
		CAnimatedValueBlendable<NLMISC::CRGBA>	*pValue=(CAnimatedValueBlendable<NLMISC::CRGBA>*)&value;

		// Blend
		Value=pValue->Value;
	}

	// The value
	NLMISC::CRGBA	Value;
};


/**
 * A template implementation of IAnimatedValue not blendable.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CAnimatedValueNotBlendable : public IAnimatedValue
{
public:
	/// A default blend method. Doesn't work for all type.
	virtual void blend (const IAnimatedValue& value, float blendFactor)
	{
		// Check types of value
		nlassert (typeid (value)==typeid(*this));

		// Cast
		CAnimatedValueNotBlendable<T>	*pValue=(CAnimatedValueNotBlendable<T>*)&value;

		// Boolean blend
		if (blendFactor<0.5f)
			Value=pValue->Value;
	}
	
	/** 
	  * An assignation method. This method assign a values in the object. 
	  *
	  * \param value is the new value.
	  */
	virtual void affect (const IAnimatedValue& value)
	{
		// Check types of value
		nlassert (typeid (value)==typeid(*this));

		// Cast
		CAnimatedValueNotBlendable<T>	*pValue=(CAnimatedValueNotBlendable<T>*)&value;

		// Blend
		Value=pValue->Value;
	}

	// The value
	T	Value;
};


typedef CAnimatedValueNotBlendable<bool>			CAnimatedValueBool;
typedef CAnimatedValueBlendable<sint32>				CAnimatedValueInt;
typedef CAnimatedValueBlendable<float>				CAnimatedValueFloat;
typedef CAnimatedValueBlendable<NLMISC::CVector>	CAnimatedValueVector;
typedef CAnimatedValueNotBlendable<std::string>		CAnimatedValueString;
typedef CAnimatedValueBlendable<NLMISC::CQuat>		CAnimatedValueQuat;
typedef CAnimatedValueBlendable<NLMISC::CRGBA>		CAnimatedValueRGBA;


} // NL3D


#endif // NL_ANIMATED_VALUE_H

/* End of animated_value.h */
