/** \file located_bindable_dialog.h
 * a dialog for located bindable properties (particles ...)
 *
 * $Id: located_bindable_dialog.h,v 1.11 2001/09/12 13:29:18 vizerie Exp $
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

#if !defined(AFX_LOCATED_BINDABLE_DIALOG_H__C715DCAB_3F07_4777_96DA_61AE2E420B09__INCLUDED_)
#define AFX_LOCATED_BINDABLE_DIALOG_H__C715DCAB_3F07_4777_96DA_61AE2E420B09__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

namespace NL3D
{
	class CPSLocatedBindable;
}



#include "ps_wrapper.h"
#include "nel/misc/rgba.h"
#include "3d/texture.h"
#include "3d/ps_plane_basis.h"


#include "dialog_stack.h"

using NLMISC::CRGBA;

class CParticleDlg;



/////////////////////////////////////////////////////////////////////////////
// CLocatedBindableDialog dialog

class CLocatedBindableDialog : public CDialog, CDialogStack
{
// Construction
public:
	// create this dialog to edit the given bindable
	CLocatedBindableDialog(NL3D::CPSLocatedBindable *bindable);   // standard constructor

	/// dtor
	~CLocatedBindableDialog();


	// init the dialog as a child of the given wnd
	void init(CParticleDlg* pParent);
// Dialog Data
	//{{AFX_DATA(CLocatedBindableDialog)
	enum { IDD = IDD_LOCATED_BINDABLE };
	CComboBox	m_BlendingMode;
	BOOL	m_IndependantSizes;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLocatedBindableDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL




// Implementation
protected:

	// enables or disabled controls for independant sizes
	void updateIndependantSizes(void);


	/// create the size control, or update it if it has been created. It returns the heivht of the control
	uint updateSizeControl();

	NL3D::CPSLocatedBindable		*_Bindable;		// the bindable being edited
	CParticleDlg					*_ParticleDlg; // the dialog that owns us
	class CAttribDlgFloat			*_SizeCtrl;	// the control used for size
	sint							_SizeCtrlX;	// x position of the control used for size
	sint							_SizeCtrlY;	// x position of the control used for size



	// Generated message map functions
	//{{AFX_MSG(CLocatedBindableDialog)
	afx_msg void OnSelchangeBlendingMode();
	afx_msg void OnIndeSizes();
	afx_msg void OnSizeWidth();
	afx_msg void OnSizeHeight();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


	//////////////////////////////////////////////
	// wrappers to various element of bindables //
	//////////////////////////////////////////////

		//////////
		// size //
		//////////
			struct CSizeWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat 
			{
			   NL3D::CPSSizedParticle *S;
			   float get(void) const { return S->getSize(); }
			   void set(const float &v) { S->setSize(v); }
			   scheme_type *getScheme(void) const { return S->getSizeScheme(); }
			   void setScheme(scheme_type *s) { S->setSizeScheme(s); }
			} _SizeWrapper;
			
		///////////
		// color //
		///////////
			struct CColorWrapper : public IPSWrapperRGBA, IPSSchemeWrapperRGBA
			{
			   NL3D::CPSColoredParticle *S;
			   CRGBA get(void) const { return S->getColor(); }
			   void set(const CRGBA &v) { S->setColor(v); }
			   scheme_type *getScheme(void) const { return S->getColorScheme(); }
			   void setScheme(scheme_type *s) { S->setColorScheme(s); }
			} _ColorWrapper;
			
		//////////////
		// angle 2D //
		//////////////
			struct CAngle2DWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
			{
			   NL3D::CPSRotated2DParticle *S;
			   float get(void) const { return S->getAngle2D(); }
			   void set(const float &v) { S->setAngle2D(v); }
			   scheme_type *getScheme(void) const { return S->getAngle2DScheme(); }
			   void setScheme(scheme_type *s) { S->setAngle2DScheme(s); }
			} _Angle2DWrapper;

		/////////////////
		// plane basis //
		/////////////////
			struct CPlaneBasisWrapper : public IPSWrapper<NL3D::CPlaneBasis>, IPSSchemeWrapper<NL3D::CPlaneBasis>
			{
			   NL3D::CPSRotated3DPlaneParticle *S;
			   NL3D::CPlaneBasis get(void) const { return S->getPlaneBasis(); }
			   void set(const NL3D::CPlaneBasis &p) { S->setPlaneBasis(p); }
			   scheme_type *getScheme(void) const { return S->getPlaneBasisScheme(); }
			   void setScheme(scheme_type *s) { S->setPlaneBasisScheme(s); }
			} _PlaneBasisWrapper;


	

		///////////////////////
		// motion blur coeff //
		///////////////////////

			struct CMotionBlurCoeffWrapper : public IPSWrapperFloat
			{
				NL3D::CPSFaceLookAt *P;
			   float get(void) const { return P->getMotionBlurCoeff(); }
			   void set(const float &v) { P->setMotionBlurCoeff(v); }
			}  _MotionBlurCoeffWrapper;

			struct CMotionBlurThresholdWrapper : public IPSWrapperFloat
			{
				NL3D::CPSFaceLookAt *P;
			   float get(void) const { return P->getMotionBlurThreshold(); }
			   void set(const float &v) { P->setMotionBlurThreshold(v); }
			}  _MotionBlurThresholdWrapper;

		///////////////
		// fanlight  //
		///////////////
			struct CFanLightWrapper : public IPSWrapperUInt
			{
				NL3D::CPSFanLight *P;
			  uint32 get(void) const { return P->getNbFans(); }
			   void set(const uint32 &v) { P->setNbFans(v); }
			}  _FanLightWrapper;

			struct CFanLightSmoothnessWrapper : public IPSWrapperUInt
			{
				NL3D::CPSFanLight *P;
			  uint32 get(void) const { return P->getPhaseSmoothness(); }
			   void set(const uint32 &v) { P->setPhaseSmoothness(v); }
			}  _FanLightSmoothnessWrapper;

			struct CFanLightPhase : public IPSWrapperFloat
			{
				NL3D::CPSFanLight *P;
				float get(void) const { return P->getPhaseSpeed(); }
			    void set(const float &v) { P->setPhaseSpeed(v); }
			}  _FanLightPhaseWrapper;
			struct CFanLightIntensityWrapper : public IPSWrapperFloat
			{
				NL3D::CPSFanLight *P;
				float get(void) const { return P->getMoveIntensity(); }
			    void set(const float &v) { P->setMoveIntensity(v); }
			}  _FanLightIntensityWrapper;

		///////////////////////
		// ribbon / tail dot //
		///////////////////////
			
			struct CTailParticleWrapper : public IPSWrapperUInt
			{
				NL3D::CPSTailParticle *P;
			    uint32 get(void) const { return P->getTailNbSeg(); }
			    void set(const uint32 &v) { P->setTailNbSeg(v); }
			}  _TailParticleWrapper;
		
		/////////////////////////////
		//		shockwave          //
		/////////////////////////////
	
			struct CRadiusCutWrapper : public IPSWrapperFloat
			{
			   NL3D::CPSShockWave *S;
			   float get(void) const { return S->getRadiusCut(); }
			   void set(const float &v) { S->setRadiusCut(v); }
			} _RadiusCutWrapper;

			struct CShockWaveNbSegWrapper : public IPSWrapperUInt
			{
			   NL3D::CPSShockWave *S;
			   uint32 get(void) const { return S->getNbSegs(); }
			   void set(const uint32 &v) { S->setNbSegs(v); }
			} _ShockWaveNbSegWrapper;


		/////////////////////////////
		// single texture (ribbon) //
		/////////////////////////////

			struct CRibbonTextureWrapper : public IPSWrapperTexture
			{
				NL3D::CPSRibbon *R;
				virtual NL3D::ITexture *get(void) { return R->getTexture(); }
				virtual void set(NL3D::ITexture *t) { R->setTexture(t); }
			} _RibbonTextureWrapper;

		//////////////////////////////
		// u / v factors for ribbon //
		//////////////////////////////

			struct CRibbonUFactorWrapper : public IPSWrapperFloat
			{
			   NL3D::CPSRibbon *R;
			   float get(void) const { return R->getUFactor(); }
			   void set(const float &u) { R->setTexture(R->getTexture(), u, R->getVFactor()); }
			} _RibbonUFactorWrapper;

			struct CRibbonVFactorWrapper : public IPSWrapperFloat
			{
			   NL3D::CPSRibbon *R;
			   float get(void) const { return R->getVFactor(); }
			   void set(const float &v) { R->setTexture(R->getTexture(), R->getUFactor(), v); }
			} _RibbonVFactorWrapper;


		///////////////////////////////
		// single texture (fanlight) //
		///////////////////////////////

			struct CFanLightTextureWrapper : public IPSWrapperTexture
			{
				NL3D::CPSFanLight *F;
				virtual NL3D::ITexture *get(void) { return F->getTexture(); }
				virtual void set(NL3D::ITexture *t) { F->setTexture(t); }
			} _FanLightTextureWrapper;




};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOCATED_BINDABLE_DIALOG_H__C715DCAB_3F07_4777_96DA_61AE2E420B09__INCLUDED_)
