/** \file nel_export_node_properties.cpp
 * Node properties dialog
 *
 * $Id: nel_export_node_properties.cpp,v 1.15 2001/12/12 10:35:26 vizerie Exp $
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
#include "nel_export.h"
#include "../nel_mesh_lib/export_lod.h"

using namespace NLMISC;

// ***************************************************************************

#define TAB_COUNT 6

// ***************************************************************************

const std::set<INode*> *listNodeCallBack;

// ***************************************************************************

class addSubLodNodeHitCallBack : public HitByNameDlgCallback 
{
public:
	INodeTab	NodeTab;
private:
	virtual TCHAR *dialogTitle()
	{
		return "Select sub lod objects to add";
	}
	virtual TCHAR *buttonText()
	{
		return "Add";
	}
	virtual BOOL singleSelect()
	{
		return FALSE;
	}
	virtual BOOL useFilter()
	{
		return TRUE;
	}
	virtual int filter(INode *node)
	{
		return (int)(listNodeCallBack->find (node)==listNodeCallBack->end());
	}
	virtual BOOL useProc()
	{
		return TRUE;
	}
	virtual void proc(INodeTab &nodeTab)
	{
		NodeTab=nodeTab;
	}
	virtual BOOL doCustomHilite()
	{
		return FALSE;
	}
	virtual BOOL showHiddenAndFrozen()
	{
		return TRUE;
	}
};

// ***************************************************************************

class CLodDialogBoxParam
{
public:
	CLodDialogBoxParam ()
	{
		for (uint i=0; i<TAB_COUNT; i++)
			SubDlg[i] = NULL;
	}

	bool					ListActived;
	std::list<std::string>	ListLodName;
	int						BlendIn;
	int						BlendOut;
	int						CoarseMesh;
	int						DynamicMesh;
	std::string				DistMax;
	std::string				BlendLength;
	int						MRM;
	int						SkinReduction;
	std::string				NbLod;
	std::string				Divisor;
	std::string				DistanceFinest;
	std::string				DistanceMiddle;
	std::string				DistanceCoarsest;

	const std::set<INode*> *ListNode;

	int						AccelType; // -1->undeterminate   0->Not  1->Portal  2->Cluster
										// 3rd bit -> Father visible
										// 4th bit -> Visible from father
										// 5th bit -> Dynamic Portal
										// 6th bit -> Clusterize
	std::string				InstanceName;	
	int						DontAddToScene;	
	std::string				InstanceGroupName;
	int						DontExport;
	int						ExportNoteTrack;
	int						ExportAnimatedMaterials;

	std::string				LumelSizeMul;
	std::string				SoftShadowRadius;
	std::string				SoftShadowConeLength;


	// misc
	int						FloatingObject;

	// Vegetable
	int						Vegetable;
	int						VegetableAlphaBlend;
	int						VegetableAlphaBlendOnLighted;
	int						VegetableAlphaBlendOffLighted;
	int						VegetableAlphaBlendOffDoubleSided;
	int						VegetableBendCenter;
	std::string				VegetableBendFactor;

	// Dialog
	HWND					SubDlg[TAB_COUNT];
};

int CALLBACK MRMDialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK AccelDialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK InstanceDialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK LightmapDialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK VegetableDialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK MiscDialogCallback (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

const char				*SubText[TAB_COUNT]	= {"LOD & MRM", "Accelerator", "Instance", "Lightmap", "Vegetable", "Misc"};
const int				SubTab[TAB_COUNT]	= {IDD_LOD, IDD_ACCEL, IDD_INSTANCE, IDD_LIGHTMAP, IDD_VEGETABLE, IDD_MISC};
DLGPROC					SubProc[TAB_COUNT]	= {MRMDialogCallback, AccelDialogCallback, InstanceDialogCallback, LightmapDialogCallback, VegetableDialogCallback, MiscDialogCallback};

// ***************************************************************************

void MRMStateChanged (HWND hwndDlg)
{
	bool enable = ( SendMessage (GetDlgItem (hwndDlg, IDC_ACTIVE_MRM), BM_GETCHECK, 0, 0)!=BST_UNCHECKED ) &&
		( SendMessage (GetDlgItem (hwndDlg, IDC_COARSE_MESH), BM_GETCHECK, 0, 0)==BST_UNCHECKED );
	EnableWindow (GetDlgItem (hwndDlg, IDC_SKIN_REDUCTION_MIN), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_SKIN_REDUCTION_MAX), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_SKIN_REDUCTION_BEST), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_NB_LOD), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_DIVISOR), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_DIST_FINEST), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_DIST_MIDDLE), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_DIST_COARSEST), enable);
}

// ***************************************************************************

void CoarseStateChanged (HWND hwndDlg)
{
	// Like if MRM button was clicked
	MRMStateChanged (hwndDlg);

	// Bouton enabled ?
	bool enable = SendMessage (GetDlgItem (hwndDlg, IDC_COARSE_MESH), BM_GETCHECK, 0, 0)==BST_UNCHECKED;
	EnableWindow (GetDlgItem (hwndDlg, IDC_ACTIVE_MRM), enable);
}

// ***************************************************************************

void VegetableStateChanged (HWND hwndDlg)
{
	// Vegetable ?
	bool enable = ( SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE), BM_GETCHECK, 0, 0)!=BST_UNCHECKED );
	
	// Enable alpha blend button
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_ALPHA_BLEND_ON), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_ALPHA_BLEND_OFF), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_CENTER_Z), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_CENTER_NULL), enable);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_BEND_FACTOR), enable);
	
	// Alpha blend ?
	bool alphaBlend = IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_ALPHA_BLEND_ON)!=BST_UNCHECKED;

	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_ON_LIGHTED_PRECOMPUTED), enable && alphaBlend);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_ON_UNLIGHTED), enable && alphaBlend);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_OFF_LIGHTED_PRECOMPUTED), enable && !alphaBlend);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_OFF_LIGHTED_DYNAMIC), enable && !alphaBlend);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_OFF_UNLIGHTED), enable && !alphaBlend);
	EnableWindow (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_OFF_DOUBLE_SIDED), enable && !alphaBlend);
}

// ***************************************************************************

int CALLBACK AccelDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG res = SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

			if (currentParam->SkinReduction!=-1)
				CheckRadioButton (hwndDlg, IDC_SKIN_REDUCTION_MIN, IDC_SKIN_REDUCTION_BEST, IDC_SKIN_REDUCTION_MIN+currentParam->SkinReduction);

			if (currentParam->AccelType != -1)
			{
				CheckRadioButton (hwndDlg, IDC_RADIOACCELNO, IDC_RADIOACCELCLUSTER, IDC_RADIOACCELNO+(currentParam->AccelType&3));
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELNO), true);
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELPORTAL), true);
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELCLUSTER), true);
				if ((currentParam->AccelType&3) == 2) // Cluster ?
				{
					EnableWindow (GetDlgItem (hwndDlg, IDC_FATHER_VISIBLE), true);
					SendMessage (GetDlgItem (hwndDlg, IDC_FATHER_VISIBLE), BM_SETCHECK, currentParam->AccelType&4, 0);
					EnableWindow (GetDlgItem (hwndDlg, IDC_VISIBLE_FROM_FATHER), true);
					SendMessage (GetDlgItem (hwndDlg, IDC_VISIBLE_FROM_FATHER), BM_SETCHECK, currentParam->AccelType&8, 0);
				}
				else
				{
					EnableWindow (GetDlgItem (hwndDlg, IDC_FATHER_VISIBLE), false);
					EnableWindow (GetDlgItem (hwndDlg, IDC_VISIBLE_FROM_FATHER), false);
				}
				
				if ((currentParam->AccelType&3) == 1) // Portal ?
				{
					EnableWindow (GetDlgItem (hwndDlg, IDC_DYNAMIC_PORTAL), true);
					SendMessage (GetDlgItem (hwndDlg, IDC_DYNAMIC_PORTAL), BM_SETCHECK, currentParam->AccelType&16, 0);
				}
				else
					EnableWindow (GetDlgItem (hwndDlg, IDC_DYNAMIC_PORTAL), false);

				if ((currentParam->AccelType&3) == 0) // Not an accelerator
				{
					EnableWindow (GetDlgItem (hwndDlg, IDC_CLUSTERIZE), true);
					SendMessage (GetDlgItem (hwndDlg, IDC_CLUSTERIZE), BM_SETCHECK, currentParam->AccelType&32, 0);
				}
				else
					EnableWindow (GetDlgItem (hwndDlg, IDC_CLUSTERIZE), false);

			}
			else
			{
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELNO), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELPORTAL), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELCLUSTER), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_FATHER_VISIBLE), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_VISIBLE_FROM_FATHER), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_DYNAMIC_PORTAL), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_CLUSTERIZE), false);
			}
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam)) 
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							if (IsDlgButtonChecked (hwndDlg, IDC_RADIOACCELNO) == BST_CHECKED)
								currentParam->AccelType = 0;
							if (IsDlgButtonChecked (hwndDlg, IDC_RADIOACCELPORTAL) == BST_CHECKED)
								currentParam->AccelType = 1;
							if (IsDlgButtonChecked (hwndDlg, IDC_RADIOACCELCLUSTER) == BST_CHECKED)
								currentParam->AccelType = 2;

							if (IsDlgButtonChecked (hwndDlg, IDC_FATHER_VISIBLE) == BST_CHECKED)
								currentParam->AccelType |= 4;
							if (IsDlgButtonChecked (hwndDlg, IDC_VISIBLE_FROM_FATHER) == BST_CHECKED)
								currentParam->AccelType |= 8;
							if (IsDlgButtonChecked (hwndDlg, IDC_DYNAMIC_PORTAL) == BST_CHECKED)
								currentParam->AccelType |= 16;
							if (IsDlgButtonChecked (hwndDlg, IDC_CLUSTERIZE) == BST_CHECKED)
								currentParam->AccelType |= 32;

							// Quit
							EndDialog(hwndDlg, IDOK);
						}
					break;
					case IDC_RADIOACCELNO:
						EnableWindow (GetDlgItem(hwndDlg, IDC_FATHER_VISIBLE), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_VISIBLE_FROM_FATHER), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_DYNAMIC_PORTAL), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_CLUSTERIZE), true);
						break;
					case IDC_RADIOACCELPORTAL:
						EnableWindow (GetDlgItem(hwndDlg, IDC_FATHER_VISIBLE), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_VISIBLE_FROM_FATHER), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_DYNAMIC_PORTAL), true);
						EnableWindow (GetDlgItem(hwndDlg, IDC_CLUSTERIZE), false);
						break;
					case IDC_RADIOACCELCLUSTER:
						EnableWindow (GetDlgItem(hwndDlg, IDC_FATHER_VISIBLE), true);
						EnableWindow (GetDlgItem(hwndDlg, IDC_VISIBLE_FROM_FATHER), true);
						EnableWindow (GetDlgItem(hwndDlg, IDC_DYNAMIC_PORTAL), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_CLUSTERIZE), false);
						break;
					case IDC_FATHER_VISIBLE:
					case IDC_VISIBLE_FROM_FATHER:
					case IDC_DYNAMIC_PORTAL:
					case IDC_CLUSTERIZE:
						{
							if (SendMessage (hwndButton, BM_GETCHECK, 0, 0)==BST_INDETERMINATE)
								SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						}
						break;
				}
			}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}


int CALLBACK MRMDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG res = SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

			// Window text
			std::string winName=(*(currentParam->ListNode->begin()))->GetName();
			winName="Node properties ("+winName+((currentParam->ListNode->size()>1)?" ...)":")");
			SetWindowText (hwndDlg, winName.c_str());

			// Set default state
			SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_IN), BM_SETCHECK, currentParam->BlendIn, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_OUT), BM_SETCHECK, currentParam->BlendOut, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_COARSE_MESH), BM_SETCHECK, currentParam->CoarseMesh, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_DYNAMIC_MESH), BM_SETCHECK, currentParam->DynamicMesh, 0);


			EnableWindow (GetDlgItem (hwndDlg, IDC_LIST1), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_ADD), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_REMOVE), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_UP), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_DOWN), currentParam->ListActived);
			
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MAX), currentParam->DistMax.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_BLEND_LENGTH), currentParam->BlendLength.c_str());

			SendMessage (GetDlgItem (hwndDlg, IDC_ACTIVE_MRM), BM_SETCHECK, currentParam->MRM, 0);
			CoarseStateChanged (hwndDlg);

			if (currentParam->SkinReduction!=-1)
				CheckRadioButton (hwndDlg, IDC_SKIN_REDUCTION_MIN, IDC_SKIN_REDUCTION_BEST, IDC_SKIN_REDUCTION_MIN+currentParam->SkinReduction);

			SetWindowText (GetDlgItem (hwndDlg, IDC_NB_LOD), currentParam->NbLod.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIVISOR), currentParam->Divisor.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_FINEST), currentParam->DistanceFinest.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MIDDLE), currentParam->DistanceMiddle.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_COARSEST), currentParam->DistanceCoarsest.c_str());

			// Iterate list
			HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);
			std::list<std::string>::iterator ite=currentParam->ListLodName.begin();
			while (ite!=currentParam->ListLodName.end())
			{
				// Insert string
				SendMessage (hwndList, LB_ADDSTRING, 0, (LPARAM) ite->c_str());
				ite++;
			}
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam)) 
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							// Set default state
							currentParam->BlendIn=SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_IN), BM_GETCHECK, 0, 0);
							currentParam->BlendOut=SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_OUT), BM_GETCHECK, 0, 0);
							currentParam->CoarseMesh=SendMessage (GetDlgItem (hwndDlg, IDC_COARSE_MESH), BM_GETCHECK, 0, 0);
							currentParam->DynamicMesh=SendMessage (GetDlgItem (hwndDlg, IDC_DYNAMIC_MESH), BM_GETCHECK, 0, 0);
							
							char tmp[512];
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MAX), tmp, 512);
							currentParam->DistMax=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_BLEND_LENGTH), tmp, 512);
							currentParam->BlendLength=tmp;

							currentParam->MRM=SendMessage (GetDlgItem (hwndDlg, IDC_ACTIVE_MRM), BM_GETCHECK, 0, 0);

							currentParam->SkinReduction=-1;
							if (IsDlgButtonChecked (hwndDlg, IDC_SKIN_REDUCTION_MIN)==BST_CHECKED)
								currentParam->SkinReduction=0;
							if (IsDlgButtonChecked (hwndDlg, IDC_SKIN_REDUCTION_MAX)==BST_CHECKED)
								currentParam->SkinReduction=1;
							if (IsDlgButtonChecked (hwndDlg, IDC_SKIN_REDUCTION_BEST)==BST_CHECKED)
								currentParam->SkinReduction=2;

							GetWindowText (GetDlgItem (hwndDlg, IDC_NB_LOD), tmp, 512);
							currentParam->NbLod=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIVISOR), tmp, 512);
							currentParam->Divisor=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_FINEST), tmp, 512);
							currentParam->DistanceFinest=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MIDDLE), tmp, 512);
							currentParam->DistanceMiddle=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_COARSEST), tmp, 512);
							currentParam->DistanceCoarsest=tmp;

							// Iterate list
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);
							currentParam->ListLodName.clear ();

							// Insert item in the list
							uint itemCount=SendMessage (hwndList, LB_GETCOUNT, 0, 0);
							for (uint item=0; item<itemCount; item++)
							{
								// Get the string
								SendMessage (hwndList, LB_GETTEXT, item, (LPARAM) tmp);

								// Push it back
								currentParam->ListLodName.push_back (tmp);
							}
						}
					break;
					case IDC_ADD:
						{
							// Callback for the select node dialog
							addSubLodNodeHitCallBack callBack;
							listNodeCallBack=currentParam->ListNode;
							if (theCNelExport.ip->DoHitByNameDialog(&callBack))
							{
								// Add the selected object in the list
								HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);
								for (uint i=0; i<(uint)callBack.NodeTab.Count(); i++)
									SendMessage (hwndList, LB_ADDSTRING, 0, (LPARAM) callBack.NodeTab[i]->GetName());
							}
						}
						break;
					case IDC_REMOVE:
						{
							// List handle
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);

							// Delete selected string
							int sel=SendMessage (hwndList, LB_GETCURSEL, 0, 0);
							if (sel!=LB_ERR)
							{
								SendMessage (hwndList, LB_DELETESTRING, sel, 0);

								// New selection
								if (sel==SendMessage (hwndList, LB_GETCOUNT, 0, 0))
									sel--;
								if (sel>=0)
									SendMessage (hwndList, LB_SETCURSEL, sel, 0);
							}
						}
					break;
					case IDC_UP:
						{
							// List handle
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);

							// Delete selected string
							int sel=SendMessage (hwndList, LB_GETCURSEL, 0, 0);
							if ((sel!=LB_ERR)&&(sel>0))
							{
								// Get the text
								char text[512];
								SendMessage (hwndList, LB_GETTEXT, sel, (LPARAM) (LPCTSTR) text);

								// Move up the item
								SendMessage (hwndList, LB_INSERTSTRING, sel-1, (LPARAM) text);
								SendMessage (hwndList, LB_DELETESTRING, sel+1, 0);

								// New selection
								SendMessage (hwndList, LB_SETCURSEL, sel-1, 0);
							}
						}
					break;
					case IDC_DOWN:
						{
							// List handle
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);

							// Delete selected string
							int sel=SendMessage (hwndList, LB_GETCURSEL, 0, 0);
							if ( (sel!=LB_ERR) && (sel<SendMessage (hwndList, LB_GETCOUNT, 0, 0)-1 ) )
							{
								// Get the text
								char text[512];
								SendMessage (hwndList, LB_GETTEXT, sel, (LPARAM) (LPCTSTR) text);

								// Move down the item
								SendMessage (hwndList, LB_INSERTSTRING, sel+2, (LPARAM) text);
								SendMessage (hwndList, LB_DELETESTRING, sel, 0);

								// New selection
								SendMessage (hwndList, LB_SETCURSEL, sel+1, 0);
							}
						}
					break;
					// 3 states management
					case IDC_COARSE_MESH:
						{
							if (SendMessage (hwndButton, BM_GETCHECK, 0, 0)==BST_INDETERMINATE)
								SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
							CoarseStateChanged (hwndDlg);
						}
						break;
					case IDC_ACTIVE_MRM:
						{
							if (SendMessage (hwndButton, BM_GETCHECK, 0, 0)==BST_INDETERMINATE)
								SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
							MRMStateChanged (hwndDlg);
						}
						break;
					case IDC_BLEND_IN:
					case IDC_BLEND_OUT:
					case IDC_DYNAMIC_MESH:
						{
							if (SendMessage (hwndButton, BM_GETCHECK, 0, 0)==BST_INDETERMINATE)
								SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						}
						break;
				}
			}
			else if (HIWORD(wParam)==LBN_DBLCLK)
			{
				// List item double clicked
				uint wID = SendMessage (GetDlgItem (hwndDlg, IDC_LIST1), LB_GETCURSEL, 0, 0);
				if (wID!=LB_ERR)
				{
					// Get the node name
					char name[512];
					SendMessage (GetDlgItem (hwndDlg, IDC_LIST1), LB_GETTEXT, wID, (LPARAM) (LPCTSTR) name);

					// Find the node
					INode *nodeDblClk=theCNelExport.ip->GetINodeByName(name);
					if (nodeDblClk)
					{
						// Build a set
						std::set<INode*> listNode;
						listNode.insert (nodeDblClk);

						// Call editor for this node
						theCNelExport.OnNodeProperties (listNode);
					}
				}
			}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}





int CALLBACK InstanceDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG res = SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_NAME), currentParam->InstanceName.c_str());
			SendMessage (GetDlgItem (hwndDlg, IDC_DONT_ADD_TO_SCENE), BM_SETCHECK, currentParam->DontAddToScene, 0);

			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_GROUP_NAME), currentParam->InstanceGroupName.c_str());

			EnableWindow (GetDlgItem (hwndDlg, IDC_DONT_EXPORT), true);
			SendMessage (GetDlgItem (hwndDlg, IDC_DONT_EXPORT), BM_SETCHECK, currentParam->DontExport, 0);
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam)) 
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							char tmp[512];
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_NAME), tmp, 512);
							currentParam->InstanceName=tmp;
							currentParam->DontAddToScene=SendMessage (GetDlgItem (hwndDlg, IDC_DONT_ADD_TO_SCENE), BM_GETCHECK, 0, 0);
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_GROUP_NAME), tmp, 512);
							currentParam->InstanceGroupName=tmp;
							currentParam->DontExport=SendMessage (GetDlgItem (hwndDlg, IDC_DONT_EXPORT), BM_GETCHECK, 0, 0);
						}
					break;
					case IDC_DONT_ADD_TO_SCENE:
					case IDC_DONT_EXPORT:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						break;
				}
			}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}





int CALLBACK LightmapDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG res = SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_LUMELSIZEMUL), currentParam->LumelSizeMul.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_RADIUS), currentParam->SoftShadowRadius.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_CONELENGTH), currentParam->SoftShadowConeLength.c_str());
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam)) 
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							// Set default state
							char tmp[512];
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_LUMELSIZEMUL), tmp, 512);
							currentParam->LumelSizeMul = tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_RADIUS), tmp, 512);
							currentParam->SoftShadowRadius = tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_CONELENGTH), tmp, 512);
							currentParam->SoftShadowConeLength = tmp;
						}
					break;
					case IDC_EDIT_LUMELSIZEMUL:
					case IDC_EDIT_SOFTSHADOW_RADIUS:
					case IDC_EDIT_SOFTSHADOW_CONELENGTH:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						break;
				}
			}
			else if (HIWORD(wParam)==LBN_DBLCLK)
			{
				// List item double clicked
				uint wID = SendMessage (GetDlgItem (hwndDlg, IDC_LIST1), LB_GETCURSEL, 0, 0);
				if (wID!=LB_ERR)
				{
					// Get the node name
					char name[512];
					SendMessage (GetDlgItem (hwndDlg, IDC_LIST1), LB_GETTEXT, wID, (LPARAM) (LPCTSTR) name);

					// Find the node
					INode *nodeDblClk=theCNelExport.ip->GetINodeByName(name);
					if (nodeDblClk)
					{
						// Build a set
						std::set<INode*> listNode;
						listNode.insert (nodeDblClk);

						// Call editor for this node
						theCNelExport.OnNodeProperties (listNode);
					}
				}
			}
		break;

		case WM_CLOSE:
			EndDialog(hwndDlg,1);
		break;

		case WM_DESTROY:						
		break;
	
		default:
		return FALSE;
	}
	return TRUE;
}






int CALLBACK VegetableDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG res = SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

			SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE), BM_SETCHECK, currentParam->Vegetable, 0);
			
			CheckRadioButton(hwndDlg, IDC_VEGETABLE_ALPHA_BLEND_ON, IDC_VEGETABLE_ALPHA_BLEND_OFF, IDC_VEGETABLE_ALPHA_BLEND_ON+currentParam->VegetableAlphaBlend);
			
			CheckRadioButton(hwndDlg, IDC_VEGETABLE_AB_ON_LIGHTED_PRECOMPUTED, IDC_VEGETABLE_AB_ON_UNLIGHTED, IDC_VEGETABLE_AB_ON_LIGHTED_PRECOMPUTED+currentParam->VegetableAlphaBlendOnLighted);
			
			CheckRadioButton(hwndDlg, IDC_VEGETABLE_AB_OFF_LIGHTED_PRECOMPUTED, IDC_VEGETABLE_AB_OFF_UNLIGHTED, IDC_VEGETABLE_AB_OFF_LIGHTED_PRECOMPUTED+currentParam->VegetableAlphaBlendOffLighted);
			
			SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_OFF_DOUBLE_SIDED), BM_SETCHECK, currentParam->VegetableAlphaBlendOffDoubleSided, 0);
			
			CheckRadioButton(hwndDlg, IDC_CENTER_NULL, IDC_CENTER_Z, IDC_CENTER_NULL+currentParam->VegetableBendCenter);

			SetWindowText (GetDlgItem (hwndDlg, IDC_VEGETABLE_BEND_FACTOR), currentParam->VegetableBendFactor.c_str());

			VegetableStateChanged (hwndDlg);
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam)) 
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							currentParam->Vegetable=SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE), BM_GETCHECK, 0, 0);

							if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_ALPHA_BLEND_ON) == BST_CHECKED)
								currentParam->VegetableAlphaBlend = 0;
							else if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_ALPHA_BLEND_OFF) == BST_CHECKED)
								currentParam->VegetableAlphaBlend = 1;
							else 
								currentParam->VegetableAlphaBlend = -1;
							
							if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_AB_ON_LIGHTED_PRECOMPUTED) == BST_CHECKED)
								currentParam->VegetableAlphaBlendOnLighted = 0;
							else if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_AB_ON_UNLIGHTED) == BST_CHECKED)
								currentParam->VegetableAlphaBlendOnLighted = 1;
							else 
								currentParam->VegetableAlphaBlendOnLighted = -1;
							
							if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_AB_OFF_LIGHTED_PRECOMPUTED) == BST_CHECKED)
								currentParam->VegetableAlphaBlendOffLighted = 0;
							else if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_AB_OFF_LIGHTED_DYNAMIC) == BST_CHECKED)
								currentParam->VegetableAlphaBlendOffLighted = 1;
							else if (IsDlgButtonChecked (hwndDlg, IDC_VEGETABLE_AB_OFF_UNLIGHTED) == BST_CHECKED)
								currentParam->VegetableAlphaBlendOffLighted = 2;
							else 
								currentParam->VegetableAlphaBlendOffLighted = -1;
							
							currentParam->VegetableAlphaBlendOffDoubleSided = SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE_AB_OFF_DOUBLE_SIDED), BM_GETCHECK, 0, 0);

							if (IsDlgButtonChecked (hwndDlg, IDC_CENTER_NULL) == BST_CHECKED)
								currentParam->VegetableBendCenter = 0;
							else if (IsDlgButtonChecked (hwndDlg, IDC_CENTER_Z) == BST_CHECKED)
								currentParam->VegetableBendCenter = 1;
							else 
								currentParam->VegetableBendCenter = -1;
							
							char tmp[512];
							GetWindowText (GetDlgItem (hwndDlg, IDC_VEGETABLE_BEND_FACTOR), tmp, 512);
							currentParam->VegetableBendFactor = tmp;
						}
					break;
					case IDC_VEGETABLE:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
					case IDC_VEGETABLE_ALPHA_BLEND_ON:
					case IDC_VEGETABLE_ALPHA_BLEND_OFF:
						VegetableStateChanged (hwndDlg);
						break;
					case IDC_VEGETABLE_AB_OFF_DOUBLE_SIDED:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						break;
				}
			}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}




int CALLBACK MiscDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG res = SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_NOTE_TRACK), BM_SETCHECK, currentParam->ExportNoteTrack, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_FLOATING_OBJECT), BM_SETCHECK, currentParam->FloatingObject, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_ANIMATED_MATERIALS), BM_SETCHECK, currentParam->ExportAnimatedMaterials, 0);
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam)) 
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							currentParam->ExportNoteTrack=SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_NOTE_TRACK), BM_GETCHECK, 0, 0);
							currentParam->FloatingObject=SendMessage (GetDlgItem (hwndDlg, IDC_FLOATING_OBJECT), BM_GETCHECK, 0, 0);
							currentParam->ExportAnimatedMaterials = SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_ANIMATED_MATERIALS), BM_GETCHECK, 0, 0);
						}
					break;
					case IDC_EXPORT_NOTE_TRACK:
					case IDC_FLOATING_OBJECT:
					case IDC_EXPORT_ANIMATED_MATERIALS:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						break;
				}
			}
		break;

		default:
		return FALSE;
	}
	return TRUE;
}






int CALLBACK LodDialogCallback (
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG res = SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

			// Window text
			std::string winName=(*(currentParam->ListNode->begin()))->GetName();
			winName="Node properties ("+winName+((currentParam->ListNode->size()>1)?" ...)":")");
			SetWindowText (hwndDlg, winName.c_str());

			// Move dialog
			RECT windowRect, desktopRect;
			GetWindowRect (hwndDlg, &windowRect);
			HWND desktop=GetDesktopWindow ();
			GetClientRect (desktop, &desktopRect);
			SetWindowPos (hwndDlg, NULL, (desktopRect.right-desktopRect.left-(windowRect.right-windowRect.left))/2,
				(desktopRect.bottom-desktopRect.top-(windowRect.bottom-windowRect.top))/2, 0, 0, SWP_NOOWNERZORDER|SWP_NOREPOSITION|SWP_NOSIZE|SWP_NOZORDER);

			// List of windows to create

			// Get the tab client rect in screen
			RECT tabRect;
			GetClientRect (GetDlgItem (hwndDlg, IDC_TAB), &tabRect);
			tabRect.top += 30;
			tabRect.left += 5;
			tabRect.right -= 5;
			tabRect.bottom -= 5;
			ClientToScreen (GetDlgItem (hwndDlg, IDC_TAB), (POINT*)&tabRect.left);
			ClientToScreen (GetDlgItem (hwndDlg, IDC_TAB), (POINT*)&tabRect.right);

			// For each tab
			for (uint tab=0; tab<TAB_COUNT; tab++)
			{
				// Insert a tab
				TCITEM tabItem;
				tabItem.mask = TCIF_TEXT;
				tabItem.pszText = (char*)SubText[tab];
				SendMessage (GetDlgItem (hwndDlg, IDC_TAB), TCM_INSERTITEM, SendMessage (GetDlgItem (hwndDlg, IDC_TAB), TCM_GETITEMCOUNT, 0, 0), (LPARAM)&tabItem);

				// Create the dialog
				currentParam->SubDlg[tab] = CreateDialogParam (hInstance, MAKEINTRESOURCE(SubTab[tab]), GetDlgItem (hwndDlg, IDC_TAB), SubProc[tab], (LONG)lParam);

				// To client coord
				RECT client = tabRect;
				ScreenToClient (currentParam->SubDlg[tab], (POINT*)&client.left);
				ScreenToClient (currentParam->SubDlg[tab], (POINT*)&client.right);

				// Resize and pos it
				SetWindowPos (currentParam->SubDlg[tab], NULL, client.left, client.top, client.right-client.left, client.bottom-client.top, SWP_NOOWNERZORDER|SWP_NOZORDER);
			}

			// Show the first dialog
			ShowWindow (currentParam->SubDlg[0], SW_SHOW);
		}
		break;

		case WM_NOTIFY:
			{
				LPNMHDR pnmh = (LPNMHDR) lParam;
				switch (pnmh->code)
				{
				case TCN_SELCHANGE:
					{
						
						uint curSel=SendMessage (pnmh->hwndFrom, TCM_GETCURSEL, 0, 0);
						for (uint tab=0; tab<TAB_COUNT; tab++)
						{
							ShowWindow (currentParam->SubDlg[tab], (tab == curSel)?SW_SHOW:SW_HIDE);
						}
						break;
					}
				}
				break;
			}
		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam)) 
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							// Ok in each other window
							for (uint tab=0; tab<TAB_COUNT; tab++)
							{
								// Send back an ok message
								SendMessage (currentParam->SubDlg[tab], uMsg, wParam, lParam);
							}
							// Quit
							EndDialog(hwndDlg, IDOK);
						}
					break;
				}
			}
			break;
		case WM_CLOSE:
			EndDialog(hwndDlg,1);
		break;

		case WM_DESTROY:						
		break;
	
		default:
		return FALSE;
	}
	return TRUE;
/*	CLodDialogBoxParam *currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			// Param pointers
			LONG res = SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)lParam);
			currentParam=(CLodDialogBoxParam *)GetWindowLong(hwndDlg, GWL_USERDATA);

			// Window text
			std::string winName=(*(currentParam->ListNode->begin()))->GetName();
			winName="Node properties ("+winName+((currentParam->ListNode->size()>1)?" ...)":")");
			SetWindowText (hwndDlg, winName.c_str());

			// Set default state
			SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_IN), BM_SETCHECK, currentParam->BlendIn, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_OUT), BM_SETCHECK, currentParam->BlendOut, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_COARSE_MESH), BM_SETCHECK, currentParam->CoarseMesh, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_DYNAMIC_MESH), BM_SETCHECK, currentParam->DynamicMesh, 0);


			EnableWindow (GetDlgItem (hwndDlg, IDC_LIST1), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_ADD), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_REMOVE), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_UP), currentParam->ListActived);
			EnableWindow (GetDlgItem (hwndDlg, IDC_DOWN), currentParam->ListActived);
			
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MAX), currentParam->DistMax.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_BLEND_LENGTH), currentParam->BlendLength.c_str());

			SendMessage (GetDlgItem (hwndDlg, IDC_ACTIVE_MRM), BM_SETCHECK, currentParam->MRM, 0);
			CoarseStateChanged (hwndDlg);

			if (currentParam->SkinReduction!=-1)
				CheckRadioButton (hwndDlg, IDC_SKIN_REDUCTION_MIN, IDC_SKIN_REDUCTION_BEST, IDC_SKIN_REDUCTION_MIN+currentParam->SkinReduction);

			SetWindowText (GetDlgItem (hwndDlg, IDC_NB_LOD), currentParam->NbLod.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIVISOR), currentParam->Divisor.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_FINEST), currentParam->DistanceFinest.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MIDDLE), currentParam->DistanceMiddle.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_DIST_COARSEST), currentParam->DistanceCoarsest.c_str());

			// Iterate list
			HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);
			std::list<std::string>::iterator ite=currentParam->ListLodName.begin();
			while (ite!=currentParam->ListLodName.end())
			{
				// Insert string
				SendMessage (hwndList, LB_ADDSTRING, 0, (LPARAM) ite->c_str());
				ite++;
			}

			if (currentParam->AccelType != -1)
			{
				CheckRadioButton (hwndDlg, IDC_RADIOACCELNO, IDC_RADIOACCELCLUSTER, IDC_RADIOACCELNO+(currentParam->AccelType&3));
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELNO), true);
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELPORTAL), true);
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELCLUSTER), true);
				if ((currentParam->AccelType&3) == 2) // Cluster ?
				{
					EnableWindow (GetDlgItem (hwndDlg, IDC_FATHER_VISIBLE), true);
					SendMessage (GetDlgItem (hwndDlg, IDC_FATHER_VISIBLE), BM_SETCHECK, currentParam->AccelType&4, 0);
					EnableWindow (GetDlgItem (hwndDlg, IDC_VISIBLE_FROM_FATHER), true);
					SendMessage (GetDlgItem (hwndDlg, IDC_VISIBLE_FROM_FATHER), BM_SETCHECK, currentParam->AccelType&8, 0);
				}
				else
				{
					EnableWindow (GetDlgItem (hwndDlg, IDC_FATHER_VISIBLE), false);
					EnableWindow (GetDlgItem (hwndDlg, IDC_VISIBLE_FROM_FATHER), false);
				}
				
				if ((currentParam->AccelType&3) == 1) // Portal ?
				{
					EnableWindow (GetDlgItem (hwndDlg, IDC_DYNAMIC_PORTAL), true);
					SendMessage (GetDlgItem (hwndDlg, IDC_DYNAMIC_PORTAL), BM_SETCHECK, currentParam->AccelType&16, 0);
				}
				else
					EnableWindow (GetDlgItem (hwndDlg, IDC_DYNAMIC_PORTAL), false);

				if ((currentParam->AccelType&3) == 0) // Not an accelerator
				{
					EnableWindow (GetDlgItem (hwndDlg, IDC_CLUSTERIZE), true);
					SendMessage (GetDlgItem (hwndDlg, IDC_CLUSTERIZE), BM_SETCHECK, currentParam->AccelType&32, 0);
				}
				else
					EnableWindow (GetDlgItem (hwndDlg, IDC_CLUSTERIZE), false);

			}
			else
			{
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELNO), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELPORTAL), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_RADIOACCELCLUSTER), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_FATHER_VISIBLE), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_VISIBLE_FROM_FATHER), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_DYNAMIC_PORTAL), false);
				EnableWindow (GetDlgItem (hwndDlg, IDC_CLUSTERIZE), false);
			}

			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_NAME), currentParam->InstanceName.c_str());
			SendMessage (GetDlgItem (hwndDlg, IDC_DONT_ADD_TO_SCENE), BM_SETCHECK, currentParam->DontAddToScene, 0);

			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_GROUP_NAME), currentParam->InstanceGroupName.c_str());

			EnableWindow (GetDlgItem (hwndDlg, IDC_DONT_EXPORT), true);
			SendMessage (GetDlgItem (hwndDlg, IDC_DONT_EXPORT), BM_SETCHECK, currentParam->DontExport, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_NOTE_TRACK), BM_SETCHECK, currentParam->ExportNoteTrack, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_FLOATING_OBJECT), BM_SETCHECK, currentParam->FloatingObject, 0);
			SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE), BM_SETCHECK, currentParam->Vegetable, 0);

			if (currentParam->DontExport == BST_UNCHECKED)
				EnableWindow (GetDlgItem (hwndDlg, IDC_EXPORT_NOTE_TRACK), true);
			else
				EnableWindow (GetDlgItem (hwndDlg, IDC_EXPORT_NOTE_TRACK), false);

			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_LUMELSIZEMUL), currentParam->LumelSizeMul.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_RADIUS), currentParam->SoftShadowRadius.c_str());
			SetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_CONELENGTH), currentParam->SoftShadowConeLength.c_str());
			
			// Move dialog
			RECT windowRect, desktopRect;
			GetWindowRect (hwndDlg, &windowRect);
			HWND desktop=GetDesktopWindow ();
			GetClientRect (desktop, &desktopRect);
			SetWindowPos (hwndDlg, NULL, (desktopRect.right-desktopRect.left-(windowRect.right-windowRect.left))/2,
				(desktopRect.bottom-desktopRect.top-(windowRect.bottom-windowRect.top))/2, 0, 0, SWP_NOOWNERZORDER|SWP_NOREPOSITION|SWP_NOSIZE|SWP_NOZORDER);
		}
		break;

		case WM_COMMAND:
			if( HIWORD(wParam) == BN_CLICKED )
			{
				HWND hwndButton = (HWND) lParam;
				switch (LOWORD(wParam)) 
				{
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
					break;
					case IDOK:
						{
							// Set default state
							currentParam->BlendIn=SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_IN), BM_GETCHECK, 0, 0);
							currentParam->BlendOut=SendMessage (GetDlgItem (hwndDlg, IDC_BLEND_OUT), BM_GETCHECK, 0, 0);
							currentParam->CoarseMesh=SendMessage (GetDlgItem (hwndDlg, IDC_COARSE_MESH), BM_GETCHECK, 0, 0);
							currentParam->DynamicMesh=SendMessage (GetDlgItem (hwndDlg, IDC_DYNAMIC_MESH), BM_GETCHECK, 0, 0);
							
							char tmp[512];
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MAX), tmp, 512);
							currentParam->DistMax=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_BLEND_LENGTH), tmp, 512);
							currentParam->BlendLength=tmp;

							currentParam->MRM=SendMessage (GetDlgItem (hwndDlg, IDC_ACTIVE_MRM), BM_GETCHECK, 0, 0);

							currentParam->SkinReduction=-1;
							if (IsDlgButtonChecked (hwndDlg, IDC_SKIN_REDUCTION_MIN)==BST_CHECKED)
								currentParam->SkinReduction=0;
							if (IsDlgButtonChecked (hwndDlg, IDC_SKIN_REDUCTION_MAX)==BST_CHECKED)
								currentParam->SkinReduction=1;
							if (IsDlgButtonChecked (hwndDlg, IDC_SKIN_REDUCTION_BEST)==BST_CHECKED)
								currentParam->SkinReduction=2;

							GetWindowText (GetDlgItem (hwndDlg, IDC_NB_LOD), tmp, 512);
							currentParam->NbLod=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIVISOR), tmp, 512);
							currentParam->Divisor=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_FINEST), tmp, 512);
							currentParam->DistanceFinest=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_MIDDLE), tmp, 512);
							currentParam->DistanceMiddle=tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_DIST_COARSEST), tmp, 512);
							currentParam->DistanceCoarsest=tmp;

							// Iterate list
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);
							currentParam->ListLodName.clear ();

							// Insert item in the list
							uint itemCount=SendMessage (hwndList, LB_GETCOUNT, 0, 0);
							for (uint item=0; item<itemCount; item++)
							{
								// Get the string
								SendMessage (hwndList, LB_GETTEXT, item, (LPARAM) tmp);

								// Push it back
								currentParam->ListLodName.push_back (tmp);
							}

							if (IsDlgButtonChecked (hwndDlg, IDC_RADIOACCELNO) == BST_CHECKED)
								currentParam->AccelType = 0;
							if (IsDlgButtonChecked (hwndDlg, IDC_RADIOACCELPORTAL) == BST_CHECKED)
								currentParam->AccelType = 1;
							if (IsDlgButtonChecked (hwndDlg, IDC_RADIOACCELCLUSTER) == BST_CHECKED)
								currentParam->AccelType = 2;

							if (IsDlgButtonChecked (hwndDlg, IDC_FATHER_VISIBLE) == BST_CHECKED)
								currentParam->AccelType |= 4;
							if (IsDlgButtonChecked (hwndDlg, IDC_VISIBLE_FROM_FATHER) == BST_CHECKED)
								currentParam->AccelType |= 8;
							if (IsDlgButtonChecked (hwndDlg, IDC_DYNAMIC_PORTAL) == BST_CHECKED)
								currentParam->AccelType |= 16;
							if (IsDlgButtonChecked (hwndDlg, IDC_CLUSTERIZE) == BST_CHECKED)
								currentParam->AccelType |= 32;

							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_NAME), tmp, 512);
							currentParam->InstanceName=tmp;
							currentParam->DontAddToScene=SendMessage (GetDlgItem (hwndDlg, IDC_DONT_ADD_TO_SCENE), BM_GETCHECK, 0, 0);
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_INSTANCE_GROUP_NAME), tmp, 512);
							currentParam->InstanceGroupName=tmp;
							currentParam->DontExport=SendMessage (GetDlgItem (hwndDlg, IDC_DONT_EXPORT), BM_GETCHECK, 0, 0);
							currentParam->ExportNoteTrack=SendMessage (GetDlgItem (hwndDlg, IDC_EXPORT_NOTE_TRACK), BM_GETCHECK, 0, 0);
							currentParam->FloatingObject=SendMessage (GetDlgItem (hwndDlg, IDC_FLOATING_OBJECT), BM_GETCHECK, 0, 0);
							currentParam->Vegetable=SendMessage (GetDlgItem (hwndDlg, IDC_VEGETABLE), BM_GETCHECK, 0, 0);

							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_LUMELSIZEMUL), tmp, 512);
							currentParam->LumelSizeMul = tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_RADIUS), tmp, 512);
							currentParam->SoftShadowRadius = tmp;
							GetWindowText (GetDlgItem (hwndDlg, IDC_EDIT_SOFTSHADOW_CONELENGTH), tmp, 512);
							currentParam->SoftShadowConeLength = tmp;

							// Quit
							EndDialog(hwndDlg, IDOK);
						}
					break;
					case IDC_ADD:
						{
							// Callback for the select node dialog
							addSubLodNodeHitCallBack callBack;
							listNodeCallBack=currentParam->ListNode;
							if (theCNelExport.ip->DoHitByNameDialog(&callBack))
							{
								// Add the selected object in the list
								HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);
								for (uint i=0; i<(uint)callBack.NodeTab.Count(); i++)
									SendMessage (hwndList, LB_ADDSTRING, 0, (LPARAM) callBack.NodeTab[i]->GetName());
							}
						}
						break;
					case IDC_REMOVE:
						{
							// List handle
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);

							// Delete selected string
							int sel=SendMessage (hwndList, LB_GETCURSEL, 0, 0);
							if (sel!=LB_ERR)
							{
								SendMessage (hwndList, LB_DELETESTRING, sel, 0);

								// New selection
								if (sel==SendMessage (hwndList, LB_GETCOUNT, 0, 0))
									sel--;
								if (sel>=0)
									SendMessage (hwndList, LB_SETCURSEL, sel, 0);
							}
						}
					break;
					case IDC_UP:
						{
							// List handle
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);

							// Delete selected string
							int sel=SendMessage (hwndList, LB_GETCURSEL, 0, 0);
							if ((sel!=LB_ERR)&&(sel>0))
							{
								// Get the text
								char text[512];
								SendMessage (hwndList, LB_GETTEXT, sel, (LPARAM) (LPCTSTR) text);

								// Move up the item
								SendMessage (hwndList, LB_INSERTSTRING, sel-1, (LPARAM) text);
								SendMessage (hwndList, LB_DELETESTRING, sel+1, 0);

								// New selection
								SendMessage (hwndList, LB_SETCURSEL, sel-1, 0);
							}
						}
					break;
					case IDC_DOWN:
						{
							// List handle
							HWND hwndList=GetDlgItem (hwndDlg, IDC_LIST1);

							// Delete selected string
							int sel=SendMessage (hwndList, LB_GETCURSEL, 0, 0);
							if ( (sel!=LB_ERR) && (sel<SendMessage (hwndList, LB_GETCOUNT, 0, 0)-1 ) )
							{
								// Get the text
								char text[512];
								SendMessage (hwndList, LB_GETTEXT, sel, (LPARAM) (LPCTSTR) text);

								// Move down the item
								SendMessage (hwndList, LB_INSERTSTRING, sel+2, (LPARAM) text);
								SendMessage (hwndList, LB_DELETESTRING, sel, 0);

								// New selection
								SendMessage (hwndList, LB_SETCURSEL, sel+1, 0);
							}
						}
					break;
					// 3 states management
					case IDC_COARSE_MESH:
						{
							if (SendMessage (hwndButton, BM_GETCHECK, 0, 0)==BST_INDETERMINATE)
								SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
							CoarseStateChanged (hwndDlg);
						}
						break;
					case IDC_ACTIVE_MRM:
						{
							if (SendMessage (hwndButton, BM_GETCHECK, 0, 0)==BST_INDETERMINATE)
								SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
							MRMStateChanged (hwndDlg);
						}
						break;
					case IDC_BLEND_IN:
					case IDC_BLEND_OUT:
					case IDC_DYNAMIC_MESH:
						{
							if (SendMessage (hwndButton, BM_GETCHECK, 0, 0)==BST_INDETERMINATE)
								SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						}
						break;
					case IDC_RADIOACCELNO:
						EnableWindow (GetDlgItem(hwndDlg, IDC_FATHER_VISIBLE), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_VISIBLE_FROM_FATHER), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_DYNAMIC_PORTAL), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_CLUSTERIZE), true);
						break;
					case IDC_RADIOACCELPORTAL:
						EnableWindow (GetDlgItem(hwndDlg, IDC_FATHER_VISIBLE), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_VISIBLE_FROM_FATHER), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_DYNAMIC_PORTAL), true);
						EnableWindow (GetDlgItem(hwndDlg, IDC_CLUSTERIZE), false);
						break;
					case IDC_RADIOACCELCLUSTER:
						EnableWindow (GetDlgItem(hwndDlg, IDC_FATHER_VISIBLE), true);
						EnableWindow (GetDlgItem(hwndDlg, IDC_VISIBLE_FROM_FATHER), true);
						EnableWindow (GetDlgItem(hwndDlg, IDC_DYNAMIC_PORTAL), false);
						EnableWindow (GetDlgItem(hwndDlg, IDC_CLUSTERIZE), false);
						break;
					case IDC_DONT_ADD_TO_SCENE:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						break;
					case IDC_DONT_EXPORT:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);

						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_UNCHECKED)
							EnableWindow (GetDlgItem(hwndDlg, IDC_EXPORT_NOTE_TRACK), true);
						else
							EnableWindow (GetDlgItem(hwndDlg, IDC_EXPORT_NOTE_TRACK), false);
						break;
					case IDC_EXPORT_NOTE_TRACK:
						if (SendMessage (hwndButton, BM_GETCHECK, 0, 0) == BST_INDETERMINATE)
							SendMessage (hwndButton, BM_SETCHECK, BST_UNCHECKED, 0);
						break;
				}
			}
			else if (HIWORD(wParam)==LBN_DBLCLK)
			{
				// List item double clicked
				uint wID = SendMessage (GetDlgItem (hwndDlg, IDC_LIST1), LB_GETCURSEL, 0, 0);
				if (wID!=LB_ERR)
				{
					// Get the node name
					char name[512];
					SendMessage (GetDlgItem (hwndDlg, IDC_LIST1), LB_GETTEXT, wID, (LPARAM) (LPCTSTR) name);

					// Find the node
					INode *nodeDblClk=theCNelExport.ip->GetINodeByName(name);
					if (nodeDblClk)
					{
						// Build a set
						std::set<INode*> listNode;
						listNode.insert (nodeDblClk);

						// Call editor for this node
						theCNelExport.OnNodeProperties (listNode);
					}
				}
			}
		break;

		case WM_CLOSE:
			EndDialog(hwndDlg,1);
		break;

		case WM_DESTROY:						
		break;
	
		default:
		return FALSE;
	}
	return TRUE;*/
}

