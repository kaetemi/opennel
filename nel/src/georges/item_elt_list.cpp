/** \file item_elt_list.cpp
 * Georges system files
 *
 * $Id: item_elt_list.cpp,v 1.1 2002/02/14 10:40:39 corvazier Exp $
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
#include "nel/georges/item_elt_list.h"
#include "nel/georges/item_elt_atom.h"
#include "nel/georges/item_elt_struct.h"
#include "nel/georges/form_body_elt.h"
#include "nel/georges/form_body_elt_list.h"

namespace NLGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemEltList::CItemEltList( CLoader* const _pl ) : CItemElt( _pl )
{
	infos = ITEM_ISLIST;
	piemodel = 0;
}

CItemEltList::~CItemEltList()
{
	if( piemodel )
		delete piemodel;
	piemodel = 0;
	Clear();
}

void CItemEltList::Clear()
{
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
		if( *it )
			delete( *it );
	vpie.clear();
}

void CItemEltList::BuildItem( CItemElt* const _pie )
{
	piemodel = _pie;
	piemodel->AddInfos( ITEM_ISLISTCHILD );
	piemodel->SetListParent( this );
	nlassert( piemodel );
}

CStringEx CItemEltList::GetFormula() const 
{
	nlassert( piemodel );
	return( CStringEx("list<") + piemodel->GetFormula() + ">" );
}

void CItemEltList::FillParent( const CFormBodyElt* const _pfbe )
{
	unsigned int i = 0;
	CStringEx sx;
	CFormBodyElt* pfbe = _pfbe->GetElt(0);
	if( pfbe )
		Clear();	
	while( pfbe )
	{
		CItemElt* pie = piemodel->Clone();
		pie->AddInfos( ITEM_ISLISTCHILD );
		pie->SetListParent( this );
		pie->FillParent( pfbe );
		sx.format( "#%d", i );
		pie->SetName( sx );
		vpie.push_back( pie );
		pfbe = _pfbe->GetElt(++i);
	}
}

void CItemEltList::FillCurrent( const CFormBodyElt* const _pfbe )
{
	unsigned int i = 0;
	CStringEx sx;
	CFormBodyElt* pfbe = _pfbe->GetElt(0);
	if( pfbe )
		Clear();	
	if( !pfbe ) 
		return;
	while( pfbe )
	{
		CItemElt* pie = piemodel->Clone();
		pie->AddInfos( ITEM_ISLISTCHILD );
		pie->SetListParent( this );
		pie->FillCurrent( pfbe );
		sx.format( "#%d", i );
		pie->SetName( sx );
		vpie.push_back( pie );
		pfbe = _pfbe->GetElt(++i);
	}
	SetModified( true );
}

CItemElt* CItemEltList::Clone()
{
	CItemEltList* piel = new CItemEltList( pl );
	piel->BuildItem( piemodel );
	return( piel );
}

CFormBodyElt* CItemEltList::BuildForm()
{
	if( bmodified )
		for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
			(*it)->SetModified( true );
	CFormBodyEltList* pfbel = new CFormBodyEltList();
	pfbel->SetName( sxname );
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
		pfbel->AddElt( (*it)->BuildForm() );
	return( pfbel );
}

unsigned int CItemEltList::GetNbElt() const
{
	unsigned int nb = 0;
	for( std::vector< CItemElt* >::const_iterator it = vpie.begin(); it != vpie.end(); ++it )
		nb += (*it)->GetNbElt();
	return( ++nb );
}

CItemElt* CItemEltList::GetElt( const unsigned int _index ) const
{
	if( !_index )
		return( ( CItemElt * )( this ) );
	unsigned int isum = 1;				
	for( std::vector< CItemElt* >::const_iterator it = vpie.begin(); it != vpie.end(); ++it )
	{
		unsigned int nb = (*it)->GetNbElt();
		if( isum+nb > _index )
			return( (*it)->GetElt( _index-isum ) );
		isum += nb;
	}
	return( 0 );
}

CItemElt* CItemEltList::GetElt( const CStringEx _sxname ) const
{
	for( std::vector< CItemElt* >::const_iterator it = vpie.begin(); it != vpie.end(); ++it )
		if( (*it)->GetName() == _sxname )
			return( *it );
	return( 0 );
}

bool CItemEltList::SetModified( const unsigned int _index )
{
	SetModified( true );
	if( !_index )
		return( true );
	unsigned int isum = 1;				
	for( std::vector< CItemElt* >::const_iterator it = vpie.begin(); it != vpie.end(); ++it )
	{
		unsigned int nb = (*it)->GetNbElt();
		if( isum+nb > _index )
			return( (*it)->SetModified( _index-isum ) );
		isum += nb;
	}
	return( false );
}

void CItemEltList::SetModified( const bool _b )
{
	bmodified = _b;
	for( std::vector< CItemElt* >::const_iterator it = vpie.begin(); it != vpie.end(); ++it )
		(*it)->SetModified( _b );
}

void CItemEltList::NewElt()
{
	CItemElt* pie = piemodel->Clone();
	pie->AddInfos( ITEM_ISLISTCHILD );
	pie->SetListParent( this );
	CStringEx sx;
	sx.format( "#%d", vpie.size() );
	pie->SetName( sx );
	vpie.push_back( pie );
	SetModified( true );
}

void CItemEltList::AddElt( const CItemElt* const _pie )
{
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
		if( (*it) == _pie )
		{
			CItemElt* pie = piemodel->Clone();
			pie->AddInfos( ITEM_ISLISTCHILD );
			pie->SetListParent( this );
			vpie.insert( it, pie );
			VerifyName();
			return;
		}
}

void CItemEltList::DelElt( CItemElt* const _pie )
{
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
		if( (*it) == _pie )
		{
			delete( _pie );
			vpie.erase( it );
			VerifyName();
			return;
		}
}

void CItemEltList::VerifyName()
{
	unsigned int i = 0;
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
	{
			CStringEx sx;
			sx.format( "#%d", i++ );
			(*it)->SetName( sx );
	}
}

}