/** \file texture_grouped.h
 * <File description>
 *
 * $Id: texture_grouped.h,v 1.5 2002/05/28 16:57:01 vizerie Exp $
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

#ifndef NL_TEXTURE_GROUPED_H
#define NL_TEXTURE_GROUPED_H

#include "nel/misc/types_nl.h"
#include "3d/texture.h"
#include "nel/misc/uv.h"



namespace NL3D {


using NLMISC::CSmartPtr;
using NLMISC::CUV;

/**
 * This kind texture is used for grouping several other textures. Each texture must have the same size. 
 * The textures are copied into one single surface, so  animation can be performed only by UV shifting (if there's no wrapping).
 * This is useful when objects sorting (by texture) is too complex or cost too much time (particles for examples...)
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CTextureGrouped : public ITexture
{
public:

	/// default ctor. by default, there are no texture present	 	
	CTextureGrouped();

	/// copy ctor	
	CTextureGrouped(const CTextureGrouped &src);
	

	/// = operator
	CTextureGrouped &operator=(const CTextureGrouped &src);

	/** Check if all the textures in a tab have the same size and the same pixel format	
	 *  \param textureTab : pointer to a texture list*
	 *  \nbTex the number of textures in the list (>0)
	 *  \see setTextures()
	 */
	bool areValid(CSmartPtr<ITexture> *textureTab, uint nbTex);

	/** This set the textures to be used. They all must have the same size.
	 *  An assertion is thrown otherwise.	 
	 *  WARNING : if you touch one of the textures in the tab later, you may need to touch this one if it changed
	 *  \param textureTab : pointer to a texture list
	 *  \nbTex the number of textures in the list (>0)
	 *  \see haveValidSizes()
	 */
	void setTextures(CSmartPtr<ITexture> *textureTab, uint nbTex);
	

	/// Retrieve the number of textures grouped in this one
	uint32 getNbTextures(void) const { return _NbTex; }


	/** Retrieve pointers to the textures.
	 *  \param textureTab a tab containing enough space for the pointers
	 *  \see getNbTextures()
	 */

	void getTextures(CSmartPtr<ITexture> *textureTab) const;

	// get a texture in the list
	CSmartPtr<ITexture> getTexture(uint32 index) { return _Textures[index]; }
	
	/** Get the U-delta and V delta in the groupedTexture for one unit texture (they all have the same size).	 
	 *  return (0, 0) if no textures have been set
	 */
	const CUV &getUVDelta(void) const
	{
		return _DeltaUV;
	}

	/// Get the origin UV for one texture. Its index is the same than in the tab that was sent to setTextures()
	const CUV &getUV(uint32 index) const
	{
		return _TexUVs[index].uv0;
	}


	/** 
	 * sharing system.	 
	 */	
	virtual bool			supportSharing() const;
	virtual std::string		getShareName() const;


	/** 
	 * Generate the texture.	 
	 */	
	void doGenerate();

	/// serialization
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);


	/// a group of 4 uvs
	struct TFourUV
	{
		CUV uv0, uv1, uv2, uv3;
	};

	// a list of uv's
	typedef std::vector< TFourUV > TFourUVList;

	// Get a tab of 4 UVs for a texture in the group : 0 = top-left, 1 = top-right, 2 = bottom-right, 3 = bottom-left
	const TFourUV &getUVQuad(uint texIndex)
	{
		if (texIndex < _NbTex)
		{
			return _TexUVs[texIndex];
		}
		else
		{
			return _TexUVs[texIndex % _NbTex];
		}
	}


	virtual void release();

	
	NLMISC_DECLARE_CLASS(CTextureGrouped);

protected:	
	uint32 _NbTex; // for caching

	/// pointers to the original textures
	typedef std::vector< CSmartPtr<ITexture> > TTexList;
	TTexList _Textures;

	/// uv delta for one texture in the group
	CUV _DeltaUV;

	/// the UVs for each texture in the group
	TFourUVList _TexUVs;

	// Copy this class attributes from src; Used by the = operator and the copy ctor
	void duplicate(const CTextureGrouped &src);

	// make textures as a group of dummy. This is used when textures formet are incompatible
	void makeDummies(CSmartPtr<ITexture> *textureTab, uint nbTex);

	// display a warning to tell that a set of grouped textures are incompatibles
	void displayIncompatibleTextureWarning(CSmartPtr<ITexture> *textureTab, uint nbTex);
};



} // NL3D


#endif // NL_TEXTURE_GROUPED_H

/* End of texture_grouped.h */
