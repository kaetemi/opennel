/*
 * This file contain the Snowballs Position Service.
 *
 * $Id: main.cpp,v 1.8 2002/02/11 10:25:00 lecroart Exp $
 */

/*
 * Copyright, 2000 - 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */


// This include is mandatory to use NeL. It include NeL types.
#include <nel/misc/types_nl.h>

#include <nel/misc/vector.h>

#include <nel/misc/time_nl.h>

// We're using the NeL Service framework, and layer 4
#include <nel/net/service.h>
#include <nel/net/net_manager.h>

#include <map>
#include <list>

#include "physics.h"


using namespace NLMISC;
using namespace NLNET;
using namespace std;


#define PLAYER_RADIUS      1.0f
#define SNOWBALL_RADIUS    0.1f
#define START_SNOW_ID      2000000000
#define THROW_ANIM_OFFSET  1000


// Define information used for all connected players to the shard.
struct _player
{
	_player( uint32 Id, string Name, uint8 Race, CVector Position ) :
		id( Id ), name( Name ), race( Race ), position( Position ) { }
	uint32  id;
	string  name;
	uint8   race;
	CVector position;
};

// List of all the players connected to the shard.
typedef map<uint32, _player> _pmap;
_pmap playerList;

// Define informations used for the snowballs management
struct _snowball
{
	_snowball( uint32 Id, uint32 Owner, CTrajectory Traj, float ExplosionRadius ) :
		id( Id ), owner( Owner ), traj( Traj ), explosionRadius( ExplosionRadius ) { }
	uint32      id;
	uint32      owner;
	CTrajectory traj;
	float       explosionRadius;
};

// List of all the games snowballs
list<_snowball> snoList;


/****************************************************************************
 * Function:   cbAddEntity
 *             Callback function called when the Position Service receive a
 *             "ADD_ENTITY" message
 * Arguments:
 *             - msgin:  the incomming message
 *             - from:   the "sockid" of the sender (usually useless for a
 *                       CCallbackClient)
 *             - server: the CCallbackNetBase object (which really is a
 *                       CCallbackServer object, for a server)
 ****************************************************************************/
void cbAddEntity ( CMessage& msgin, TSockId from, CCallbackNetBase& server )
{
	bool    all;
	uint32  id;
	string  name;
	uint8   race;
	CVector startPoint;

	// Extract the incomming message content from the Frontend and print it
	msgin.serial( id );
	msgin.serial( name );
	msgin.serial( race );
	msgin.serial( startPoint );
	nlinfo( "Received ADD_ENTITY line." );

	// Prepare to send back the message.
	all = true;
	CMessage msgout( CNetManager::getSIDA( "POS" ), "ADD_ENTITY" );
	msgout.serial( all );
	msgout.serial( id );
	msgout.serial( id );
	msgout.serial( name );
	msgout.serial( race );
	msgout.serial( startPoint );

	/*
	 * Send the message to all the connected Frontend. If we decide to send
	 * it back to the sender, that last argument should be 'from' inteed of '0'
	 */
	CNetManager::send( "POS", msgout, 0 );

	nlinfo( "Send back ADD_ENTITY line." );

	// Send ADD_ENTITY message about all already connected client to the new one.
	all = false;
	_pmap::iterator ItPlayer;
	for (ItPlayer = playerList.begin(); ItPlayer != playerList.end(); ++ItPlayer)
	{
		CMessage msgout( CNetManager::getSIDA( "POS" ), "ADD_ENTITY" );
		msgout.serial( all );
		msgout.serial( id );
		msgout.serial( ((*ItPlayer).second).id );
		msgout.serial( ((*ItPlayer).second).name );
		msgout.serial( ((*ItPlayer).second).race );
		msgout.serial( ((*ItPlayer).second).position );

		CNetManager::send( "POS", msgout, from );
	}

	nlinfo( "Send ADD_ENTITY line about all already connected clients to the new one." );

	// ADD the current added entity in the player list.
	playerList.insert( _pmap::value_type( id,
										  _player( id, name, race, startPoint ) ));
}


/****************************************************************************
 * Function:   cbPosition
 *             Callback function called when the Position Service receive a
 *             "ENTITY_POS" message
 * Arguments:
 *             - msgin:  the incomming message
 *             - from:   the "sockid" of the sender (usually useless for a
 *                       CCallbackClient)
 *             - server: the CCallbackNetBase object (which really is a
 *                       CCallbackServer object, for a server)
 ****************************************************************************/
