/** \file editable_range.h
 * a dialog that help to choose a numeric value of any types. 
 *
 * $Id: editable_range.h,v 1.11 2002/08/08 11:00:45 lecroart Exp $
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


#if !defined(AFX_EDITABLE_RANGE_H__0B1EFF2B_FA0E_4AC8_88B8_416605043BF9__INCLUDED_)
#define AFX_EDITABLE_RANGE_H__0B1EFF2B_FA0E_4AC8_88B8_416605043BF9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// editable_range.h : header file
//

#include <string>
#include "edit_attrib_dlg.h"
#include "range_manager.h"
#include "range_selector.h"
#include "ps_wrapper.h"
#include "bound_checker.h"


/////////////////////////////////////////////////////////////////////////////
// CEditableRange dialog

class CEditableRange : public CEditAttribDlg
{
public:
	

	/**
	 * construct the dialog by giving it an Id. It will be used to save the user preference
	 * Each dialog of this type must have its own id in the app
	 */
	CEditableRange(const std::string &id);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditableRange)
	enum { IDD = IDD_EDITABLE_RANGE };
	CSliderCtrl	m_SliderCtrl;
	CEdit	m_ValueCtrl;
	CButton	m_UpdateValue;
	CButton	m_SelectRange;
	CString	m_MinRange;
	CString	m_MaxRange;
	CString	m_Value;
	int		m_SliderPos;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditableRange)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


	// init the dialog at the given position
	public:

	virtual void init(uint32 x, uint32 y, CWnd *pParent);

	BOOL EnableWindow( BOOL bEnable = TRUE );


public:
	/// for derivers : this must querries the value to the desired manager and set the dialog values
	virtual void updateRange(void) = 0;
	/** for derivers : value update from a linked object (reader). See examples in subclasses
	 */
	virtual void updateValueFromReader(void) = 0;

	// empty the values and the slider
	void emptyDialog(void);

	
	// validate the lower an upper bound of a range from their string representation
	virtual bool editableRangeValueValidator(const CString &lo, const CString &up) = 0;

	// update the dialog display
	void update();

// Implementation
protected:

	
	/** for derivers : value update : this is call with a float ranging from 0.f to 1.f (from the slider)
	 *  And it must convert it to the desired value, changing the value of this dialog
	 */
	virtual void updateValueFromSlider(double sliderValue) = 0;

	

	/// the text has changed, and the user has pressed update
	virtual void updateValueFromText(void) = 0;	




	// the range tune button was pressed. It muist show a dialog that allows the user to choose the range he wants
	virtual void selectRange(void) = 0;

	

	// the unique id of this dialog
	std::string _Id;


	// Generated message map functions
	//{{AFX_MSG(CEditableRange)
	virtual BOOL OnInitDialog();
	afx_msg void OnReleasedcaptureSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelectRange();
	afx_msg void OnSetfocusValue();
	afx_msg void OnUpdateValue();	
	afx_msg void OnKillfocusValue();
	afx_msg void OnChangeValue();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/** here, we define a template for editable ranges EditableRanges, that help to instanciate it
 *  Derivers have just to specialize 2 static methods : value2CString and string2value
 */

template <typename T>
class CEditableRangeT : public CEditableRange, public CBoundChecker<T>
{
public:
/// ctor, it gives the range for the values
	CEditableRangeT(const std::string &id, T defaultMin, T defaultMax)
		: CEditableRange(id), _Range(defaultMin, defaultMax), _Wrapper(NULL)		
	{		
	}
	
	// set an interface of a wrapper  to read / write values in the particle system
	void setWrapper(IPSWrapper<T> *wrapper) { _Wrapper = wrapper; }



public:	
	// SPECIALIZE THAT. write value into the given CString
	static void value2CString(T value, CString &dest);
	// SPECIALIZE THAT. convert a CString into a value, return NULL if ok, or a pointer to an error message
	static const char *string2value(const CString &value, T &result);
	
	

	void updateRange(void)
	{
		// retrieve our range
		_Range = CRangeManager<T>::GetRange(_Id, _Range.first, _Range.second);
		value2CString(_Range.first, m_MinRange);
		value2CString(_Range.second, m_MaxRange);		
	/*	if (_Wrapper)
		{
			setValue(_Wrapper->get());
		}*/
		UpdateData(FALSE);
	}
	void updateValueFromReader(void)
	{
		if (_Wrapper)
		{
			setValue(_Wrapper->get());		
		}
	}
protected:	
	

