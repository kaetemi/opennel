/** \file u_retriever_bank.h
 * <File description>
 *
 * $Id: u_retriever_bank.h,v 1.2 2001/06/22 15:03:05 corvazier Exp $
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

#ifndef NL_U_RETRIEVER_BANK_H
#define NL_U_RETRIEVER_BANK_H

#include "nel/misc/types_nl.h"


namespace NLPACS {


/**
 * <Class description>
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class URetrieverBank
{
public:

	/**
	  * Create a retriever bank.
	  *
	  * \param retrieverBank is the global retriver bank path file name. This method use CPath to find the retriever
	  * \return the retriver bank interface or NULL if the bank was not found.
	  */
	static URetrieverBank	*createRetrieverBank (const char* retrieverBank);

	/**
	  * Delete a retriever bank.
	  */
	static void				deleteRetrieverBank (URetrieverBank *retrieverBank);
};


} // NLPACS


#endif // NL_U_RETRIEVER_BANK_H

/* End of u_retriever_bank.h */
