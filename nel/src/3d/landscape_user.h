/** \file landscape_user.h
 * <File description>
 *
 * $Id: landscape_user.h,v 1.22 2003/11/18 11:02:51 berenguier Exp $
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

#ifndef NL_LANDSCAPE_USER_H
#define NL_LANDSCAPE_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_landscape.h"
#include "3d/landscape_model.h"
#include "3d/zone_manager.h"
#include "3d/scene.h"

#define NL3D_MEM_LANDSCAPE						NL_ALLOC_CONTEXT( 3dLand )

namespace NL3D
{


//****************************************************************************
/**
 * ULandscape Implementation
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CLandscapeUser : public ULandscape
{
private:
	// The Scene.
	CScene				*_Scene;
	// The landscape, owned by the scene.
	CLandscapeModel		*_Landscape;
	// The zoneManager.
	CZoneManager		_ZoneManager;

public:

	/// \name Object
	// @{
	CLandscapeUser(CScene *scene)
	{
		NL3D_MEM_LANDSCAPE
		nlassert(scene);
		_Scene= scene;
		_Landscape= (CLandscapeModel*)_Scene->createModel(LandscapeModelId);
	}
	virtual	~CLandscapeUser()
	{
		NL3D_MEM_LANDSCAPE
		_Scene->deleteModel(_Landscape);
		_Landscape= NULL;
	}
	// @}


	/// \name Load
	/// All those load methods use CPath to search files.
	// @{
	/// Set the zonePath from where zones are loaded.
	virtual	void	setZonePath(const std::string &zonePath);
	/// Load the tile banks:  the ".bank" and the  ".farbank".
	virtual	void	loadBankFiles(const std::string &tileBankFile, const std::string &farBankFile);
	/// Flush the tiles
	virtual void	flushTiles (NLMISC::IProgressCallback &progress);
	/// Postfix tile filename
	virtual void	postfixTileFilename (const char *postfix);
	/// Postfix vegetable filename
	virtual void	postfixTileVegetableDesc (const char *postfix);
	/// Load all Zones around a position. Call at init only!! (no zone must exist before). This is a blocking call.
	virtual	void	loadAllZonesAround(const CVector &pos, float radius);
	virtual	void	loadAllZonesAround(const CVector &pos, float radius, std::vector<std::string> &zonesAdded);
	/// Delete old zones, or load new zones, around a position. new Zones are loaded async.
	virtual	void	refreshZonesAround(const CVector &pos, float radius);
	virtual	void	refreshZonesAround(const CVector &pos, float radius, std::string &zoneAdded, std::string &zoneRemoved);
	/// Delete old zones, or load new zones, around a position, until it is finished. This is a blocking call.
	virtual	void	refreshAllZonesAround(const CVector &pos, float radius, std::vector<std::string> &zonesAdded, std::vector<std::string> &zonesRemoved, 
		NLMISC::IProgressCallback &progress);
	virtual	void	getAllZoneLoaded(std::vector<std::string>	&zoneLoaded) const;
	// @}


	/// \name Lighting
	// @{
	/**
	  *  Setup the light color use for static illumination.
	  *  NB: This setup will be visible only for new texture far/near computed (when player move or see dynamic lighting).
	  *
	  *  \param diffuse is the color of the diffuse componante of the lighting.
	  *  \param ambiant is the color of the ambiante componante of the lighting.
	  *  \param multiply is the multiply factor. Final color is (diffuse*multiply*shading+ambiant*(1.0-shading))
	  */
	virtual	void	setupStaticLight (const CRGBA &diffuse, const CRGBA &ambiant, float multiply);

	virtual	void	setPointLightDiffuseMaterial(CRGBA diffuse);
	virtual	CRGBA	getPointLightDiffuseMaterial () const;


	virtual	void	setUpdateLightingFrequency(float freq);

	virtual	void	updateLightingAll();

	// @}


	/// \name Parameters
	// @{
	/// Set threshold for subdivsion quality. The lower is threshold, the more the landscape is subdivided. Default: 0.001.
	virtual	void	setThreshold (float thre);
	/// Get threshold.
	virtual	float	getThreshold () const;
	/// Set tile near distance. Default 50.f. maximized to length of Far alpha transition).
	virtual	void	setTileNear (float tileNear);
	/// Get tile near distance.
	virtual	float	getTileNear () const;
	/// Set Maximum Tile subdivision. Valid values must be in [0..4]  (assert). Default is 0 (for now :) ).
	virtual	void	setTileMaxSubdivision (uint tileDiv);
	/// Get Maximum Tile subdivision.
	virtual	uint 	getTileMaxSubdivision ();
	// @}


	/// \name Misc
	// @{
	/// Return the name of the zone around a particular position (in NL3D basis!).
	virtual	std::string	getZoneName(const CVector &pos);

	virtual	void		show()
	{
		NL3D_MEM_LANDSCAPE
		_Landscape->show();
	}
	virtual	void		hide()
	{
		NL3D_MEM_LANDSCAPE
		_Landscape->hide();
	}

	virtual	void		enableAdditive (bool enable);
	virtual	bool		isAdditiveEnabled () const;

	virtual	void			setRefineCenterAuto(bool mode);
	virtual bool			getRefineCenterAuto() const;
	virtual void			setRefineCenterUser(const CVector &refineCenter);
	virtual const CVector	&getRefineCenterUser() const;
	
	// @}


	/// \name HeightField DeltaZ.
	// @{
	virtual	CVector		getHeightFieldDeltaZ(float x, float y) const;
	virtual	void		setHeightField(const CHeightMap &hf);
	// @}

	/// Micro-Vegetation.
	// @{
	virtual	void		enableVegetable(bool enable);
	virtual	void		loadVegetableTexture(const std::string &textureFileName);
	virtual	void		setupVegetableLighting(const CRGBA &ambient, const CRGBA &diffuse, const CVector &directionalLight);
	virtual	void		setVegetableWind(const CVector &windDir, float windFreq, float windPower, float windBendMin);
	virtual	void		setVegetableUpdateLightingFrequency(float freq);
	// @}


	/// \name Dynamic Lighting management
	// @{
	virtual	void		setDLMGlobalVegetableColor(CRGBA gvc);
	virtual	CRGBA		getDLMGlobalVegetableColor() const;
	// @}

	/// \name ShadowMapping
	// @{
	virtual void			enableReceiveShadowMap(bool state);
	virtual bool			canReceiveShadowMap() const;
	// @}

public:
	/// \name Accessor for CLandscapeUser.
	// @{
	CLandscapeModel		*getLandscape()
	{
		NL3D_MEM_LANDSCAPE
		return _Landscape;
	}
	// @}

};


} // NL3D


#endif // NL_LANDSCAPE_USER_H

/* End of landscape_user.h */
