/** \file loader.cpp
 * Georges system files
 *
 * $Id: loader.cpp,v 1.1 2002/02/14 10:40:39 corvazier Exp $
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
#include "nel/georges/loader.h"

#include "nel/misc/path.h"
#include "nel/georges/form_body_elt_atom.h"
#include "nel/georges/form_body_elt_struct.h"
#include "nel/georges/form_body_elt_list.h"
#include "nel/georges/mold_elt_type.h"

namespace NLGEORGES
{

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLoader::CLoader()
{
	ml.SetLoader( this );
}

CLoader::~CLoader()
{
}

void CLoader::LoadForm( CForm& _f, const CStringEx& _sxfullname )
{
	fl.LoadForm( _f, _sxfullname );
}

void CLoader::LoadForm( CForm& _f, const CStringEx& _sxfullname, const CStringEx& _sxdate ) 
{
	fl.LoadForm( _f, _sxfullname, _sxdate );
}

void CLoader::LoadSearchForm( CForm& _f, const CStringEx& _sxfilename )
{
	fl.LoadForm( _f, WhereIsForm( _sxfilename ) );
}

void CLoader::LoadSearchForm( CForm& _f, const CStringEx& _sxfilename, const CStringEx& _sxdate ) 
{
	fl.LoadForm( _f, WhereIsForm( _sxfilename ), _sxdate );
}

void CLoader::SaveForm( CForm& _f, const CStringEx& _sxfullname )
{
	fl.SaveForm( _f, _sxfullname );
}

CMoldElt* CLoader::LoadMold( const CStringEx _sxfilename )
{
	return( ml.LoadMold( _sxfilename ) );
}

CMoldElt* CLoader::LoadMold( const CStringEx _sxfilename, const CStringEx _sxdate ) 
{
	return( ml.LoadMold( _sxfilename, _sxdate ) );
}

void CLoader::SetWorkDirectory( const CStringEx _sxworkdirectory )
{
	if( sxworkdirectory != _sxworkdirectory )
	{
		sxworkdirectory = _sxworkdirectory;
		NLMISC::CPath::removeAllAlternativeSearchPath();
		NLMISC::CPath::addSearchPath( sxworkdirectory, true, true );
		NLMISC::CPath::addSearchPath( sxrootdirectory, true, true );
	}
}

void CLoader::SetRootDirectory( const CStringEx _sxrootdirectory )
{
	if( sxrootdirectory != _sxrootdirectory )
	{
		sxrootdirectory = _sxrootdirectory;
		NLMISC::CPath::removeAllAlternativeSearchPath();
		NLMISC::CPath::addSearchPath( sxworkdirectory, true, true );
		NLMISC::CPath::addSearchPath( sxrootdirectory, true, true );
	}
}

CStringEx CLoader::GetWorkDirectory() const
{
	return( sxworkdirectory );
}

CStringEx CLoader::GetRootDirectory() const
{
	return( sxrootdirectory );
}

CStringEx CLoader::WhereIsDfnTyp( const CStringEx _sxfilename )
{
	return( NLMISC::CPath::lookup( _sxfilename, false ) );
}

CStringEx CLoader::WhereIsForm( const CStringEx _sxfilename )
{
	return( NLMISC::CPath::lookup( _sxfilename, false ) );
}

void CLoader::MakeDfn( const CStringEx _sxfullname, const std::vector< std::pair< CStringEx, CStringEx > >* const _pvdefine )
{
	CFormFile pff;
	CForm f;

	CFormBodyEltStruct* pbody = f.GetBody();
	CFormBodyEltAtom* pfbea;

	if( ( _pvdefine )&&( !_pvdefine->empty() ) )
		for( std::vector< std::pair< CStringEx, CStringEx > >::const_iterator it = _pvdefine->begin(); it != _pvdefine->end(); ++it )
		{
			pfbea = new CFormBodyEltAtom;
			pfbea->SetName( it->first );
			pfbea->SetValue( it->second );
			pbody->AddElt( pfbea );
		}

	pff.SetForm( f );
	pff.Save( _sxfullname );
}

void CLoader::MakeTyp( const CStringEx _sxfullname, const CStringEx _sxtype, const CStringEx _sxformula, const CStringEx _sxenum, const CStringEx _sxlow, const CStringEx _sxhigh, const CStringEx _sxdefault, const std::vector< std::pair< CStringEx, CStringEx > >* const _pvpredef , const std::vector< std::pair< CStringEx, CStringEx > >* const _pvparent )
{
	CFormFile pff;
	CForm f;

	CFormBodyEltStruct* pbody = f.GetBody();
	CFormBodyEltAtom* pfbea;
	CFormBodyEltList* pfbel;
	CFormBodyEltStruct* pfbes;

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Type" );
	pfbea->SetValue( _sxtype );
	pbody->AddElt( pfbea );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Enum" );
	pfbea->SetValue( _sxenum );
	pbody->AddElt( pfbea );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Formula" );
	pfbea->SetValue( _sxformula );
	pbody->AddElt( pfbea );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Lowlimit" );
	pfbea->SetValue( _sxlow );
	pbody->AddElt( pfbea );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "Highlimit" );
	pfbea->SetValue( _sxhigh );
	pbody->AddElt( pfbea );

	pfbea = new CFormBodyEltAtom;
	pfbea->SetName( "DefaultValue" );
	pfbea->SetValue( _sxdefault );
	pbody->AddElt( pfbea );

	CStringEx sx;

	if( ( _pvpredef )&&( !_pvpredef->empty() ) )
	{
		pfbel = new CFormBodyEltList;
		pfbel->SetName( "Predef" );
		int i = 0;
		for( std::vector< std::pair< CStringEx, CStringEx > >::const_iterator it = _pvpredef->begin(); it != _pvpredef->end(); ++it )
		{
			pfbes = new CFormBodyEltStruct;
			sx.format( "#%d", i++ );
			pfbes->SetName( sx );
			pfbel->AddElt( pfbes );
			
			pfbea = new CFormBodyEltAtom;
			pfbea->SetName( "Designation" );
			pfbea->SetValue( it->first );
			pfbes->AddElt( pfbea );

			pfbea = new CFormBodyEltAtom;
			pfbea->SetName( "Substitute" );
			pfbea->SetValue( it->second );
			pfbes->AddElt( pfbea );
		}
		pbody->AddElt( pfbel );
	}

	if( ( _pvparent )&&( !_pvparent->empty() ) )
	{
		pfbel = new CFormBodyEltList;
		pfbel->SetName( "Parents" );
		int i = 0;
		for( std::vector< std::pair< CStringEx, CStringEx > >::const_iterator it = _pvparent->begin(); it != _pvparent->end(); ++it )
		{
			pfbes = new CFormBodyEltStruct;
			sx.format( "#%d", i++ );
			pfbes->SetName( sx );
			pfbel->AddElt( pfbes );

			pfbea = new CFormBodyEltAtom;
			pfbea->SetName( "Activity" );
			pfbea->SetValue( it->first );
			pfbes->AddElt( pfbea );

			pfbea = new CFormBodyEltAtom;
			pfbea->SetName( "Filename" );
			pfbea->SetValue( it->second );
			pfbes->AddElt( pfbea );
		}
		pbody->AddElt( pfbel );
	}

	pff.SetForm( f );
	pff.Save( _sxfullname );
}

void CLoader::SetTypPredef( const CStringEx _sxfilename, const std::vector< CStringEx >& _pvsx )
{
	CMoldElt*pme = LoadMold( _sxfilename );
	CMoldEltType* pmet = dynamic_cast< CMoldEltType* >( pme );
	pmet->SetTypPredef( _pvsx );
	pmet->Save();
}


/*
CStringEx CLoader::WhereIs( const CStringEx _sxdirectory, const CStringEx _sxfilename )
{
	if( _sxfilename.empty() )
		return( CStringEx() );

	_finddata_t info;
	CStringEx searchname = _sxdirectory +_sxfilename;
	long lhandle = _findfirst( searchname.c_str(), &info );
	if( lhandle != -1 )
	{
		_findclose( lhandle );
		return( searchname );
	}

	searchname = CStringEx( _sxdirectory + "*.*" );
	lhandle = _findfirst( searchname.c_str(), &info ); 
	do
	{
		if( !(info.attrib & _A_SUBDIR ) )
			continue;
		if( ( info.name == "." )||( info.name == ".." ) )
			continue;
		CStringEx sxresult = WhereIs( CStringEx( _sxdirectory + info.name ), _sxfilename );
		if( !sxresult.empty() )
			return( sxresult );
	}
	while( _findnext( lhandle, &info ) != -1 );
	
	_findclose( lhandle );
	return( CStringEx() );
}

CStringEx CLoader::WhereIsDfnTyp( const CStringEx _sxfilename )
{
	CStringEx sxfullname = WhereIs( CStringEx( sxworkdirectory +"dfn/" ), _sxfilename );
	if( sxfullname.empty() && ( sxrootdirectory != sxworkdirectory ) )
		sxfullname = WhereIs( CStringEx( sxrootdirectory +"dfn/" ), _sxfilename );
	return( sxfullname );
}

CStringEx CLoader::WhereIsForm( const CStringEx _sxfilename )
{
	CStringEx sxfullname = WhereIs( sxworkdirectory, _sxfilename );
	if( sxfullname.empty() && ( sxrootdirectory != sxworkdirectory ) )
		sxfullname = WhereIs( sxrootdirectory, _sxfilename );
	return( sxfullname );
}
*/

}