void cbPosition ( CMessage& msgin, TSockId from, CCallbackNetBase& server )
{
	uint32  id;
	CVector pos;
	float   angle;
	uint32  state;

	// Extract the incomming message content from the Frontend and print it
	msgin.serial( id );
	msgin.serial( pos );
	msgin.serial( angle );
	msgin.serial( state );
	//nlinfo( "Received ENTITY_POS line." );

	// Update position information in the player list
	_pmap::iterator ItPlayer;
	ItPlayer = playerList.find( id );
	if ( ItPlayer == playerList.end() )
	{
		nlinfo( "Player id %u not found !", id );
	}
	else
	{
		((*ItPlayer).second).position = pos;
		//nlinfo( "Player position updated" );
	}

	// Prepare to send back the message.
	CMessage msgout( CNetManager::getSIDA( "POS" ), "ENTITY_POS" );
	msgout.serial( id );
	msgout.serial( pos );
	msgout.serial( angle );
	msgout.serial( state );

	/*
	 * Send the message to all the connected Frontend. If we decide to send
	 * it back to the sender, that last argument should be 'from' inteed of '0'
	 */
	CNetManager::send( "POS", msgout, 0 );

	//nlinfo( "Send back ENTITY_POS line." );
}


/****************************************************************************
 * Function:   cbRemoveEntity
 *             Callback function called when the Position Service receive a
 *             "REMOVE_ENTITY" message
 * Arguments:
 *             - msgin:  the incomming message
 *             - from:   the "sockid" of the sender (usually useless for a
 *                       CCallbackClient)
 *             - server: the CCallbackNetBase object (which really is a
 *                       CCallbackServer object, for a server)
 ****************************************************************************/
void cbRemoveEntity ( CMessage& msgin, TSockId from, CCallbackNetBase& server )
{
	uint32 id;

	// Extract the incomming message content from the Frontend and print it
	msgin.serial( id );
	nlinfo( "Received REMOVE_ENTITY line." );

	// Prepare to send back the message.
	CMessage msgout( CNetManager::getSIDA( "POS" ), "REMOVE_ENTITY" );
	msgout.serial( id );

	/*
	 * Send the message to all the connected Frontend. If we decide to send
	 * it back to the sender, that last argument should be 'from' inteed of '0'
	 */
	CNetManager::send( "POS", msgout, 0 );

	// Remove player form the player list.
	playerList.erase( id );	

	nlinfo( "Send back REMOVE_ENTITY line. %d players left ...",
			playerList.size() );
}


/****************************************************************************
 * Function:   cbSnowball
 *             Callback function called when the Position Service receive a
 *             "SNOWBALL" message
 * Arguments:
 *             - msgin:  the incomming message
 *             - from:   the "sockid" of the sender (usually useless for a
 *                       CCallbackClient)
 *             - server: the CCallbackNetBase object (which really is a
 *                       CCallbackServer object, for a server)
 ****************************************************************************/
void cbSnowball ( CMessage& msgin, TSockId from, CCallbackNetBase& server )
{
	static uint32 snowballId = START_SNOW_ID;

	uint32  playerId;
	CVector start,
			target;
	float   speed,
			explosionRadius;

	// Extract the incomming message content from the Frontend and print it
	msgin.serial( playerId );
	msgin.serial( start );
	msgin.serial( target );
	msgin.serial( speed );
	msgin.serial( explosionRadius );
	nlinfo( "Received SNOWBALL line." );

	// Store new snowballs informations
	CTrajectory traj;
	traj.init( start, target, speed, CTime::getLocalTime() + THROW_ANIM_OFFSET );
	_snowball snowball = _snowball( snowballId, playerId, traj, explosionRadius );
	snoList.push_front( snowball );

	// Prepare to send back the message.
	CMessage msgout( CNetManager::getSIDA( "POS" ), "SNOWBALL" );
	msgout.serial( snowballId );
	msgout.serial( playerId );
	msgout.serial( start );
	msgout.serial( target );
	msgout.serial( speed );
	msgout.serial( explosionRadius );

	snowballId++;

	/*
	 * Send the message to all the connected Frontend. If we decide to send
	 * it back to the sender, that last argument should be 'from' inteed of '0'
	 */
	CNetManager::send( "POS", msgout, 0 );

	nlinfo( "Send back SNOWBALL line." );
}


