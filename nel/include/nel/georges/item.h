/** \file item.h
 * Georges system files
 *
 * $Id: item.h,v 1.2 2002/03/12 09:23:28 besson Exp $
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

#ifndef NLGEORGES_ITEM_H
#define NLGEORGES_ITEM_H

#include "nel/georges/string_ex.h"
#include "nel/georges/form.h"

namespace NLGEORGES
{

class CLoader;
class CItemElt;
class CItemEltAtom;
class CItemEltStruct;
class CItemEltList;

class CItem  
{
protected:
	CLoader* pl;
	CItemEltStruct* pitemes;
//	CItemEltList* pitemelparents;
//	CItemEltAtom* pitemeacomments;
	std::vector< std::pair< CStringEx, CStringEx > > vsxparents;
	CStringEx moldfilename;

public:
	CItem();
	virtual ~CItem();

	void Clear();
	void SetLoader( CLoader* const _pl );
	void New( const CStringEx& _sxdfnfilename );
	void Load( const CStringEx& _sxfilename );
	void Load( const CStringEx& _sxfilename, const CStringEx _sxdate );
	void Save( const CStringEx& _sxfilename );
	bool Update();

	// Convert CItem to a CForm (in is this)
	void MakeForm (CForm &out);
	// Convert CForm to CItem (out is this)
	void MakeItem (CForm &in);

	void SetCurrentValue( const unsigned int _index, const CStringEx s );
	unsigned int GetNbElt() const;
	unsigned int GetNbParents() const;
	unsigned int GetNbElt( const unsigned int _index ) const;
	unsigned int GetInfos( const unsigned int _index ) const;
	CStringEx GetName( const unsigned int _index ) const;
	CStringEx GetCurrentResult( const unsigned int _index ) const;
	CStringEx GetCurrentValue( const unsigned int _index ) const;
	CStringEx GetFormula( const unsigned int _index ) const;
	bool IsEnum( const unsigned int _index ) const; 
	bool IsPredef( const unsigned int _index ) const; 
	bool CanEdit( const unsigned int _index ) const; 
	void GetListPredef( const unsigned int _index, std::vector< CStringEx >& _vsx ) const;

	CItemElt* GetElt( const unsigned int _index ) const;
	CItemElt* GetElt( const CStringEx _sxname ) const;

	void AddList( const unsigned int _index ) const;
	void DelListChild( const unsigned int _index ) const;
	void VirtualSaveLoad();

	void AddParent( const unsigned int _index = 0);
	void DelParent( const unsigned int _index );
	void ClearParents ();
	CStringEx GetParent( const unsigned int _index ) const;
	void SetParent( const unsigned int _index, const CStringEx _sx );
	CStringEx GetActivity( const unsigned int _index ) const;
	void SetActivity( const unsigned int _index, const CStringEx _sx );
};

} // NLGEORGES

#endif // NLGEORGES_ITEM_H
