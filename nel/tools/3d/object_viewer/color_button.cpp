/** \file color_button.cpp
 * display a color box...
 *
 * $Id: color_button.cpp,v 1.4 2002/11/04 15:40:44 boucher Exp $
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


// color_button.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "color_button.h"


WNDPROC CColorButton::_BasicButtonWndProc = NULL  ;


/////////////////////////////////////////////////////////////////////////////
// CColorButton

CColorButton::CColorButton() : _Color(CRGBA::White)
{		
}

CColorButton::~CColorButton()
{
}


BEGIN_MESSAGE_MAP(CColorButton, CButton)
	//{{AFX_MSG_MAP(CColorButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorButton message handlers

void CColorButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Add your code to draw the specified item
	HDC dc = lpDrawItemStruct->hDC ;
	
	RECT r ;

	GetClientRect(&r) ;

	r.left += 4 ;
	r.top +=  4 ;
	r.bottom -= 4 ;
	r.right -= 4 ;
	GetClientRect(&r) ;
	CBrush b ;
	b.CreateSolidBrush(RGB(_Color.R, _Color.G, _Color.B)) ;
	::FillRect(dc, &r, (HBRUSH) b) ;	


}
