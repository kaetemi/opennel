/** \file object_vector.h
 * <File description>
 *
 * $Id: object_vector.h,v 1.2 2002/02/06 16:51:35 berenguier Exp $
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

#ifndef NL_OBJECT_VECTOR_H
#define NL_OBJECT_VECTOR_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/stream.h"
#include "nel/misc/debug.h"


namespace NLMISC {


// ***************************************************************************
/**	Exception raised when a reallocation fails.
 *
 */
struct	EReallocationFailed : public Exception
{
	EReallocationFailed() : Exception( "Can't reallocate memory" ) {}
};


// ***************************************************************************
/**
 * The purpose of this class is to copy most (but not all) of stl vector<> features, without
 *	some of the speed/memory problems involved:
 *		- size of a vector<T> is 16 bytes typically. size of a CObjectVector is 8 bytes (only a ptr and a size).
 *		- CObjectVector<T>::size() is faster than vector::size()
 *		- CObjectVector<T>::resize() is faster because it do not copy from a default value, it just call the 
 *			default constructor of the objects.
 *		- clear() actually free memory (no reserve scheme)
 *
 *	Object contructors, destructors, operator= are correctly called, unless 
 *	EnableObjectBehavior template argument is set to false (default is true)
 *	In this case: ctor, dtor are not called, and operator=() use memcpy.
 *
 *	Of course some features are not implemented (for benefit of speed/memory):
 *		- no reserve() scheme
 *
 *	Also, for now, not all vector<> features are implemented (iterator, erase etc...).
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
template<class T, bool EnableObjectBehavior=true>	class CObjectVector
{
public:

	/// \name Object
	// @{
	CObjectVector()
	{
		_Ptr= NULL;
		_Size= 0;
	}
	~CObjectVector()
	{
		clear();
	}
	/** copy cons.
	 *	\throw EReallocationFailed() if realloc fails.
	 */
	CObjectVector(const CObjectVector &vec)
	{
		_Ptr= NULL;
		_Size= 0;
		operator=(vec);
	}
	/** copy the array.
	 *	\throw EReallocationFailed() if realloc fails.
	 */
	CObjectVector	&operator=(const CObjectVector &vec)
	{
		// *this=*this mgt.
		if(this==&vec)
			return *this;
		// resize to the same size as vec.
		resize(vec._Size);
		// copy All the array.
		copy(0, _Size, vec._Ptr);

		return *this;
	}
	// @}


	/// \name Allocation
	// @{

	/** clear the array.
	 */
	void		clear()
	{
		destruct(0, _Size);
		free(_Ptr);
		_Ptr= NULL;
		_Size= 0;
	}

	/** resize the array.
	 *	If reallocation occurs, ptr returned by getPtr() may not be the same.
	 *	When reallocation occurs, memory is coped, but operator= are not called.
	 *	\throw EReallocationFailed() if realloc fails.
	 */
	void		resize(uint32 size)
	{
		// if same size, no-op.
		if(size==_Size)
			return;

		// if empty, just clear.
		if(size==0)
			clear();
		// crop the array?
		else if(size<_Size)
		{
			// destruct the objects to be freed
			destruct(size, _Size);
			// realloc
			myRealloc(size);
			_Size= size;
		}
		// else, enlarge the array
		else
		{
			// realloc first
			myRealloc(size);
			// For all new elements, construct them.
			construct(_Size, size);
			// change size.
			_Size= size;
		}
	}

	// @}


	/// \name Accessor
	// @{

	/** return size of the array (in number of elements)
	 */
	uint32		size() const {return _Size;}

	/** Element accessor. no check is made on index. (No exception, no nlassert())
	 */
	T			&operator[](uint index) const
	{
		return _Ptr[index];
	}

	/** return a ptr on the first element of the array. NULL if empty.
	 */
	T			*getPtr() const {return _Ptr;}

	// @}


	/// \name Tools
	// @{

	/** copy elements from an array ptr to this vector, beetween dstFirst element (included) and dstLast element (not included).
	 *	nlassert if last is too big. copy(y, x, ...) where y>=x is valid, and nothing is copied.
	 */
	void		copy(uint32 dstFirst, uint32 dstLast, const T *srcPtr)
	{
		// test if something to copy.
		if(dstFirst>=dstLast)
			return;
		nlassert(dstLast<=_Size);
		// if not object elements
		if(!EnableObjectBehavior)
		{
			// just memcpy
			memcpy(_Ptr+dstFirst, srcPtr, (dstLast-dstFirst)*sizeof(T));
		}
		else
		{
			// call ope= for all elements.
			for(uint i=dstFirst; i<dstLast; i++, srcPtr++)
			{
				_Ptr[i]= *srcPtr;
			}
		}
	}


	/** fill elements with a value, beetween dstFirst element (included) and dstLast element (not included).
	 */
	void		fill(uint32 dstFirst, uint32 dstLast, const T &value)
	{
		// test if something to copy.
		if(dstFirst>=dstLast)
			return;
		nlassert(dstLast<=_Size);
		// call ope= for all elements.
		for(uint i=dstFirst; i<dstLast; i++)
		{
			_Ptr[i]= value;
		}
	}

	/** fill all elements with a value
	 */
	void		fill(const T &value)
	{
		// call ope= for all elements.
		for(uint i=0; i<_Size; i++)
		{
			_Ptr[i]= value;
		}
	}



	/** Serial this ObjectVector.
	 *	NB: actually, the serial of a vector<> and the serial of a CObjectVector is the same in the stream.
	 */
	void		serial(NLMISC::IStream &f)
	{
		// Open a node header
		f.xmlPushBegin ("VECTOR");

		// Attrib size
		f.xmlSetAttrib ("size");

		sint32	len=0;
		if(f.isReading())
		{
			f.serial(len);

			// Open a node header
			f.xmlPushEnd ();

			// special version for vector: adjut good size.
			contReset(*this);
			resize (len);

			// Read the vector
			for(sint i=0;i<len;i++)
			{
				f.xmlPush ("ELM");

				f.serial(_Ptr[i]);

				f.xmlPop ();
			}
		}
		else
		{
			len= size();
			f.serial(len);

			// Close the node header
			f.xmlPushEnd ();

			// Write the vector
			for(sint i=0;i<len;i++)
			{
				f.xmlPush ("ELM");

				f.serial(_Ptr[i]);

				f.xmlPop ();
			}
		}

		// Close the node
		f.xmlPop ();
	}

	// @}



