// MoldLoader.cpp: implementation of the CMoldLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MoldLoader.h"
#include "MoldElt.h"
#include "MoldEltDefine.h"
#include "MoldEltType.h"
#include "MoldEltDefineList.h"
#include "MoldEltTypeList.h"
#include "Loader.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMoldLoader::CMoldLoader()
{
}

CMoldLoader::~CMoldLoader()
{
	Clear();
}

void CMoldLoader::Clear()
{
	for( std::vector< CMoldElt* >::iterator it = vpme.begin(); it != vpme.end(); ++it )
		if( *it )
			delete( *it );
	vpme.clear();
	mmold.clear();
}

void CMoldLoader::SetLoader( CLoader* const _pl )
{
	nlassert( _pl );
	pl = _pl;
}

CMoldElt* CMoldLoader::LoadMold( const CStringEx _sxfilename )
{
	CStringEx sxfn = _sxfilename;	
	sxfn.purge();
	if( sxfn.empty() )
		return( 0 );											

	// liste?
	bool blst = ( sxfn.find( "list<" ) != -1 );
	if( blst )
	{
		unsigned int ipos = sxfn.find( ">" );
		if( ipos < 0 )
			return( 0 );											
		sxfn.mid( 5, ipos-5 ); 
	}

	// find extension
	int ipos = sxfn.reverse_find('.');
	if( ipos < 0 )
		return( 0 );											
	CStringEx sxfileextension = sxfn.get_right( sxfn.length()-ipos-1 );

	// find if loaded
	CMoldElt* pme;
	CStringEx sxfullname = pl->WhereIsDfnTyp( sxfn );
	sxfullname.make_lower();
	std::map< CStringEx, CMoldElt* >::iterator it = mmold.find( sxfullname );
	if( it != mmold.end() )
		if( blst )
		{
			if( sxfileextension == "dfn" )
				pme = new CMoldEltDefineList( pl, dynamic_cast< CMoldEltDefine* >( it->second ) );
			else if( sxfileextension == "typ" )
					pme = new CMoldEltTypeList( pl, dynamic_cast< CMoldEltType* >( it->second ) );
				else
					return 0;
			pme->SetName( it->second->GetName() );
			vpme.push_back( pme );			
			return( pme );
		}
		else
			return( it->second );

	// load
	if( sxfileextension == "dfn" )
	{
		pme = new CMoldEltDefine( pl );
		vpme.push_back( pme );			
		if( blst )
		{
			pme = new CMoldEltDefineList( pl, dynamic_cast< CMoldEltDefine* >( pme ) );
			vpme.push_back( pme );			
		}
	}
	else if( sxfileextension == "typ" )
		{
			pme = new CMoldEltType( pl );
			vpme.push_back( pme );			
			if( blst )
			{
				pme = new CMoldEltTypeList( pl, dynamic_cast< CMoldEltType* >( pme ) );
				vpme.push_back( pme );			
			}
		}
	else 
		return( 0 );

	pme->SetName( sxfn );
	pme->Load( sxfullname );
	mmold.insert( std::make_pair( sxfullname, pme->GetMold() ) );
	return( pme );
}

CMoldElt* CMoldLoader::LoadMold( const CStringEx _sxfilename, const CStringEx _sxdate )
{
	return( 0 );
}
/*
CMoldElt* CMoldLoader::Find( const CStringEx _sxfullname )
{
	std::map< CStringEx, CMoldElt* >::iterator it = mmold.find( _sxfullname );
	if( it != mmold.end() )
		return( it->second );
	return( 0 );
}
*/