	void updateValueFromText(void)
	{
		T value;
		const char *message = string2value(m_Value, value);
		if (!message)
		{
			const char *mess = validateUpperBound(value) 
					  ,*mess2 = validateLowerBound(value);
			if (mess || mess2)
			{
				MessageBox(mess ? mess : mess2, "error");
				return;
			}
			

			_Wrapper->set(value);
			setValue(value);
			return;
		}
		
		MessageBox(message, "error");
		
	}
	void selectRange(void)
	{
		CString lowerBound, upperBound;	
		value2CString(_Range.first, lowerBound);
		value2CString(_Range.second, upperBound);
	
		CRangeSelector rs(lowerBound, upperBound, this);

		if (rs.DoModal() == IDOK)
		{
			string2value(rs.getLowerBound(), _Range.first);
			string2value(rs.getUpperBound(), _Range.second);				
			CRangeManager<T>::SetRange(_Id, _Range.first, _Range.second);
			updateRange();
		}
	}
	void updateValueFromSlider(double sliderValue)
	{
		nlassert(_Wrapper);
		
		T value = _Range.first  + (T) ((_Range.second - _Range.first) * sliderValue);
		value2CString(value, m_Value);

		if (_Wrapper)
		{
			_Wrapper->set(value);
		}

		UpdateData(FALSE);
	}

	/// set a new value that will affect both slider and value display
	void setValue(T value)
	{
		value2CString(value, m_Value);

	//	_Wrapper->set(value);

		if (value < _Range.first)
		{
			m_SliderPos = (uint) (m_SliderCtrl.GetRangeMin());
		}
		else
		if (value > _Range.second)
		{
			m_SliderPos = (uint) (m_SliderCtrl.GetRangeMax());
		}
		else
		{
			if (_Range.second != _Range.first)
			{
				m_SliderPos = (uint) ((double) (value - _Range.first) / (_Range.second - _Range.first) * (m_SliderCtrl.GetRangeMax() - m_SliderCtrl.GetRangeMin())
									+ m_SliderCtrl.GetRangeMin());
			}
			else
			{
				m_SliderPos = m_SliderCtrl.GetRangeMin();
			}
		}	
		UpdateData(FALSE);	
	}


	virtual bool editableRangeValueValidator(const CString &lo, const CString &up)
	{
		T upT, loT;

		const char *message = string2value(lo, loT);
		if (message)
		{
			::MessageBox(NULL, message, "Range selection error", MB_OK);
			return false;
		}
		const char *mess = validateUpperBound(loT)
					  ,*mess2 = validateLowerBound(loT);
		if (mess ||  mess2)
		{
			MessageBox(mess ? mess : mess2, "error");
			return false;
		}

		message = string2value(up, upT);
		if (message)
		{
			::MessageBox(NULL, message, "Range selection error", MB_OK);
			return false;
		}

		mess = validateUpperBound(upT);
		mess2 = validateLowerBound(upT);
		if (mess || mess2)
		{
			MessageBox(mess ? mess : mess2, "error");
			return false;
		}


		if (upT <= loT)
		{
			::MessageBox(NULL, "upper bound must be strictly greater than lower bound", "Range selection error", MB_OK);
			return false;
		}

		return true;
	}
	
	// min max values
	std::pair<T, T> _Range;

	// wrapper to the particle system
	IPSWrapper<T> *_Wrapper;


};


///////////////////////////////////////////////////////
// IMPLEMENTATION OF TEMPLATE METHOD SPECIALIZATIONS //
///////////////////////////////////////////////////////


////////////////////////////////////////////////////////
// float specialization.                              //
////////////////////////////////////////////////////////

CEditableRangeT<float>::CEditableRangeT(const std::string &id, float defaultMin, float defaultMax ) 
	: CEditableRange(id), _Range(defaultMin, defaultMax), _Wrapper(NULL)
{
}

inline void CEditableRangeT<float>::value2CString(float value, CString &dest)
{
	dest.Format("%g", (double) value);
}

inline const char *CEditableRangeT<float>::string2value(const CString &value, float &result)
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

////////////////////////////////////////////////////////
// uint32 specialization.                             //
////////////////////////////////////////////////////////

CEditableRangeT<uint32>::CEditableRangeT(const std::string &id, uint32 defaultMin , uint32 defaultMax )
	: CEditableRange(id), _Range(defaultMin, defaultMax), _Wrapper(NULL)
{
}

inline void CEditableRangeT<uint32>::value2CString(uint32 value, CString &dest)
{
	dest.Format("%d", value);
}

inline const char *CEditableRangeT<uint32>::string2value(const CString &value, uint32 &result)
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


////////////////////////////////////////////////////////
// sint32 specialization.                             //
////////////////////////////////////////////////////////

CEditableRangeT<sint32>::CEditableRangeT(const std::string &id, sint32 defaultMin , sint32 defaultMax )
	: CEditableRange(id), _Range(defaultMin, defaultMax), _Wrapper(NULL)
{
}

inline void CEditableRangeT<sint32>::value2CString(sint32 value, CString &dest)
{
	dest.Format("%d", value);
}

inline const char *CEditableRangeT<sint32>::string2value(const CString &value, sint32 &result)
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


// some typedefs


typedef CEditableRangeT<float> CEditableRangeFloat;
typedef CEditableRangeT<uint32> CEditableRangeUInt;
typedef CEditableRangeT<sint32> CEditableRangeInt;



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.



#endif // !defined(AFX_EDITABLE_RANGE_H__0B1EFF2B_FA0E_4AC8_88B8_416605043BF9__INCLUDED_)
