/** \file item_elt.cpp
 * Georges system files
 *
 * $Id: item_elt.cpp,v 1.2 2002/02/20 15:26:06 besson Exp $
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
#include "nel/georges/item_elt.h"

namespace NLGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemElt::CItemElt( CLoader* const _pl )
{
	nlassert( _pl );
	pl = _pl;
	listparent = 0;
	bmodified = false;
}

CItemElt::~CItemElt()
{
}

bool CItemElt::SetModified( const unsigned int _index )
{
	if( !_index )
		bmodified = true;
	return( !_index );
}

void CItemElt::SetModified( const bool _b )
{
	bmodified = _b;
}

unsigned int CItemElt::GetInfos() const
{
	return( infos );
}

void CItemElt::AddInfos( const unsigned int _infos )
{
	infos |= _infos;
}

void CItemElt::SetListParent( CItemElt* const _listparent )
{
	listparent = _listparent;
}

CItemElt* CItemElt::GetListParent() const
{
	return( listparent );
}

void CItemElt::SetName( const CStringEx _sxname )
{
	sxname = _sxname;
}

CStringEx CItemElt::GetName() const
{
	return( sxname );
}

CStringEx CItemElt::GetParent() const
{
	return( sxparent );
}

CStringEx CItemElt::GetFormula() const
{
	return( CStringEx() );
}

CStringEx CItemElt::GetParentResult() const
{
	return( sxparentresult );
}

CStringEx CItemElt::GetParentValue() const
{
	return( sxparentvalue );
}

CStringEx CItemElt::GetCurrentResult() const
{
	return( sxcurrentresult );
}

CStringEx CItemElt::GetCurrentValue() const
{
	return( sxcurrentvalue );
}

void CItemElt::SetParentValue( const CStringEx _sxparentvalue )
{
}

void CItemElt::SetCurrentValue( const CStringEx _sxcurrentvalue )
{
}

void CItemElt::FillParent( const CFormBodyElt* const _pfbe )
{
}

void CItemElt::FillCurrent( const CFormBodyElt* const _pfbe )
{
}

CItemElt* CItemElt::Clone()
{
	return( 0 );
}

CFormBodyElt* CItemElt::BuildForm()
{
	return( 0 );
}

unsigned int CItemElt::GetNbElt() const
{
	return( 0 );
}

CItemElt* CItemElt::GetElt( const unsigned int _index ) const
{
	if( !_index )
		return( ( CItemElt* )( this ) );
	return( 0 );
}

CItemElt* CItemElt::GetElt( const CStringEx _sxname ) const
{
	return( 0 );
}

unsigned int CItemElt::GetNbChild ()
{
	return 0;
}

CItemElt* CItemElt::GetChild (unsigned int _index)
{
	return NULL;
}

}
