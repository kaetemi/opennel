/** \file landscape_model.h
 * <File description>
 *
 * $Id: landscape_model.h,v 1.4 2001/09/14 09:44:25 berenguier Exp $
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

#ifndef NL_LANDSCAPE_MODEL_H
#define NL_LANDSCAPE_MODEL_H

#include "nel/misc/types_nl.h"
#include "3d/transform.h"
#include "3d/render_trav.h"
#include "3d/landscape.h"


namespace NL3D 
{

class	CLandscape;

// ***************************************************************************
const NLMISC::CClassId		LandscapeModelId=NLMISC::CClassId(0x5a573b55, 0x6b395829);


// ***************************************************************************
/**
 * The model for MOT. A landscape is not designed to move, but easier here.
 * If you translate/rotate this model, nothing happens. Landscape cannot move.
 * See CLandscape for more information on Landscape.
 * \see CLandscape.
 */
class	CLandscapeModel : public CTransform
{
public:
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();

public:
	CLandscape		Landscape;

	/** Set additive value
	  * 
	  * \param additive new additive value. [0, 1]
	  */
	void			setAdditive (float additive)
	{
		_Additive=additive;
	}

	/** Get additive value
	  * 
	  * \return the additive value. [0, 1]
	  */
	float			getAdditive () const
	{
		return _Additive;
	}

	/** Set additive set
	  * 
	  * \param enable is true to activbe additive, false to disactive it.
	  */
	void			enableAdditive (bool enable)
	{
		_ActiveAdditive=enable;
	}

	/** Get additive set
	  * 
	  * \return true to if additive is actived, else false.
	  */
	bool			isAdditive () const
	{
		return _ActiveAdditive;
	}

protected:
	CLandscapeModel()
	{
		Landscape.init();
		_ActiveAdditive=false;
		_Additive=1.f;
	}
	virtual ~CLandscapeModel() {}

private:
	static IModel	*creator() {return new CLandscapeModel;}

	bool	_ActiveAdditive;
	float	_Additive;

	friend class CLandscapeClipObs;
	friend class CLandscapeRenderObs;

	// The current small pyramid, for faster clip.
	CPlane		CurrentPyramid[NL3D_TESSBLOCK_NUM_CLIP_PLANE];
};


// Clip obs.
//*************
/// Landscape Clip observer.
class	CLandscapeClipObs : public CTransformClipObs
{
public:

	/// link this model to the RootCluster.
	virtual	void	init();

	/// Hey!! I'm renderable!!!
	virtual	bool	isRenderable() const {return true;}

	/// clip.
	virtual	bool	clip(IBaseClipObs *caller);

	// The creator.
	static IObs	*creator() {return new CLandscapeClipObs;}
};
// Render obs.
//*************
/// Landscape Render observer.
class	CLandscapeRenderObs : public CTransformRenderObs
{
public:
	/// Do nothing, and don't traverseSons() too.
	virtual	void	traverse(IObs *caller);

	// The creator.
	static IObs	*creator() {return new CLandscapeRenderObs;}
};




} // NL3D


#endif // NL_LANDSCAPE_MODEL_H

/* End of landscape_model.h */
