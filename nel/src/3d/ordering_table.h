/** \file ordering_table.h
 * Generic Ordering Table
 *
 * $Id: ordering_table.h,v 1.5 2002/07/01 09:14:03 berenguier Exp $
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

#ifndef NL_ORDERING_TABLE_H
#define NL_ORDERING_TABLE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include <vector>

namespace NL3D 
{

// ***************************************************************************
/**
 * 
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2000
 */
template<class T> class COrderingTable
{

public:

	COrderingTable();
	~COrderingTable();

	/**
	 * Initialization.
	 * The ordering table has a range from 0 to nNbEntries-1
	 */
	void init( uint32 nNbEntries );

	/**
	 * Just return the number of entries in the ordering table
	 */
	uint32 getSize();

	/**
	 * Put the ordering table to empty
	 *	\param maxElementToInsert prepare allocator for insert by setting maximum insert() that will arise.
	 */
	void reset(uint maxElementToInsert);

	/**
	 * Insert an element in the ordering table
	 *	NB: element is inserted in front of the list at nEntryPos (for optim consideration)
	 *	NB: nlassert in debug if num of insert() calls exceed value passed in reset()
	 *	NB: nlassert in debug if nEntryPos is => getSize()
	 */
	void insert( uint32 nEntryPos, T *pValue );

	/**
	 * Traversing operations
	 * 
	 *	OrderingTable<Face> ot;
	 *	ot.begin();
	 *	while( ot.get() != NULL )
	 *	{
	 *		Face *pF = ot.get();
	 *		// Do the treatment you want here
	 *		ot.next();
	 *	}
	 */
	void begin();

	/**
	 * Get the currently selected element.
	 */
	T* get();

	/**
	 * Move selection pointer to the next element
	 */
	void next();

// =================
// =================
// IMPLEMENTATION.
// =================
// =================
private:

	struct CNode
	{
		T *val;
		CNode *next;

		CNode()
		{ 
			val = NULL;
			next = NULL;
		}
	};

	// a raw allocator of node.
	std::vector<CNode>	_Allocator;
	CNode				*_CurAllocatedNode;

	uint32 _nNbElt;
	CNode* _Array;
	CNode* _SelNode;

};

// ***************************************************************************
template<class T> COrderingTable<T>::COrderingTable()
{
	_nNbElt = 0;
	_Array = NULL;
	_SelNode = NULL;
	_CurAllocatedNode= NULL;
}

// ***************************************************************************
template<class T> COrderingTable<T>::~COrderingTable()
{
	if( _Array != NULL )
		delete [] _Array;
}

// ***************************************************************************
template<class T> void COrderingTable<T>::init( uint32 nNbEntries )
{
	if( _Array != NULL )
	{
		reset(0);
		delete [] _Array;
	}
	_nNbElt = nNbEntries;
	_Array = new CNode[_nNbElt];
	reset(0);
}

// ***************************************************************************
template<class T> uint32 COrderingTable<T>::getSize()
{
	return _nNbElt;
}

// ***************************************************************************
template<class T> void COrderingTable<T>::reset(uint maxElementToInsert)
{
	// reset allocation
	maxElementToInsert= max(1U, maxElementToInsert);
	_Allocator.resize(maxElementToInsert);
	_CurAllocatedNode= &_Allocator[0];

	// reset OT.
	for( uint32 i = 0; i < _nNbElt-1; ++i )
	{
		_Array[i].val = NULL;
		_Array[i].next = &_Array[i+1];
	}
	_Array[_nNbElt-1].val  = NULL;
	_Array[_nNbElt-1].next = NULL;
}

// ***************************************************************************
template<class T> void COrderingTable<T>::insert( uint32 nEntryPos, T *pValue )
{
#ifdef NL_DEBUG
	// check not so many calls to insert()
	nlassert( !_Allocator.empty() && _CurAllocatedNode < (&_Allocator[0])+_Allocator.size() );
	// check good entry size
	nlassert( nEntryPos < _nNbElt );
#endif
	// get the head list node
	CNode *headNode = &_Array[nEntryPos];
	// alocate a new node
	CNode *nextNode = _CurAllocatedNode++;
	// fill this new node with data of head node
	nextNode->val= headNode->val;
	nextNode->next= headNode->next;
	// and replace head node with new data: consequence is pValue is insert in front of the list
	headNode->val= pValue;
	headNode->next= nextNode;
	// NB: prec of headNode is still correclty linked to headNode.
}

// ***************************************************************************
template<class T> void COrderingTable<T>::begin()
{
	_SelNode = &_Array[0];
	if( _SelNode->val == NULL )
		next();
}

// ***************************************************************************
template<class T> T* COrderingTable<T>::get()
{
	if( _SelNode != NULL )
		return _SelNode->val;
	else
		return NULL;
}

// ***************************************************************************
template<class T> void COrderingTable<T>::next()
{
	_SelNode = _SelNode->next;
	while( ( _SelNode != NULL )&&( _SelNode->val == NULL ) )
		_SelNode = _SelNode->next;
}

} // NL3D


#endif // NL_ORDERING_TABLE_H

/* End of ordering_table.h */