/****************************************************************************
 * CallbackArray
 *
 * It define the functions to call when receiving a specific message
 ****************************************************************************/
TCallbackItem CallbackArray[] =
{
	{ "ADD_ENTITY",    cbAddEntity    },
	{ "ENTITY_POS",    cbPosition     },
	{ "REMOVE_ENTITY", cbRemoveEntity },
	{ "SNOWBALL",      cbSnowball     }
};


/****************************************************************************
 * Function:   SendHITMsg
 *             Send HIT message to all clients
 * 
 * Arguments:
 *             - snowball:  snowball id
 *             - victim:    player touched by the snowball
 *             - direct:    define if the hit is direct or by the explosion
 *                          area
 ****************************************************************************/
void SendHITMsg ( uint32 snowball, uint32 victim, bool direct )
{
	CMessage msgout( CNetManager::getSIDA( "POS" ), "HIT" );

	msgout.serial( snowball );
	msgout.serial( victim );
	msgout.serial( direct );

	CNetManager::send( "POS", msgout, 0 );
}


/****************************************************************************
 * CPositionService
 ****************************************************************************/
class CPositionService : public IService
{
public:

	// Initialisation
	void init()
	{
		DebugLog->addNegativeFilter ("NETL");
	}

	// Update fonction, called at every frames
	bool update()
	{
		//_snowball snowball;
		CVector   snoPos;
		float     distance;
		bool      removeSnowball;

		// Get the Current time
		TTime currentTime = CTime::getLocalTime();
		list<_snowball>::iterator ItSnowball;

		// Check collision of snowballs with players
		ItSnowball = snoList.begin();
		while (  ItSnowball != snoList.end() )
		{
			removeSnowball = false;

			list<_snowball>::iterator ItSb = ItSnowball++;
			_snowball snowball = (*ItSb);

			// Test collision (direct and explosion with players)
			_pmap::iterator ItPlayer;
			for (ItPlayer = playerList.begin(); ItPlayer != playerList.end(); ++ItPlayer)
			{
				_player player = (*ItPlayer).second;

				/*
				 * Snowballs can't touch the guy which throw it, Like that
				 * players could not kill them self (intentionally or not :-)
				 */
				if ( player.id == snowball.owner )
				{
					continue;
				}

				// Get the current snowball position
				snoPos = snowball.traj.eval( currentTime );

				// Test direct collision with players
				distance = (player.position - snoPos).norm();
				if ( distance < ( PLAYER_RADIUS + SNOWBALL_RADIUS ) )
				{
					nlinfo( "HIT on player %u by player %u.",
							player.id, snowball.owner );

					// Send HIT message
					SendHITMsg( snowball.id, player.id, true );

					// Flag the snowball to be removed from the list
					removeSnowball = true;
				}

				// Snowballs touch his stop Position
				if ( snowball.traj.getStopTime() < currentTime )
				{
					// Test for explosion victims
					distance = (player.position - snoPos).norm();
					if ( distance < ( PLAYER_RADIUS + snowball.explosionRadius ) )
					{
						nlinfo( "Explosion hit on player %u by player %u.",
								player.id, snowball.owner );

						// Send HIT message
						SendHITMsg( snowball.id, player.id, false );
					}

					// Flag the snowball to be removed from the list
					removeSnowball = true;
				}
			}


			// Removed if flaged snowball
			if ( removeSnowball == true )
			{
				snoList.erase( ItSb );
				nlinfo( "Removed outdated SNOWBALL id %u.", snowball.id );
			}

		}

		return true;
	}

};


/****************************************************************************
 * SNOWBALLS POSITION SERVICE MAIN Function
 *
 * This call create a main function for the POSITION service:
 *
 *    - based on the base service class "IService", no need to inherit from it
 *    - having the short name "POSITION"
 *    - having the long name "position_service"
 *    - listening on an automatically allocated port (0) by the naming service
 *    - and callback actions set to "CallbackArray"
 *
 ****************************************************************************/
NLNET_SERVICE_MAIN( CPositionService,
					"POS",
					"position_service",
					0,
					CallbackArray )


/* end of file */
