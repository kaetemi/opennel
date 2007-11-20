/** \file i18n/main.cpp
 * example of the NeL internationalisation features
 *
 * $Id$
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

#include <string>
#include <iostream>

// contains all i18n features
#include "nel/misc/i18n.h"

using namespace NLMISC;

int main (int argc, char **argv)
{

	nlinfo ("Please, choose 'en', 'fr' or 'de' and press <return>");

	std::string langName;
	std::getline(std::cin, langName);

	// load the language
	CI18N::load (langName);

	nlinfo (CI18N::get("hi").toString().c_str ());

	nlinfo (CI18N::get("PresentI18N").toString().c_str (), "Ace");

	nlinfo (CI18N::get("ExitStr").toString().c_str ());
	getchar ();

	return EXIT_SUCCESS;
}

/* MERGE: this is the result of merging branch_mtr_nostlport with trunk (NEL-16)
 */
