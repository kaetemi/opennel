/** \file driver_material_inline.h
 * <File description>
 *
 * $Id: driver_material_inline.h,v 1.2 2001/07/11 08:24:59 besson Exp $
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

#ifndef NL_DRIVER_MATERIAL_INLINE_H
#define NL_DRIVER_MATERIAL_INLINE_H

#include "nel/misc/debug.h"

namespace NL3D
{

// --------------------------------------------------

inline bool CMaterial::texturePresent(uint8 n) const
{
	nlassert(n<IDRV_MAT_MAXTEXTURES);
	if (_Textures[n])
	{
		return(true);
	}
	return(false);
}

inline ITexture*		CMaterial::getTexture(uint8 n) const
{ 
	nlassert(n<IDRV_MAT_MAXTEXTURES);
	return(_Textures[n]); 
}

inline void CMaterial::setSrcBlend(TBlend val)
{
	_SrcBlend=val;
	_Touched|=IDRV_TOUCHED_BLENDFUNC;
}

inline void CMaterial::setDstBlend(TBlend val)
{
	_DstBlend=val;
	_Touched|=IDRV_TOUCHED_BLENDFUNC;
}

inline void CMaterial::setBlend(bool active)
{
	if (active)	_Flags|=IDRV_MAT_BLEND;
	else		_Flags&=~IDRV_MAT_BLEND;
	_Touched|=IDRV_TOUCHED_BLEND;
}

inline void CMaterial::setDoubleSided(bool active)
{
	if (active)	_Flags|=IDRV_MAT_DOUBLE_SIDED;
	else		_Flags&=~IDRV_MAT_DOUBLE_SIDED;
	_Touched|=IDRV_TOUCHED_DOUBLE_SIDED;
}

inline void CMaterial::setAlphaTest(bool active)
{
	if (active)	_Flags|=IDRV_MAT_ALPHA_TEST;
	else		_Flags&=~IDRV_MAT_ALPHA_TEST;
	_Touched|=IDRV_TOUCHED_ALPHA_TEST;
}

inline void	CMaterial::setBlendFunc(TBlend src, TBlend dst)
{
	_SrcBlend=src;
	_DstBlend=dst;
	_Touched|=IDRV_TOUCHED_BLENDFUNC;
}


inline void CMaterial::setZFunc(ZFunc val)
{
	_ZFunction=val;
	_Touched|=IDRV_TOUCHED_ZFUNC;
}

inline void	CMaterial::setZWrite(bool active)
{
	if (active)	_Flags|=IDRV_MAT_ZWRITE;
	else		_Flags&=~IDRV_MAT_ZWRITE;
	_Touched|=IDRV_TOUCHED_ZWRITE;
}

inline void CMaterial::setZBias(float val)
{
	_ZBias=val;
	_Touched|=IDRV_TOUCHED_ZBIAS;
}

inline void CMaterial::setColor(NLMISC::CRGBA rgba)
{
	_Color=rgba;
	_Touched|=IDRV_TOUCHED_COLOR;
}

inline void CMaterial::setLighting(	bool active, bool DefMat,
									NLMISC::CRGBA emissive, 
									NLMISC::CRGBA ambient, 
									NLMISC::CRGBA diffuse, 
									NLMISC::CRGBA specular,
									float	shininess)
{
	if (active)
	{
		_Flags|=IDRV_MAT_LIGHTING;
		if (DefMat)
		{
			_Flags|=IDRV_MAT_DEFMAT;
		}
		else
		{
			_Flags&=~IDRV_MAT_DEFMAT;
		}
	}
	else
	{
		_Flags&=~IDRV_MAT_LIGHTING;
	}
	_Emissive=emissive;
	_Ambient=ambient;
	_Diffuse=diffuse;
	_Specular=specular;
	_Shininess= shininess;
	_Touched|=IDRV_TOUCHED_LIGHTING;
}


// ***************************************************************************
inline void	CMaterial::setEmissive( CRGBA emissive)
{
	_Emissive= emissive;
	_Touched|=IDRV_TOUCHED_LIGHTING;
}
// ***************************************************************************
inline void	CMaterial::setAmbient( CRGBA ambient)
{
	_Ambient= ambient;
	_Touched|=IDRV_TOUCHED_LIGHTING;
}
// ***************************************************************************
inline void	CMaterial::setDiffuse( CRGBA diffuse)
{
	// Keep opacity.
	_Diffuse.R= diffuse.R;
	_Diffuse.G= diffuse.G;
	_Diffuse.B= diffuse.B;
	_Touched|=IDRV_TOUCHED_LIGHTING;
}
// ***************************************************************************
inline void	CMaterial::setOpacity( uint8	opa )
{
	_Diffuse.A= opa;
	_Touched|=IDRV_TOUCHED_LIGHTING;
}
// ***************************************************************************
inline void	CMaterial::setSpecular( CRGBA specular)
{
	_Specular= specular;
	_Touched|=IDRV_TOUCHED_LIGHTING;
}
// ***************************************************************************
inline void	CMaterial::setShininess( float shininess )
{
	_Shininess= shininess;
	_Touched|=IDRV_TOUCHED_LIGHTING;
}


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
inline void					CMaterial::texEnvOpRGB(uint stage, TTexOperator ope)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	_TexEnvs[stage].Env.OpRGB= ope;
}
// ***************************************************************************
inline void					CMaterial::texEnvArg0RGB(uint stage, TTexSource src, TTexOperand oper)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	_TexEnvs[stage].Env.SrcArg0RGB= src;
	_TexEnvs[stage].Env.OpArg0RGB= oper;
}
// ***************************************************************************
inline void					CMaterial::texEnvArg1RGB(uint stage, TTexSource src, TTexOperand oper)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	nlassert(src!=Texture);
	_TexEnvs[stage].Env.SrcArg1RGB= src;
	_TexEnvs[stage].Env.OpArg1RGB= oper;
}


// ***************************************************************************
inline void					CMaterial::texEnvOpAlpha(uint stage, TTexOperator ope)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	_TexEnvs[stage].Env.OpAlpha= ope;
}
// ***************************************************************************
inline void					CMaterial::texEnvArg0Alpha(uint stage, TTexSource src, TTexOperand oper)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	nlassert(oper==SrcAlpha || oper==InvSrcAlpha);
	_TexEnvs[stage].Env.SrcArg0Alpha= src;
	_TexEnvs[stage].Env.OpArg0Alpha= oper;
}
// ***************************************************************************
inline void					CMaterial::texEnvArg1Alpha(uint stage, TTexSource src, TTexOperand oper)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	nlassert(oper==SrcAlpha || oper==InvSrcAlpha);
	nlassert(src!=Texture);
	_TexEnvs[stage].Env.SrcArg1Alpha= src;
	_TexEnvs[stage].Env.OpArg1Alpha= oper;
}


// ***************************************************************************
inline void					CMaterial::texConstantColor(uint stage, CRGBA color)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	_TexEnvs[stage].ConstantColor= color;
}


// ***************************************************************************
inline uint32				CMaterial::getTexEnvMode(uint stage)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	return _TexEnvs[stage].EnvPacked;
}
// ***************************************************************************
inline void					CMaterial::setTexEnvMode(uint stage, uint32 packed)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	_TexEnvs[stage].EnvPacked= packed;
}
// ***************************************************************************
inline CRGBA				CMaterial::getTexConstantColor(uint stage)
{
	nlassert(_ShaderType==CMaterial::Normal);
	nlassert(stage>=0 && stage<IDRV_MAT_MAXTEXTURES);
	return _TexEnvs[stage].ConstantColor;
}


// ***************************************************************************
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
inline void					CMaterial::setUserColor(CRGBA userColor)
{
	nlassert(_ShaderType==CMaterial::UserColor);
	// setup stage 0 constant color (don't use texConstantColor() because of assert).
	_TexEnvs[0].ConstantColor= userColor;
}

// ***************************************************************************
inline CRGBA				CMaterial::getUserColor() const
{
	nlassert(_ShaderType==CMaterial::UserColor);
	// setup stage 0 constant color (don't use getTexConstantColor() because of assert).
	return _TexEnvs[0].ConstantColor;
}



}

#endif

