/** \file resource_ptr.h
 * CResourcePtr class.
 *
 * $Id: resource_ptr.h,v 1.1 2004/11/17 16:12:01 corvazier Exp $
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

#ifndef NL_RESOURCE_PTR_H
#define NL_RESOURCE_PTR_H


#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/stream.h"
#include "nel/misc/smart_ptr.h"

#include <stdio.h>

namespace NLMISC
{

// For debug only.
#define	REF_TRACE(_s)	((void)0)
//#define	REF_TRACE(_s)	nlinfo("%s: %d \n", _s, pinfo!=&CRefCount::NullPtrInfo?pinfo->RefCount:0)

// ***************************************************************************
/**
 * CResourcePtr: an handle on a ptr. T Must derive from CRefCount.
 * A CResourcePtr is a CRefPtr able to find its pointer using a key.
 *
 * This way, the resource can be loaded, unloaded, changed and the pointer stays valid.
 * Sample:
 *\code

	// Our resource type
	class CResource
	{
	public:
		int data;
	};
	
	// Provided a class to retrieve a resource using a key (here it is a string)
	class CResourceFinder
	{
	public:
		static void *getResource (const std::string &key)
		{
			static CResource a;
			static CResource b;
			if (key == "a")
				return &a;
			if (key == "b")
				return &b;
			return NULL;
		}
	};

	// Pointer typedef 
	typedef CResourcePtr<CResource, std::string, CResourceFinder> TAPtr;

	// Exemple of utilsation
	void	foo()
	{
		TAPtr	rscA("a");

		//	The test call the cast operator and update the cached pointer.
		//	If the pointer has been deleted, the function is called to retrieve a new pointer.
		//	If the resource doesn't exist, it returns NULL.
		if (rscA)
		{
			// Use the pointer in cache, this is fast
			rscA->thisIsGood();
		}
	}

 \endcode
 *
 * \b PERFORMANCE \b WARNING! operator=() and the cast operator are about 10 times slower than normal pointers. So use them wisely.
 * A good way to do it : using CResourcePtr in a debug mode in which the resources can be unloaded/reloaded, and using CStaticResourcePtr, else.
 * Also, an object used with a CResourcePtr will allocate a small PtrInfo (one only per object, not per ptr).
 * TPtr is the type of the pointed object.
 * TKey is the key used to retrieve the object pointer.
 * TResourceFinder is a class with a static method "static void *getResource (const TKey &key)" that returns a pointer on an object using a key.
 * If the function return a non NULL pointer (resource found), the ResourcePtr is initialised with it. 
 * \sa CSmartPtr, CRefCount, CRefPtr
 * \author Cyril 'Hulud' Corvazier
 */
template <class TPtr, class TKey, class TResourceFinder>
class CResourcePtr
{
private:
	CRefCount::CPtrInfo		*pinfo;		// A ptr to the handle of the object.
	TKey					Key;		// The key used to find the pointer
    mutable TPtr			*Ptr;		// A cache for pinfo->Ptr. UseFull to speed up  ope->()  and  ope*()

	void	unRef()  const;				// Just release the handle pinfo, but do not update pinfo/Ptr, if deleted.

public:

	/// Init a NULL Ptr.
    CResourcePtr();
	/// Retrieve a ptr using the key. If the pointer doesn't exist, the CResourcePtr is NULL.
    CResourcePtr(const TKey &key);
	/// Copy constructor.
    CResourcePtr(const CResourcePtr &copy);
	/// Release the RefPtr.
    ~CResourcePtr(void);


	/// Cast operator. Check if the object has been deleted somewhere, and return NULL if this is the case.
	operator TPtr*()	const;
	/// Cast operator. Check if the object has been deleted somewhere, and return NULL if this is the case.
	TPtr* getPtr()		const;

	/** Indirection operator. Doesn't update the ptr and doesn't test if the pointer has been deleted, and doesn't check NULL.
	  * You must test the pointer first using the cast operator.
	  */
    TPtr& operator*(void)	const;
	/** Selection operator. Doesn't update the ptr and doesn't test if the pointer has been deleted, and doesn't check NULL.
	  * You must test the pointer first using the cast operator.
	  */
    TPtr* operator->(void)	const;

	/// Retrieve a ptr using the key. If the pointer doesn't exist, the CResourcePtr is NULL.
	CResourcePtr& operator=(const TKey &key);
	/// operator=. Giving a NULL pointer is a valid operation.
    CResourcePtr& operator=(const CResourcePtr &copy);
};

// ***************************************************************************
/**
 * CStaticResourcePtr: a CResourcePtr remplacement that doesn't support the resource unload/reload.
 * The object size is the same than a standard pointer.
 * The pointer resource is retrieved at once and is keeped all along the life of the object.
 * No memory / cpu overhead.
 * Typically, you will use the CResourcePtr in a "reloadable resource version" of your software and
 * the CStaticResourcePtr in the release version of your software.
 *
 * \author Cyril 'Hulud' Corvazier
 */
template <class TPtr, class TKey, class TResourceFinder>
class CStaticResourcePtr
{
private:
    mutable TPtr			*Ptr;		// The pointer

public:

	/// Init a NULL Ptr.
    CStaticResourcePtr();
	/// Retrieve a ptr using the key. If the pointer doesn't exist, the CStaticResourcePtr is NULL.
    CStaticResourcePtr(const TKey &key);
	/// Copy constructor.
    CStaticResourcePtr(const CStaticResourcePtr &copy);
	/// Release the RefPtr.
    ~CStaticResourcePtr(void);


	/// Cast operator. No check are done.
	operator TPtr*()	const;
	/// Cast operator. No check are done.
	TPtr* getPtr()		const;

	/// Indirection operator. No check are done.
    TPtr& operator*(void)	const;
	/// Selection operator. No check are done.
    TPtr* operator->(void)	const;

	/// Retrieve a ptr using the key. If the pointer doesn't exist, the CStaticResourcePtr is NULL.
	CStaticResourcePtr& operator=(const TKey &key);
	/// operator=. Giving a NULL pointer is a valid operation.
    CStaticResourcePtr& operator=(const CStaticResourcePtr &copy);
};


}


// ***************************************************************************
// ***************************************************************************
// Implementation.
// ***************************************************************************
// ***************************************************************************


#include "resource_ptr_inline.h"
#undef	SMART_TRACE
#undef	REF_TRACE

#endif // NL_RESOURCE_PTR_H

/* End of resource_ptr.h */
