/** \file o_xml.h
 * Output xml stream
 *
 * $Id: o_xml.h,v 1.4 2002/05/17 06:34:14 corvazier Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#ifndef NL_O_XML_H
#define NL_O_XML_H

//#define NL_DONT_USE_EXTERNAL_CODE
#undef NL_DONT_USE_EXTERNAL_CODE

#ifndef NL_DONT_USE_EXTERNAL_CODE

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"

// Include from libxml2
#include <libxml/parser.h>


namespace NLMISC {


/**
 * Output xml stream
 *
 * This class is an xml formated output stream.
 *
 * This stream use an internal stream to output final xml code.
 \code
	// Check exceptions
	try
	{
		// File stream
		COFile file;

		// Open the file
		file.open ("output.xml");

		// Create the XML stream
		COXml output;

		// Init
		if (output.init (&file, "1.0"))
		{
			// Serial the class
			myClass.serial (output);

			// Flush the stream, write all the output file
			output.flush ();
		}

		// Close the file
		file.close ();
	}
 	catch (Exception &e)
	{
	}
\endcode
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class COXml : public IStream
{
	friend int xmlOutputWriteCallbackForNeL ( void *context, const char *buffer, int len );
	friend int xmlOutputCloseCallbackForNeL ( void *context );
public:

	/** Stream ctor
	  *
	  */
	COXml ();

	/** Stream initialisation
	  *
	  * \param stream is the stream the class will use to output xml code.
	  * this pointer is held by the class but won't be deleted.
	  * \param version is the version to write in the XML header. Default is 1.0.
	  * \return true if initialisation is successful, false if the stream passed is not an output stream.
	  */
	bool init (IStream *stream, const char *version="1.0");

	/** Default dstor
	  *
	  * Flush the stream.
	  */
	virtual ~COXml ();

	/** Flush the stream.
	  *
	  * You can only flush the stream when all xmlPushBegin - xmlPop have been closed.
	  */
	void flush ();

	/** Get root XML document pointer
	  */
	xmlDocPtr getDocument ();

private:

	/// From IStream
	virtual void	serial(uint8 &b);
	virtual void	serial(sint8 &b);
	virtual void	serial(uint16 &b);
	virtual void	serial(sint16 &b);
	virtual void	serial(uint32 &b);
	virtual void	serial(sint32 &b);
	virtual void	serial(uint64 &b);
	virtual void	serial(sint64 &b);
	virtual void	serial(float &b);
	virtual void	serial(double &b);
	virtual void	serial(bool &b);
#ifndef NL_OS_CYGWIN
	virtual void	serial(char &b);
#endif
	virtual void	serial(std::string &b);
	virtual void	serial(ucstring &b);
	virtual void	serialBuffer(uint8 *buf, uint len);
	virtual void	serialBit(bool &bit);

	virtual bool	xmlPushBeginInternal (const char *nodeName);
	virtual bool	xmlPushEndInternal ();
	virtual bool	xmlPopInternal ();
	virtual bool	xmlSetAttribInternal (const char *attribName);
	virtual bool	xmlBreakLineInternal ();
	virtual bool	xmlCommentInternal (const char *comment);

	// Internal functions
	void			serialSeparatedBufferOut( const char *value );
	inline void		flushContentString ();

	// Push mode
	bool			_PushBegin;

	// Attribute defined
	bool			_AttribPresent;

	// Attribute name
	std::string		_AttribName;

	// The internal stream
	IStream			*_InternalStream;
	
	// Document pointer
	xmlDocPtr		_Document;

	// Document version
	std::string		_Version;

	// Current nodes
	xmlNodePtr		_CurrentNode;

	// Current content string
	std::string		_ContentString;
};


} // NLMISC

#endif // NL_DONT_USE_EXTERNAL_CODE

#endif // NL_O_XML_H

/* End of o_xml.h */
