/** \file gtk_displayer.h
 * Gtk Implementation of the CWindowDisplayer (look at window_displayer.h)
 *
 * $Id: gtk_displayer.h,v 1.1 2001/11/05 15:42:49 lecroart Exp $
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

#ifndef NL_GTK_DISPLAYER_H
#define NL_GTK_DISPLAYER_H

#ifdef NL_USE_GTK

#include "nel/misc/types_nl.h"

#include "nel/misc/displayer.h"
#include "nel/misc/reader_writer.h"

#include "window_displayer.h"

#include <gtk/gtk.h>

namespace NLMISC {


/**
 * this displayer displays on a gtk windows.
 * MT = Main Thread, DT = Display Thread
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CGtkDisplayer : public NLMISC::CWindowDisplayer
{
public:

	CGtkDisplayer (const char *displayerName = "") : CWindowDisplayer(displayerName)
	{
		needSlashR = false;
	}

	virtual ~CGtkDisplayer ();

private:

	// called by DT only
	void	updateLabels ();

	// called by DT only
	void	open (std::string WindowNameEx, sint x, sint y, sint w, sint h, sint hs);
	// called by DT only
	void	clear ();
	// called by DT only
	void	display_main ();

	// all these variables above is used only by the DT
	friend sint updateInterf (void *data);
	friend gint ButtonClear(GtkWidget *Widget, GdkEventKey *Event, gpointer *Data);
};

} // NLMISC

#endif // NL_USE_GTK

#endif // NL_GTK_DISPLAYER_H

/* End of gtk_displayer.h */
