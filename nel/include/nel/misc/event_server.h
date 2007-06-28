/** \file event_server.h
 * events server
 *
 * $Id$
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

#ifndef NL_EVENT_SERVER_H
#define NL_EVENT_SERVER_H

#include "types_nl.h"
#include "class_id.h"
#include <map>
#include <list>


namespace NLMISC {

class IEventEmitter;
class IEventListener;
class CEvent;

/*===================================================================*/

typedef std::multimap<CClassId, IEventListener*> mapListener;


/**
 * CEventServer
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CEventServer
{
	mapListener _Listeners;
	std::list<IEventEmitter*> _Emitters;
	std::list<CEvent*> _Events;
	bool		_Pumping;

public:
	CEventServer();
	virtual ~CEventServer();

	/** 
	 * add event to the list
	 * \param event
	 */	
	void postEvent(CEvent * event);

	/** 
	 * get call every callbacks associated with event id
	 * \param allWindows is true : pump the messages of all windows, not only the driver window.
	 */	
	void pump(bool allWindows = false);
		
	/** 
	 * Add a callback (associated with an id)
	 * \param id of the CEvent class to hook
	 * \param listener to use with this event
	 */	
	void addListener(CClassId id, IEventListener* listener );
	
	/** 
	 * Remove a callback
	 * \param id of event's callback
	 * \param listener to be removed
	 */	
	void removeListener(CClassId id, IEventListener* listener );

	/** 
	 * Add an Emitter to the server 
	 * \param emitter
	 */	
	void addEmitter(IEventEmitter * emitter);
	
	/** 
	 * Remove an Emitter from the server 
	 * \param emitter
	 */	
	void removeEmitter(IEventEmitter * emitter);

protected:
	/** 
	 * call every callbacks associated with event id
	 * \param event
	 * \return true if the pointer must be delete, false if it not. (post to another message queue...)
	 */	
	virtual bool pumpEvent(CEvent* event);
};


/*===================================================================*/

} // NLMISC


#endif // NL_EVENT_SERVER_H

/* End of event_server.h */
