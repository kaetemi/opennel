/** \file create_file_dlg.cpp
 *
 * $Id: create_file_dlg.cpp,v 1.1 2004/06/17 08:15:05 vizerie Exp $
 */

/* Copyright, 2000-2004 Nevrax Ltd.
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

// create_file_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "create_file_dlg.h"
#include <shlobj.h>


/////////////////////////////////////////////////////////////////////////////
// CCreateFileDlg dialog


CCreateFileDlg::CCreateFileDlg(const CString &title, const std::string &defaultBasePath, const std::string &extension, CWnd* pParent /*=NULL*/)
	: CDialog(CCreateFileDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateFileDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	_Title = title;
	_Extension = extension;
	_DefaultBasePath = defaultBasePath;
}


void CCreateFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateFileDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateFileDlg, CDialog)
	//{{AFX_MSG_MAP(CCreateFileDlg)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateFileDlg message handlers

void CCreateFileDlg::OnBrowse() 
{
	char chosenPath[MAX_PATH];
	// browse folder
	CString title = getStrRsc(IDS_CHOOSE_BASE_PATH);
	BROWSEINFO bi;		
	bi.hwndOwner = m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = chosenPath;
	bi.lpszTitle = (LPCTSTR) title;
	bi.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_EDITBOX;
	bi.lpfn = NULL;
	bi.lParam = NULL;
	bi.iImage = 0;
	LPITEMIDLIST result = SHBrowseForFolder(&bi);
	if (result != NULL && SHGetPathFromIDList(result, chosenPath))
	{
		GetDlgItem(IDC_LOCATION)->SetWindowText((LPCTSTR) chosenPath);
	}
}


//*************************************************************************************************
BOOL CCreateFileDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();	
	SetWindowText((LPCTSTR) _Title);
	GetDlgItem(IDC_LOCATION)->SetWindowText(_DefaultBasePath.c_str());
	if (!_DefaultBasePath.empty())
	{
		GetDlgItem(IDC_FILENAME)->SetFocus();
	}
	else
	{
		GetDlgItem(IDC_LOCATION)->SetFocus();
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//*************************************************************************************************
void CCreateFileDlg::OnOK()
{
	CString filename;
	GetDlgItem(IDC_FILENAME)->GetWindowText(filename);
	_Filename = (LPCTSTR) filename;
	CString location;
	GetDlgItem(IDC_LOCATION)->GetWindowText(location);
	_Path = (LPCTSTR) location;	
	if (_Path.empty())
	{
		localizedMessageBox(*this, IDS_EMPTY_PATH, IDS_ERROR, MB_ICONEXCLAMATION);
		return;
	}	
	if (_Filename.empty())
	{
		localizedMessageBox(*this, IDS_FILENAME_PATH, IDS_ERROR, MB_ICONEXCLAMATION);
		return;
	}
	// check that filename is valid (should be a single file name)
	if (_Filename != NLMISC::CFile::getFilename(_Filename))
	{
		localizedMessageBox(*this, IDS_INVALID_FILENAME, IDS_ERROR, MB_ICONEXCLAMATION);
		return;
	}
	// attempt to create containing folder
	if (!NLMISC::CFile::isExists(_Path))
	{
		bool result = NLMISC::CFile::createDirectory(_Path);
		if (!result)
		{
			MessageBox((LPCTSTR) (getStrRsc(IDS_COULDNT_CREATE_DIRECTORY) + _Path.c_str()), getStrRsc(IDS_ERROR), MB_ICONEXCLAMATION);
			return;
		}
	}	
	std::string oldPath = NLMISC::CPath::getCurrentPath();
	if (!NLMISC::CPath::setCurrentPath(_Path.c_str()))
	{
		MessageBox((LPCTSTR) (getStrRsc(IDS_COULDNT_CREATE_DIRECTORY) + _Path.c_str()), getStrRsc(IDS_ERROR), MB_ICONEXCLAMATION);
		return;
	}
	_FullPath = NLMISC::CPath::getFullPath(_Filename, false);
	NLMISC::CPath::setCurrentPath(oldPath.c_str());
	// append extension if not present
	if (NLMISC::nlstricmp(NLMISC::CFile::getExtension(_Filename), _Extension) != 0)
	{
		_Filename += "." + _Extension;
		_FullPath += "." + _Extension;
	}
	CDialog::OnOK();
}

void CCreateFileDlg::OnCancel()
{
	_Filename = "";
	_Path = "";
	_FullPath = "";
	CDialog::OnCancel();
}

//*************************************************************************************************
bool CCreateFileDlg::touchFile()
{
	std::string path = getPath();
	std::string filename = getFileName();
	std::string fullPath = getFullPath();		
	// check if file already exists
	if (NLMISC::CFile::isExists(fullPath))
	{
		int result = MessageBox((LPCTSTR) (CString(filename.c_str()) + getStrRsc(IDS_OVERWRITE_FILE)), getStrRsc(IDS_WARNING), MB_ICONEXCLAMATION);
		if (result !=	IDOK) return false;
	}
	// create a dummy file		
	NLMISC::COFile testFile;
	if (!testFile.open(fullPath))
	{
		localizedMessageBox(*this, IDS_CANNOT_CREATE_FILE, IDS_ERROR, MB_ICONEXCLAMATION);
		return false;
	}
	testFile.close();
	return true;
}
		
