/** \file logic_event.h
 * 
 *
 * $Id: logic_event.h,v 1.1 2002/02/14 12:58:09 corvazier Exp $
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

#ifndef LOGIC_EVENT_H
#define LOGIC_EVENT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/entity_id.h"

//#include "game_share/sid.h"


namespace NLLOGIC
{

class CLogicStateMachine;


/**
 * CLogicEventMessage
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2001
 */
class CLogicEventMessage
{
public:
	
	/// true if the message has to be sent
	bool ToSend;

	/// true if the message has been sent
	bool Sent;

	/// message destination
	std::string Destination;

	/// message destination id
	NLMISC::CEntityId DestinationId;

	/// message id
	std::string MessageId;

	/// message arguments
	std::string Arguments;

	/**
	 *	Default constructor
	 */
	CLogicEventMessage()
	{
		ToSend = false;
		Sent = false;
		Destination = "no_destination";
		MessageId = "no_id";
		DestinationId.Type = 0xfe;
		DestinationId.CreatorId = 0;
		DestinationId.DynamicId = 0;
		Arguments = "no_arguments";
	}

	/**
	 * serial
	 */
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
};


/**
 * CLogicEventAction
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2001
 */
class CLogicEventAction
{
public:
	
	/// true if this action consist in a state change, false if it's a message
	bool IsStateChange;

	/// state name for state change
	std::string StateChange;

	/// event message
	CLogicEventMessage EventMessage;

	/**
	 * Default constructor
	 */
	CLogicEventAction()
	{
		IsStateChange = false;
	}

	/**
	 * This message will be sent as soon as the dest id will be given
	 */
	void enableSendMessage();

	/**
	 * serial
	 */
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

};



/**
 * CLogicEvent
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2001
 */
class CLogicEvent
{
	/// state machine managing this event
	CLogicStateMachine * _LogicStateMachine;

public:
	
	/// condition name
	std::string ConditionName;
	
	/// event action
	CLogicEventAction EventAction;

	/**
	 * Default constructor
	 */
	CLogicEvent()
	{
		_LogicStateMachine = 0;
		ConditionName = "no_condition";
	}

	/**
	 *	Reset this event
	 */
	void reset();

	/**
	 *	Set the logic state machine
	 *
	 * \param logicStateMachine is the state machine containing this block
	 */
	void setLogicStateMachine( CLogicStateMachine * logicStateMachine );

	/**
	 * Test the condition 
	 *
	 * \return true if condition is fulfiled
	 */
	bool testCondition();

	/**
	 * serial
	 */
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

};

} // NLLOGIC

#endif //LOGIC_EVENT



