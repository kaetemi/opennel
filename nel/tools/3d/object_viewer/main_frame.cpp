// main_frame.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "main_frame.h"
#include "set_value_dlg.h"
#include "particle_dlg.h"
#include "about_dialog.h"
#include "choose_lag.h"

#include <nel/misc/file.h>

#include <3d/nelu.h>
#include <3d/mesh.h>
#include <3d/transform_shape.h>
#include <3d/mesh_instance.h>
#include <3d/skeleton_model.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NLMISC;
using namespace NL3D;


void CSceneDlgMouseListener::addToServer (NLMISC::CEventServer& server)
{
	server.addListener (EventMouseDownId, this);
}

void CSceneDlgMouseListener::releaseFromServer (NLMISC::CEventServer& server)
{
	server.removeListener (EventMouseDownId, this);
}

void CSceneDlgMouseListener::operator ()(const CEvent& event)
{	
	if (event == EventMouseDownId)
	{
		CEventMouse* mouseEvent=(CEventMouse*)&event;
		if (mouseEvent->Button == rightButton)
		{
			const CEvent3dMouseListener &ml = ObjViewerDlg->getMouseListener() ;

			CMenu  menu ;
			CMenu* subMenu ;
	
			menu.LoadMenu(IDR_MOVE_ELEMENT) ;

			

			menu.CheckMenuItem(ID_ENABLE_ELEMENT_XROTATE, ml.getModelMatrixRotationAxis() == CEvent3dMouseListener::xAxis 
															? MF_CHECKED : MF_UNCHECKED ) ;
			menu.CheckMenuItem(ID_ENABLE_ELEMENT_YROTATE, ml.getModelMatrixRotationAxis() == CEvent3dMouseListener::yAxis 
															? MF_CHECKED : MF_UNCHECKED ) ;
			menu.CheckMenuItem(ID_ENABLE_ELEMENT_ZROTATE, ml.getModelMatrixRotationAxis() == CEvent3dMouseListener::zAxis 
															? MF_CHECKED : MF_UNCHECKED ) ;


			subMenu = menu.GetSubMenu(0);    
			nlassert(subMenu) ;

			// compute the screen coordinate from the main window

			HWND wnd = (HWND) CNELU::Driver->getDisplay() ;
			nlassert(::IsWindow(wnd)) ;
			RECT r ;
			::GetWindowRect(wnd, &r) ;						

			sint x = r.left + (sint) (mouseEvent->X * (r.right - r.left)) ;
			sint y = r.top + (sint) ((1.f - mouseEvent->Y) * (r.bottom - r.top)) ;
			
			::TrackPopupMenu(subMenu->m_hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, x, y, 0, SceneDlg->m_hWnd, NULL) ;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

//IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

CMainFrame::CMainFrame( CObjectViewer *objView, winProc windowProc )
{
	DriverWindowProc=windowProc;
	ObjView=objView;
	AnimationWindow=false;
	AnimationSetWindow=false;
	MixerSlotsWindow=false;
	ParticlesWindow=false;
	MoveElement=false;
	MoveMode=true;
	X=true;
	Y=true;
	Z=true;
	Euler=false;

	_RightButtonMouseListener.ObjViewerDlg = ObjView ;
	_RightButtonMouseListener.SceneDlg = this ;
	_RightButtonMouseListener.addToServer(CNELU::EventServer) ;
}

CMainFrame::~CMainFrame()
{
	ObjView->_MainFrame=NULL;
	_RightButtonMouseListener.releaseFromServer (CNELU::EventServer);
}


BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_COMMAND(ID_CLEAR, OnClear)
	ON_COMMAND(ID_EDIT_MOVEELEMENT, OnEditMoveelement)
	ON_COMMAND(ID_EDIT_X, OnEditX)
	ON_COMMAND(ID_EDIT_Y, OnEditY)
	ON_COMMAND(ID_EDIT_Z, OnEditZ)
	ON_COMMAND(ID_ENABLE_ELEMENT_XROTATE, OnEnableElementXrotate)
	ON_COMMAND(ID_ENABLE_ELEMENT_YROTATE, OnEnableElementYrotate)
	ON_COMMAND(ID_ENABLE_ELEMENT_ZROTATE, OnEnableElementZrotate)
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_COMMAND(ID_FILE_LOADCONFIG, OnFileLoadconfig)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVECONFIG, OnFileSaveconfig)	
	ON_COMMAND(ID_VIEW_FIRSTPERSONMODE, OnViewFirstpersonmode)
	ON_COMMAND(ID_VIEW_OBJECTMODE, OnViewObjectmode)
	ON_COMMAND(ID_VIEW_RESET_CAMERA, OnResetCamera)
	ON_COMMAND(ID_VIEW_SETBACKGROUND, OnViewSetbackground)
	ON_COMMAND(ID_VIEW_SETMOVESPEED, OnViewSetmovespeed)
	ON_COMMAND(ID_WINDOW_ANIMATION, OnWindowAnimation)
	ON_COMMAND(ID_WINDOW_ANIMATIONSET, OnWindowAnimationset)
	ON_COMMAND(ID_WINDOW_MIXERSSLOTS, OnWindowMixersslots)
	ON_COMMAND(ID_WINDOW_PARTICLES, OnWindowParticles)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_UPDATE_COMMAND_UI(ID_WINDOW_ANIMATION, OnUpdateWindowAnimation)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_ANIMATIONSET, OnUpdateWindowAnimationset)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_MIXERSSLOTS, OnUpdateWindowMixersslots)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_PARTICLES, OnUpdateWindowParticles)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECTMODE, OnUpdateViewObjectmode)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FIRSTPERSONMODE, OnUpdateViewFirstpersonmode)
	ON_UPDATE_COMMAND_UI(ID_EDIT_X, OnUpdateEditX)
	ON_UPDATE_COMMAND_UI(ID_EDIT_Y, OnUpdateEditY)
	ON_UPDATE_COMMAND_UI(ID_EDIT_Z, OnUpdateEditZ)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MOVEELEMENT, OnUpdateEditMoveelement)
	ON_COMMAND(ID_HELP_ABOUTOBJECTVIEWER, OnHelpAboutobjectviewer)
	ON_COMMAND(IDM_SET_LAG, OnSetLag)
	ON_COMMAND(IDM_REMOVE_ALL_INSTANCES_FROM_SCENE, OnRemoveAllInstancesFromScene)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (CNELU::Driver)
		DriverWindowProc (CNELU::Driver, m_hWnd, message, wParam, lParam);
		
	return CFrameWnd::WindowProc(message, wParam, lParam);
}

