/** \file form.h
 * Georges system files
 *
 * $Id: form.h,v 1.2 2002/04/02 08:59:15 corvazier Exp $
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

#ifndef NLGEORGES_FORM_H
#define NLGEORGES_FORM_H

#include "nel/misc/stream.h"
#include "nel/georges/form_head.h"
#include "nel/georges/form_body_elt_struct.h"

namespace NLGEORGES
{

class CFormBodyElt;

// La classe CForm représente une fiche. 
// Elle comprend Head et Body.
// Elle comporte des opérateurs qui permettent de l'additionner ou de la soustraire dans le cadre des historiques.
class CForm  
{
protected:
	CFormHead			head;
	CFormBodyEltStruct	body;
	bool				bmodified;

public:
	CForm();
	CForm( const CForm& _f );
	virtual ~CForm();

	void serial( NLMISC::IStream& s );
	
	CStringEx GetDate() const;
	CStringEx GetVersion() const;
	CStringEx GetUser() const;
	CStringEx GetComment() const;
	void SetDate( const CStringEx _sxdate );
	void SetVersion( const CStringEx _sxversion );
	void SetUser( const CStringEx _sxuser );
	void SetComment( const CStringEx _sxcomment );
	
	
	CStringEx GetParent( unsigned int _index ) const;
	CStringEx GetActivity( unsigned int _index ) const;
	CFormBodyElt* GetElt( const CStringEx _sxname ) const;
	CFormBodyEltStruct* GetBody() const;
	bool GetModified() const;
	void SetModified( const bool _b );
	
	CForm&	operator  =( const CForm& _f );
	CForm&	operator +=( const CForm& _f );
	CForm&	operator -=( const CForm& _f );
	bool	operator ==( const CForm& _f ) const;
};

} // NLGEORGES

#endif // NLGEORGES_FORM_H

