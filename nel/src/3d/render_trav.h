/** \file render_trav.h
 * <File description>
 *
 * $Id: render_trav.h,v 1.5 2002/02/11 16:54:27 berenguier Exp $
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

#ifndef NL_RENDER_TRAV_H
#define NL_RENDER_TRAV_H

#include "3d/trav_scene.h"
#include "3d/ordering_table.h"
#include "3d/layered_ordering_table.h"
#include "nel/misc/rgba.h"
#include "nel/3d/viewport.h"
#include "3d/light_contribution.h"
#include <vector>


namespace	NL3D
{

using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;

class	IBaseRenderObs;
class	IBaseHrcObs;
class	IBaseClipObs;
class	IBaseLightObs;
class	IDriver;

class	CTransform;

// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		RenderTravId= NLMISC::CClassId(0x572456ee, 0x3db55f23);


// ***************************************************************************
/**
 * The Render traversal.
 * The purpose of this traversal is to render a list of models. This traversals is tightly linked to the cliptraversal.
 * The clipTraversals insert directly the observers with CRenderTrav::addRenderObs(obs). The traverse() method should 
 * render all the render observer with IDriver.
 *
 * This traversal is an exception since it is not designed to have a graph of observers. But this behavior is permitted,
 * and the root (if any) is traversed.
 *
 * \b USER \b RULES: Before using traverse() on a render traversal, you should:
 *	- setFrustum() the camera shape (focale....)
 *	- setCamMatrix() for the camera transform
 *
 * NB: see CScene for 3d conventions (orthonormal basis...)
 *
 * \sa CScene IBaseRenderObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CRenderTrav : public ITravCameraScene
{
public:

	/// Constructor
	CRenderTrav();

	/// \name ITrav/ITravScene Implementation.
	//@{
	IObs				*createDefaultObs() const;
	NLMISC::CClassId	getClassId() const {return RenderTravId;}
	sint				getRenderOrder() const {return 4000;}
	/** First traverse the root (if any), then render the render list.
	 * NB: no Driver clear buffers (color or ZBuffer) are done....
	 * \warning If an observer exist both in the graph and in the render list, it will be effectively traversed twice.
	 */
	void				traverse();
	//@}

	/// \name RenderList.
	//@{
	/// Clear the list of rendered observers.
	void			clearRenderList();
	/// Add an observer to the list of rendered observers. \b DOESN'T \b CHECK if already inserted.
	void			addRenderObs(IBaseRenderObs *o);
	//@}


	void			setDriver(IDriver *drv) {Driver= drv;}
	IDriver			*getDriver() {return Driver;}
	void			setViewport (const CViewport& viewport) 
	{
		_Viewport = viewport;
	}
	CViewport		getViewport () const
	{
		return _Viewport;
	}

	bool isCurrentPassOpaque() { return _CurrentPassOpaque; }

	/** Set the order or rendering for transparent objects.
	  * In real case, with direct order, we have:
	  * - Underwater is rendered.
	  * - Water is rendered.
	  * - Objects above water are rendered.
	  */
	void  setLayersRenderingOrder(bool directOrder = true) { _LayersRenderingOrder = directOrder; }
	bool  getLayersRenderingOrder() const { return _LayersRenderingOrder; }



	/// \name Render Lighting Setup.
	// @{

	// False by default. setuped by CScene
	bool						LightingSystemEnabled;

	// Global ambient. Default is (50,50,50).
	NLMISC::CRGBA				AmbientGlobal;
	// The Sun Setup.
	NLMISC::CRGBA				SunAmbient, SunDiffuse, SunSpecular;
	// set the direction of the sun. dir is normalized.
	void						setSunDirection(const CVector &dir);
	const CVector				getSunDirection() const {return _SunDirection;}

	// @}


// ******************
public:

	/// \name Render Lighting Setup. FOR OBSERVERS ONLY.
	// @{

	/** reset the lighting setup in the driver (all lights are disabled).
	 *	called at begining of traverse(). Must be called by any model (before and after rendering)
	 *	that wish to use CDriver::setLight() instead of the standard behavior with changeLightSetup()
	 */
	void		resetLightSetup();

	/** setup the driver to the given lightContribution.
	 *	if lightContribution==NULL, then all currently enabled lights are disabled.
	 *	NB: lightContribution is cached, so if same than preceding, no-op.
	 *	cache cleared at each frame with resetLightSetup().
	 *	NB: models which are sticked or skinned on a skeleton have same lightContribution 
	 *	because lightSetup is made on the skeleton only. Hence the interest of this cache.
	 *
	 *	\param useLocalAttenuation if true, use Hardware Attenuation, else use global one 
	 *	(attenuation with AttFactor)
	 */
	void		changeLightSetup(CLightContribution	*lightContribution, bool useLocalAttenuation);

	// @}

private:
	
	// A grow only list of observers to be rendered.
	std::vector<IBaseRenderObs*>	RenderList;
	// Ordering Table to sort transparent objects
	COrderingTable<IBaseRenderObs>			OrderOpaqueList;
	CLayeredOrderingTable<IBaseRenderObs>	OrderTransparentList;

	IDriver			*Driver;
	CViewport		_Viewport;

	// Temporary for the render
	bool			_CurrentPassOpaque;	
	bool			_LayersRenderingOrder;


	/// \name Render Lighting Setup.
	// @{
	// The last setup.
	CLightContribution			*_CacheLightContribution;
	bool						_LastLocalAttenuation;
	// The number of light enabled
	uint						_NumLightEnabled;

	// More precise setup
	uint						_LastSunFactor;
	CPointLight					*_LastPointLight[NL3D_MAX_LIGHT_CONTRIBUTION];
	uint8						_LastPointLightFactor[NL3D_MAX_LIGHT_CONTRIBUTION];
	bool						_LastPointLightLocalAttenuation[NL3D_MAX_LIGHT_CONTRIBUTION];

	CVector						_SunDirection;

	// @}

};


// ***************************************************************************
/**
 * The base interface for render observers.
 * Render observers MUST derive from IBaseRenderObs.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - leave traverse() to undefined.
 * - implement the init() method, to set shortcut to neighbor observers.
 *
 * \b DERIVER \b RULES:
 * - implement the notification system (see IObs and IObs() for details).
 * - implement the traverse(). See traverse() for more information. 
 * - possibly (but obviously not) modify/extend the graph methods (such as a graph behavior).
 *
 * \sa CRenderTrav
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class IBaseRenderObs : public IObs
{
public:
	/// Shortcut to observers.
	IBaseHrcObs		*HrcObs;
	IBaseClipObs	*ClipObs;
	IBaseLightObs	*LightObs;

public:

	/// Constructor.
	IBaseRenderObs()
	{
		HrcObs=NULL;
		ClipObs= NULL;
		LightObs= NULL;
	}
	/// Build shortcut to Hrc, Clip and Light.
	virtual	void	init();


	/** The base render method.
	 * The observers should not traverseSons(), for speed improvement.
	 */
	virtual	void	traverse(IObs *caller)=0;

	/**
	 * To avoid dynamic casting in mot fault of yoyo
	 */
	virtual CTransform* getTransformModel() 
	{
		return NULL;
	}
};



// ***************************************************************************
/**
 * The default render observer, used by unspecified models.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - define the traverse() method to DO NOTHING..
 *
 * \sa IBaseRenderObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CDefaultRenderObs : public IBaseRenderObs
{
public:


	/// Do nothing, and don't traverseSons() too.
	virtual	void	traverse(IObs *caller)
	{
	}
};


}


#endif // NL_RENDER_TRAV_H

/* End of render_trav.h */
