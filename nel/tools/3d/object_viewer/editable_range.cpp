/** \file  editable_range.cpp
 * a dialog that help to choose a numeric value of any types. 
 *
 * $Id: editable_range.cpp,v 1.7 2001/09/13 14:27:34 vizerie Exp $
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

// editable_range.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "editable_range.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#include "range_manager.h"
#include "range_selector.h"


/////////////////////////////////////////////////////////////////////////////
// CEditableRange dialog


CEditableRange::CEditableRange(const std::string &id) 
	: _Id(id)
{
	//{{AFX_DATA_INIT(CEditableRange)
	m_MinRange = _T("");
	m_MaxRange = _T("");
	m_Value = _T("");
	m_SliderPos = 0;
	//}}AFX_DATA_INIT
		


}


BOOL CEditableRange::EnableWindow( BOOL bEnable)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	m_ValueCtrl.EnableWindow(bEnable);
	m_SliderCtrl.EnableWindow(bEnable);
	m_UpdateValue.EnableWindow(bEnable);
	m_SelectRange.EnableWindow(bEnable);

	UpdateData(FALSE);

	return CEditAttribDlg::EnableWindow(bEnable);
}

void CEditableRange::init(uint32 x, uint32 y, CWnd *pParent)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	Create(IDD_EDITABLE_RANGE, pParent);
	RECT r;
	GetClientRect(&r);
	MoveWindow(x, y, r.right, r.bottom);
	// set the slider size
	CSliderCtrl *sl = (CSliderCtrl *) GetDlgItem(IDC_SLIDER);	
	ShowWindow(SW_SHOW);
}


void CEditableRange::DoDataExchange(CDataExchange* pDX)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditableRange)
	DDX_Control(pDX, IDC_SLIDER, m_SliderCtrl);
	DDX_Control(pDX, IDC_VALUE, m_ValueCtrl);
	DDX_Control(pDX, IDC_UPDATE_VALUE, m_UpdateValue);
	DDX_Control(pDX, IDC_SELECT_RANGE, m_SelectRange);
	DDX_Text(pDX, IDC_MIN_RANGE, m_MinRange);
	DDX_Text(pDX, IDC_MAX_RANGE, m_MaxRange);
	DDX_Text(pDX, IDC_VALUE, m_Value);
	DDX_Slider(pDX, IDC_SLIDER, m_SliderPos);
	//}}AFX_DATA_MAP
}




BEGIN_MESSAGE_MAP(CEditableRange, CDialog)
	//{{AFX_MSG_MAP(CEditableRange)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER, OnReleasedcaptureSlider)
	ON_BN_CLICKED(IDC_SELECT_RANGE, OnSelectRange)	
	ON_EN_SETFOCUS(IDC_VALUE, OnSetfocusValue)
	ON_WM_KEYDOWN()
	ON_BN_CLICKED(IDC_UPDATE_VALUE, OnUpdateValue)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditableRange message handlers






BOOL CEditableRange::OnInitDialog() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CDialog::OnInitDialog();
	
	updateRange();
	updateValueFromReader();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CEditableRange::OnReleasedcaptureSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	UpdateData();
	CSliderCtrl *sl = (CSliderCtrl *) GetDlgItem(IDC_SLIDER);	
	if (
		(sl->GetRangeMax() -  sl->GetRangeMin()) != 0
		)
	{
		updateValueFromSlider(m_SliderPos * 1.f / (sl->GetRangeMax() -  sl->GetRangeMin()));		
	}
	else
	{
		updateValueFromSlider(0);
	}
	*pResult = 0;
}

void CEditableRange::OnSelectRange() 
{
	selectRange();
}



void CEditableRange::OnUpdateValue() 
{
	UpdateData();	
	updateValueFromText();
}


void CEditableRange::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == 13)
	{
		UpdateData();	
		updateValueFromText();		
	}
}


void CEditableRange::emptyDialog(void)
{
	m_Value = CString("");
	m_SliderPos = 0;
	UpdateData(FALSE);
}

void CEditableRange::OnSetfocusValue() 
{
	CEdit *ce = (CEdit *) GetDlgItem(IDC_VALUE);
	ce->PostMessage(EM_SETSEL, 0, -1);	
	ce->Invalidate();
}


///////////////////////////////////////////////////////
// IMPLEMENTATION OF TEMPLATE METHOD SPECIALIZATIONS //
///////////////////////////////////////////////////////


	//////////////////////////
	// float editable range //
	//////////////////////////

CEditableRangeT<float>::CEditableRangeT(const std::string &id, float defaultMin, float defaultMax ) 
			: CEditableRange(id), _Range(defaultMin, defaultMax), _Wrapper(NULL)
		{
		}

		void CEditableRangeT<float>::value2CString(float value, CString &dest)
		{
			dest.Format("%g", (double) value);
		}
		const char *CEditableRangeT<float>::string2value(const CString &value, float &result)
		{			
			if (sscanf((LPCTSTR) value, "%f", &result) == 1)
			{			
				return NULL;
			}
			else
			{
				return "invalid value";
			}	
		}

	///////////////////////////
	// uint32 editable range //
	///////////////////////////

		CEditableRangeT<uint32>::CEditableRangeT(const std::string &id, uint32 defaultMin , uint32 defaultMax )
		: CEditableRange(id), _Range(defaultMin, defaultMax), _Wrapper(NULL)
		{
		}

		void CEditableRangeT<uint32>::value2CString(uint32 value, CString &dest)
		{
			dest.Format("%d", value);
		}
		const char *CEditableRangeT<uint32>::string2value(const CString &value, uint32 &result)
		{			
			uint32 tmp;
			if (sscanf((LPCTSTR) value, "%d", &tmp) == 1)
			{
				if (strchr((LPCTSTR) value, '-'))
				{
					return "negative values not allowed";
				}
				else
				{
					result = tmp;
					return NULL;
				}
			}
			else
			{
				return "invalid value";
			}	
		}

	///////////////////////////
	// sint32 editable range //
	///////////////////////////

		CEditableRangeT<sint32>::CEditableRangeT(const std::string &id, sint32 defaultMin , sint32 defaultMax )
		: CEditableRange(id), _Range(defaultMin, defaultMax), _Wrapper(NULL)
		{
		}

		void CEditableRangeT<sint32>::value2CString(sint32 value, CString &dest)
		{
			dest.Format("%d", value);
		}
		const char *CEditableRangeT<sint32>::string2value(const CString &value, sint32 &result)
		{			
			uint32 tmp;
			if (sscanf((LPCTSTR) value, "%d", &tmp) == 1)
			{				
				result = tmp;
				return NULL;				
			}
			else
			{
				return "invalid value";
			}	
		}