// ***************************************************************************

void CMainFrame::update ()
{
	ObjView->_AnimationDlg->ShowWindow (AnimationWindow?SW_SHOW:SW_HIDE);
	ObjView->_AnimationSetDlg->ShowWindow (AnimationSetWindow?SW_SHOW:SW_HIDE);
	ObjView->_SlotDlg->ShowWindow (MixerSlotsWindow?SW_SHOW:SW_HIDE);
	ObjView->_ParticleDlg->ShowWindow (ParticlesWindow?SW_SHOW:SW_HIDE);
}

// ***************************************************************************

void CMainFrame::registerValue (bool read)
{
	if (read)
	{
		// Get value from the register
		HKEY hKey;
		if (RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_OBJ_VIEW_SCENE_DLG, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
		{
			DWORD len=sizeof (BOOL);
			DWORD type;
			NLMISC::CRGBA bgCol ;
			RegQueryValueEx (hKey, "ViewAnimation", 0, &type, (LPBYTE)&AnimationWindow, &len);
			len=sizeof (BOOL);
			RegQueryValueEx (hKey, "ViewAnimationSet", 0, &type, (LPBYTE)&AnimationSetWindow, &len);
			len=sizeof (BOOL);
			RegQueryValueEx (hKey, "ViewSlots", 0, &type, (LPBYTE)&MixerSlotsWindow, &len);
			len=sizeof (float);
			RegQueryValueEx (hKey, "ViewParticles", 0, &type, (LPBYTE)&ParticlesWindow, &len);
			len=sizeof (float);
			RegQueryValueEx (hKey, "MoveSpeed", 0, &type, (LPBYTE)&MoveSpeed, &len);
			len=sizeof (BOOL);
			RegQueryValueEx (hKey, "ObjectMode", 0, &type, (LPBYTE)&MoveMode, &len);
			len=sizeof(NLMISC::CRGBA) ;
			RegQueryValueEx (hKey, "BackGroundColor", 0, &type, (LPBYTE)&BgColor, &len);
		}
	}
	else
	{
		HKEY hKey;
		if (RegCreateKey(HKEY_CURRENT_USER, REGKEY_OBJ_VIEW_SCENE_DLG, &hKey)==ERROR_SUCCESS)
		{
			RegSetValueEx(hKey, "ViewAnimation", 0, REG_BINARY, (LPBYTE)&AnimationWindow, sizeof(bool));
			RegSetValueEx(hKey, "ViewAnimationSet", 0, REG_BINARY, (LPBYTE)&AnimationSetWindow, sizeof(bool));
			RegSetValueEx(hKey, "ViewSlots", 0, REG_BINARY, (LPBYTE)&MixerSlotsWindow, sizeof(bool));
			RegSetValueEx(hKey, "ViewParticles", 0, REG_BINARY, (LPBYTE)&ParticlesWindow, sizeof(bool));
			RegSetValueEx(hKey, "MoveSpeed", 0, REG_BINARY, (LPBYTE)&MoveSpeed, sizeof(float));
			RegSetValueEx(hKey, "ObjectMode", 0, REG_BINARY, (LPBYTE)&MoveMode, sizeof(BOOL));
			RegSetValueEx(hKey, "BackGroundColor", 0, REG_BINARY, (LPBYTE)&BgColor, sizeof(NLMISC::CRGBA));
		}
	}
}

// ***************************************************************************

void CMainFrame::OnResetCamera() 
{
	// One object found at least
	bool found=false;
	
	// Pointer on the CMesh;
	CVector hotSpot=CVector (0,0,0);

	// Reset the radius
	float radius=10.f;

	// Look for a first mesh
	uint m;
	for (m=0; m<ObjView->_ListTransformShape.size(); m++)
	{
		CTransformShape *pTransform=dynamic_cast<CTransformShape*>(ObjView->_ListTransformShape[m]);
		if (pTransform)
		{
			IShape *pShape=pTransform->Shape;
			CSkeletonModel *pSkelModel=pTransform->getSkeletonModel ();

			// Get bounding box
			CAABBox boundingBox;
			pShape->getAABBox(boundingBox);
			
			if (!pSkelModel)
			{
				// Reset the hotspot
				hotSpot=pTransform->getMatrix()*boundingBox.getCenter();

				// Reset the radius
				radius=boundingBox.getRadius();
				radius=pTransform->getMatrix().mulVector (CVector (radius, 0, 0)).norm();
				found=true;
				m++;
				break;
			}
			else
			{
				// Get first bone
				if (pSkelModel->Bones.size())
				{
					// Ok, it is the root.
					hotSpot=pSkelModel->Bones[0].getMatrix()*boundingBox.getCenter();

					// Reset the radius
					radius=boundingBox.getRadius();
					radius=pSkelModel->Bones[0].getMatrix().mulVector (CVector (radius, 0, 0)).norm();
					found=true;
					m++;
					break;
				}
			}
		}
	}

	// For each model in the list
	for (; m<ObjView->_ListTransformShape.size(); m++)
	{
		// Pointer on the CTransformShape;
		CTransformShape *pTransform=dynamic_cast<CTransformShape*>(ObjView->_ListTransformShape[m]);
		if (pTransform)
		{
			IShape *pShape=pTransform->Shape;
			CSkeletonModel *pSkelModel=pTransform->getSkeletonModel ();

			// New radius and hotSpot
			CVector hotSpot2;
			float radius2;
			bool setuped=false;

			// Get the bounding box
			CAABBox boundingBox;
			pShape->getAABBox(boundingBox);

			if (!pSkelModel)
			{
				// Get the hotspot
				hotSpot2=pTransform->getMatrix()*boundingBox.getCenter();

				// Get the radius
				radius2=boundingBox.getRadius();
				radius2=pTransform->getMatrix().mulVector (CVector (radius2, 0, 0)).norm();

				// Ok found it
				setuped=true;
			}
			else
			{
				// Get first bone
				if (pSkelModel->Bones.size())
				{
					// Get the hotspot
					hotSpot2=pSkelModel->Bones[0].getMatrix()*boundingBox.getCenter();

					// Get the radius
					radius2=boundingBox.getRadius();
					radius2=pSkelModel->Bones[0].getMatrix().mulVector (CVector (radius2, 0, 0)).norm();

					// Ok found it
					setuped=true;
				}
			}

			if (setuped)
			{
				// *** Merge with previous

				// Get vector center to center
				CVector vect=hotSpot-hotSpot2;
				vect.normalize();
				
				// Get the right position
				CVector right=hotSpot+vect*radius;
				if ((right-hotSpot2).norm()<radius2)
					right=hotSpot2+vect*radius2;
				
				// Get the left position
				CVector left=hotSpot2-vect*radius2;
				if ((left-hotSpot).norm()<radius)
					left=hotSpot-vect*radius;

				// Get new center
				hotSpot=(left+right)/2.f;

				// Get new size
				radius=(left-right).norm()/2.f;
			}
		}
	}

	// Setup scene center
	ObjView->_SceneCenter=hotSpot;

	// Setup camera
	CNELU::Camera->lookAt (hotSpot+CVector(0.57735f,0.57735f,0.57735f)*radius, hotSpot);

	// Setup mouse listener
	ObjView->_MouseListener.setMatrix (CNELU::Camera->getMatrix());
	ObjView->_MouseListener.setFrustrum (CNELU::Camera->getFrustum());
	ObjView->_MouseListener.setViewport (CViewport());
	ObjView->_MouseListener.setHotSpot (hotSpot);
	ObjView->_MouseListener.setMouseMode (CEvent3dMouseListener::edit3d);
}

void CMainFrame::OnClear() 
{
	// *** Clear the scene.

	// Remove all the instance
	uint i;
	for (i=0; i<ObjView->_ListTransformShape.size(); i++)
		CNELU::Scene.deleteInstance (ObjView->_ListTransformShape[i]);

	// Clear the pointer array
	ObjView->_ListTransformShape.clear ();

	// Erase the channel mixer
	ObjView->_ChannelMixer.clear ();

	// Erase the entry of the viewer
	ObjView->_ListMeshes.clear ();
}

void CMainFrame::OnEditMoveelement() 
{
	MoveElement^=true;
	UpdateData() ;
	if (!MoveElement) // switch back to camera mode ?
	{
		ObjView->getMouseListener().enableModelMatrixEdition(false) ;
	}
	else
	{
		ObjView->getMouseListener().enableModelMatrixEdition() ;
		ObjView->getMouseListener().setModelMatrix(ObjView->getParticleDialog()->getElementMatrix()) ;
	}

	/*ctrl->EnableXCtrl.EnableWindow(MoveElement) ;
	EnableYCtrl.EnableWindow(MoveElement) ;
	EnableZCtrl.EnableWindow(MoveElement) ;*/
}

void CMainFrame::OnEditX() 
{
	X^=true;
	ObjView->getMouseListener().enableModelTranslationAxis(CEvent3dMouseListener::xAxis, X!=0);
}

void CMainFrame::OnEditY() 
{
	Y^=true;
	ObjView->getMouseListener().enableModelTranslationAxis(CEvent3dMouseListener::yAxis, Y!=0);
}

void CMainFrame::OnEditZ() 
{
	Z^=true;
	ObjView->getMouseListener().enableModelTranslationAxis(CEvent3dMouseListener::zAxis, Z!=0);
}

void CMainFrame::OnEnableElementXrotate() 
{
	ObjView->getMouseListener().setModelMatrixRotationAxis(CEvent3dMouseListener::xAxis);
}

void CMainFrame::OnEnableElementYrotate() 
{
	ObjView->getMouseListener().setModelMatrixRotationAxis(CEvent3dMouseListener::yAxis);
}

void CMainFrame::OnEnableElementZrotate() 
{
	ObjView->getMouseListener().setModelMatrixRotationAxis(CEvent3dMouseListener::zAxis);
}

void CMainFrame::OnFileExit() 
{
	DestroyWindow ();
}

void CMainFrame::OnFileLoadconfig() 
{
	// Update UI
	update ();

	// Create a dialog
	static char BASED_CODE szFilter[] = "NeL Object viewer config (*.ovcgf)|*.ovcgf|All Files (*.*)|*.*||";
	CFileDialog fileDlg( TRUE, ".ovcgf", "*.ovcgf", OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter);
	if (fileDlg.DoModal()==IDOK)
	{
		// Open the file
		CIFile file;
		if (file.open ((const char*)fileDlg.GetPathName()))
		{
			try
			{
				ObjView->serial (file);
			}
			catch (Exception& e)
			{
				MessageBox (e.what(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
			}
		}
		else
		{
			// Create a message
			char msg[512];
			_snprintf (msg, 512, "Can't open the file %s for reading.", (const char*)fileDlg.GetPathName());
			MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
		}
	}
}

#include <dlgs.h>       // for standard control IDs for commdlg

void CMainFrame::OnFileOpen() 
{
	// update UI
	update ();

	// Create a dialog
	static char BASED_CODE szFilter[] = 
		"All NeL Files (*.shape;*.ps;*.ig)\0*.shape;*.ps;*.ig\0"
		"NeL Shape Files (*.shape)\0*.shape\0"
		"NeL Particule System Files (*.ps)\0*.ps\0"
		"NeL Instance Group Files (*.ig)\0*.ig\0"
		"All Files (*.*)\0*.*\0\0";

	// Filename buffer
	char buffer[65535];
	buffer[0]=0;

	OPENFILENAME openFile;
	memset (&openFile, 0, sizeof (OPENFILENAME));
	openFile.lStructSize = sizeof (OPENFILENAME);
	openFile.hwndOwner = this->m_hWnd;
    openFile.lpstrFilter = szFilter;
    openFile.nFilterIndex = 0;
    openFile.lpstrFile = buffer;
    openFile.nMaxFile = 65535;
    openFile.Flags = OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_ALLOWMULTISELECT|OFN_ENABLESIZING|OFN_EXPLORER;
    openFile.lpstrDefExt = "*.shape;*.ig;*.ps";
	

	if (GetOpenFileName(&openFile))
	{		
		// Build an array of name
		std::vector<std::string> meshFilename;

		// Filename pointer
		char *c=buffer;

		// Read the path
		CString path = buffer;
		if (path.GetLength()>openFile.nFileOffset)
		{
			// Double zero at the end
			c[path.GetLength()+1]=0;

			// Path is empty
			path = "";
		}
		else
		{
			// Adda slash
			path += "\\";

			// Look for the next string
			while (*(c++)) {}
		}

		// For each file selected
		while (*c)
		{
			// File name
			char filename[256];
			char *ptr=filename;

			// Read a file name
			while (*c)
			{
				*(ptr++)=*(c++);
			}
			*ptr=0;
			c++;

			// File name
			CString name = path + filename;

			// file is an ig ?
			if (name.Find(".ig") != -1)
			{
				// Load the instance group
				if (ObjView->loadInstanceGroup (name))
				{
					// Reset the camera
					OnResetCamera();

					// Touch the channel mixer
					ObjView->reinitChannels ();
				}
			}
			else
			{
				// Add it in the array
				meshFilename.push_back ((const char*)name);
			}
		}

		// Some mesh to load ?
		if ( !meshFilename.empty() )
		{	
			// Create a dialog for the skel
			static char BASED_CODE szFilter2[] = "NeL Skeleton Files (*.skel)|*.skel|All Files (*.*)|*.*||";
			CFileDialog fileDlg2 ( TRUE, ".skel", "*.skel", OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter2);
			if (fileDlg2.DoModal()==IDOK)
			{
				// Load the shape with a skeleton
				if (ObjView->loadMesh (meshFilename, fileDlg2.GetPathName()))
				{
					// Reset the camera
					OnResetCamera();

					// Touch the channel mixer
					ObjView->reinitChannels ();
				}
			}
			else
			{
				// Load the shape without skeleton
				if (ObjView->loadMesh (meshFilename, ""))
				{
					// Reset the camera
					OnResetCamera();

					// Touch the channel mixer
					ObjView->reinitChannels ();
				}
			}
		}
	}
}

void CMainFrame::OnFileSaveconfig() 
{
	// Update UI
	update ();

	// Create a dialog
	static char BASED_CODE szFilter[] = "NeL Object viewer config (*.ovcgf)|*.ovcgf|All Files (*.*)|*.*||";
	CFileDialog fileDlg( FALSE, ".ovcgf", "*.ovcgf", OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter);
	if (fileDlg.DoModal()==IDOK)
	{
		// Open the file
		COFile file;
		if (file.open ((const char*)fileDlg.GetPathName()))
		{
			try
			{
				ObjView->serial (file);
			}
			catch (Exception& e)
			{
				MessageBox (e.what(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
			}
		}
		else
		{
			// Create a message
			char msg[512];
			_snprintf (msg, 512, "Can't open the file %s for writing", (const char*)fileDlg.GetPathName());
			MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
		}
	}
}



void CMainFrame::OnViewFirstpersonmode() 
{
	MoveMode=false;
	ToolBar.Invalidate ();
}

void CMainFrame::OnViewObjectmode() 
{
	MoveMode=true;
	ToolBar.Invalidate ();
}

void CMainFrame::OnViewSetbackground() 
{
	static COLORREF colTab[16] = { 0, 0xff0000, 0x00ff00, 0xffff00, 0x0000ff, 0xff00ff, 0x00ffff, 0xffffff
								   , 0x7f7f7f, 0xff7f7f, 0x7fff7f, 0xffff7f, 0x7f7fff, 0xff7fff, 0x7fffff, 0xff7f00 } ;	
	BgColor = ObjView->getBackGroundColor() ;
	CHOOSECOLOR cc ;
	cc.lStructSize = sizeof(CHOOSECOLOR) ;
	cc.hwndOwner = this->m_hWnd ;
	cc.Flags = CC_RGBINIT | CC_ANYCOLOR | CC_FULLOPEN  ;	
	cc.rgbResult = RGB(BgColor.R, BgColor.G, BgColor.B) ;
	cc.lpCustColors = colTab ;

	if (::ChooseColor(&cc) == IDOK)
	{		
		BgColor.R = (uint8) (cc.rgbResult & 0xff) ;
		BgColor.G = (uint8) ((cc.rgbResult & 0xff00) >> 8) ;
		BgColor.B = (uint8) ((cc.rgbResult & 0xff0000) >> 16) ;
	
		ObjView->setBackGroundColor(BgColor) ;
	}	
}

void CMainFrame::OnViewSetmovespeed() 
{
	// Set value
	CSetValueDlg valueDlg;

	// Set default value
	valueDlg.Value=toString (MoveSpeed).c_str();
	valueDlg.Title="Select your move speed";

	// Open dialog
	if (valueDlg.DoModal ()==IDOK)
	{
		// Get deflaut value
		sscanf ((const char*)valueDlg.Value, "%f", &MoveSpeed);
	}
}

void CMainFrame::OnWindowAnimation() 
{
	AnimationWindow^=true;
	update ();
}

void CMainFrame::OnWindowAnimationset() 
{
	AnimationSetWindow^=true;
	update ();
}

void CMainFrame::OnWindowMixersslots() 
{
	MixerSlotsWindow^=true;
	update ();
}

void CMainFrame::OnWindowParticles() 
{
	ParticlesWindow^=true;
	update ();
}

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	/*ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,*/
};

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	// Parent create
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create tool bar
	if (!ToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!ToolBar.LoadToolBar(IDR_TOOL_EDIT))
	{
		return -1;      // fail to create
	}

	// Create the status bar
	StatusBar.Create (this);
	StatusBar.SetIndicators (indicators,
		  sizeof(indicators)/sizeof(UINT));

	// Docking
	ToolBar.SetButtonStyle (0, TBBS_CHECKGROUP);
	ToolBar.SetButtonStyle (1, TBBS_CHECKGROUP);
	ToolBar.SetButtonStyle (3, TBBS_CHECKBOX);
	ToolBar.SetButtonStyle (5, TBBS_CHECKBOX);
	ToolBar.SetButtonStyle (6, TBBS_CHECKBOX);
	ToolBar.SetButtonStyle (7, TBBS_CHECKBOX);
	ToolBar.EnableDocking(CBRS_ALIGN_ANY);

	InitialUpdateFrame (NULL, TRUE);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&ToolBar);

	return 0;
}

BOOL CMainFrame::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	return FALSE;
}

void CMainFrame::OnUpdateWindowAnimation(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (AnimationWindow);
}

void CMainFrame::OnUpdateWindowAnimationset(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (AnimationSetWindow);
}

void CMainFrame::OnUpdateWindowMixersslots(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (MixerSlotsWindow);
}

void CMainFrame::OnUpdateWindowParticles(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (ParticlesWindow);
}

void CMainFrame::OnUpdateViewObjectmode(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (MoveMode);
}

void CMainFrame::OnUpdateViewFirstpersonmode(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (!MoveMode);
}

void CMainFrame::OnUpdateEditX(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (X);
}

void CMainFrame::OnUpdateEditY(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (Y);
}

void CMainFrame::OnUpdateEditZ(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (Z);
}

void CMainFrame::OnUpdateEditMoveelement(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (MoveElement);
}

void CMainFrame::OnHelpAboutobjectviewer() 
{
	CAboutDialog about;
	about.DoModal();
}


void CMainFrame::OnSetLag()
{
CChooseLag cl;
cl.m_LagTime = ObjView->getLag();
if (cl.DoModal() == IDOK)
{
	ObjView->setLag(cl.m_LagTime);
}
}


void CMainFrame::OnRemoveAllInstancesFromScene()
{
	if (MessageBox("Delete all instances from scene ?", "Object Viewer", MB_YESNO) == IDYES)
	{
		ObjView->removeAllInstancesFromScene();
		
		// Reset the camera
		OnResetCamera();

		// Touch the channel mixer
		ObjView->reinitChannels ();
	}
}

