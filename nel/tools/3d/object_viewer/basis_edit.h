/** \file basis_edit.h
 * a dialog to edit the orientation of a basis
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

#if !defined(AFX_BASIS_EDIT_H__7EBC9DBD_5DDD_44F6_8C3D_6F8FB0A1FFCD__INCLUDED_)
#define AFX_BASIS_EDIT_H__7EBC9DBD_5DDD_44F6_8C3D_6F8FB0A1FFCD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include "nel/3d/ps_plane_basis.h"
#include "ps_wrapper.h"
#include "edit_attrib_dlg.h"




/////////////////////////////////////////////////////////////////////////////
// CBasisEdit dialog

class CBasisEdit : public CEditAttribDlg
{
// Construction
public:
	CBasisEdit(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBasisEdit)
	enum { IDD = IDD_BASIS_EDIT };
	CScrollBar	m_PhiCtrl;
	CScrollBar	m_PsiCtrl;
	CScrollBar	m_ThetaCtrl;
	UINT	m_Psi;
	UINT	m_Theta;
	UINT	m_Phi;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBasisEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

public:
	// set a wrapper to get the datas
	void setWrapper(IPSWrapper<NL3D::CPlaneBasis> *wrapper) { _Wrapper = wrapper ; }

	// create and init the dialog
	void init(uint32 x, uint32 y, CWnd *pParent) ;


protected:

	void updateAnglesFromReader(void) ;


	IPSWrapper<NL3D::CPlaneBasis> *_Wrapper ;

	// Generated message map functions
	//{{AFX_MSG(CBasisEdit)
	afx_msg void OnPaint();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


void DrawBasisInDC(const CPoint &center, float size, const NLMISC::CMatrix &m, CDC &dc, NLMISC::CRGBA col[3]) ;


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BASIS_EDIT_H__7EBC9DBD_5DDD_44F6_8C3D_6F8FB0A1FFCD__INCLUDED_)

/* Merge OpenNeL SVN
 */
