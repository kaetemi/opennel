/** \file form_body_elt_list.cpp
 * Georges system files
 *
 * $Id: form_body_elt_list.cpp,v 1.2 2002/03/04 12:53:29 cado Exp $
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
#include "nel/georges/form_body_elt_list.h"

namespace NLGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFormBodyEltList::CFormBodyEltList()
{
}

CFormBodyEltList::CFormBodyEltList( const CFormBodyEltList& _fbel )
{
	sxname = _fbel.sxname;
	for( std::vector< CFormBodyElt* >::const_iterator it = _fbel.vpbodyelt.begin(); it != _fbel.vpbodyelt.end(); ++it )
		vpbodyelt.push_back( (*it)->Clone() );
}

CFormBodyEltList::~CFormBodyEltList()
{
	Clear();
}

void CFormBodyEltList::Clear()
{
	for( std::vector< CFormBodyElt* >::iterator it = vpbodyelt.begin(); it != vpbodyelt.end(); ++it )
		if( *it )
			delete *it;
	vpbodyelt.clear();
}

void CFormBodyEltList::serial( NLMISC::IStream& s)
{

	s.xmlPushBegin( "List" );
		s.xmlSetAttrib( "Name" );
			s.serial( sxname );
	s.xmlPushEnd();
		s.serialContPolyPtr( vpbodyelt );
	s.xmlPop();
}

std::vector< CFormBodyElt* >::iterator CFormBodyEltList::Find( const CStringEx _sxname ) 
{
  std::vector< CFormBodyElt* >::iterator it;
	for( it = vpbodyelt.begin(); it != vpbodyelt.end(); ++it )
		if( (*it)->GetName() == _sxname )
			return it;
	return it;
}

std::vector< CFormBodyElt* >::const_iterator CFormBodyEltList::Find( const CStringEx _sxname ) const 
{
  std::vector< CFormBodyElt* >::const_iterator it;
	for( it = vpbodyelt.begin(); it != vpbodyelt.end(); ++it )
		if( (*it)->GetName() == _sxname )
			return it;
	return it;
}

CFormBodyElt& CFormBodyEltList::operator =( const CFormBodyElt& _fbe )
{
	const CFormBodyEltList* pfbel = dynamic_cast< const CFormBodyEltList* >( &_fbe );      
	nlassert( pfbel );
	sxname = pfbel->sxname;
	return( *this );
}

CFormBodyElt& CFormBodyEltList::operator +=( const CFormBodyElt& _fbe )
{
	const CFormBodyEltList* pfbel = dynamic_cast< const CFormBodyEltList* >( &_fbe );      
	nlassert( pfbel );
	Clear();	
	for( std::vector< CFormBodyElt* >::const_iterator it = pfbel->vpbodyelt.begin(); it != pfbel->vpbodyelt.end(); ++it )
		vpbodyelt.push_back( (*it)->Clone() );
	return( *this );
}

CFormBodyElt& CFormBodyEltList::operator -=( const CFormBodyElt& _fbe )
{
	const CFormBodyEltList* pfbel = dynamic_cast< const CFormBodyEltList* >( &_fbe );      
	nlassert( pfbel );
	if( *this == *pfbel )
		Clear();
	return( *this );
}

bool CFormBodyEltList::operator ==( const CFormBodyElt& _fbe ) const
{
	const CFormBodyEltList* pfbel = dynamic_cast< const CFormBodyEltList* >( &_fbe );      
	nlassert( pfbel );
	if( vpbodyelt.size() != pfbel->vpbodyelt.size() )
	return( false );
	std::vector< CFormBodyElt* >::const_iterator it = pfbel->vpbodyelt.begin();
	std::vector< CFormBodyElt* >::const_iterator iu = vpbodyelt.begin();
	while( iu != vpbodyelt.end() )
		if( !(**iu++ == **it++) )
			return false;
	return( true );
}

CFormBodyElt* CFormBodyEltList::Clone() const 
{
	return( new CFormBodyEltList( *this ) );
}

bool CFormBodyEltList::Empty() const
{
	return( vpbodyelt.empty() );
}

CFormBodyElt* CFormBodyEltList::GetElt( const unsigned int _index ) const
{
	if( _index >= vpbodyelt.size() )
		return( 0 );
	return( vpbodyelt[_index] ); 
}

CFormBodyElt* CFormBodyEltList::GetElt( const CStringEx _sxname ) const
{
	std::vector< CFormBodyElt* >::const_iterator it = Find( _sxname );
	if( it == vpbodyelt.end() ) 
		return( 0 );
	return( *it );
}

void CFormBodyEltList::AddElt( CFormBodyElt* const pfbe )
{
	if( !pfbe )
		return;
	vpbodyelt.push_back( pfbe );
}

}
