/** \file move_listener.cpp
 * <File description>
 *
 * $Id: move_listener.cpp,v 1.3.40.1 2006/01/11 15:05:12 boucher Exp $
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

#include "move_listener.h"
#include "nel/misc/event_server.h"
#include "nel/../../src/3d/driver.h"
#include "nel/../../src/3d/nelu.h"


using namespace NLMISC;

namespace NL3D {





/****************************************************************\
						 CMoveListener()
\****************************************************************/
CMoveListener::CMoveListener()
{
	_ViewMatrix.identity();
	_Viewport.initFullScreen();
	
	_Mode = WALK;

	_MouseFree = false;

	_CursorInit = false;

	_Width = 0;
	_Height = 0;

	_Top=1.f;
	_Bottom=1.f;
	_Left=1.f;
	_Right=1.f;
	_Depth=1.f;

	_RotZ = 0;
	_RotX = 0;

	_TransSpeed = 1.66f;
	_RotSpeed = 1;

	_CurrentTime = 0;
	_LastTime = 0;

	_EyesHeight = 1.80f;
}


/****************************************************************\
						 changeViewMatrix()
\****************************************************************/
void CMoveListener::changeViewMatrix()
{
	float time = (sint64)(_CurrentTime-_LastTime)*0.001f;

	// Forward
	if(CNELU::AsyncListener.isKeyDown(KeyUP))
	{
		if(_Mode==WALK)
		{
			_Pos.x -= time*_TransSpeed * (float)sin(_RotZ);
			_Pos.y += time*_TransSpeed * (float)cos(_RotZ);
		}
		else
		{
			_Pos.x -= time*_TransSpeed * (float)sin(_RotZ)*(float)cos(_RotX);
			_Pos.y += time*_TransSpeed * (float)cos(_RotZ)*(float)cos(_RotX);
			_Pos.z += time*_TransSpeed * (float)sin(_RotX);
		}
	}
	// Backward
	if(CNELU::AsyncListener.isKeyDown(KeyDOWN))
	{
		if(_Mode==WALK)
		{
			_Pos.x += time*_TransSpeed * (float)sin(_RotZ);
			_Pos.y -= time*_TransSpeed * (float)cos(_RotZ);
		}
		else
		{
			_Pos.x += time*_TransSpeed * (float)sin(_RotZ)*(float)cos(_RotX);
			_Pos.y -= time*_TransSpeed * (float)cos(_RotZ)*(float)cos(_RotX);
			_Pos.z -= time*_TransSpeed * (float)sin(_RotX);
		}
	}

	// Strafe left
	if(CNELU::AsyncListener.isKeyDown(KeyLEFT))
	{
		_Pos.x = _Pos.x - time*_TransSpeed*(float)cos(_RotZ);
		_Pos.y = _Pos.y - time*_TransSpeed*(float)sin(_RotZ);
	}
	// Strafe right
	if(CNELU::AsyncListener.isKeyDown(KeyRIGHT))
	{
		_Pos.x = _Pos.x + time*_TransSpeed*(float)cos(_RotZ);
		_Pos.y = _Pos.y + time*_TransSpeed*(float)sin(_RotZ);
	}
	// Up
	if(CNELU::AsyncListener.isKeyDown(KeySHIFT))
	{
		_Pos.z += time*_TransSpeed;
	}
	// Down
	if(CNELU::AsyncListener.isKeyDown(KeyCONTROL))
	{
		_Pos.z -= time*_TransSpeed;
	}
	
	if(CNELU::AsyncListener.isKeyPushed(Key1))
	{
		_TransSpeed = 5/3.6f; // 5km/h
	}
	if(CNELU::AsyncListener.isKeyPushed(Key2))
	{
		_TransSpeed = 10/3.6f; // 10km/h
	}
	if(CNELU::AsyncListener.isKeyPushed(Key3))
	{
		_TransSpeed = 20/3.6f; // 20km/h
	}
	if(CNELU::AsyncListener.isKeyPushed(Key4))
	{
		_TransSpeed = 40/3.6f; //40km/h
	}
	if(CNELU::AsyncListener.isKeyPushed(Key5))
	{
		_TransSpeed = 80/3.6f; //80km/h
	}
	if(CNELU::AsyncListener.isKeyPushed(Key6))
	{
		_TransSpeed = 160/3.6f; //160km/h
	}
	if(CNELU::AsyncListener.isKeyPushed(Key7))
	{
		_TransSpeed = 320/3.6f; //320km/h
	}
	if(CNELU::AsyncListener.isKeyPushed(Key8))
	{
		_TransSpeed = 640/3.6f; //640km/h
	}
	
	// Transform view matrix
	_ViewMatrix.identity();
	_ViewMatrix.translate(CVector(0,0,_EyesHeight));
	_ViewMatrix.translate(_Pos);
	_ViewMatrix.rotateZ(_RotZ);
	_ViewMatrix.rotateX(_RotX);

	CNELU::Camera->setMatrix(_ViewMatrix);

}


/****************************************************************\
						 operator()
\****************************************************************/
void CMoveListener::operator()(const CEvent& event)
{
	static const float eps = 0.001f;

	// Rotation
	if (event==EventMouseMoveId)
	{
		CEventMouse* mouseEvent=(CEventMouse*)&event;

		if(!_MouseFree)
		{
			if(_CursorInit)
			{
				float difx =  0.5f-mouseEvent->X;
				if ( (float)fabs(difx) > eps)
				{
					_RotZ += _RotSpeed*(difx);
				}
				float dify =  -(0.5f-mouseEvent->Y);
				if ( (float)fabs(dify) > eps)
				{
					_RotX += _RotSpeed*(dify);
				}	
			}
			else
			{
				_CursorInit = true;
			}
			_Scene->getDriver()->setMousePos(0.5,0.5);
		}
	}

	changeViewMatrix();
}


/****************************************************************\
						 addToServer()
\****************************************************************/
void CMoveListener::addToServer(CEventServer& server)
{
	server.addListener (EventMouseMoveId, this);
}


/****************************************************************\
						 removeFromServer()
\****************************************************************/
void CMoveListener::removeFromServer (CEventServer& server)
{
	server.removeListener (EventMouseMoveId, this);
}

}; // NL3D
