/** \file item_elt_atom.cpp
 * Georges system files
 *
 * $Id: item_elt_atom.cpp,v 1.4 2002/03/14 17:29:57 besson Exp $
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
#include "nel/georges/item_elt_atom.h"
#include "nel/georges/mold_elt_type.h"
#include "nel/georges/mold_elt_type_list.h"
#include "nel/georges/form_body_elt.h"
#include "nel/georges/form_body_elt_atom.h"

namespace NLGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemEltAtom::CItemEltAtom( CLoader* const _pl ) : CItemElt( _pl )
{
	infos = ITEM_ISATOM;
	pmet = 0; 
}

CItemEltAtom::~CItemEltAtom()
{
}

void CItemEltAtom::BuildItem( CMoldElt* const _pme )
{
	if( _pme->IsList() )
		pmet = dynamic_cast< CMoldEltTypeList* >( _pme );      
	else
		pmet = dynamic_cast< CMoldEltType* >( _pme );      
	nlassert( pmet );
	sxparentvalue.clear();
	sxcurrentvalue.clear();
	sxoldparentvalue.clear();
	sxoldcurrentvalue.clear();

	sxparentresult = pmet->GetDefaultValue();
	sxcurrentresult = pmet->GetDefaultValue();

	//SetParentValue(pmet->GetDefaultValue());

	if( pmet->IsEnum() )
		infos |= ITEM_ISENUM;
}

CStringEx CItemEltAtom::GetFormula() const 
{
	nlassert( pmet );
	return( pmet->GetFormula() );
}

void CItemEltAtom::SetParentValue( const CStringEx _sxparentvalue )
{
	nlassert( pmet );
	sxparentvalue = pmet->Format( _sxparentvalue );
	sxparentresult = pmet->CalculateResult( pmet->GetDefaultValue(), sxparentvalue );
	if( sxparentvalue.empty() )
		sxparentresult = pmet->GetDefaultValue();
}

void CItemEltAtom::SetCurrentValue( const CStringEx _sxcurrentvalue )
{
	nlassert( pmet );
	bmodified = true;
	sxcurrentvalue = pmet->Format( _sxcurrentvalue );
	sxcurrentresult = pmet->CalculateResult( sxparentresult, sxcurrentvalue );
	if( !sxcurrentvalue.empty() )
		return;
	sxcurrentvalue.clear();
	sxcurrentresult = sxparentresult;
}

void CItemEltAtom::FillParent( const CFormBodyElt* const _pfbe )
{
	SetParentValue( _pfbe->GetValue() );
	sxoldparentvalue = sxparentvalue;
	SetCurrentValue( sxparentvalue );
}

void CItemEltAtom::FillCurrent( const CFormBodyElt* const _pfbe )
{
	SetCurrentValue( _pfbe->GetValue() );
	sxoldcurrentvalue = sxcurrentvalue;
}

CItemElt* CItemEltAtom::Clone()
{
	CItemEltAtom* piea = new CItemEltAtom( pl );
	piea->BuildItem( pmet );
	return( piea );
}

CFormBodyElt* CItemEltAtom::BuildForm()
{
	if( !bmodified && sxoldcurrentvalue.empty() ) /* && sxcurrentvalue.empty() ??? */
		return( 0 );
	CFormBodyEltAtom* pfbea = new CFormBodyEltAtom();
	pfbea->SetName( sxname );
	pfbea->SetValue( sxcurrentvalue );
	return( pfbea );
}

unsigned int CItemEltAtom::GetNbElt() const
{
	return( 1 );
}

CMoldEltType* CItemEltAtom::GetMoldType() const
{
	return( pmet );
}

unsigned int CItemEltAtom::GetNbChild ()
{
	return 0;
}

CItemElt* CItemEltAtom::GetChild (unsigned int _index)
{
	return NULL;
}

}