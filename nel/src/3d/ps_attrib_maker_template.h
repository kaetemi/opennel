/** \file ps_attrib_maker_template.h
 * <File description>
 *
 * $Id: ps_attrib_maker_template.h,v 1.9 2001/07/13 17:01:44 vizerie Exp $
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

#ifndef NL_PS_ATTRIB_MAKER_TEMPLATE_H
#define NL_PS_ATTRIB_MAKER_TEMPLATE_H

#include "nel/misc/types_nl.h"
#include "3d/ps_attrib_maker_helper.h"
#include "3d/ps_plane_basis.h"


namespace NL3D {

/*
 *	In this file, we define several template that helps to create attributes maker such as gradient (of float, int, vector etc) 
 * attributes maker are used in the particle system to generate values, such as size, color etc. see ps_attrib_maker.h
 * for more informations
 */




/** a blending function
 * it blends between t1 and t2 by the alpha amount
 * specializing this function may help with some types of data that don't have the needed operator (NLMISC::CRGBA)
 */

template <typename T>
inline T PSValueBlend(const T &t1, const T &t2, float alpha)
{
	return T(alpha * t2 + (1.f - alpha) * t1) ;
}


/// NLMISC::CRGBA specialization of the PSValueBlend function
template <>
inline NLMISC::CRGBA PSValueBlend(const NLMISC::CRGBA &t1, const NLMISC::CRGBA &t2, float alpha)
{
	NLMISC::CRGBA result ;
	result.blendFromui(t1, t2, (uint) (255.0f * alpha)) ;
	return result ;
}


/// CPlaneBasis specilization of the PSValueBlend function
template <>
inline CPlaneBasis PSValueBlend(const CPlaneBasis &t1, const CPlaneBasis &t2, float alpha)
{	
	return CPlaneBasis(PSValueBlend(t1.getNormal(), t2.getNormal(), alpha)) ;
}


/// base struct for blending function (exact or sampled)
  
template <typename T> struct CPSValueBlendFuncBase
{
	virtual void getValues(T &startValue, T &endValue) const = 0 ;
	virtual void setValues(T startValue, T endValue) = 0 ;
} ;



/**
 * This temlate functor blend exactly between 2 value (no samples)
 * To accomplish blending, it use the template function PSValueBlend
 * It is used by CPSValueBlend 
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 * \see PSValueBlend
 */
template <typename T> class CPSValueBlendFunc : public CPSValueBlendFuncBase<T>
{
public:
	/// this produce Values
	#ifdef NL_OS_WINDOWS
		__forceinline
	#endif
	T operator()(CAnimationTime time) const
	{

		return PSValueBlend(_StartValue, _EndValue, time) ;	// a cast to T is necessary, because 
														// the specialization couls be done with integer
	}

	/// restrieve the start and end Value

	virtual void getValues(T &startValue, T &endValue) const
	{
		startValue = (*this)(0) ;
		endValue = (*this)(1) ;
	}	

	/// set the Values

	virtual void setValues(T startValue, T endValue)
	{
		_StartValue = startValue ;
		_EndValue = endValue ;
	}

	/// ctor
	CPSValueBlendFunc() {}

	/// serialization
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialVersion(1) ;
		f.serial(_StartValue, _EndValue) ;
	}

	T getMaxValue(void) const
	{
		return std::max((*this)(0), (*this)(1)) ;
	}

protected:
	T _StartValue, _EndValue ;	
};


/** This is a Value blender class. The blending between value is not sampled with this class.
  *  So it may be slow, but it is exact.
  *  It work with most type, but some of them may need special  blending between value : 
  *  if so you must specialize the template function PSValueBlend defined in this file
  *  to do the job...
  *  To use this, just derive a class, create a ctor, and declare it to the class registry 
  *  
  *  in the ctor, you should call _F.setValue to init the functor object.
  */

template <typename T> class CPSValueBlender : public CPSAttribMakerT<T, CPSValueBlendFunc<T> >
{
public:	
	/** ctor
	 *  With nbCycles, you can set the pattern frequency. It is usually one. See ps_attrib_maker.h
	 *  For further details
	 */
	CPSValueBlender(float nbCycles) : CPSAttribMakerT<T, CPSValueBlendFunc<T> >(nbCycles)
	{	
	}

	virtual T getMaxValue(void) const { return _F.getMaxValue() ; }

	// serialization is done by CPSAttribMakerT


} ;




