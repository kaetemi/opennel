/** \file _form_elt.h
 * Georges form element class
 *
 * $Id: form_elm.h,v 1.11 2002/08/21 11:39:06 coutelas Exp $
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

#ifndef _NLGEORGES__FORM_ELT_H
#define _NLGEORGES__FORM_ELT_H

#include "nel/georges/u_form_elm.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"

#include "georges/form_dfn.h"

namespace NLGEORGES
{

class CType;
class CFormDfn;
class CForm;

/**
  * Base class of form elments
  */
class CFormElm : public UFormElm
{
	friend class CForm;
	friend class CType;
	friend class CFormDfn;
public:

	// Contructor
	CFormElm (CForm *form, CFormElm *parentNode, const CFormDfn *parentDfn, uint parentIndex);

	// Destructor
	virtual ~CFormElm ();

	// Get the form pointer
	CForm			*getForm () const;

	// Is the element are used by this form ?
	virtual bool	isUsed (const CForm *form) const;

	// Get the form name of the element
	virtual void	getFormName (std::string &result, const CFormElm *child=NULL) const = 0;

	// From UFormElm
	virtual bool	getNodeByName (const UFormElm **result, const char *name, TWhereIsNode *where, bool verbose=true) const;
	virtual bool	getNodeByName (UFormElm **result, const char *name, TWhereIsNode *where, bool verbose=true);
	virtual bool	getValueByName (std::string &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getValueByName (sint8 &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getValueByName (uint8 &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getValueByName (sint16 &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getValueByName (uint16 &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getValueByName (sint32 &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getValueByName (uint32 &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getValueByName (float &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getValueByName (double &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getValueByName (bool &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getValueByName (NLMISC::CRGBA &result, const char *name, bool evaluate, TWhereIsValue *where) const;
	virtual bool	setValueByName (const char *value, const char *name, bool *created);
	virtual bool	setValueByName (sint8 value, const char *name, bool *created);
	virtual bool	setValueByName (uint8 value, const char *name, bool *created);
	virtual bool	setValueByName (sint16 value, const char *name, bool *created);
	virtual bool	setValueByName (uint16 value, const char *name, bool *created);
	virtual bool	setValueByName (sint32 value, const char *name, bool *created);
	virtual bool	setValueByName (uint32 value, const char *name, bool *created);
	virtual bool	setValueByName (float value, const char *name, bool *created);
	virtual bool	setValueByName (double value, const char *name, bool *created);
	virtual bool	setValueByName (bool value, const char *name, bool *created);
	virtual bool	setValueByName (NLMISC::CRGBA value, const char *name, bool *created);
	virtual UFormElm	*getParent () const;
	virtual bool	isArray () const;
	virtual bool	getArraySize (uint &size) const;
	virtual bool	getArrayNode (const UFormElm **result, uint arrayIndex) const;
	virtual bool	getArrayNode (UFormElm **result, uint arrayIndex);
	virtual bool	getArrayNodeName (std::string &result, uint arrayIndex) const;
	virtual bool	getArrayValue (std::string &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getArrayValue (sint8 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getArrayValue (uint8 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getArrayValue (sint16 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getArrayValue (uint16 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getArrayValue (sint32 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getArrayValue (uint32 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getArrayValue (float &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getArrayValue (double &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getArrayValue (bool &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	virtual bool	getArrayValue (NLMISC::CRGBA &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	virtual bool	isStruct () const;
	virtual bool	isVirtualStruct () const;
	virtual bool	getDfnName (std::string &dfnName ) const;
	virtual bool	getStructSize (uint &size) const;
	virtual bool	getStructNodeName (uint element, std::string &result) const;
	virtual bool	getStructNode (uint element, const UFormElm **result) const;
	virtual bool	getStructNode (uint element, UFormElm **result);
	virtual bool	isAtom () const;
	virtual bool	getValue (std::string &result, bool evaluate) const;
	virtual bool	getValue (sint8 &result, bool evaluate) const;
	virtual bool	getValue (uint8 &result, bool evaluate) const;
	virtual bool	getValue (sint16 &result, bool evaluate) const;
	virtual bool	getValue (uint16 &result, bool evaluate) const;
	virtual bool	getValue (sint32 &result, bool evaluate) const;
	virtual bool	getValue (uint32 &result, bool evaluate) const;
	virtual bool	getValue (float &result, bool evaluate) const;
	virtual bool	getValue (double &result, bool evaluate) const;
	virtual bool	getValue (bool &result, bool evaluate) const;
	virtual bool	getValue (NLMISC::CRGBA &result, bool evaluate) const;

	// ** Convert functions

	static inline bool convertValue (sint8 &result, const char *value);
	static inline bool convertValue (uint8 &result, const char *value);
	static inline bool convertValue (sint16 &result,	const char *value);
	static inline bool convertValue (uint16 &result,	const char *value);
	static inline bool convertValue (sint32 &result,	const char *value);
	static inline bool convertValue (uint32 &result,	const char *value);
	static inline bool convertValue (float &result, const char *value);
	static inline bool convertValue (double &result, const char *value);
	static inline bool convertValue (bool &result, const char *value);
	static inline bool convertValue (NLMISC::CRGBA &result, const char *value);

	// ** Internal node access

	// Create for a node by name. If the node already exist, return it
	bool	createNodeByName (const char *name, const CFormDfn **parentDfn, uint &indexDfn, 
										const CFormDfn **nodeDfn, const CType **nodeType, 
										CFormElm **node, UFormDfn::TEntryType &type, 
										bool &array, bool &created);

	/**
	  * Delete a node by name. If the node already exist, return it
	  *Delete its parent if not used 
	  */
	bool	deleteNodeByName (const char *name, const CFormDfn **parentDfn, uint &indexDfn, 
										const CFormDfn **nodeDfn, const CType **nodeType, 
										CFormElm **node, UFormDfn::TEntryType &type, 
										bool &array);

	// Search for a node by name
	bool	getNodeByName (const char *name, const CFormDfn **parentDfn, uint &indexDfn, 
										const CFormDfn **nodeDfn, const CType **nodeType, 
										CFormElm **node, UFormDfn::TEntryType &type, 
										bool &array, bool &parentVDfnArray, bool verbose) const;

	/** 
	  * Insert an array node by name
	  * The index asked must be < the size of the array.
	  */
	bool	arrayInsertNodeByName (const char *name, const CFormDfn **parentDfn, uint &indexDfn, 
										const CFormDfn **nodeDfn, const CType **nodeType, 
										CFormElm **node, UFormDfn::TEntryType &type, 
										bool &array, bool verbose, uint arrayIndex) const;

	/** 
	  * Delete an array node by name
	  * The index asked must be < the size of the array.
	  */
	bool	arrayDeleteNodeByName (const char *name, const CFormDfn **parentDfn, uint &indexDfn, 
										const CFormDfn **nodeDfn, const CType **nodeType, 
										CFormElm **node, UFormDfn::TEntryType &type, 
										bool &array, bool verbose, uint arrayIndex) const;
protected:
	
	// Action to perform
	enum TNodeAction
	{
		Return,
		Create,
		Delete,
	};

	/**
	  * Is createNode == Create, (*node)->Form must be == to the form argument.
	  * Is createNode == Return, form argument is not used, can be undefined.
	  *
	  * Only form, name, and action, must be defined. 
	  * Then, else (*parentDfn / indexDfn ) or *node must be defined.
	  * Others value are for result only.
	  */
	static bool	getIternalNodeByName (CForm *form, const char *name, const CFormDfn **parentDfn, uint &indexDfn, 
										const CFormDfn **nodeDfn, const CType **nodeType, 
										CFormElm **node, UFormDfn::TEntryType &type, 
										bool &array, TNodeAction action, bool &created, bool &parentVDfnArray, bool verbose);

	/**
	  * Unlink a child
	  */
	virtual void unlink (CFormElm *child);

public:

	// Get next token, return NULL if last token
	static const char* tokenize (const char *name, std::string &str, uint &errorIndex, uint &code);

	// ** IO functions
	virtual xmlNodePtr	write (xmlNodePtr node, const CForm *form, const char *structName = NULL, bool forceWrite = false) const = 0;

protected:

	// The form of this node
	CForm				*Form;

	// The parent node of this node
	CFormElm			*ParentNode;

	// The parent DFN of this node
	const CFormDfn		*ParentDfn;

	// The index in the parent DFN for this node
	uint				ParentIndex;

private:
	// Tokens
	enum TToken
	{
		TokenString = 0,
		TokenPoint,
		TokenArrayBegin,
		TokenArrayEnd,
	};
};

/**
  * Define a structure of elements
  *
  * This structure has pointers on named sub structures in Elements.
  * If a sub structure is empty, its pointer is NULL.
  */
class CFormElmStruct : public CFormElm
{
public:
	// Default constructor
	CFormElmStruct (CForm *form, CFormElm *parentNode, const CFormDfn *parentDfn, uint parentIndex);
	~CFormElmStruct ();

	// Clear sub elements
	void clean ();

	// Smart pointer on the form definition for this structure
	NLMISC::CSmartPtr<CFormDfn>	FormDfn;

	// Pointer on the parent element
	//CFormElmStruct					*Parent;

	// A struct element
	class CFormElmStructElm
	{
	public:
		CFormElmStructElm ()
		{
			Element = NULL;
		}

		std::string		Name;
		CFormElm*		Element;
	};


	// Build form a DFN
	void				build (const CFormDfn *dfn);

	// From UFormElm
	bool				isStruct () const;
	bool				getStructSize (uint &size) const;
	bool				getStructNodeName (uint element, std::string &result) const;
	bool				getStructNode (uint element, const UFormElm **result) const;
	bool				getStructNode (uint element, UFormElm **result);

	// From CFormElm
	bool				isUsed (const CForm *form) const;
	xmlNodePtr			write (xmlNodePtr node, const CForm *form, const char *structName, bool forceWrite = false) const;
	void				unlink (CFormElm *child);
	void				getFormName (std::string &result, const CFormElm *child) const;


	// Call by CFormLoader
	void				read (xmlNodePtr node, CFormLoader &loader, const CFormDfn *dfn, CForm *form);

	// Sub Elements
	std::vector<CFormElmStructElm>		Elements;
};

/**
  * Define an array of elements
  */
class CFormElmVirtualStruct : public CFormElmStruct
{
public:

	CFormElmVirtualStruct (CForm *form, CFormElm *parentNode, const CFormDfn *parentDfn, uint parentIndex);

	// The Dfn filename used by this struct
	std::string			DfnFilename;

	// From UFormElm
	bool				isVirtualStruct () const;
	bool				getDfnName (std::string &dfnName ) const;

	// From CFormElm
	bool				isUsed (const CForm *form) const;
	xmlNodePtr			write (xmlNodePtr node, const CForm *form, const char *structName, bool forceWrite = false) const;

	// Call by CFormLoader
	void				read (xmlNodePtr node, CFormLoader &loader, CForm *form);
};

/**
  * Define an array of elements
  */
class CFormElmArray : public CFormElm
{
public:
	// Default constructor
	CFormElmArray (CForm *form, const CFormDfn *formDfn, const CType *type, CFormElm *parentNode, const CFormDfn *parentDfn, uint parentIndex);
	~CFormElmArray ();
	void clean ();

	// Smart pointer on the form definition for this structure
	NLMISC::CSmartPtr<CFormDfn>	FormDfn;

	// Pointer on the type (the smart pointer in hold by CFormDfn)
	const CType			*Type;

	// From UFormElm
	bool				isArray () const;
	bool				getArraySize (uint &size) const;
	bool				getArrayNode (const UFormElm **result, uint arrayIndex) const;
	bool				getArrayNode (UFormElm **result, uint arrayIndex);
	bool				getArrayNodeName (std::string &result, uint arrayIndex) const;
	bool				getArrayValue (std::string &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool				getArrayValue (sint8 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool				getArrayValue (uint8 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool				getArrayValue (sint16 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool				getArrayValue (uint16 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool				getArrayValue (sint32 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool				getArrayValue (uint32 &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool				getArrayValue (float &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool				getArrayValue (double &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool				getArrayValue (bool &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;
	bool				getArrayValue (NLMISC::CRGBA &result, uint arrayIndex, bool evaluate, TWhereIsValue *where) const;

	// From CFormElm
	xmlNodePtr			write (xmlNodePtr node, const CForm *form, const char *structName, bool forceWrite = false) const;
	bool				setParent (CFormElm *parent);
	void				unlink (CFormElm *child);
	bool				isUsed (const CForm *form) const;
	void				getFormName (std::string &result, const CFormElm *child) const;

	// Call by CFormLoader

	// Read an array
	void				read (xmlNodePtr node, CFormLoader &loader, CForm *form);

	// A struct element
	class CElement
	{
	public:
		CElement ()
		{
			Element = NULL;
		}

		std::string		Name;
		CFormElm*		Element;
	};

	// Array of elements
	std::vector<CElement>		Elements;
};

/**
  * Signed integer element
  */
class CFormElmAtom : public CFormElm
{
	friend class CForm;
	friend class CFormElm;
	friend class CType;
public:
	// Default constructor
	CFormElmAtom (CForm *form, CFormElm *parentNode, const CFormDfn *parentDfn, uint parentIndex);

	// Pointer on the parent element
	//CFormElmAtom				*Parent;

	// Pointer on the type (the smart pointer in hold by CFormDfn)
	const CType					*Type;

	// From CFormElm
	xmlNodePtr					write (xmlNodePtr node, const CForm *form, const char *structName, bool forceWrite = false) const;
	bool						setParent (CFormElm *parent);
	void						getFormName (std::string &result, const CFormElm *child) const;

	// Call by CFormLoader
	void						read (xmlNodePtr node, CFormLoader &loader, const CType *type, CForm *form);

	// From UFormElm
	bool						isAtom () const;
	bool						getValue (std::string &result, bool evaluate) const;
	bool						getValue (sint8 &result, bool evaluate) const;
	bool						getValue (uint8 &result, bool evaluate) const;
	bool						getValue (sint16 &result, bool evaluate) const;
	bool						getValue (uint16 &result, bool evaluate) const;
	bool						getValue (sint32 &result, bool evaluate) const;
	bool						getValue (uint32 &result, bool evaluate) const;
	bool						getValue (float &result, bool evaluate) const;
	bool						getValue (double &result, bool evaluate) const;
	bool						getValue (bool &result, bool evaluate) const;
	bool						getValue (NLMISC::CRGBA &result, bool evaluate) const;

	// Set the value, the elt been used
	void						setValue (const char *value);

private:
	// The value
	std::string					Value;
};

// ***************************************************************************
// CFormElm inlines
// ***************************************************************************

inline bool CFormElm::convertValue (sint8 &result, const char *value)
{
	float tmp;
	if (sscanf (value, "%f", &tmp) == 1)
	{
		NLMISC::clamp (tmp, -128.f, 127.f);
		result = (sint8)tmp;
		return true;
	}
	else
	{
		// Error message
		nlwarning ("Georges (CFormElm::convertValue) : Can't convert the string %s in sint8.", value);
	}
	return false;
}

// ***************************************************************************

inline bool CFormElm::convertValue (uint8 &result, const char *value)
{
	float tmp;
	if (sscanf (value, "%f", &tmp) == 1)
	{
		NLMISC::clamp (tmp, 0.f, 255.f);
		result = (uint8)tmp;
		return true;
	}
	else
	{
		// Error message
		nlwarning ("Georges (CFormElm::convertValue) : Can't convert the string %s in uint8.", value);
	}
	return false;
}

// ***************************************************************************

inline bool CFormElm::convertValue (sint16 &result,	const char *value)
{
	float tmp;
	if (sscanf (value, "%f", &tmp) == 1)
	{
		NLMISC::clamp (tmp, -32768.f, 32767.f);
		result = (sint16)tmp;
		return true;
	}
	else
	{
		// Error message
		nlwarning ("Georges (CFormElm::convertValue) : Can't convert the string %s in sint16.", value);
	}
	return false;
}

// ***************************************************************************

inline bool CFormElm::convertValue (uint16 &result,	const char *value)
{
	float tmp;
	if (sscanf (value, "%f", &tmp) == 1)
	{
		NLMISC::clamp (tmp, 0.f, 65535.f);
		result = (uint16)tmp;
		return true;
	}
	else
	{
		// Error message
		nlwarning ("Georges (CFormElm::convertValue) : Can't convert the string %s in uint16.", value);
	}
	return false;
}

// ***************************************************************************

inline bool CFormElm::convertValue (sint32 &result,	const char *value)
{
	float tmp;
	if (sscanf (value, "%f", &tmp) == 1)
	{
		NLMISC::clamp (tmp, -2147483648.f, 2147483647.f);
		result = (sint32)tmp;
		return true;
	}
	else
	{
		// Error message
		nlwarning ("Georges (CFormElm::convertValue) : Can't convert the string %s in sint32.", value);
	}
	return false;
}

// ***************************************************************************

inline bool CFormElm::convertValue (uint32 &result,	const char *value)
{
	float tmp;
	if (sscanf (value, "%f", &tmp) == 1)
	{
		NLMISC::clamp (tmp, 0.f, 4294967295.f);
		result = (sint32)tmp;
		return true;
	}
	else
	{
		// Error message
		nlwarning ("Georges (CFormElm::convertValue) : Can't convert the string %s in uint32.", value);
	}
	return false;
}

// ***************************************************************************

inline bool CFormElm::convertValue (float &result, const char *value)
{
	if (sscanf (value, "%f", &result) == 1)
	{
		return true;
	}
	else
	{
		// Error message
		nlwarning ("Georges (CFormElm::convertValue) : Can't convert the string %s in float.", value);
	}
	return false;
}

// ***************************************************************************

inline bool CFormElm::convertValue (double &result, const char *value)
{
	float tmp;
	if (sscanf (value, "%f", &tmp) == 1)
	{
		result = tmp;
		return true;
	}
	else
	{
		// Error message
		nlwarning ("Georges (CFormElm::convertValue) : Can't convert the string %s in double.", value);
	}
	return false;
}

// ***************************************************************************

inline bool CFormElm::convertValue (bool &result, const char *value)
{
	int tmp;
	if (sscanf (value, "%d", &tmp) == 1)
	{
		result = tmp != 0;
		return true;
	}
	else
	{
		std::string temp = value;
		temp = NLMISC::strlwr (temp);
		if (strcmp (temp.c_str (), "true") == 0)
		{
			result  = true;
			return true;
		}
		if (strcmp (temp.c_str (), "false") == 0)
		{
			result  = false;
			return true;
		}
	}

	// Error message
	nlwarning ("Georges (CFormElm::convertValue) : Can't convert the string %s in boolean.", value);
	
	return false;
}

// ***************************************************************************

inline bool CFormElm::convertValue (NLMISC::CRGBA &result, const char *value)
{
	float r, g, b;
	if (sscanf (value, "%f,%f,%f", &r, &g, &b) == 1)
	{
		NLMISC::clamp (r, 0.f, 255.f);
		NLMISC::clamp (g, 0.f, 255.f);
		NLMISC::clamp (b, 0.f, 255.f);
		result.R = (uint8)r;
		result.G = (uint8)g;
		result.B = (uint8)b;
		return true;
	}

	// Error message
	nlwarning ("Georges (CFormElm::convertValue) : Can't convert the string %s in RGB color.", value);
	
	return false;
}

// ***************************************************************************

} // NLGEORGES

#endif // _NLGEORGES_FORM_ELM_H

