/** \file reader_writer.cpp
 *
 * $Id: reader_writer.cpp,v 1.1 2001/10/16 09:28:36 legros Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#include "nel/misc/reader_writer.h"


namespace NLMISC {

CReaderWriter::CReaderWriter()
{
	_ReadersLevel = 0;
}

CReaderWriter::~CReaderWriter()
{
	// here some checks to avoid a reader/writer still working while we flush the mutexes...
}



} // NLMISC
