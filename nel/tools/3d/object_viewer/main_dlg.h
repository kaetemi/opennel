/** \file main_dlg.cpp
 * <File description>
 *
 * $Id: main_dlg.h,v 1.4 2001/06/15 16:24:45 corvazier Exp $
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

#if !defined(AFX_MAIN_DLG_H__9B22CB90_1929_11D5_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_MAIN_DLG_H__9B22CB90_1929_11D5_9CD4_0050DAC3A412__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// main_dlg.h : header file
//

#include "slot_dlg.h"
#include <3d/channel_mixer.h>
#include <3d/animation_playlist.h>

/////////////////////////////////////////////////////////////////////////////
// CMainDlg dialog

class CMainDlg : public CDialog
{
// Construction
public:
	CMainDlg(class CObjectViewer* main, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMainDlg)
	enum { IDD = IDD_MAIN_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	void	init (NL3D::CAnimationSet* animationSet);
	void	setAnimTime (float animStart, float animEnd);
	void	getSlot ();
	void	setSlot ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
public:
	NL3D::CAnimationSet*		AnimationSet;
	CSlotDlg					Slots[NL3D::CChannelMixer::NumAnimationSlot];
	NL3D::CAnimationPlaylist	Playlist;
	CObjectViewer*				Main;

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMainDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAIN_DLG_H__9B22CB90_1929_11D5_9CD4_0050DAC3A412__INCLUDED_)
