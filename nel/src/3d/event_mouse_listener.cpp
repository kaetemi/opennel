/** \file event_mouse_listener.cpp
 * <File description>
 *
 * $Id: event_mouse_listener.cpp,v 1.6 2001/06/15 16:06:17 vizerie Exp $
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

#include "nel/3d/event_mouse_listener.h"
#include "nel/misc/event_server.h"
#include "nel/3d/camera.h"
#include "nel/misc/time_nl.h"

using namespace NLMISC;

namespace NL3D 
{


CEvent3dMouseListener::CEvent3dMouseListener()
{
	_Matrix.identity();
	_ModelMatrix.identity() ;
	_EnableModelMatrixEdition = false ;
	_HotSpot.set (0,0,0);
	_Viewport.initFullScreen();
	_Frustrum.init (2.f, 2.f, -1.f, 1.f);
	_MouseMode=nelStyle;
	setSpeed (10.f);
	_LastTime=CTime::getLocalTime ();
}

void CEvent3dMouseListener::operator ()(const CEvent& event)
{
	CEventMouse* mouseEvent=(CEventMouse*)&event;
	if (event==EventMouseMoveId)
	{
		bool bRotate=false;
		bool bTranslateXY=false;
		bool bTranslateZ=false;
		bool bZoom=false;

		// Rotate Axis
		CVector axis;

		if (_MouseMode==nelStyle)
		{
			bRotate=(mouseEvent->Button==(ctrlButton|rightButton));
			bTranslateXY=(mouseEvent->Button==(ctrlButton|leftButton));
			bTranslateZ=(mouseEvent->Button==(ctrlButton|shiftButton|leftButton));
			bZoom=(mouseEvent->Button==(altButton|leftButton));
			axis=_HotSpot;
		}
		else if (_MouseMode==edit3d)
		{
			bRotate=(mouseEvent->Button==(altButton|middleButton)) || (mouseEvent->Button==(altButton|leftButton));
			bTranslateXY=(mouseEvent->Button==(ctrlButton|leftButton)) || (mouseEvent->Button==middleButton);
			bTranslateZ=(mouseEvent->Button==(ctrlButton|shiftButton|leftButton)) || (mouseEvent->Button==(ctrlButton|middleButton));
			bZoom=(mouseEvent->Button==(shiftButton|leftButton)) || (mouseEvent->Button==(ctrlButton|altButton|middleButton));
			axis=_HotSpot;
		}
		else // if (_MouseMode==firstPerson)
		{
			bRotate=(mouseEvent->Button&leftButton)!=0;
			bTranslateXY=false;
			bTranslateZ=false;
			bZoom=false;
			axis=_Matrix.getPos();
		}

		if (bRotate)
		{
			// First in the hotSpot
			CMatrix comeFromHotSpot=_Matrix;
			comeFromHotSpot.setPos (axis);

			// Then turn along the Z axis with X mouse
			CMatrix turnZ;
			turnZ.identity();
			turnZ.rotateZ ((float)Pi*2.f*(_X-mouseEvent->X));

			// Then turn along the X axis with Y mouse
			CMatrix turnX;
			turnX.identity();
			turnX.rotateX ((float)Pi*2.f*(mouseEvent->Y-_Y));

			// Then come back from hotspot
			CMatrix goToHotSpot=comeFromHotSpot;
			goToHotSpot.invert();

			// Make the matrix
			CMatrix negPivot, Pivot;
			negPivot.identity();
			negPivot.setPos (-axis);
			Pivot.identity();
			Pivot.setPos (axis);

			// Make this transformation \\//
			//_Matrix=Pivot*turnZ*negPivot*comeFromHotSpot*turnX*goToHotSpot*_Matrix;
			Pivot*=turnZ;
			Pivot*=negPivot;
			Pivot*=comeFromHotSpot;
			Pivot*=turnX;
			Pivot*=goToHotSpot;
			Pivot*=_Matrix;

			_Matrix=Pivot;

			// Normalize, too much transformation could give an ugly matrix..
			_Matrix.normalize (CMatrix::XYZ);
		}

		if (bTranslateXY||bTranslateZ||bZoom)
		{
			// Move in plane

			CPlane plane;

			// Plane of the hotspot
			if (! _EnableModelMatrixEdition)
			{			
				plane.make (_Matrix.getJ(), axis);
			}
			else
			{			
				plane.make (_Matrix.getJ(), _ModelMatrix.getPos());
			}


			// Get ray from mouse point
			CVector worldPoint1, worldPoint2;
			CVector pos, dir;
			_Viewport.getRayWithPoint (_X, _Y, pos, dir, _Matrix, _Frustrum);
			worldPoint1=plane.intersect (pos, pos+dir);
			_Viewport.getRayWithPoint (mouseEvent->X, mouseEvent->Y, pos, dir, _Matrix, _Frustrum);
			worldPoint2=plane.intersect (pos, pos+dir);

			// Move the camera
			if (bTranslateXY)
			{
				if (! _EnableModelMatrixEdition)
				{
					_Matrix.setPos(_Matrix.getPos()+worldPoint1-worldPoint2);
				}
				else
				{
					_ModelMatrix.setPos(_ModelMatrix.getPos()-worldPoint1+worldPoint2);
				}
			}
			else if (bTranslateZ)
			{
				CVector vect=worldPoint1-worldPoint2;
				if (! _EnableModelMatrixEdition)
				{
					_Matrix.setPos(_Matrix.getPos()+_Matrix.getK()*(vect.x+vect.y+vect.z));
				}
				else
				{
					_ModelMatrix.setPos(_ModelMatrix.getPos()+_Matrix.getK()*(vect.x+vect.y+vect.z));
				}
			}
			else if (bZoom)
			{
				CVector vect=worldPoint1-worldPoint2;
				CVector direc=axis-_Matrix.getPos();
				direc.normalize();
				if (! _EnableModelMatrixEdition)
				{
					_Matrix.setPos(_Matrix.getPos()+direc*(vect.x+vect.y+vect.z));
				}
				else
				{
					_ModelMatrix.setPos(_ModelMatrix.getPos()+direc*(vect.x+vect.y+vect.z));
				}
			}
		}
		
		if (bTranslateZ)
		{
			// Move up
		}

		// Update mouse position
		_X=mouseEvent->X;
		_Y=mouseEvent->Y;
	}
	else if (event==EventMouseDownId)
	{
		// Update mouse position
		_X=mouseEvent->X;
		_Y=mouseEvent->Y;
	}
	else if (event==EventMouseUpId)
	{
		// Update mouse position
		_X=mouseEvent->X;
		_Y=mouseEvent->Y;
	}
	else if (event==EventMouseWheelId)
	{
		// Zoom..
		CEventMouseWheel* mouseEvent=(CEventMouseWheel*)&event;

		CVector direc=_HotSpot-_Matrix.getPos();
		if (! _EnableModelMatrixEdition)
		{
			_Matrix.setPos(_Matrix.getPos()+direc*(mouseEvent->Direction?0.1f:-0.1f));
		}
		else
		{
			_ModelMatrix.setPos(_ModelMatrix.getPos()+direc*(mouseEvent->Direction?0.1f:-0.1f));
		}
	}
}

void CEvent3dMouseListener::addToServer (CEventServer& server)
{
	server.addListener (EventMouseMoveId, this);
	server.addListener (EventMouseDownId, this);
	server.addListener (EventMouseUpId, this);
	server.addListener (EventMouseWheelId, this);
	_AsyncListener.addToServer (server);
}

void CEvent3dMouseListener::removeFromServer (CEventServer& server)
{
	server.removeListener (EventMouseMoveId, this);
	server.removeListener (EventMouseDownId, this);
	server.removeListener (EventMouseUpId, this);
	server.removeListener (EventMouseWheelId, this);
	_AsyncListener.removeFromServer (server);
}

const NLMISC::CMatrix& CEvent3dMouseListener::getViewMatrix ()
{
	// Mode first person ?
	if (_MouseMode==firstPerson)
	{
		// CVector
		CVector dir (0,0,0);
		bool find=false;

		// Key pushed ?
		if (_AsyncListener.isKeyDown (KeyUP))
		{
			dir+=CVector (0, 1, 0);
			find=true;
		}
		if (_AsyncListener.isKeyDown (KeyDOWN))
		{
			dir+=CVector (0, -1, 0);
			find=true;
		}
		if (_AsyncListener.isKeyDown (KeyRIGHT))
		{
			dir+=CVector (1, 0, 0);
			find=true;
		}
		if (_AsyncListener.isKeyDown (KeyLEFT))
		{
			dir+=CVector (-1, 0, 0);
			find=true;
		}
		if (_AsyncListener.isKeyDown (KeyNEXT))
		{
			dir+=CVector (0, 0, -1);
			find=true;
		}
		if (_AsyncListener.isKeyDown (KeyPRIOR))
		{
			dir+=CVector (0, 0, 1);
			find=true;
		}

		// key found ?
		if (find)
		{
			// Time elapsed
			uint32 milli=(uint32)(CTime::getLocalTime ()-_LastTime);

			// Speed
			float dPos=_Speed*(float)milli/1000.f;

			// Good direction
			dir.normalize ();
			dir*=dPos;

			// Orientation
			dir=_Matrix.mulVector (dir);

			// New position
			_ModelMatrix.setPos (_ModelMatrix.getPos ()+dir);
		}
	}

	// Last time
	_LastTime=CTime::getLocalTime ();

	
	// Return the matrix
	return _Matrix;
}



}; // NL3D
