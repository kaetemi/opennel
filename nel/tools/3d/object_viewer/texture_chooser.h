/** \file texture_chooser.h
 * A dailog that helps to choose particles texture
 * $Id: texture_chooser.h,v 1.4 2001/06/27 16:37:17 vizerie Exp $
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
 * MA 02
*/

#if !defined(AFX_TEXTURE_CHOOSER_H__FE10F78E_0B69_4EB0_8FC7_A48FAEB904FD__INCLUDED_)
#define AFX_TEXTURE_CHOOSER_H__FE10F78E_0B69_4EB0_8FC7_A48FAEB904FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// texture_chooser.h : header file
//


#include "nel/misc/smart_ptr.h"
#include "3d/texture.h"
#include "edit_attrib_dlg.h"

#include "ps_wrapper.h"


using NLMISC::CSmartPtr ;




/////////////////////////////////////////////////////////////////////////////
// CTextureChooser dialog

class CTextureChooser : public CEditAttribDlg
{
// Construction
public:
	// construct the object with the given texture
	CTextureChooser();   // standard constructor

	~CTextureChooser();

	virtual void init(uint32 x, uint32 y, CWnd *pParent = NULL) ;



	// set a wrapper to get the datas
	void setWrapper(IPSWrapperTexture *wrapper) { _Wrapper = wrapper ; }

// Dialog Data
	//{{AFX_DATA(CTextureChooser)
	enum { IDD = IDD_TEXTURE_CHOOSER };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextureChooser)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	IPSWrapperTexture *_Wrapper ;

	// handle to the current bitmap being displayed
	HBITMAP _CurrBitmap ;

	// update the current bitmap
	void textureToBitmap() ;

	// the current texture
	CSmartPtr<NL3D::ITexture> _Texture ;

	// Generated message map functions
	//{{AFX_MSG(CTextureChooser)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowseTexture();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP() ;

	
} ;




//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTURE_CHOOSER_H__FE10F78E_0B69_4EB0_8FC7_A48FAEB904FD__INCLUDED_)