// ***************************************************************************

void CNelExport::OnNodeProperties (const std::set<INode*> &listNode)
{
	// Get 
	uint nNumSelNode=listNode.size();

	if (nNumSelNode)
	{
		// Get the selected node
		INode* node=*listNode.begin();

		// Dialog box param
		CLodDialogBoxParam param;

		// Value of the properties
		param.ListNode=&listNode;
		param.ListActived=true;
		param.BlendIn=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_IN, BST_UNCHECKED);
		param.BlendOut=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_OUT, BST_UNCHECKED);
		param.CoarseMesh=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_COARSE_MESH, BST_UNCHECKED);
		param.DynamicMesh=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DYNAMIC_MESH, BST_UNCHECKED);
		float floatTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DIST_MAX, NEL3D_APPDATA_LOD_DIST_MAX_DEFAULT);
		param.DistMax=toString (floatTmp);
		floatTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_LENGTH, NEL3D_APPDATA_LOD_BLEND_LENGTH_DEFAULT);
		param.BlendLength=toString (floatTmp);
		param.MRM=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_MRM, BST_UNCHECKED);
		param.SkinReduction=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_SKIN_REDUCTION, NEL3D_APPDATA_LOD_SKIN_REDUCTION_DEFAULT);

		int intTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_NB_LOD, NEL3D_APPDATA_LOD_NB_LOD_DEFAULT);
		param.NbLod=toString (intTmp);
		intTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DIVISOR, NEL3D_APPDATA_LOD_DIVISOR_DEFAULT);
		param.Divisor=toString(intTmp);
		floatTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_FINEST, NEL3D_APPDATA_LOD_DISTANCE_FINEST_DEFAULT);
		param.DistanceFinest=toString(floatTmp);
		floatTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE_DEFAULT);
		param.DistanceMiddle=toString(floatTmp);
		floatTmp=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_COARSEST, NEL3D_APPDATA_LOD_DISTANCE_COARSEST_DEFAULT);
		param.DistanceCoarsest=toString(floatTmp);

		// Lod names
		int nameCount=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_NAME_COUNT, 0);
		int name;
		for (name=0; name<std::min (nameCount, NEL3D_APPDATA_LOD_NAME_COUNT_MAX); name++)
		{
			// Get a string
			std::string nameLod=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_NAME+name, "");
			if (nameLod!="")
			{
				param.ListLodName.push_back (nameLod);
			}
		}
		param.AccelType = CExportNel::getScriptAppData (node, NEL3D_APPDATA_ACCEL, 32);

		param.InstanceName=CExportNel::getScriptAppData (node, NEL3D_APPDATA_INSTANCE_NAME, "");
		param.DontAddToScene=CExportNel::getScriptAppData (node, NEL3D_APPDATA_DONT_ADD_TO_SCENE, BST_UNCHECKED);
		param.InstanceGroupName=CExportNel::getScriptAppData (node, NEL3D_APPDATA_IGNAME, "");
		param.DontExport=CExportNel::getScriptAppData (node, NEL3D_APPDATA_DONTEXPORT, BST_UNCHECKED);
		param.ExportNoteTrack=CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_NOTE_TRACK, BST_UNCHECKED);
		param.ExportAnimatedMaterials=CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS, BST_UNCHECKED);
		param.FloatingObject=CExportNel::getScriptAppData (node, NEL3D_APPDATA_FLOATING_OBJECT, BST_UNCHECKED);
		param.LumelSizeMul=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LUMELSIZEMUL, "1.0");
		param.SoftShadowRadius=CExportNel::getScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_RADIUS, toString(NEL3D_APPDATA_SOFTSHADOW_RADIUS_DEFAULT));
		param.SoftShadowConeLength=CExportNel::getScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_CONELENGTH, toString(NEL3D_APPDATA_SOFTSHADOW_CONELENGTH_DEFAULT));

		// Vegetable
		param.Vegetable = CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE, BST_UNCHECKED);
		param.VegetableAlphaBlend = CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND, 0);
		param.VegetableAlphaBlendOnLighted = CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_ON_LIGHTED, BST_UNCHECKED);
		param.VegetableAlphaBlendOffLighted = CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_LIGHTED, BST_UNCHECKED);
		param.VegetableAlphaBlendOffDoubleSided = CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_DOUBLE_SIDED, BST_UNCHECKED);
		param.VegetableBendCenter = CExportNel::getScriptAppData (node, NEL3D_APPDATA_BEND_CENTER, 0);
		param.VegetableBendFactor = toString (CExportNel::getScriptAppData (node, NEL3D_APPDATA_BEND_FACTOR, NEL3D_APPDATA_BEND_FACTOR_DEFAULT));

		// Something selected ?
		std::set<INode*>::const_iterator ite=listNode.begin();
		ite++;
		while (ite!=listNode.end())
		{
			// Get the selected node
			node=*ite;

			// Get the properties
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_IN, BST_UNCHECKED)!=param.BlendIn)
				param.BlendIn=BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_OUT, BST_UNCHECKED)!=param.BlendOut)
				param.BlendOut=BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_COARSE_MESH, BST_UNCHECKED)!=param.CoarseMesh)
				param.CoarseMesh=BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DYNAMIC_MESH, BST_UNCHECKED)!=param.DynamicMesh)
				param.DynamicMesh=BST_INDETERMINATE;
			if (param.DistMax!=toString (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DIST_MAX, NEL3D_APPDATA_LOD_DIST_MAX_DEFAULT)))
				param.DistMax="";
			if (param.BlendLength!=toString (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_LENGTH, NEL3D_APPDATA_LOD_BLEND_LENGTH_DEFAULT)))
				param.BlendLength="";

			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_MRM, BST_UNCHECKED)!=param.MRM)
				param.MRM=BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_SKIN_REDUCTION, NEL3D_APPDATA_LOD_SKIN_REDUCTION_DEFAULT)!=param.SkinReduction)
				param.SkinReduction=-1;
			if (toString(CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_NB_LOD, NEL3D_APPDATA_LOD_NB_LOD_DEFAULT))!=param.NbLod)
				param.NbLod="";
			if (toString(CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DIVISOR, NEL3D_APPDATA_LOD_DIVISOR_DEFAULT))!=param.Divisor)
				param.Divisor="";
			if (toString(CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_FINEST, NEL3D_APPDATA_LOD_DISTANCE_FINEST_DEFAULT))!=param.DistanceFinest)
				param.DistanceFinest="";
			if (toString(CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE_DEFAULT))!=param.DistanceMiddle)
				param.DistanceMiddle="";
			if (toString(CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_COARSEST, NEL3D_APPDATA_LOD_DISTANCE_COARSEST_DEFAULT))!=param.DistanceCoarsest)
				param.DistanceCoarsest="";

			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_ACCEL, 32)!=param.AccelType)
				param.AccelType = -1;

			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_DONT_ADD_TO_SCENE, BST_UNCHECKED)!=param.DontAddToScene)
				param.DontAddToScene = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_INSTANCE_NAME, "")!=param.InstanceName)
				param.InstanceName = "";
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_IGNAME, "")!=param.InstanceGroupName)
				param.InstanceName = "";
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_DONTEXPORT, BST_UNCHECKED)!=param.DontExport)
				param.DontExport= BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_NOTE_TRACK, BST_UNCHECKED)!=param.ExportNoteTrack)
				param.ExportNoteTrack = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS, BST_UNCHECKED)!=param.ExportAnimatedMaterials)
				param.ExportAnimatedMaterials = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_FLOATING_OBJECT, BST_UNCHECKED)!=param.FloatingObject)
				param.FloatingObject = BST_INDETERMINATE;

			
			// Vegetable
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE, BST_UNCHECKED)!=param.Vegetable)
				param.Vegetable = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND, 0)!=param.VegetableAlphaBlend)
				param.VegetableAlphaBlend = -1;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_ON_LIGHTED, 0)!=param.VegetableAlphaBlendOnLighted)
				param.VegetableAlphaBlendOnLighted = -1;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_LIGHTED, 0)!=param.VegetableAlphaBlendOffLighted)
				param.VegetableAlphaBlendOffLighted = -1;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_DOUBLE_SIDED, BST_UNCHECKED)!=param.VegetableAlphaBlendOffDoubleSided)
				param.VegetableAlphaBlendOffDoubleSided = BST_INDETERMINATE;
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_BEND_CENTER, BST_UNCHECKED)!=param.VegetableBendCenter)
				param.VegetableBendCenter = -1;
			if (toString(CExportNel::getScriptAppData (node, NEL3D_APPDATA_BEND_FACTOR, NEL3D_APPDATA_BEND_FACTOR_DEFAULT))!=param.VegetableBendFactor)
				param.VegetableBendFactor = "";

			// Lightmap
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_LUMELSIZEMUL, "1.0")!=param.LumelSizeMul)
				param.LumelSizeMul = "";
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_RADIUS, toString(NEL3D_APPDATA_SOFTSHADOW_RADIUS_DEFAULT))!=param.SoftShadowRadius)
				param.SoftShadowRadius = "";
			if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_CONELENGTH, toString(NEL3D_APPDATA_SOFTSHADOW_CONELENGTH_DEFAULT))!=param.SoftShadowConeLength)
				param.SoftShadowConeLength = "";

			// Get name count for this node
			std::list<std::string> tmplist;
			nameCount=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_NAME_COUNT, 0);
			for (name=0; name<std::min (nameCount, NEL3D_APPDATA_LOD_NAME_COUNT_MAX); name++)
			{
				// Get a string
				std::string nameLod=CExportNel::getScriptAppData (node, NEL3D_APPDATA_LOD_NAME+name, "");
				if (nameLod!="")
				{
					tmplist.push_back (nameLod);
				}
			}
			// Compare with original list
			if (tmplist!=param.ListLodName)
			{
				// Not the same, so clear
				param.ListLodName.clear();
				param.ListActived=false;
			}

			// Next sel
			ite++;
		}

		if (DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_NODE_PROPERTIES), ip->GetMAXHWnd(), LodDialogCallback, (long)&param)==IDOK)
		{
			// Next node
			ite=listNode.begin();
			while (ite!=listNode.end())
			{
				// Get the selected node
				node=*ite;

				// Ok pushed, copy returned params
				if (param.BlendIn!=BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_IN, param.BlendIn);
				if (param.BlendOut!=BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_OUT, param.BlendOut);
				if (param.CoarseMesh!=BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_COARSE_MESH, param.CoarseMesh);
				if (param.DynamicMesh!=BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_DYNAMIC_MESH, param.DynamicMesh);

				if (param.DistMax!="")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_DIST_MAX, param.DistMax);
				if (param.BlendLength!="")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_BLEND_LENGTH, param.BlendLength);

				if (param.MRM!=BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_MRM, param.MRM);
				if (param.SkinReduction!=-1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_SKIN_REDUCTION, param.SkinReduction);
				if (param.NbLod!="")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_NB_LOD, param.NbLod);
				if (param.Divisor!="")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_DIVISOR, param.Divisor);
				if (param.DistanceFinest!="")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_FINEST, param.DistanceFinest);
				if (param.DistanceMiddle!="")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_MIDDLE, param.DistanceMiddle);
				if (param.DistanceCoarsest!="")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_DISTANCE_COARSEST, param.DistanceCoarsest);

				if (param.AccelType != -1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_ACCEL, param.AccelType);

				if (param.InstanceName != "")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_INSTANCE_NAME, param.InstanceName);
				if (param.DontAddToScene != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_DONT_ADD_TO_SCENE, param.DontAddToScene);
				if (param.InstanceGroupName != "")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_IGNAME, param.InstanceGroupName);
				if (param.DontExport != BST_INDETERMINATE)
				{
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_DONTEXPORT, param.DontExport);
					if (param.ExportNoteTrack != BST_INDETERMINATE)
						CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_NOTE_TRACK, param.ExportNoteTrack);
					if (param.ExportAnimatedMaterials != BST_INDETERMINATE)
						CExportNel::setScriptAppData (node, NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS, param.ExportAnimatedMaterials);
				}
				if (param.LumelSizeMul != "")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LUMELSIZEMUL, param.LumelSizeMul);
				if (param.SoftShadowRadius != "")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_RADIUS, param.SoftShadowRadius);
				if (param.SoftShadowConeLength != "")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_SOFTSHADOW_CONELENGTH, param.SoftShadowConeLength);
				if (param.FloatingObject != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_FLOATING_OBJECT, param.FloatingObject);

				// Vegetable
				if (param.Vegetable != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_VEGETABLE, param.Vegetable);
				if (param.VegetableAlphaBlend != -1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND, param.VegetableAlphaBlend);
				if (param.VegetableAlphaBlendOnLighted != -1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_ON_LIGHTED, param.VegetableAlphaBlendOnLighted);
				if (param.VegetableAlphaBlendOffLighted != -1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_LIGHTED, param.VegetableAlphaBlendOffLighted);
				if (param.VegetableAlphaBlendOffDoubleSided != BST_INDETERMINATE)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_VEGETABLE_ALPHA_BLEND_OFF_DOUBLE_SIDED, param.VegetableAlphaBlendOffDoubleSided);
				if (param.VegetableBendCenter != -1)
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_BEND_CENTER, param.VegetableBendCenter);
				if (param.VegetableBendFactor != "")
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_BEND_FACTOR, param.VegetableBendFactor);

				if (param.ListActived)
				{
					// Write size of the list
					uint sizeList=std::min (param.ListLodName.size(), (uint)NEL3D_APPDATA_LOD_NAME_COUNT_MAX);
					CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_NAME_COUNT, (int)sizeList);
				
					// Write the strings
					uint stringIndex=0;
					std::list<std::string>::iterator ite=param.ListLodName.begin();
					while (ite!=param.ListLodName.end())
					{
						// Write the string
						CExportNel::setScriptAppData (node, NEL3D_APPDATA_LOD_NAME+stringIndex, *ite);
						stringIndex++;
						ite++;
					}
				}

				// Next node
				ite++;
			}
		}
	}
}

// ***************************************************************************
