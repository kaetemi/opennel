/** \file animation_set_dlg.cpp
 * <File description>
 *
 * $Id: animation_set_dlg.h,v 1.4 2001/04/26 17:57:41 corvazier Exp $
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

#if !defined(AFX_ANIMATION_SET_DLG_H__AA6100E2_19FA_11D5_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_ANIMATION_SET_DLG_H__AA6100E2_19FA_11D5_9CD4_0050DAC3A412__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// animation_set_dlg.h : header file
//

#include <nel/3d/animation_set.h>

/////////////////////////////////////////////////////////////////////////////
// CAnimationSetDlg dialog

class CAnimationSetDlg : public CDialog
{
	friend class CObjectViewer;
// Construction
public:
	CAnimationSetDlg (class CObjectViewer* objView, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAnimationSetDlg)
	enum { IDD = IDD_ANIMATION_SET };
	CTreeCtrl	Tree;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimationSetDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Load an animation
	void loadAnimation (const char* fileName);
	void addAnimation (NL3D::CAnimation* pAnim, const char* name);
	void loadSkeleton (const char* fileName);

	// Pointer on an animation set
	CObjectViewer*				_ObjView;
	std::vector<std::string>	_ListAnimation;
	std::vector<std::string>	_ListSkeleton;

	// Generated message map functions
	//{{AFX_MSG(CAnimationSetDlg)
	afx_msg void OnAddAnimation();
	afx_msg void OnReset();
	afx_msg void OnAddSkelWt();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMATION_SET_DLG_H__AA6100E2_19FA_11D5_9CD4_0050DAC3A412__INCLUDED_)