// *******************
private:

	/// Ptr on our array.
	T				*_Ptr;
	/// size of the array, in number of elements.
	uint32			_Size;


private:
	// realloc, and manage allocation failure. Don't modify _Size.
	void	myRealloc(uint32 size)
	{
		// try to realloc the array.
		T	*newPtr= (T*)realloc(_Ptr, size*sizeof(T));
		// if realloc failure
		if(newPtr==NULL)
		{
			// leave the array in a clean state.
			clear();
			// exception.
			throw EReallocationFailed();
		}
		else
		{
			_Ptr= newPtr;
		}
	}


	// For all elements in the range, construct.
	void	construct(uint32 i0, uint32 i1)
	{
		// don't do it if elements don't need it.
		if(!EnableObjectBehavior)
			return;
		// for all elements
		for(uint i=i0;i<i1;i++)
		{
			// call ctor.
			new (_Ptr+i) T;
		}
	}

	// For all elements in the range, destruct.
	void	destruct(uint32 i0, uint32 i1)
	{
		// don't do it if elements don't need it.
		if(!EnableObjectBehavior)
			return;
		// for all elements
		for(uint i=i0;i<i1;i++)
		{
			// call dtor.
			_Ptr[i].~T();
		}
	}

};


} // NLMISC


#endif // NL_OBJECT_VECTOR_H

/* End of object_vector.h */
