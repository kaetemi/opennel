/** \file form_body_elt_list.h
 * Georges system files
 *
 * $Id: form_body_elt_list.h,v 1.1 2002/02/14 10:39:35 corvazier Exp $
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

#ifndef NLGEORGES_FORM_BODY_ELT_LIST_H
#define NLGEORGES_FORM_BODY_ELT_LIST_H


#include "nel/misc/stream.h"
#include "nel/georges/form_body_elt.h"

namespace NLGEORGES
{


// La classe CFormBodyEltList permet d'obtenir un ensemble de CFormBodyElt sous forme de liste. 
// Dans l'opérateur +=, l'ensemble des anciens éléments sont éliminés. Les nouveaux les remplacent.
class CFormBodyEltList : public CFormBodyElt  
{
protected:
	std::vector< CFormBodyElt* > vpbodyelt;
	std::vector< CFormBodyElt* >::iterator Find( const CStringEx _sxname );
	std::vector< CFormBodyElt* >::const_iterator Find( const CStringEx _sxname ) const;

public:
	NLMISC_DECLARE_CLASS( CFormBodyEltList );
	CFormBodyEltList();
	CFormBodyEltList( const CFormBodyEltList& _fbel );
	virtual ~CFormBodyEltList();
	virtual void serial( NLMISC::IStream& s );

	virtual CFormBodyElt&	operator  =( const CFormBodyElt& _fbe );
	virtual CFormBodyElt&	operator +=( const CFormBodyElt& _fbe );
	virtual CFormBodyElt&	operator -=( const CFormBodyElt& _fbe );
	virtual bool			operator ==( const CFormBodyElt& _fbe ) const;

	virtual CFormBodyElt* Clone() const;
	virtual bool Empty() const;
	virtual CFormBodyElt* GetElt( const unsigned int _index ) const;
	virtual CFormBodyElt* GetElt( const CStringEx _sxname ) const;

	void AddElt( CFormBodyElt* const pfbe );
	void Clear();
};

} // NLGEORGES

#endif // NLGEORGES_FORM_BODY_ELT_LIST_H

