/** \file class_registry.h
 * This File handles CClassRegistry.
 *
 * $Id: class_registry.h,v 1.10 2002/03/15 13:45:12 legros Exp $
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

#ifndef NL_CLASS_REGISTRY_H
#define NL_CLASS_REGISTRY_H

#include	"nel/misc/types_nl.h"
#include	"nel/misc/common.h"
#include	<typeinfo>
#include	<string>
#include	<set>


namespace	NLMISC
{


// ======================================================================================================
/**
 * Class Registry Exception.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct ERegistry : public Exception
{
	ERegistry() : Exception( "Registry error" ) {};

	ERegistry( const std::string& str ) : Exception( str ) {};
};

struct ERegisteredClass : public ERegistry
{
	ERegisteredClass() : ERegistry( "Class already registered" ) {};
};

struct EUnregisteredClass : public ERegistry
{
	EUnregisteredClass() : ERegistry( "Class not registered" ) {}
};


// ======================================================================================================
/**
 * An Object Streamable interface.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class IClassable
{
public:
	virtual std::string		getClassName() =0;
};


// ======================================================================================================
/**
 * The Class registry where we can instanciate IClassable objects from their names.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CClassRegistry
{
public:

	///	Register your class for future Instanciation.
	static	void		registerClass(const std::string &className, IClassable* (*creator)(), const std::string &typeidCheck) throw(ERegistry);

	/// Create an object from his class name.
	static	IClassable	*create(const std::string &className) throw(ERegistry);

	/// check if the object has been correctly registered. Must be used for debug only, and Must compile with RTTI.
	static	bool		checkObject(IClassable* obj);


private:
	struct	CClassNode
	{
		std::string			ClassName;
		std::string			TypeIdCheck;
		IClassable*	(*Creator)();
		bool	operator<(const CClassNode &n) const
		{
			return ClassName<n.ClassName;
		}
	};
	static	std::set<CClassNode>		*RegistredClasses;

	/// Inits the ClassRegistry (especially RegistredClasses)
	static void			init();
};


/// Usefull Macros.
#define	NLMISC_DECLARE_CLASS(_class_)					\
	virtual std::string	getClassName() {return #_class_;}		\
	static	NLMISC::IClassable	*creator() {return new _class_;}
#define	NLMISC_REGISTER_CLASS(_class_) NLMISC::CClassRegistry::registerClass(#_class_, _class_::creator, typeid(_class_).name());



}	// namespace NLMISC.


#endif // NL_STREAM_H

/* End of stream.h */
