/** \file mold_elt_type_list.cpp
 * Georges system files
 *
 * $Id: mold_elt_type_list.cpp,v 1.2 2002/02/21 16:54:42 besson Exp $
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
#include "nel/georges/mold_elt_type_list.h"
#include "nel/georges/georges_loader.h"
#include "nel/georges/form_body_elt.h"
#include "nel/georges/type_unit_int_unsigned.h"
#include "nel/georges/type_unit_int_signed.h"
#include "nel/georges/type_unit_double.h"
#include "nel/georges/type_unit_string.h"
#include "nel/georges/type_unit_file_name.h"

namespace NLGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMoldEltTypeList::CMoldEltTypeList( CLoader* const _pl, CMoldEltType* const _pmet ) : CMoldEltType( _pl )
{
	pmet = _pmet;
	blist = true;
	benum = pmet->IsEnum();
	sxname = pmet->GetName();
}

CMoldEltTypeList::~CMoldEltTypeList()
{
}

void CMoldEltTypeList::Load( const CStringEx _sxfullname )						// TODO: Load with parents...
{
	pmet->Load( _sxfullname );
	benum = pmet->IsEnum();
	sxname = pmet->GetName();
}

void CMoldEltTypeList::Load( const CStringEx _sxfullname, const CStringEx _sxdate )
{
	pmet->Load( _sxfullname, _sxdate );
}

CStringEx CMoldEltTypeList::GetFormula()													 
{
	return( pmet->GetFormula() );
}

CStringEx CMoldEltTypeList::Format( const CStringEx _sxvalue ) const													
{
	return( pmet->Format( _sxvalue ) );
}

CStringEx CMoldEltTypeList::CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const	
{
	return( pmet->CalculateResult( _sxbasevalue, _sxvalue ) );
}

CStringEx CMoldEltTypeList::GetDefaultValue() const	
{
	return( pmet->GetDefaultValue() );
}

unsigned int CMoldEltTypeList::GetType() const
{
	return( pmet->GetType() );
}

CStringEx CMoldEltTypeList::GetPredefSubstitute( const CStringEx _sxdesignation ) const
{
	return( pmet->GetPredefSubstitute( _sxdesignation ) );
}

CStringEx CMoldEltTypeList::GetPredefDesignation( const CStringEx _sxsubstitute ) const
{
	return( pmet->GetPredefDesignation( _sxsubstitute ) );
}

CStringEx CMoldEltTypeList::GetPredefDesignation( const unsigned int _index ) const
{
	return( pmet->GetPredefDesignation( _index ) );
}

CMoldElt* CMoldEltTypeList::GetMold()
{
	return( pmet );
}

}