/** \file mold_elt.h
 * Georges system files
 *
 * $Id: mold_elt.h,v 1.1 2002/02/14 10:39:35 corvazier Exp $
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

#ifndef NLGEORGES_MOLD_ELT_H
#define NLGEORGES_MOLD_ELT_H

#include "nel/georges/string_ex.h"

namespace NLGEORGES
{

class CLoader;

class CMoldElt  
{
protected:
	CLoader*	pl;
	bool		benum;
	bool		blist;
	CStringEx	sxname;
	CStringEx	sxfullname;
	
public:
	CMoldElt( CLoader* const _pl );
	virtual ~CMoldElt();

	CStringEx	GetName() const;													
	void SetName( const CStringEx& _sxname );
	bool IsEnum() const;
	bool IsList() const;
/*
	virtual CStringEx	GetName() const;													
	virtual void SetName( const CStringEx& _sxname );
	virtual bool IsEnum() const;
	virtual bool IsList() const;
*/
	virtual	CStringEx GetFormula();												
	virtual void Load( const CStringEx _sxfullname );
	virtual void Load( const CStringEx _sxfullname, const CStringEx _sxdate );
	virtual CStringEx GetEltName( const unsigned int _index ) const;
	virtual CMoldElt* GetEltPtr( const unsigned int _index ) const;
	virtual unsigned int GetType() const; 
	virtual CMoldElt* GetMold(); 
};

} // NLGEORGES

#endif // NLGEORGES_MOLD_ELT_H
