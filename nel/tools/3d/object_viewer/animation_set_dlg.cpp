/** \file animation_set_dlg.cpp
 * implementation file
 *
 * $Id: animation_set_dlg.cpp,v 1.8 2001/10/16 14:57:07 corvazier Exp $
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

#include "std_afx.h"
#include "object_viewer.h"
#include "animation_set_dlg.h"

#include <nel/misc/file.h>
#include <3d/track_keyframer.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NLMISC;
using namespace NL3D;

/////////////////////////////////////////////////////////////////////////////
// CAnimationSetDlg dialog


CAnimationSetDlg::CAnimationSetDlg(CObjectViewer* objView, CWnd* pParent /*=NULL*/)
	: CDialog(CAnimationSetDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnimationSetDlg)
	UseMixer = 0;
	//}}AFX_DATA_INIT

	_ObjView=objView;
}


void CAnimationSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnimationSetDlg)
	DDX_Control(pDX, IDC_PLAYLIST, PlayList);
	DDX_Control(pDX, IDC_TREE2, SkelTree);
	DDX_Control(pDX, IDC_TREE, Tree);
	DDX_Radio(pDX, IDC_USE_LIST, UseMixer);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAnimationSetDlg, CDialog)
	//{{AFX_MSG_MAP(CAnimationSetDlg)
	ON_BN_CLICKED(IDC_ADD_ANIMATION, OnAddAnimation)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_BN_CLICKED(IDC_ADD_SKEL_WT, OnAddSkelWt)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_LIST_INSERT, OnListInsert)
	ON_BN_CLICKED(IDC_LIST_UP, OnListUp)
	ON_BN_CLICKED(IDC_LIST_DOWN, OnListDown)
	ON_BN_CLICKED(IDC_LIST_DELETE, OnListDelete)
	ON_BN_CLICKED(IDC_SET_ANIM_LENGTH, OnSetAnimLength)
	ON_BN_CLICKED(IDC_USE_LIST, OnUseList)
	ON_BN_CLICKED(IDC_USE_MIXER, OnUseMixer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimationSetDlg message handlers

// ***************************************************************************

void CAnimationSetDlg::OnAddAnimation () 
{
	// TODO: Add your control notification handler code here
	static char BASED_CODE szFilter[] = "NeL Animation Files (*.anim)|*.anim|All Files (*.*)|*.*||";
	CFileDialog fileDlg( TRUE, ".anim", "*.anim", OFN_ALLOWMULTISELECT|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter);
	if (fileDlg.DoModal()==IDOK)
	{
		// Open the file
		try
		{
			// Get first file
			POSITION pos=fileDlg.GetStartPosition( );
			while (pos)
			{
				// Get the name
				CString filename=fileDlg.GetNextPathName(pos);

				// Load the animation
				loadAnimation (filename);

				// Touch the channel mixer
				_ObjView->reinitChannels ();
			}
		}
		catch (Exception& e)
		{
			MessageBox (e.what(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
		}
	}
}

// ***************************************************************************

void CAnimationSetDlg::OnAddSkelWt() 
{
	// TODO: Add your control notification handler code here
	static char BASED_CODE szFilter[] = "NeL Skeleton Weight Template Files (*.swt)|*.swt|All Files (*.*)|*.*||";
	CFileDialog fileDlg( TRUE, ".swt", "*.swt", OFN_ALLOWMULTISELECT|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter);
	if (fileDlg.DoModal()==IDOK)
	{
		// Open the file
		try
		{
			// Get first file
			POSITION pos=fileDlg.GetStartPosition( );
			while (pos)
			{
				// Get the name
				CString filename=fileDlg.GetNextPathName(pos);

				// Load the animation
				loadSkeleton (filename);

				// Touch the channel mixer
				_ObjView->reinitChannels  ();
			}
		}
		catch (Exception& e)
		{
			MessageBox (e.what(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
		}
	}
}

// ***************************************************************************

void CAnimationSetDlg::OnReset () 
{
	// Reset the channel mixer slots
	_ObjView->resetSlots ();

	// Clear the filename array
	_ListAnimation.clear();
	_ListSkeleton.clear();

	// Clear the list
	PlayList.ResetContent ();

	// Clear the TREE
	Tree.DeleteAllItems ();

	// Clear the TREE
	SkelTree.DeleteAllItems ();
}

// ***************************************************************************

void CAnimationSetDlg::loadAnimation (const char* fileName)
{
	// Open the file
	CIFile file;
	if (file.open (fileName))
	{
		// Get the animation name
		char name[256];
		_splitpath (fileName, NULL, NULL, name, NULL);

		// Make an animation
		CAnimation *anim=new CAnimation;

		// Serial it
		anim->serial (file);

		// Add the animation
		addAnimation (anim, name);

		// Add the filename in the list
		_ListAnimation.push_back (fileName);
	}
	else
	{
		// Create a message
		char msg[512];
		_snprintf (msg, 512, "Can't open the file %s for reading.", fileName);
		MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
	}

}

// ***************************************************************************

void CAnimationSetDlg::addAnimation (NL3D::CAnimation* anim, const char* name)
{
	// Add an animation
	uint id = _ObjView->_AnimationSet.addAnimation (name, anim);

	// Rebuild the animationSet
	_ObjView->_AnimationSet.build ();

	// Insert an intem
	HTREEITEM item=Tree.InsertItem (name);
	Tree.SetItemData (item, id);
	nlassert (item!=NULL);

	// For all tracks in the animation
	std::set<std::string> setString;
	anim->getTrackNames (setString);
	std::set<std::string>::iterator ite=setString.begin();
	while (ite!=setString.end())
	{
		// Add this string
		HTREEITEM newItem = Tree.InsertItem (ite->c_str(), item);
		Tree.SetItemData (newItem, 0xffffffff);

		// Get the track
		ITrack *track=anim->getTrack (anim->getIdTrackByName (*ite));

		// Keyframer ?
		UTrackKeyframer *keyTrack=dynamic_cast<UTrackKeyframer *>(track);
		if (keyTrack)
		{
			// Get number of keys
			std::vector<CAnimationTime> keys;
			keyTrack->getKeysInRange (track->getBeginTime ()-1, track->getEndTime ()+1, keys);

			// Print track info
			char name[512];
			_snprintf (name, 512, "%s (%f - %f) %d keys", typeid(*track).name(), track->getBeginTime (), track->getEndTime (), keys.size());
			HTREEITEM keyItem = Tree.InsertItem (name, newItem);
			Tree.SetItemData (keyItem, 0xffffffff);
		}
		else
		{
			// Print track info
			char name[512];
			_snprintf (name, 512, "%s (%f - %f)", typeid(*track).name(), track->getBeginTime (), track->getEndTime ());
			HTREEITEM keyItem = Tree.InsertItem (name, newItem);
			Tree.SetItemData (keyItem, 0xffffffff);
		}

		ite++;
	}
}

// ***************************************************************************

void CAnimationSetDlg::loadSkeleton (const char* fileName)
{
	// Open the file
	CIFile file;
	if (file.open (fileName))
	{
		// Get the animation name
		char name[256];
		_splitpath (fileName, NULL, NULL, name, NULL);

		// Get the skeleton pointer
		CSkeletonWeight* skel=new CSkeletonWeight;

		// Serial it
		skel->serial (file);

		// Add an animation
		_ObjView->_AnimationSet.addSkeletonWeight (name, skel);

		// Add the filename in the list
		_ListSkeleton.push_back (fileName);

		// Insert an intem
		HTREEITEM item=SkelTree.InsertItem (name);
		nlassert (item!=NULL);

		// Get number of node in this skeleton weight
		uint numNode=skel->getNumNode ();

		// Add the nodein the tree
		for (uint n=0; n<numNode; n++)
		{
			char percent[512];
			sprintf (percent, "%s (%f%%)", skel->getNodeName (n).c_str(), skel->getNodeWeight(n)*100);
			// Add this string
			SkelTree.InsertItem (percent, item);
		}
	}
	else
	{
		// Create a message
		char msg[512];
		_snprintf (msg, 512, "Can't open the file %s for reading.", fileName);
		MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
	}

}

void CAnimationSetDlg::OnDestroy() 
{
	setRegisterWindowState (this, REGKEY_OBJ_VIEW_ANIMATION_SET_DLG);

	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here	
}

void CAnimationSetDlg::OnListInsert() 
{
	// Update
	UpdateData ();

	// Get selected animation
	HTREEITEM item = Tree.GetSelectedItem ();
	if (item && (Tree.GetItemData (item)!=0xffffffff))
	{
		// Insert the string
		int itemList = PlayList.InsertString (-1, Tree.GetItemText(item));
		PlayList.SetItemData (itemList, Tree.GetItemData (item));

		// Reselect the item
		Tree.SelectItem (item);
	}
	
	// Update back
	UpdateData (FALSE);
}

void CAnimationSetDlg::OnListUp() 
{
	// Update
	UpdateData ();

	// Get selected item
	int sel = PlayList.GetCurSel ();
	if ((sel != LB_ERR) && (sel>0))
	{
		// Backup the string
		CString text;
		PlayList.GetText (sel, text);
		DWORD data = PlayList.GetItemData (sel);

		// Remove the node
		PlayList.DeleteString (sel);

		// Insert the string
		int pos = PlayList.InsertString (sel-1, text);
		PlayList.SetItemData (pos, data);
		PlayList.SetCurSel (pos);
	}
	
	// Update back
	UpdateData (FALSE);
}

void CAnimationSetDlg::OnListDown() 
{
	// Update
	UpdateData ();

	// Get selected item
	int sel = PlayList.GetCurSel ();
	if ((sel != LB_ERR) && (sel<PlayList.GetCount()-1))
	{
		// Backup the string
		CString text;
		PlayList.GetText (sel, text);
		DWORD data = PlayList.GetItemData (sel);

		// Remove the node
		PlayList.DeleteString (sel);

		// Insert the string
		int pos = PlayList.InsertString (sel+1, text);
		PlayList.SetItemData (pos, data);
		PlayList.SetCurSel (pos);
	}
	
	// Update back
	UpdateData (FALSE);
}

void CAnimationSetDlg::OnListDelete() 
{
	// Update
	UpdateData ();

	// Get selected item
	int sel = PlayList.GetCurSel ();
	if (sel != LB_ERR)
	{
		// Remove the node
		PlayList.DeleteString (sel);

		if (sel>=PlayList.GetCount ())
			sel--;
		if (sel>=0)
			PlayList.SetCurSel (sel);
	}
	
	// Update back
	UpdateData (FALSE);
}

void CAnimationSetDlg::OnSetAnimLength() 
{
	// There is some anim ?
	if (PlayList.GetCount())
	{
		// Calculate the length
		float length = 0;

		// For each animation in the list
		for (uint i=0; i<(uint)PlayList.GetCount(); i++)
		{
			// Get the animation
			CAnimation *anim = anim=_ObjView->_AnimationSet.getAnimation (PlayList.GetItemData (i));

			// Add the length
			length += anim->getEndTime () - anim->getBeginTime ();
		}

		// Adjuste length
		_ObjView->setAnimTime (0, length * _ObjView->getFrameRate ());
	}
}

void CAnimationSetDlg::OnUseList() 
{
	// Disable channels
	_ObjView->enableChannels ();
}

void CAnimationSetDlg::OnUseMixer() 
{
	// Enable channels
	_ObjView->enableChannels ();
}
