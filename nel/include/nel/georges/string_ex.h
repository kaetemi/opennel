/** \file string_ex.h
 * Georges system files
 *
 * $Id: string_ex.h,v 1.1 2002/02/14 10:39:35 corvazier Exp $
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

#ifndef NLGEORGES_STRING_EX_H
#define NLGEORGES_STRING_EX_H

#include "nel/misc/stream.h"

namespace NLGEORGES
{

class CStringEx : public std::basic_string<char, std::char_traits<char>, std::allocator<char> >  
{

public:
	CStringEx()						: std::basic_string<char, std::char_traits<char>, std::allocator<char> >() {;} 
	CStringEx( char* _pc ) 			: std::basic_string<char, std::char_traits<char>, std::allocator<char> >( (char *)_pc ) {};
	CStringEx( std::string _s ) 	: std::basic_string<char, std::char_traits<char>, std::allocator<char> >( (std::string)_s) {}; 
	CStringEx( const char _c, int _i ) 	{ append( _i, _c ); }
	
	virtual ~CStringEx();
	CStringEx	get_remove() const;
	CStringEx	get_remove( const char ) const;
	CStringEx	get_make_lower() const;
	CStringEx	get_make_upper() const;
	CStringEx	get_trim_left() const;
	CStringEx	get_trim_left( const char _c ) const;
	CStringEx	get_trim_right() const; 
	CStringEx	get_trim_right( const char _c ) const;
	CStringEx	get_trim() const;
	CStringEx	get_trim( const char _c ) const;
	CStringEx	get_mid( const int nFirst, const int nCount ) const; 
	CStringEx	get_mid( const int nFirst ) const; 
	CStringEx	get_left( const int nCount ) const; 
	CStringEx	get_right( const int nCount ) const;
	CStringEx	get_purge() const;

	void		remove();
	void		remove( const char _c );
	void		make_lower();
	void		make_upper();
	void		trim_left();
	void		trim_left( const char _c );
	void		trim_right();
	void		trim_right( const char _c );
	void		trim();
	void		trim( const char _c );
	void		mid( const int nFirst, const int nCount ); 
	void		mid( const int nFirst ); 
	void		left( const int nCount ); 
	void		right( const int nCount );
	void		purge();

	void		format( const char*, ... ); 
	int			reverse_find( const char _c ) const; 
	
	void	serial( NLMISC::IStream& s ) { s.serial( (std::string&)(*this) );}

	bool	operator <= ( const CStringEx& s ) const;
	bool	operator < ( const CStringEx& s ) const;
};

} // NLGEORGES

#endif // NLGEORGES_STRING_EX_H
