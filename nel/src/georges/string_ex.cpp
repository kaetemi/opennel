/** \file string_ex.cpp
 * Georges system files
 *
 * $Id: string_ex.cpp,v 1.2 2002/03/04 12:53:29 cado Exp $
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

#include "stdgeorges.h"
#include "nel/georges/string_ex.h"

namespace NLGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace std;
CStringEx::~CStringEx()
{

}

void CStringEx::remove( const char _c )
{
	if( empty() )
		return;
	iterator it = begin();
	while( it != end() ) 
	{
		if( (*it) == _c )
			it = erase( it );
		else
			++it;
	}
}

void CStringEx::remove()
{
	remove(' ');
	remove('\t');
	remove('\n');
}

void	CStringEx::make_lower( )
{
	for( iterator it = begin(); it != end(); ++it )
		if( (*it >= 'A')&&(*it <= 'Z') )
			*it += 'a'-'A';
}

void	CStringEx::make_upper( )
{
	for( iterator it = begin(); it != end(); ++it )
		if( (*it >= 'a')&&(*it <= 'z') )
			*it += 'A'-'a';
}

void	CStringEx::trim_left( )
{
	if( empty() )
		return;
	iterator it;
	for( it = begin(); (it != end())&&( (*it==' ')||(*it=='\t')||(*it=='\n') ); ++it );
	erase( begin(), it );
}

void	CStringEx::trim_left( const char _c )
{
	if( empty() )
		return;
	iterator it;
	for( it = begin(); (it != end())&&( *it == _c ); ++it );
	erase( begin(), it );
}

void	CStringEx::trim_right( ) 
{
	if( empty() )
		return;
	iterator it = end();
	--it;
	while( it != begin() )
	{
		iterator i = it--;
		if( (*i==' ')||(*i=='\t')||(*i=='\n') )
			erase( i );
		else
			break;
	}
	if( (*it==' ')||(*it=='\t')||(*it=='\n') )
		erase( it );
}

void	CStringEx::trim_right( char c )
{
	if( empty() )
		return;
	iterator it = end();
	while( it != begin() )
	{
		iterator i = it--;
		if( *i == c )
			erase( i );
		else
			break;
	}
	if( *it == c )
		erase( it );
}

void CStringEx::trim()
{
	trim_left();
	trim_right();
}

void CStringEx::purge()
{
	make_lower();
	remove(' ');
	remove('\t');
	remove('\n');
}

void CStringEx::trim( const char _c )
{
	trim_left( _c );
	trim_right( _c );
}

void	CStringEx::mid( const int nFirst )
{
	CStringEx s( *this );
	erase();
	append( s.get_mid( nFirst ));
}

void	CStringEx::mid( const int nFirst, const int nCount )
{
	CStringEx s( *this );
	erase();
	append( s.get_mid( nFirst, nCount ));
}

void	CStringEx::left( const int nCount )
{
	CStringEx s( *this );
	erase();
	append( s.get_left( nCount ));
}

void	CStringEx::right( const int nCount )
{
	CStringEx s( *this );
	erase();
	append( s.get_right( nCount ));
}


CStringEx CStringEx::get_remove( const char _c ) const
{
	CStringEx s( *this );
	s.remove( _c );
	return( s );
}

CStringEx CStringEx::get_remove() const
{
	CStringEx s( *this );
	s.remove();
	return( s );
}

CStringEx CStringEx::get_make_lower() const
{
	CStringEx s( *this );
	s.make_lower();
	return( s );
}

CStringEx CStringEx::get_make_upper() const
{
	CStringEx s( *this );
	s.make_upper();
	return( s );
}

CStringEx CStringEx::get_trim_left() const
{
	CStringEx s( *this );
	s.trim_left();
	return( s );
}

CStringEx CStringEx::get_trim_left( const char _c ) const
{
	CStringEx s( *this );
	s.trim_left( _c );
	return( s );
}

CStringEx CStringEx::get_trim_right() const
{
	CStringEx s( *this );
	s.trim_right();
	return( s );
}

CStringEx CStringEx::get_trim_right( const char _c ) const
{
	CStringEx s( *this );
	s.trim_right( _c );
	return( s );
}

CStringEx CStringEx::get_trim() const
{
	CStringEx s( *this );
	s.trim();
	return( s );
}

CStringEx CStringEx::get_purge() const
{
	CStringEx s( *this );
	s.purge();
	return( s );
}

CStringEx CStringEx::get_trim( const char _c ) const
{
	CStringEx s( *this );
	s.trim( _c );
	return( s );
}

CStringEx	CStringEx::get_mid( const int nFirst ) const
{
	if( !size() )
	  {
	    CStringEx object;
		return( object );
	  }
	return( get_right( size()-nFirst ) );
}

CStringEx	CStringEx::get_mid( const int nFirst, const int nCount ) const
{
	if( !size() )
	  {
	    CStringEx object;
		return( object );
	  }
	return( substr( nFirst, nCount ) );
}

CStringEx	CStringEx::get_left( const int nCount ) const
{
	if( !size() )
	  {
	    CStringEx object;
		return( object );
	  }
	return( substr( 0, nCount ) );
}

CStringEx	CStringEx::get_right( const int nCount ) const
{
	if( !size() )
	  {
	    CStringEx object;
		return( object );
	  }
	return( substr( size()-nCount, nCount ) );
}


bool  CStringEx::operator <= ( const CStringEx& s ) const 
{
	const_iterator it = begin();
	const_iterator is = s.begin();
	while( ( it != end() )&&( is != s.end() ) )
	{
		if( *it != *is )
			return( *it < *is );
		it++;
		is++;
	}
	return( ( it == end() )&&( is == s.end() ) );
}

bool  CStringEx::operator < ( const CStringEx& s ) const 
{
	const_iterator it = begin();
	const_iterator is = s.begin();
	while( ( it != end() )&&( is != s.end() ) )
	{
		if( *it != *is )
			return( *it < *is );
		it++;
		is++;
	}
	return( is != s.end() );
}

int CStringEx::reverse_find( const char _c ) const
{
	unsigned int i = length();
	const_iterator it = end();
	while( it != begin() )
	{
		--it;
		--i;
		if( *it == _c )
			return( i );
	}
	return( npos );
}

void CStringEx::format( const char* s, ... )
{
	char *p = new char[256];
	va_list ap;
	va_start(ap, s);
	int x = vsprintf( p, s, ap);
	erase();
	append(p);
	delete[] p;
}

/*
   #include <stdio.h>
   #include <stdlib.h>
   #include <stdarg.h>
   char *newfmt(const char *fmt, ...)
   {
	   char *p;
	   va_list ap;
	   if ((p = malloc(128)) == NULL)
			   return (NULL);
	   va_start(ap, fmt);
	   (void) vsnprintf(p, 128, fmt, ap);
	   va_end(ap);
	   return (p);
   }
*/

}
