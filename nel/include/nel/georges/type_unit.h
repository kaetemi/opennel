/** \file type_unit.h
 * Georges system files
 *
 * $Id: type_unit.h,v 1.2 2002/02/20 18:05:28 lecroart Exp $
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

#ifndef NLGEORGES_TYPE_UNIT_H
#define NLGEORGES_TYPE_UNIT_H

#include "nel/georges/string_ex.h"

namespace NLGEORGES
{

class CTypeUnit  
{
protected:
	CStringEx	sxformula;
	CStringEx	sxhighlimit;
	CStringEx	sxlowlimit;
	CStringEx	sxdefaultvalue;

public:
	CTypeUnit( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv, const CStringEx _sxf );
	virtual ~CTypeUnit();

	CStringEx GetFormula() const;													
	CStringEx GetDefaultValue() const;													
	CStringEx GetLowLimit() const;													
	CStringEx GetHighLimit() const;													
	virtual	void	SetDefaultValue( const CStringEx _sxdv );
	virtual	void	SetLowLimit( const CStringEx _sxll );
	virtual	void	SetHighLimit( const CStringEx _sxhl );
	virtual	CStringEx Format( const CStringEx _sxvalue ) const;									
	virtual	CStringEx CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const;	
};

} // NLGEORGES

#endif // NLGEORGES_TYPE_UNIT_H
