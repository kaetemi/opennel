/** \file form_body_elt.cpp
 * Georges system files
 *
 * $Id: form_body_elt.cpp,v 1.1 2002/02/14 10:40:39 corvazier Exp $
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
#include "nel/georges/form_body_elt.h"

namespace NLGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFormBodyElt::CFormBodyElt()
{
}

CFormBodyElt::CFormBodyElt( const CFormBodyElt& _fbe )
{
}

CFormBodyElt::~CFormBodyElt()
{
}

void CFormBodyElt::serial( NLMISC::IStream& s)
{
}

CFormBodyElt& CFormBodyElt::operator =( const CFormBodyElt& _fbe )
{
	return( *this );
}

CFormBodyElt& CFormBodyElt::operator +=( const CFormBodyElt& _fbe )
{
	return( *this );
}

CFormBodyElt& CFormBodyElt::operator -=( const CFormBodyElt& _fbe )
{
	return( *this );
}

bool CFormBodyElt::operator ==( const CFormBodyElt& _fbe ) const
{
	return( false );
}

CFormBodyElt* CFormBodyElt::Clone() const 
{
	return( new CFormBodyElt( *this ) );
}

CStringEx CFormBodyElt::GetName() const
{
	return( sxname );
}

void CFormBodyElt::SetName( const CStringEx _sxname )
{
	sxname = _sxname;
}

bool CFormBodyElt::Empty() const
{
	return( false );
}

CStringEx CFormBodyElt::GetValue() const
{
	return( CStringEx() );
}

CFormBodyElt* CFormBodyElt::GetElt( const unsigned int _index ) const
{
	return( 0 );
}

CFormBodyElt* CFormBodyElt::GetElt( const CStringEx _sxname ) const
{
	return( 0 );
}

}