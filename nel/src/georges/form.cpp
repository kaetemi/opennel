/** \file form.cpp
 * Georges system files
 *
 * $Id: form.cpp,v 1.1 2002/02/14 10:40:39 corvazier Exp $
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

#include "nel/georges/form.h"
#include "nel/georges/form_body_elt.h"

namespace NLGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CForm::CForm()
{
	body.SetName( "BODY" );
	bmodified = false;
}

CForm::CForm( const CForm& _f )
{
	bmodified = _f.bmodified;
	head = _f.head;
	body = _f.body;
}

CForm::~CForm()
{
}

void CForm::serial( NLMISC::IStream& s )
{
	s.serial( head );
	s.serial( body );
}

CStringEx CForm::GetDate() const
{
	return( head.GetDate() );
}

CStringEx CForm::GetVersion() const
{
	return( head.GetVersion() );
}

CStringEx CForm::GetUser() const
{
	return( head.GetUser() );
}

CStringEx CForm::GetComment() const
{
	return( body.GetComment() );
}

void CForm::SetDate( const CStringEx _sxdate )
{
	head.SetDate( _sxdate );
}

void CForm::SetVersion( const CStringEx _sxversion )
{
	head.SetVersion( _sxversion );
}

void CForm::SetUser( const CStringEx _sxuser )
{
	head.SetUser( _sxuser );
}

void CForm::SetComment( const CStringEx _sxcomment )
{
	body.SetComment( _sxcomment );
}

CStringEx CForm::GetParent( unsigned int _index ) const
{
	return( body.GetParent( _index ) );
}

CStringEx CForm::GetActivity( unsigned int _index ) const
{
	return( body.GetActivity( _index ) );
}

CFormBodyElt* CForm::GetElt( const CStringEx _sxname ) const
{
	return( body.GetElt( _sxname ) );
}

CFormBodyEltStruct* CForm::GetBody() const
{
	return( (CFormBodyEltStruct*)(&body) );
}

bool CForm::GetModified() const
{
	return( bmodified );
}

void CForm::SetModified( const bool _b )
{
	bmodified = _b;
}

CForm& CForm::operator =( const CForm& _f )
{
	head = _f.head;
	body = _f.body;
	bmodified = true;
	return( *this );
}

CForm& CForm::operator +=( const CForm& _f )
{
	head = _f.head;
	body += _f.body;
	bmodified = true;
	return( *this );
}

CForm& CForm::operator -=( const CForm& _f )
{
	body -= _f.body;
	bmodified = true;
	return( *this );
}

bool CForm::operator ==( const CForm& _f ) const
{
	return( (body == _f.body) );
}

}