/**
 * This temlate functor blend between 2 values by performing n samples (n = template parameter)
 * It may be faster that the CPSValueBlendFunc in some cases.
 * To accomplish blending, it use the template function PSValueBlend
 * It is used by CPSValueBlend 
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 * \see PSValueBlend
 */

template <typename T, const uint n> class CPSValueBlendSampleFunc : public CPSValueBlendFuncBase<T>
{
public:
	/// this produce Values
	#ifdef NL_OS_WINDOWS
			__forceinline
	#endif
	T operator()(CAnimationTime time) const
	{

		return _Values[(uint) (time * n)] ;	
	}

	/// restrieve the start and end Value

	virtual void getValues(T &startValue, T &endValue) const
	{
		startValue = _Values[0] ;
		endValue = _Values[n] ;
	}	

	/// set the Values

	virtual void setValues(T startValue, T endValue)
	{
		float step = 1.f / n ;
		float alpha = 0.0f ;
		for (uint k = 0 ; k < n ; ++k)
		{
			_Values[k] = PSValueBlend(startValue, endValue, alpha) ;
			alpha += step ;
		}
		_Values[n] = endValue ;
	}

	/// ctor
	CPSValueBlendSampleFunc() {}

	/// serialization
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialVersion(1) ;
		if (f.isReading())
		{
			T t1, t2 ;
			f.serial(t1, t2) ;
			setValues(t1, t2) ;
		}
		else
		{
			f.serial(_Values[0], _Values[n]) ;
		}
	}

	T getMaxValue(void) const
	{
		return std::max((*this)(0), (*this)(1)) ;
	}

protected:
	T  _Values[n + 1] ;
};




/** This is a Values blender (sampled version, with n sample) class, that operate on value of type T
 *  To use this, just derive a class from a specialization of this template , create a ctor, and declare it to the class registry
 *  in the ctor, you should call _F.setValue to init the functor object
 */

template <typename T, const uint n> class CPSValueBlenderSample : public CPSAttribMakerT<T, CPSValueBlendSampleFunc<T, n> >
{
public:

	/** ctor
	 *  With nbCycles, you can set the pattern frequency. It is usually one. See ps_attrib_maker.h
	 *  For further details
	 */

	CPSValueBlenderSample(float nbCycles) : CPSAttribMakerT<T, CPSValueBlendSampleFunc<T, n> >(nbCycles)
	{
	}

	virtual T getMaxValue(void) const { return _F.getMaxValue() ; }	

} ;




/**
 * This functor blend between several Value. Intermediate value are sampled with a given number of steps
 * It is used by CPSValueGradient, that you can use to have gradients with your own types
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
template <typename T> class CPSValueGradientFunc
{
public:
	/// this produce Values
	#ifdef NL_OS_WINDOWS
		__forceinline
	#endif
	T operator()(CAnimationTime time) const
	{
		nlassert(_Tab) ;
		return _Tab[(uint32) (time * _NbValues)] ;
	}

	/// copie the keys values in the specified table
	void getValues(T *tab) const
	{
		nlassert(tab) ;
		T *pt = tab ;
		T *src = _Tab ;
		for (uint32 k = 0 ; k <= ((_NbValues - 1) / _NbStages) ; ++k)
		{
			*pt++ = *src ;
			src += _NbStages ; // jump the interpolated values
		}
	}

	/// get one value
	T getValue(uint index)	const
	{		
		nlassert(index < getNumValues()) ;
		nlassert(_Tab) ;
		return _Tab[index * _NbStages] ;
	}




	uint32 getNumValues(void) const { return ((_NbValues - 1) / _NbStages) + 1 ; }

	/** set the colors
	 *  \param numValue number of Values, must be >= 2
	 *  \ValueTab a table containing the Values. Value will be blended, so you must only provide keyframe Values	 
	 *  \param nbStages The result is sampled into a table by linearly interpolating values. This give the number of step between each value
	 *  WARNING : for integer types, some specilization exist that ensure correct interpolation. see below
	 */


	inline void setValues(const T *ValueTab, uint32 numValues, uint32 nbStages) ;
	
	/// get the number of stages between each value
	uint32 getNumStages(void) const { return _NbStages ; }

	/// change the number of stages between each value
	void setNumStages(uint32 numStages)
	{
		std::vector<T> v ;
		v.resize(getNumValues()) ;
		getValues(&v[0]) ;
		setValues(&v[0], getNumValues(), numStages) ;
	}

	/// serialization
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;


	T getMaxValue(void) const
	{
		return _MaxValue ;
	}

	/// ctor
	CPSValueGradientFunc() : _Tab(NULL), _NbValues(0), _NbStages(0)
	{
	}

	/// dtor
	~CPSValueGradientFunc()
	{
		delete[] _Tab ;
	}
	
