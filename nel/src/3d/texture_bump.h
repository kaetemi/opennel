/** \file texture_bump.h
 * <File description>
 *
 * $Id: texture_bump.h,v 1.5 2002/02/15 17:12:31 vizerie Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#ifndef NL_TEXTURE_BUMP_H
#define NL_TEXTURE_BUMP_H

#include "nel/misc/types_nl.h"
#include "3d/texture.h"

namespace NL3D {


/**
 * This texture is a helper to build a DsDt texture from a height map (provided it is supported by hardware)
 * The height map should be convertible to a RGBA bitmap
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CTextureBump : public ITexture
{
public:
	NLMISC_DECLARE_CLASS(CTextureBump);
	/// ctor
	CTextureBump();

	/// set the height map used to generate this bump map
	void				setHeightMap(ITexture *heightMap);
	/// get the height map used to generate this bumpmap
	ITexture			*getHeightMap() { return _HeightMap; };
	/// get the height map used to generate this bumpmap (const version)
	const ITexture		*getHeightMap() const { return _HeightMap; };

	// serial this texture datas
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	
	virtual bool			supportSharing() const;
	
	virtual std::string		getShareName() const;

	void					enableSharing(bool enabled = true) { _DisableSharing = !enabled; }

	bool					isSharingEnabled() const { return !_DisableSharing; }

	/// absolute offsets are taken from the gradient of the height map
	void					setAbsoluteOffsets(bool use = true) { _UseAbsoluteOffsets = true; }
	bool					getAbsoluteOffsets() const { return _UseAbsoluteOffsets; }

	/** Force normalization of this texture when it is generated, so that the deltas reach their maximum amplitude.
	  * After the texture generation, the factor needed to normalize can be obtained
	  */
	void					forceNormalize(bool force = true) { _ForceNormalize = force; }

	/// Test wether normalization is forced with that texture
	bool				    isNormalizationForced() const { return _ForceNormalize; }

	/// Get the normalization factor. This is valid only if the texture has been generated
	float					getNormalizationFactor() 
	{ 
		nlassert(_ForceNormalize);
		return _NormalizationFactor; 
	}

	// inherited from ITexture. release this texture, and its datas
	virtual void release();	
	
protected:
	// inherited from ITexture. Generate this bumpmap pixels
	virtual void doGenerate();	
	NLMISC::CSmartPtr<ITexture> _HeightMap;
	float						_NormalizationFactor;
	bool						_DisableSharing;
	bool						_UseAbsoluteOffsets;
	bool						_ForceNormalize;
private:
	/// we don't allow for mipmap for bump so we redefine this to prevent the user from doing this on the base class Itexture
	virtual         void setFilterMode(TMagFilter magf, TMinFilter minf);
};


} // NL3D


#endif // NL_TEXTURE_BUMP_H

/* End of texture_bump.h */
