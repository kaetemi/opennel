/** \file event_mouse_listener.h
 * <File description>
 *
 * $Id: event_mouse_listener.h,v 1.8 2001/06/15 16:06:17 vizerie Exp $
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

#ifndef NL_EVENT_MOUSE_LISTENER_H
#define NL_EVENT_MOUSE_LISTENER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/event_listener.h"
#include "nel/misc/matrix.h"
#include "nel/3d/tmp/viewport.h"
#include "nel/3d/tmp/frustum.h"
#include "nel/3d/tmp/u_3d_mouse_listener.h"


namespace NL3D 
{


using NLMISC::CVector;
using NLMISC::CMatrix;


/**
 * CEvent3dMouseListener is a listener that handle a 3d matrix with mouse events.
 * This can be the view matrix, or the matrix of any object.
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CEvent3dMouseListener : public NLMISC::IEventListener, public U3dMouseListener
{
public:
	/**
	  * Mouse mode.
	  * There is two move modes: 3d editor style and NeL style. Default mode is NeL style.
	  * 
	  * (nelStyle) NeL style is:
	  * MouseRotateHotSpot:			CTRL + RIGHTMOUSE
	  * MouseTranslateXYHotSpot:	CTRL + LEFTMOUSE
	  * MouseTranslateZHotSpot:		CTRL + SHIFT + LEFTMOUSE
	  * MouseZoomHotSpot:			ALT + LEFTMOUSE
	  * 
	  * (edit3dStyle) 3d editor style is:
	  * MouseRotateHotSpot:			ALT + MIDDLEMOUSE
	  * MouseTranslateXYHotSpot:	MIDDLEMOUSE
	  * MouseTranslateZHotSpot:		CTRL + MIDDLEMOUSE
	  * 
	  * (firstPerson) First person shooter style is:
	  * MouseRotateView:			MOUSE MOVE
	  * KeyUp:						MOVE FORWARD
	  * KeyDown:					MOVE BACKWARD
	  * Left:						STRAF LEFT
	  * Right:						STRAF RIGHT
	  * PageUp:						MOVE UP
	  * PageDown:					MOVE DOWN
	  */
	//enum TMouseMode { nelStyle, edit3d, firstPerson };

	/** 
	  * Constructor. 
	  * You should call setMatrix, setFrustrum, setViewport, setHotStop and setMouseMode to initialize
	  * the whole object. By default, the viewmatrix is identity, the frustrum is (-1,1,-1,1,1,-1), the hot spot is (0,0,0) 
	  * and the viewport is fullscreen. The mouse mode is set to the NelStyle.
	  */
	CEvent3dMouseListener();

	/// \name Setup

	/** 
	  * Set both the current view matrix to use.
	  * \param matrix is the matrix to set.
	  * \see getViewMatrix()
	  */
	void setMatrix (const NLMISC::CMatrix& matrix)
	{
		_Matrix=matrix;	
	}

	/** Set the model matrix only
	  *  
	  * param matrix is the matrix to set.
	  * getModelMatrix()
	  */
	void setModelMatrix(const NLMISC::CMatrix& matrix)
	{
		_ModelMatrix = matrix ;
	}

	/** 
	  * Set the current frustrum to use.
	  * \param frustrum is the frustrum.
	  */
	void setFrustrum (const CFrustum& frustrum)
	{
		_Frustrum=frustrum;
	}

	/** 
	  * Set the viewport in use in the window. By default, the viewport is fullwindow.
	  * \param viewport is the viewport to use. All events outside the viewport are ignored.
	  */
	void setViewport (const NL3D::CViewport& viewport)
	{
		_Viewport=viewport;
	}

	/** 
	  * Set the current hot spot.
	  * \param hotSpot is the target to use when the mouse move. It can be for exemple the center.
	  * of the selected object. The hotspot is not modified by mouse events.
	  * \see getViewMatrix()
	  */
	void setHotSpot (const CVector& hotSpot)
	{
		_HotSpot=hotSpot;
	}

	/** 
	  * Set the mouse mode.
	  * \param mouseMode is the mode you want to use.
	  * \see TMouseMode
	  */
	void setMouseMode(TMouseMode mouseMode)
	{
		_MouseMode=mouseMode;
	}

	/// enable / disable model matrix edition mode. (the default deals with the with matrix)
	void enableModelMatrixEdition(bool enable = true) 
	{ 
		_EnableModelMatrixEdition = enable ;
	}
	  

	/** 
	  * Set the speed for first person mode. Default 10.f;
	  * \param speed is in unit per second.
	  * \see TMouseMode
	  */
	void setSpeed (float speed)
	{
		_Speed=speed;
	}

	/// \name Get

	/**
	  * Get the current view matrix.	  
	  * \return The current view matrix.
	  * \see setMatrix()
	  */
	const NLMISC::CMatrix& getViewMatrix () ;


	/**
	  * Get the current model matrix.
	  * \return The current view matrix.
	  * \see setModelMatrix()
	  */
	const NLMISC::CMatrix& getModelMatrix()
	{
		return _ModelMatrix ;	
	}



	/** 
	  * Get the current hot spot.
	  * \return the target used when the mouse move. It can be for exemple the center.
	  * of the selected object. The hotspot is not modified by mouse events.
	  * \see getViewMatrix()
	  */
	CVector getHotSpot () const
	{
		return _HotSpot;
	}

	/** 
	  * Register the listener to the server.
	  */
	void addToServer (NLMISC::CEventServer& server);

	/** 
	  * Unregister the listener to the server.
	  */
	void removeFromServer (NLMISC::CEventServer& server);

private:
	/// Internal use
	virtual void operator ()(const NLMISC::CEvent& event);

	CMatrix				_Matrix;
	CMatrix				_ModelMatrix ;
	bool				_EnableModelMatrixEdition  ;
	CFrustum			_Frustrum;
	CVector				_HotSpot;
	NL3D::CViewport		_Viewport;
	bool				_LeftPushed;
	bool				_MiddlePushed;
	bool				_RightPushed;
	float				_X;
	float				_Y;
	float				_Speed;
	uint64				_LastTime;
	TMouseMode			_MouseMode;
	NLMISC::CEventListenerAsync	_AsyncListener;
}; // NL3D

}

#endif // NL_EVENT_MOUSE_LISTENER_H

/* End of event_mouse_listener.h */
