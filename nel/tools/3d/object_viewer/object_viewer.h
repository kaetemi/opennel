/** \file object_viewer.cpp
 * main header file for the OBJECT_VIEWER DLL
 *
 * $Id: object_viewer.h,v 1.29 2002/02/04 17:41:13 vizerie Exp $
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

#if !defined(AFX_OBJECT_VIEWER_H__9B22CB84_1929_11D5_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_OBJECT_VIEWER_H__9B22CB84_1929_11D5_9CD4_0050DAC3A412__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#define REGKEY_OBJ_VIEW "Software\\Nevrax\\nel\\object_viewer"
#define REGKEY_OBJ_VIEW_OPENGL_WND "Software\\Nevrax\\nel\\object_viewer\\opengl_wnd"
#define REGKEY_OBJ_VIEW_SCENE_DLG "Software\\Nevrax\\nel\\object_viewer\\scene_dlg"
#define REGKEY_OBJ_VIEW_ANIMATION_DLG "Software\\Nevrax\\nel\\object_viewer\\animation_dlg"
#define REGKEY_OBJ_VIEW_ANIMATION_SET_DLG "Software\\Nevrax\\nel\\object_viewer\\animation_set_dlg"
#define REGKEY_OBJ_VIEW_SLOT_DLG "Software\\Nevrax\\nel\\object_viewer\\slot_dlg"
#define REGKEY_OBJ_PARTICLE_DLG "Software\\Nevrax\\nel\\object_viewer\\particle_dlg"
#define REGKEY_OBJ_DAYNIGHT_DLG "Software\\Nevrax\\nel\\object_viewer\\daynight_dlg"
#define REGKEY_OBJ_WATERPOOL_DLG "Software\\Nevrax\\nel\\object_viewer\\daynight_dlg"
#define REGKEY_OBJ_VIEW_VEGETABLE_DLG "Software\\Nevrax\\nel\\object_viewer\\vegetable_dlg"




#include "resource.h"
#include "object_viewer_interface.h"
#include "main_dlg.h"
#include "animation_set_dlg.h"
#include "animation_dlg.h"

#include <3d/animation_set.h>
#include <3d/channel_mixer.h>
#include <3d/shape.h>
#include <3d/event_mouse_listener.h>
#include <3d/light.h>
#include <3d/font_manager.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/stream.h>

namespace NL3D
{
class CFontGenerator;
class CWaterPoolManager;
class CTileVegetableDesc;
class CLandscapeModel;
class CVisualCollisionManager;
class CVisualCollisionEntity;
}

class CMainFrame;

class CParticleDlg ;
class CDayNightDlg ;
class CWaterPoolEditor;
class CVegetableDlg ;



/////////////////////////////////////////////////////////////////////////////
// CObject_viewerApp
// See object_viewer.cpp for the implementation of this class
//

class CObject_viewerApp : public CWinApp
{
public:
	CObject_viewerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObject_viewerApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CObject_viewerApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

class CMeshDesc
{
public:
	CMeshDesc () {};
	CMeshDesc (std::vector<std::string> &meshNames, const char* skeletonName)
	{
		MeshNames=meshNames;
		SkeletonName=skeletonName;
	}

	std::vector<std::string>	MeshNames;
	std::string					SkeletonName;
	void			serial (NLMISC::IStream& s)
	{
		int ver=s.serialVersion (1);

		if (ver<1)
		{
			// Readonly
			nlassert (s.isReading());

			// One name
			std::string singleName;
			s.serial (singleName);
			MeshNames.clear ();
			MeshNames.push_back (singleName);
		}
		else
		{
			s.serialCont (MeshNames);
		}
		s.serial (SkeletonName);
	}
};

class CObjectViewer : public IObjectViewer
{
	friend class CMainFrame;
	friend class CAnimationSetDlg;
public:
	CObjectViewer ( );
	virtual ~CObjectViewer ();

	// Init the UI
	void initUI (HWND parent=NULL);

	// Go
	void go ();

	// Release the UI
	void releaseUI ();

	// Set single animtion.
	void setSingleAnimation (NL3D::CAnimation*	pAnim, const char* name);

	// Set automatic animation
	void setAutoAnimation (NL3D::CAnimation* pAnim);

	// Add a mesh
	NL3D::CTransformShape	*addMesh (NL3D::IShape* pMeshShape, const char* meshName, const char *meshBaseName, NL3D::CSkeletonModel* pSkel);

	// Add a skel
	NL3D::CSkeletonModel	*addSkel (NL3D::IShape* pSkelShape, const char* skelName, const char *skelBaseName);

	// remove all instances from the scene
	void					 removeAllInstancesFromScene();

	/// Force all the instances of the scene to use the given texture set (if available)
	void activateTextureSet(uint index);

	// Load a mesh
	bool loadMesh (std::vector<std::string> &meshFilename, const char* skeleton="");

	// Load an instance group
	bool loadInstanceGroup(const char *igFilename);

	// Set ambient color
	void setAmbientColor (const NLMISC::CRGBA& color);

	// Set ambient color
	void setLight (unsigned char id, const NL3D::CLight& light);




	// Load a shape
	void resetCamera ();

	// get the mouse listener
	NL3D::CEvent3dMouseListener &getMouseListener(void) { return _MouseListener ; }
	const NL3D::CEvent3dMouseListener &getMouseListener(void) const { return _MouseListener ; }

	// get the particle dialog
	CParticleDlg *getParticleDialog(void) { return _ParticleDlg ; }
	const CParticleDlg *getParticleDialog(void) const { return _ParticleDlg ; }




	/// Not exported
	// @{

	void setAnimTime (float animStart, float animEnd);

	// Reset the slots
	void resetSlots ();

	// Reinit and refill the channel mixer channels
	void reinitChannels ();

	// Return the frame rate
	float getFrameRate ();

	// Serial the config
	void serial (NLMISC::IStream& f);

	/// set the backgournd color
	void setBackGroundColor(const NLMISC::CRGBA& col) { _BackGroundColor = col ; }

	/// get the background color
	NLMISC::CRGBA getBackGroundColor(void) { return _BackGroundColor ; }

	/// Get font manager
	NL3D::CFontManager		*getFontManager () { return &_FontManager; };

	/// Get font generator
	NL3D::CFontGenerator	*getFontGenerator () { return _FontGenerator; };
	// @}

	/** an interface for objects that want to be called during the loop
	  * First use : display of an optionnal bbox for a particle system
	  */
	struct IMainLoopCallBack
	{
		/// this will be called each time the main loop is processed
		virtual void go() = 0 ;
	} ;

	/** add an object that will be notified each time a frame is processed
	  * \see removeMainLoopCallBack()
	  */
	void registerMainLoopCallBack(IMainLoopCallBack *i)  ;

	/// remove an object that was registered with registerMainLoopCallBack()
	void removeMainLoopCallBack(IMainLoopCallBack *i) ;
	  
	/// set the lag in milliseconds between each frame; 0 is the default
	void setLag(uint32 milliSec) { _Lag = milliSec; }

	/// get the lag in milliseconds
	uint32 getLag() const		  { return _Lag; }

	/// eval sound tracks
	void evalSoundTrack (float lastTime, float currentTime);

	/// Setup the playlist with the playlist
	void setupPlaylist (float time);
		
	/// Enable disable channels
	void enableChannels ();
		
	/// Setup transform positions
	void setupPositions ();

	/// Manage matrix increment
	void addTransformation (NLMISC::CMatrix &current, NL3D::CAnimation *anim, float begin, float end, NL3D::ITrack *posTrack, NL3D::ITrack *rotquatTrack, 
									   NL3D::ITrack *nextPosTrack, NL3D::ITrack *nextRotquatTrack, bool removeLast);

	/// inherited from CObjectViewerInterface
	void setWaterPoolManager(NL3D::CWaterPoolManager &wpm) { _Wpm = &wpm; }

	NL3D::CWaterPoolManager &getWaterPoolManager() { return *_Wpm; }


	/// \name Landscape Vegetable Edition
	// @{

	/// true if landscape is created
	bool		isVegetableLandscapeCreated() const {return _VegetableLandscape!=NULL;}

	/// load the landscape with help of setup in object_viewer.cfg. return true if OK.
	bool		createVegetableLandscape();

	/// if created, show the landscape
	void		showVegetableLandscape();
	/// if created, hide the landscape
	void		hideVegetableLandscape();

	/// display vegetable with landscape
	void		enableLandscapeVegetable(bool enable);

	/// refresh the vegetables in landscape, with the new vegetableSet
	void		refreshVegetableLandscape(const NL3D::CTileVegetableDesc &tvdesc);


	/// get vegetable Wind wetup.
	float		getVegetableWindPower() const {return _VegetableWindPower;}
	float		getVegetableWindBendStart() const {return _VegetableWindBendMin;}
	float		getVegetableWindFrequency() const {return _VegetableWindFreq;}
	/// set vegetable Wind wetup (updat view if possible)
	void		setVegetableWindPower(float w);
	void		setVegetableWindBendStart(float w);
	void		setVegetableWindFrequency(float w);

	/// if enable, snap the camera to the ground of the landscape.
	void		snapToGroundVegetableLandscape(bool enable);

	// @}

