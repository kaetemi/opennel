/** \file fast_ptr_list.h
 * <File description>
 *
 * $Id: fast_ptr_list.h,v 1.1 2003/03/20 15:00:58 berenguier Exp $
 */

/* Copyright, 2000-2003 Nevrax Ltd.
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

#ifndef NL_FAST_PTR_LIST_H
#define NL_FAST_PTR_LIST_H

#include "nel/misc/types_nl.h"


namespace NL3D 
{


class CFastPtrListBase;


// ***************************************************************************
/**
 * See CFastPtrListBase. Each class you want to insert in a CFastPtrList should have a CFastPtrListNode.
 */
class CFastPtrListNode
{
public:
	CFastPtrListNode() {_Owner= NULL;}
	~CFastPtrListNode() {unlink();}
	// No-op const copy
	CFastPtrListNode(const CFastPtrListNode &o) {_Owner= NULL;}

	// If linked to a list, remove me from it.
	void			unlink();

	// linked?
	bool			isLinked() const {return _Owner!=NULL;}

	// No-op operator=
	CFastPtrListNode	&operator=(CFastPtrListNode &o)
	{
		return *this;
	}


private:
	friend class CFastPtrListBase;
	CFastPtrListBase	*_Owner;
	uint32				_IndexInOwner;
};


// ***************************************************************************
/**
 * This class store actually an array of void*, for very fast acces (list is slower because of RAM access).
 *	CFastPtrListBase advantages are the insert() and erase() are in O(1)
 *	Overhead Cost is 8 bytes per node + 4 bytes in the _Nodes array.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CFastPtrListBase
{
public:
	/// Constructor
	CFastPtrListBase() {}
	~CFastPtrListBase();

	/// insert an element in the list through its Node, unlinking older if necessary
	void			insert(void *element, CFastPtrListNode *node);
	/// erase an element in the list through its Node. No-op if the list does not have this element
	void			erase(CFastPtrListNode *node);

	/// Get the head on the array of elements. NULL if none
	void			**begin() { if(_Elements.empty()) return NULL; else return &_Elements[0];}
	/// get the number of elements
	uint			size() const {return _Elements.size();}
	bool			empty() const {return _Elements.empty();}

	/// clear the list
	void			clear();

// **************
private:
	// The 2 lists of same size. Splitted in 2 lists for optimum _Elements accessing.
	std::vector<void*>				_Elements;
	std::vector<CFastPtrListNode*>	_Nodes;
};


// ***************************************************************************
/** Type Safe version of CFastPtrListBase
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
template< class T >
class CFastPtrList : public CFastPtrListBase
{
public:
	/// Constructor
	CFastPtrList() {}
	~CFastPtrList() {}

	/// insert an element in the list through its Node, unlinking older if necessary
	void			insert(T *element, CFastPtrListNode *node) {CFastPtrListBase::insert(element, node);}
	/// erase an element in the list through its Node, unlinking older if necessary
	void			erase(CFastPtrListNode *node) {CFastPtrListBase::erase(node);}

	/// Get the head on the array of elements. NULL if none
	T				**begin() {return (T**)CFastPtrListBase::begin();}
	/// get the number of elements
	uint			size() const {return CFastPtrListBase::size();}
	bool			empty() const {return CFastPtrListBase::empty();}

	/// clear the list
	void			clear() {CFastPtrListBase::clear();}

};


} // NL3D


#endif // NL_FAST_PTR_LIST_H

/* End of fast_ptr_list.h */
