/** \file particle_system_edit.h
 * Dialog used to edit global parameters of a particle system.
 *
 * $Id: particle_system_edit.h,v 1.10 2002/11/18 18:00:40 vizerie Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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


#if !defined(AFX_PARTICLE_SYSTEM_EDIT_H__CD8281FA_CA1A_4D87_B54F_509D490066A9__INCLUDED_)
#define AFX_PARTICLE_SYSTEM_EDIT_H__CD8281FA_CA1A_4D87_B54F_509D490066A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

#include "ps_wrapper.h"
#include "dialog_stack.h"
#include "popup_notify.h"
#include "editable_range.h"

namespace NL3D
{
	class CParticleSystem;
}


class CAutoLODDlg;

class CUserParamWrapper : public IPSWrapperFloat
{
public:
	NL3D::CParticleSystem *PS;
	uint32 Index;
	float get(void) const;
	void set(const float &v);
};


class CTimeThresholdWrapper : public IPSWrapperFloat
{
public:
	NL3D::CParticleSystem *PS;
	float get(void) const;		
	void set(const float &);
};

class CMaxNbIntegrationWrapper : public IPSWrapperUInt
{
public:
	NL3D::CParticleSystem *PS;
	uint32 get(void) const;		
	void set(const uint32 &);
};

class CMaxViewDistWrapper : public IPSWrapperFloat
{
public:
	NL3D::CParticleSystem *PS;
	float get(void) const;		
	void set(const float &);
};

class CLODRatioWrapper : public IPSWrapperFloat
{
public:
	NL3D::CParticleSystem *PS;
	float get(void) const;		
	void set(const float &);
};

/////////////////////////////////////////////////////////////////////////////
// CParticleSystemEdit dialog

class CParticleSystemEdit : public CDialog, public CDialogStack, public IPopupNotify
{
// Construction
public:
	CParticleSystemEdit(NL3D::CParticleSystem *ps);   // standard constructor

	~CParticleSystemEdit();

	void init(CWnd *pParent);
// Dialog Data
	//{{AFX_DATA(CParticleSystemEdit)
	enum { IDD = IDD_EDIT_PARTICLE_SYSTEM };
	CButton	m_DieWhenOutOfFrustumCtrl;
	CButton	m_DieWhenOutOfRangeCtrl;
	CComboBox	m_AnimTypeCtrl;
	CComboBox	m_PresetCtrl;
	CComboBox	m_DieOnEvent;
	CButton	m_PrecomputeBBoxCtrl;
	CEdit	m_BBoxZCtrl;
	CEdit	m_BBoxYCtrl;
	CEdit	m_BBoxXCtrl;
	CButton	m_EnableSlowDownCtrl;
	BOOL	m_AccurateIntegration;
	BOOL	m_EnableSlowDown;
	BOOL	m_DieWhenOutOfRange;
	BOOL	m_DieWhenOutOfFrustum;
	BOOL	m_EnableLoadBalancing;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParticleSystemEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NL3D::CParticleSystem *_PS;


	CEditableRangeFloat *_TimeThresholdDlg, *_MaxViewDistDlg, *_LODRatioDlg;
	CEditableRangeUInt  *_MaxIntegrationStepDlg; 
	CDialog				*_AutoLODDlg;
	CDialog				*_GlobalColorDlg;


	// Generated message map functions
	//{{AFX_MSG(CParticleSystemEdit)
	afx_msg void OnAccurateIntegration();
	afx_msg void OnEnableSlowDown();
	afx_msg void OnPrecomputeBbox();
	afx_msg void OnUpdateBbox();
	afx_msg void OnIncBbox();
	afx_msg void OnDecBbox();
	afx_msg void OnDieWhenOutOfRange();
	afx_msg void OnSelchangePsDieOnEvent();
	afx_msg void OnChangeApplyAfterDelay();
	afx_msg void OnDieWhenOutOfFrustum();
	afx_msg void OnSelchangeLifeMgtPresets();
	afx_msg void OnSelchangeAnimTypeCtrl();
	afx_msg void OnSharable();
	afx_msg void OnEditAutoLod();
	afx_msg void OnEnableAutoLod();
	afx_msg void OnForceLifeTimeUpdate();
	afx_msg void OnEditGlobalColor();
	afx_msg void OnGlobalColor();
	afx_msg void OnEnableLoadBalancing();
	afx_msg void OnGlobalUserParam1();
	afx_msg void OnGlobalUserParam2();
	afx_msg void OnGlobalUserParam3();
	afx_msg void OnGlobalUserParam4();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


	void updatePrecomputedBBoxParams(void);
	void updateIntegrationParams(void);
	void updateDieOnEventParams(void);
	void updateLifeMgtPresets();
	void childPopupClosed(CWnd *child);

	static CTimeThresholdWrapper		_TimeThresholdWrapper;
	static CMaxViewDistWrapper			_MaxViewDistWrapper;
	static CMaxNbIntegrationWrapper		_MaxNbIntegrationWrapper;
	static CLODRatioWrapper				_LODRatioWrapper;
    /// wrapper to tune user parameters
	static CUserParamWrapper			_UserParamWrapper[4];
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARTICLE_SYSTEM_EDIT_H__CD8281FA_CA1A_4D87_B54F_509D490066A9__INCLUDED_)
