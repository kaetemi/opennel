/** \file slot_dlg.h
 * <File description>
 *
 * $Id: slot_dlg.h,v 1.5 2001/04/30 16:58:31 corvazier Exp $
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

#if !defined(AFX_SLOT_DLG_H__9B22CB8B_1929_11D5_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_SLOT_DLG_H__9B22CB8B_1929_11D5_9CD4_0050DAC3A412__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// slot_dlg.h : header file
//

#include "blend_wnd.h"
#include <nel/3d/animation_set.h>
#include <nel/misc/debug.h>

/////////////////////////////////////////////////////////////////////////////
// CSlotDlg dialog

class CSlotDlg : public CDialog
{
// Construction
public:
	CSlotDlg(CWnd* pParent = NULL);   // standard constructor

	void init (uint id, NL3D::CAnimationSet* animationSet, class CObjectViewer* mainDlg);

	void setAnimation (uint animationId, NL3D::CAnimation *animation, const char* name);

	void setSkeletonTemplateWeight (uint skeletonWeightId, NL3D::CSkeletonWeight *skeleton, const char* name)
	{
		SkeletonName=name;
		SkeletonWeightTemplate=skeleton;
		SkeletonWeightId=skeletonWeightId;
	}

	bool isEmpty ()
	{
		return Animation==NULL;
	}

	void setWindowName ();
	void validateTime ();
	void updateScrollBar ();
	void setAnimTime (float animStart, float animEnd);
	float getTimeIncrement ();
	float getTimeOffset ();
	float getStartTime ();
	float getEndTime ();
	void computeLength ();

	// A CBlendWnd
	uint					Id;
	uint					AnimationId;
	uint					SkeletonWeightId;
	CBlendWnd				Blend;
	NL3D::CAnimationSet*	AnimationSet;
	NL3D::CAnimation*		Animation;
	std::string				AnimationName;
	std::string				SkeletonName;
	NL3D::CSkeletonWeight*	SkeletonWeightTemplate;
	CObjectViewer*			MainDlg;

// Dialog Data
	//{{AFX_DATA(CSlotDlg)
	enum { IDD = IDD_SLOT };
	CButton	InvertSkeletonWeightCtrl;
	CButton	AlignBlendCtrl;
	CButton	ClampCtrl;
	CScrollBar	ScrollBarCtrl;
	CSpinButtonCtrl	OffsetSpinCtrl;
	CEdit	OffsetCtrl;
	CSpinButtonCtrl	StartTimeSpinCtrl;
	CSpinButtonCtrl	StartBlendSpinCtrl;
	CSpinButtonCtrl	SpeedFactorSpinCtrl;
	CSpinButtonCtrl	SmoothnessSpinCtrl;
	CSpinButtonCtrl	EndTimeSpinCtrl;
	CSpinButtonCtrl	EndBlendSpinCtrl;
	CEdit	StartTimeCtrl;
	CEdit	StartBlendCtrl;
	CEdit	SpeddFactorCtrl;
	CEdit	SmoothnessCtrl;
	CEdit	EndTimeCtrl;
	CEdit	EndBlendCtrl;
	float	EndBlend;
	float	Smoothness;
	float	SpeedFactor;
	float	StartBlend;
	int		ClampMode;
	BOOL	SkeletonWeightInverted;
	int		Offset;
	int		StartTime;
	int		EndTime;
	BOOL	enable;
	//}}AFX_DATA

	float	StartAnimTime;
	float	EndAnimTime;
	float	AnimationLength;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSlotDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	bool	_Empty;

	// Generated message map functions
	//{{AFX_MSG(CSlotDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnDeltaposEndBlendSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposEndTimeSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSmoothnessSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpeedFactorSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposStartBlendSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposStartTimeSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeEndBlend();
	afx_msg void OnChangeEndTime();
	afx_msg void OnChangeSmoothness();
	afx_msg void OnChangeSpeedFactor();
	afx_msg void OnChangeStartBlend();
	afx_msg void OnChangeStartTime();
	afx_msg void OnSetAnimation();
	afx_msg void OnSetSkeleton();
	afx_msg void OnChangeOffset();
	afx_msg void OnDeltaposOffsetSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnClamp();
	afx_msg void OnRepeat();
	afx_msg void OnDisable();
	afx_msg void OnAlignBlend();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SLOT_DLG_H__9B22CB8B_1929_11D5_9CD4_0050DAC3A412__INCLUDED_)
