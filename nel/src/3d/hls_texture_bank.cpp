/** \file hls_texture_bank.cpp
 * TODO: File description
 *
 * $Id: hls_texture_bank.cpp,v 1.4 2005/02/22 10:19:10 besson Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#include "std3d.h"
#include "hls_texture_bank.h"


using	namespace std;
using	namespace NLMISC;

namespace NL3D 
{


// ***************************************************************************
// ***************************************************************************
// CHLSTextureBank
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CHLSTextureBank::CHLSTextureBank()
{
}
// ***************************************************************************
void			CHLSTextureBank::reset()
{
	contReset(_ColorTextures);
	contReset(_TextureInstanceData);
	contReset(_TextureInstances);
}
// ***************************************************************************
uint32			CHLSTextureBank::addColorTexture(const CHLSColorTexture &tex)
{
	_ColorTextures.push_back(tex);
	return _ColorTextures.size()-1;
}
// ***************************************************************************
void			CHLSTextureBank::addTextureInstance(const std::string &name, uint32 colorTextureId, const vector<CHLSColorDelta> &cols)
{
	string	nameLwr= toLower(name);

	// checks
	nlassert(colorTextureId<_ColorTextures.size());
	CHLSColorTexture	&colText= _ColorTextures[colorTextureId];
	nlassert(cols.size()==colText.getNumMasks());

	// new instance
	CTextureInstance	textInst;
	textInst._ColorTextureId= colorTextureId;
	textInst._DataIndex= _TextureInstanceData.size();
	// leave ptrs undefined
	textInst._DataPtr= NULL;
	textInst._ColorTexturePtr= NULL;

	// allocate/fill data
	uint32	nameSize= (nameLwr.size()+1);
	uint32	colSize= cols.size()*sizeof(CHLSColorDelta);
	_TextureInstanceData.resize(_TextureInstanceData.size() + nameSize + colSize);
	// copy name
	memcpy(&_TextureInstanceData[textInst._DataIndex], nameLwr.c_str(), nameSize);
	// copy cols
	memcpy(&_TextureInstanceData[textInst._DataIndex+nameSize], &cols[0], colSize);

	// add the instance.
	_TextureInstances.push_back(textInst);
}
// ***************************************************************************
void			CHLSTextureBank::compilePtrs()
{
	uint8	*data= &_TextureInstanceData[0];

	// For all texture instances, compute ptr.
	for(uint i=0;i<_TextureInstances.size();i++)
	{
		CTextureInstance	&text= _TextureInstances[i];
		text._DataPtr= data + text._DataIndex;
		text._ColorTexturePtr= &_ColorTextures[text._ColorTextureId];
	}
}


// ***************************************************************************
void			CHLSTextureBank::compile()
{
	// compile the ptrs.
	compilePtrs();

	// No other ops for now.
}


// ***************************************************************************
void			CHLSTextureBank::serial(NLMISC::IStream &f)
{
	sint	ver= f.serialVersion(0);

	f.serialCont(_ColorTextures);
	f.serialCont(_TextureInstanceData);
	f.serialCont(_TextureInstances);

	// Must compile ptrs.
	if(f.isReading())
	{
		// compile the ptrs only.
		compilePtrs();
	}
}


// ***************************************************************************
void			CHLSTextureBank::fillHandleArray(std::vector<CTextureInstanceHandle> &array)
{
	for(uint i=0;i<_TextureInstances.size();i++)
	{
		CTextureInstanceHandle	h;
		h.Texture= &_TextureInstances[i];
		array.push_back(h);
	}
}


// ***************************************************************************
// ***************************************************************************
// CHLSTextureBank::CTextureInstance
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CHLSTextureBank::CTextureInstance::serial(NLMISC::IStream &f)
{
	sint	ver= f.serialVersion(0);

	f.serial(_DataIndex);
	f.serial(_ColorTextureId);
}


// ***************************************************************************
bool			CHLSTextureBank::CTextureInstance::operator<(const CTextureInstance &t) const
{
	// compare the 2 strings.
	return (strcmp((const char*)_DataPtr, (const char*)t._DataPtr)<0);
}
// ***************************************************************************
bool			CHLSTextureBank::CTextureInstance::operator<=(const CTextureInstance &t) const
{
	// compare the 2 strings.
	return (strcmp((const char*)_DataPtr, (const char*)t._DataPtr)<=0);
}


// ***************************************************************************
bool			CHLSTextureBank::CTextureInstance::sameName(const char *str)
{
	return (strcmp((const char*)_DataPtr, str)==0);
}


// ***************************************************************************
void			CHLSTextureBank::CTextureInstance::buildColorVersion(NLMISC::CBitmap &out)
{
	// get ptr to color deltas.
	uint	nameSize= strlen((const char*)_DataPtr)+1;
	CHLSColorDelta		*colDeltas= (CHLSColorDelta*)(_DataPtr + nameSize);

	// build the texture.
	_ColorTexturePtr->buildColorVersion(colDeltas, out);
}


} // NL3D