protected:
	// a table of Values that interpolate the values given
	T *_Tab ;


	// number of interpolated value between each 'key'
	uint32 _NbStages ;

	// total number of value in the tab
	uint32 _NbValues ;


	// the max value
	T _MaxValue ;
};




/** This is a Values gradient class
 *  To use this, just derive a class from a specialization of this template , create a ctor, and declare it to the class registry
 *  in the ctor, you should call _F.setValue to init the functor object
 */

template <typename T> class CPSValueGradient : public CPSAttribMakerT<T, CPSValueGradientFunc<T> >
{
public:

	/** ctor
	 *  With nbCycles, you can set the pattern frequency. It is usually one. See ps_attrib_maker.h
	 *  For further details
	 */

	CPSValueGradient(float nbCycles) : CPSAttribMakerT<T, CPSValueGradientFunc<T> >(nbCycles)
	{
	}

	virtual T getMaxValue(void) const { return _F.getMaxValue() ; }
	

} ;




////////////////////////////
// methods implementations //
////////////////////////////



	
template <typename T> 
inline void CPSValueGradientFunc<T>::setValues(const T *valueTab, uint32 numValues, uint32 nbStages)
{
	nlassert(numValues > 1) ;
	nlassert(nbStages > 0) ;

	_NbStages = nbStages ;

	_MaxValue = valueTab[0] ;

	if (_Tab)
	{
		delete[] _Tab ;		
	}

	_NbValues = 1 + (numValues - 1) * nbStages ;

	_Tab = new T[_NbValues] ;


	float step = 1.0f / float(nbStages) ;
	float alpha ; 

	

	T *dest = _Tab ;

	// copy the tab performing linear interpolation between values given in parameter
	for (uint32 k = 0 ; k  < (numValues - 1) ; ++k)
	{				
		if (!(valueTab[k] < _MaxValue))
		{
			_MaxValue = valueTab[k] ;
		}

		alpha = 0 ;

		for(uint32 l = 0 ; l < nbStages ; ++l)
		{			
			// use the right version of the template function PSValueBlend
			// to do the job
			*dest++ = PSValueBlend(valueTab[k], valueTab[k + 1], alpha) ;
			alpha += step ;
		}
	}
	*dest++ = valueTab[numValues - 1] ;
}




	
template <typename T> 
void CPSValueGradientFunc<T>::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1) ;
	f.serial(_NbStages) ;
	if (f.isReading())
	{
	
		// reload the number of keys 

		uint32 numVal ;
		f.serial(numVal) ;		
		_NbValues = 1 + (numVal - 1) * _NbStages ;

		// create the table on the stack for small gradient
		if (numVal < 256)
		{
			T tab[256] ;
			for (uint32 k = 0 ; k < numVal ; ++k)
			{
				f.serial(tab[k]) ;
			}
			setValues(tab, numVal, _NbStages) ;
		}	
		else
		{
			T *tab = new T[numVal] ;
			for (uint32 k = 0 ; k < numVal ; ++k)
			{
				f.serial(tab[k]) ;
			}
			setValues(tab, numVal, _NbStages) ;
			delete[] tab ;
		}
	}
	else
	{
		// saves the number of keys
		uint32 numKeyValues = getNumValues() ;
		f.serial(numKeyValues) ;


		// save each key
		T *src = _Tab ;
		for (uint32 k = 0 ; k < numKeyValues  ; ++k)
		{
			f.serial(*src) ;
			src += _NbStages ;
		}
	}
}

} // NL3D


#endif // NL_PS_ATTRIB_MAKER_TEMPLATE_H

/* End of ps_attrib_maker_template.h */