private:

	CMainFrame									*_MainFrame;
	CAnimationDlg								*_AnimationDlg;
	CMainDlg									*_SlotDlg;
	CAnimationSetDlg							*_AnimationSetDlg;
	CParticleDlg								*_ParticleDlg ;
	CDayNightDlg								*_DayNightDlg ;
	CWaterPoolEditor							*_WaterPoolDlg ;
	CVegetableDlg								*_VegetableDlg ;
	std::vector<std::string>					_ListShapeBaseName;
	std::vector<CMeshDesc>						_ListMeshes;
	std::vector<class NL3D::CTransformShape*>	_ListTransformShape;
	NL3D::CAnimationSet							_AnimationSet;
	std::vector<NL3D::CChannelMixer>			_ChannelMixer;
	NL3D::CEvent3dMouseListener					_MouseListener;
	NLMISC::CRGBA								_HotSpotColor;
	float										_HotSpotSize;
	NLMISC::CRGBA								_BackGroundColor;
	NLMISC::CVector								_SceneCenter;

	// Font mgt
	NL3D::CFontManager							_FontManager;
	NL3D::CFontGenerator						*_FontGenerator;	
	std::string									_FontPath;
	std::vector<IMainLoopCallBack *>			_CallBackList;
	uint32										_Lag; 
	float										_CameraFocal;	
	float										_LastTime;
	NL3D::CWaterPoolManager						*_Wpm;

	/// \name Vegetable Edition
	// @{
	/// Our landscape
	NL3D::CLandscapeModel						*_VegetableLandscape;

	// File info to build it
	std::string									_VegetableLandscapeTileBank;
	std::string									_VegetableLandscapeTileFarBank;
	std::vector<std::string>					_VegetableLandscapeZoneNames;

	// Misc.
	float										_VegetableLandscapeThreshold;
	float										_VegetableLandscapeTileNear;
	NLMISC::CRGBA								_VegetableLandscapeAmbient;
	NLMISC::CRGBA								_VegetableLandscapeDiffuse;
	std::string									_VegetableTexture;
	NLMISC::CRGBA								_VegetableAmbient;
	NLMISC::CRGBA								_VegetableDiffuse;
	NLMISC::CVector								_VegetableLightDir;
	// Vegetable wind.
	NLMISC::CVector								_VegetableWindDir;
	float										_VegetableWindFreq;
	float										_VegetableWindPower;
	float										_VegetableWindBendMin;

	bool										_VegetableEnabled;

	// Collision
	bool										_VegetableSnapToGround;
	float										_VegetableSnapHeight;
	NL3D::CVisualCollisionManager				*_VegetableCollisionManager;
	NL3D::CVisualCollisionEntity				*_VegetableCollisionEntity;

	// load cfg.
	void		loadVegetableLandscapeCfg(NLMISC::CConfigFile &cf);

	// @}

};

void setRegisterWindowState (const CWnd *pWnd, const char* keyName);
void getRegisterWindowState (CWnd *pWnd, const char* keyName, bool resize);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OBJECT_VIEWER_H__9B22CB84_1929_11D5_9CD4_0050DAC3A412__INCLUDED_)
