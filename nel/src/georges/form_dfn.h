/** \file _form_dfn.h
 * Georges form definition class
 *
 * $Id: form_dfn.h,v 1.14 2002/10/08 09:13:14 corvazier Exp $
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

#ifndef _NLGEORGES__FORM_DFN_H
#define _NLGEORGES__FORM_DFN_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/georges/u_form_dfn.h"
#include "nel/georges/u_form_elm.h"
#include "header.h"
#include "georges/type.h"

bool convertDfnFile (const char *oldFileName, const char *newFileName);

namespace NLGEORGES
{

class CFormLoader;

/**
  * This class is the defnition for a familly of from.
  */
class CFormDfn : public UFormDfn
{
	friend class CForm;
	friend class CType;
	friend class CFormElm;
	friend class CFormLoader;
	friend class CFormElmStruct;
	friend bool convertDfnFile (const char *oldFileName, const char *newFileName);
public:

	// Default cstr
	CFormDfn ()
	{
		Round = 0xffffffff;
	}

	// A form defnition entry
	class CEntry
	{
		friend class CType;
		friend class CForm;
		friend class CFormElm;
		friend class CFormDfn;
		friend class CFormElmStruct;
		friend bool convertDfnFile (const char *oldFileName, const char *newFileName);
	public:

		CEntry ()
		{
			TypeElement = EntryType;
			FilenameExt = "*.*";
		}

		// Get the type
		class CType					*getTypePtr ();

		// Get the type
		const CType					*getTypePtr () const;

		// Get the dfn
		CFormDfn					*getDfnPtr ();

		// Get the dfn
		const CFormDfn				*getDfnPtr () const;

		// Get type flag
		TEntryType					getType () const;

		// Set as a type
		void						setType (CFormLoader &loader, const char *filename);

		// Set as a dfn
		void						setDfn (CFormLoader &loader, const char *filename);

		// Set as a dfn pointer
		void						setDfnPointer ();

		// Get element Name
		const std::string			&getName () const;

		// Set element Name
		void						setName (const char *name);

		// Get the filename
		const std::string			&getFilename() const;

		// Get the filename extension
		const std::string			&getFilenameExt() const;

		// Set the filename
		void						setFilename (const char *def);

		// Set the filename extension
		void						setFilenameExt (const char *ext);

		// Get default value
		const std::string			&getDefault () const;

		// Set default value
		void						setDefault (const char *def);

		// Set array flag
		void						setArrayFlag (bool flag);

		// Get array flag
		bool						getArrayFlag () const;

	private:
		// Entry name
		std::string					Name;

		// What is the type of the element ?
		TEntryType					TypeElement;

		// Is an array of this type ?
		bool						Array;

		// The filename
		std::string					Filename;

		// The default value for atom
		std::string					Default;

		// The filename
		std::string					FilenameExt;

		// Smart ptr on the type or the dfn
		NLMISC::CSmartPtr<CType>	Type;

		// Smart ptr on the type or the dfn
		NLMISC::CSmartPtr<CFormDfn>	Dfn;
	};

	// Parent DFN
	class CParent
	{
	public:
		// The parent filename
		std::string						ParentFilename;

		// The parent smart
		NLMISC::CSmartPtr<CFormDfn>	Parent;
	};

	// ** IO functions
	void							write (xmlDocPtr root, const char *filename, bool georges4CVS);

	// Count parent DFN
	uint							countParentDfn (uint32 round=0xffffffff) const;

	// Get parent DFN
	void							getParentDfn (std::vector<CFormDfn*> &array, uint32 round=0xffffffff);

	// Get parent DFN
	void							getParentDfn (std::vector<const CFormDfn*> &array, uint32 round=0xffffffff) const;

	// Get num parent
	uint							getNumParent () const;

	// Get parent count
	void							setNumParent (uint size);

	// Set a parent
	void							setParent (uint parent, CFormLoader &loader, const char *filename);

	// Get a parent
	CFormDfn						*getParent (uint parent) const;

	// Get a parent string
	const std::string				&getParentFilename (uint parent) const;

	// Get num entry
	uint							getNumEntry () const;

	// Set num entry
	void							setNumEntry (uint size);

	// Get an entry
	const CEntry					&getEntry (uint entry) const;

	// Get an entry
	CEntry							&getEntry (uint entry);

	// Form UFormDfn
	bool							getEntryType (uint entry, TEntryType &type, bool &array) const;
	bool							getEntryName (uint entry, std::string &name) const;
	bool							getEntryDfn (uint entry, UFormDfn **dfn);
	bool							getEntryType (uint entry, UType **type);
	uint							getNumParents () const;
	bool							getParent (uint parent, UFormDfn **parentRet);
	const std::string				&getComment () const;
	bool							getEntryFilename (uint entry, std::string& filename) const;
	bool							getParentFilename (uint parent, std::string &filename) const;

	// Get the sub dfn of a dfn
	CFormDfn						*getSubDfn (uint index, uint &dfnIndex);
	const CFormDfn					*getSubDfn (uint index, uint &dfnIndex) const;

	// Header
	CFileHeader						Header;

	// Error handling
	void							warning (bool exception, const char *function, const char *format, ... ) const;

private:
	// The parents array
	std::vector<CParent>			Parents;

	// A vector of entries
	std::vector<CEntry>				Entries;

	// Recurcive call count
	mutable uint32					Round;

	// The form DFN filename
	std::string						_Filename;

private:
	// Read method called by the form loader
	void							read (xmlNodePtr doc, CFormLoader &loader, bool forceLoad, const char *filename);
};

} // NLGEORGES

#endif // _NLGEORGES__FORM_DFN_H

