/** \file driver_opengl_states.cpp
 * <File description>
 *
 * $Id: driver_opengl_states.cpp,v 1.11 2001/11/30 13:15:48 berenguier Exp $
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


// Must include "driver_opengl.h" for good GL.h definition
#include "driver_opengl.h"


// ***************************************************************************
// define it For Debug purpose only. Normal use is to hide this line
//#define		NL3D_GLSTATE_DISABLE_CACHE

namespace NL3D 
{

// ***************************************************************************
	CDriverGLStates::CDriverGLStates()
{
	_TextureCubeMapSupported= false;
}


// ***************************************************************************
void			CDriverGLStates::init(bool supportTextureCubeMap)
{
	_TextureCubeMapSupported= supportTextureCubeMap;

	// By default all arrays are disabled.
	_VertexArrayEnabled= false;
	_NormalArrayEnabled= false;
	_WeightArrayEnabled= false;
	_ColorArrayEnabled= false;	
	_SecondaryColorArrayEnabled= false;
	uint	i;
	for(i=0; i<IDRV_MAT_MAXTEXTURES; i++)
	{
		_TexCoordArrayEnabled[i]= false;
	}
	for(i=0; i<CVertexBuffer::NumValue; i++)
	{
		_VertexAttribArrayEnabled[i]= false;
	}
}


// ***************************************************************************
void			CDriverGLStates::forceDefaults(uint nbStages)
{
	// Enable / disable.
	_CurBlend= false;
	_CurCullFace= true;
	_CurAlphaTest= false;
	_CurLighting= false;
	_CurZWrite= true;
	// setup GLStates.
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_LIGHTING);
	glDepthMask(GL_TRUE);

	// Func.
	_CurBlendSrc= GL_SRC_ALPHA;
	_CurBlendDst= GL_ONE_MINUS_SRC_ALPHA;
	_CurDepthFunc= GL_LEQUAL;
	_CurAlphaTestThreshold= 0.5f;
	// setup GLStates.
	glBlendFunc(_CurBlendSrc, _CurBlendDst);
	glDepthFunc(_CurDepthFunc);
	glAlphaFunc(GL_GREATER, _CurAlphaTestThreshold);

	// Materials.
	uint32			packedOne= (CRGBA(255,255,255,255)).getPacked();
	uint32			packedZero= (CRGBA(0,0,0,255)).getPacked();
	_CurEmissive= packedZero;
	_CurAmbient= packedOne;
	_CurDiffuse= packedOne;
	_CurSpecular= packedZero;
	_CurShininess= 1;

	// Lighted vertex color
	_VertexColorLighted=false;
	glDisable(GL_COLOR_MATERIAL);

	// setup GLStates.
	static const GLfloat		one[4]= {1,1,1,1};
	static const GLfloat		zero[4]= {0,0,0,1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, zero);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, one);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, one);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, zero);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, _CurShininess);
	

	// TexModes
	uint stage;
	for(stage=0;stage<nbStages; stage++)
	{
		// disable texturing.
		glActiveTextureARB(GL_TEXTURE0_ARB+stage);
		glDisable(GL_TEXTURE_2D);
		if(_TextureCubeMapSupported)
			glDisable(GL_TEXTURE_CUBE_MAP_ARB);
		_TextureMode[stage]= TextureDisabled;

		// Tex gen init
		_TexGen[stage] = false;
		_TexGenMode[stage] = GL_SPHERE_MAP;
		glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glDisable( GL_TEXTURE_GEN_S );
		glDisable( GL_TEXTURE_GEN_T );
		glDisable( GL_TEXTURE_GEN_R );
	}

	// ActiveTexture current texture to 0.
	glActiveTextureARB(GL_TEXTURE0_ARB);
	_CurrentActiveTextureARB= 0;
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	_CurrentClientActiveTextureARB= 0;

	// Depth range
	_CurZRangeDelta = 0;
	glDepthRange (0, 1);
}



// ***************************************************************************
void			CDriverGLStates::enableBlend(uint enable)
{
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enabled != _CurBlend )
#endif
	{
		// new state.
		_CurBlend= enabled;
		// Setup GLState.
		if(_CurBlend)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	}
}

// ***************************************************************************
void			CDriverGLStates::enableCullFace(uint enable)
{
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enabled != _CurCullFace )
#endif
	{
		// new state.
		_CurCullFace= enabled;
		// Setup GLState.
		if(_CurCullFace)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
	}
}

// ***************************************************************************
void			CDriverGLStates::enableAlphaTest(uint enable)
{
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enabled != _CurAlphaTest )
#endif
	{
		// new state.
		_CurAlphaTest= enabled;

		// Setup GLState.
		if(_CurAlphaTest)
		{
			glEnable(GL_ALPHA_TEST);
		}
		else
		{
			glDisable(GL_ALPHA_TEST);
		}
	}
}

// ***************************************************************************
void			CDriverGLStates::enableLighting(uint enable)
{
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enabled != _CurLighting )
#endif
	{
		// new state.
		_CurLighting= enabled;
		// Setup GLState.
		if(_CurLighting)
			glEnable(GL_LIGHTING);
		else
			glDisable(GL_LIGHTING);
	}
}

// ***************************************************************************
void			CDriverGLStates::enableZWrite(uint enable)
{
	// If different from current setup, update.
	bool	enabled= (enable!=0);
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enabled != _CurZWrite )
#endif
	{
		// new state.
		_CurZWrite= enabled;
		// Setup GLState.
		if(_CurZWrite)
			glDepthMask(GL_TRUE);
		else
			glDepthMask(GL_FALSE);
	}
}



// ***************************************************************************
void			CDriverGLStates::blendFunc(GLenum src, GLenum dst)
{
	// If different from current setup, update.
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( src!= _CurBlendSrc || dst!=_CurBlendDst )
#endif
	{
		// new state.
		_CurBlendSrc= src;
		_CurBlendDst= dst;
		// Setup GLState.
		glBlendFunc(_CurBlendSrc, _CurBlendDst);
	}
}

// ***************************************************************************
void			CDriverGLStates::depthFunc(GLenum zcomp)
{
	// If different from current setup, update.
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( zcomp != _CurDepthFunc )
#endif
	{
		// new state.
		_CurDepthFunc= zcomp;
		// Setup GLState.
		glDepthFunc(_CurDepthFunc);
	}
}


// ***************************************************************************
void			CDriverGLStates::alphaFunc(float threshold)
{
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if(threshold != _CurAlphaTestThreshold)
#endif
	{
		// new state
		_CurAlphaTestThreshold= threshold;
		// setup function.
		glAlphaFunc(GL_GREATER, _CurAlphaTestThreshold);
	}
}


// ***************************************************************************
void			CDriverGLStates::setEmissive(uint32 packedColor, const GLfloat color[4])
{
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( packedColor!=_CurEmissive )
#endif
	{
		_CurEmissive= packedColor;
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, color);
	}
}

// ***************************************************************************
void			CDriverGLStates::setAmbient(uint32 packedColor, const GLfloat color[4])
{
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( packedColor!=_CurAmbient )
#endif
	{
		_CurAmbient= packedColor;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);
	}
}

// ***************************************************************************
void			CDriverGLStates::setDiffuse(uint32 packedColor, const GLfloat color[4])
{
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( packedColor!=_CurDiffuse )
#endif
	{
		_CurDiffuse= packedColor;
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
	}
}

// ***************************************************************************
void			CDriverGLStates::setSpecular(uint32 packedColor, const GLfloat color[4])
{
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( packedColor!=_CurSpecular )
#endif
	{
		_CurSpecular= packedColor;
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
	}
}

// ***************************************************************************
void			CDriverGLStates::setShininess(float shin)
{
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( shin != _CurShininess )
#endif
	{
		_CurShininess= shin;
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shin);
	}
}


// ***************************************************************************
void			CDriverGLStates::setVertexColorLighted(bool enable)
{
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if( enable != _VertexColorLighted)
#endif
	{
		_VertexColorLighted= enable;

		if (_VertexColorLighted)
		{
			glEnable (GL_COLOR_MATERIAL);
			glColorMaterial (GL_FRONT_AND_BACK, GL_DIFFUSE);
		}
		else
		{
			glDisable (GL_COLOR_MATERIAL);
		}
	}
}

// ***************************************************************************
void		CDriverGLStates::setDepthRange (float zDelta)
{
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (zDelta != _CurZRangeDelta)
#endif
	{		
		_CurZRangeDelta = zDelta;

		// Setup the range
		glDepthRange (zDelta, 1+zDelta);
	}
}

// ***************************************************************************
void		CDriverGLStates::enableTexGen (uint stage, bool enable)
{
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (enable != _TexGen[stage])
#endif
	{		
		_TexGen[stage] = enable;

		// Enable the tex gen
		if (_TexGen[stage])
		{
			glEnable( GL_TEXTURE_GEN_S );
			glEnable( GL_TEXTURE_GEN_T );
			glEnable( GL_TEXTURE_GEN_R );
		}
		else
		{
			glDisable( GL_TEXTURE_GEN_S );
			glDisable( GL_TEXTURE_GEN_T );
			glDisable( GL_TEXTURE_GEN_R );
		}
	}
}

// ***************************************************************************
void		CDriverGLStates::setTexGenMode (uint stage, GLint mode)
{
#ifndef NL3D_GLSTATE_DISABLE_CACHE
	if (mode != _TexGenMode[stage])
#endif
	{		
		_TexGenMode[stage] = mode;

		glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, mode);
		glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, mode);
		glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, mode);
	}
}




// ***************************************************************************
void			CDriverGLStates::resetTextureMode()
{
	glDisable(GL_TEXTURE_2D);
	if (_TextureCubeMapSupported)
	{
		glDisable(GL_TEXTURE_CUBE_MAP_ARB);
	}
	_TextureMode[_CurrentActiveTextureARB]= TextureDisabled;
}


// ***************************************************************************
void			CDriverGLStates::setTextureMode(TTextureMode texMode)
{	
	TTextureMode	oldTexMode = _TextureMode[_CurrentActiveTextureARB];
	if(oldTexMode != texMode)
	{
		// Disable first old mode.
		if(oldTexMode == Texture2D)
			glDisable(GL_TEXTURE_2D);
		else if(oldTexMode == TextureCubeMap)
		{
			if(_TextureCubeMapSupported)
				glDisable(GL_TEXTURE_CUBE_MAP_ARB);
			else
				glDisable(GL_TEXTURE_2D);
		}

		// Enable new mode.
		if(texMode == Texture2D)
			glEnable(GL_TEXTURE_2D);
		else if(texMode == TextureCubeMap)
		{
			if(_TextureCubeMapSupported)
				glEnable(GL_TEXTURE_CUBE_MAP_ARB);
			else
				glDisable(GL_TEXTURE_2D);
		}

		// new mode.
		_TextureMode[_CurrentActiveTextureARB]= texMode;
	}
}


// ***************************************************************************
void			CDriverGLStates::activeTextureARB(uint stage)
{
	if( _CurrentActiveTextureARB != stage )
	{
		glActiveTextureARB(GL_TEXTURE0_ARB+stage);
		_CurrentActiveTextureARB= stage;
	}
}

// ***************************************************************************
void			CDriverGLStates::forceActiveTextureARB(uint stage)
{	
	glActiveTextureARB(GL_TEXTURE0_ARB+stage);
	_CurrentActiveTextureARB= stage;	
}


// ***************************************************************************
void			CDriverGLStates::enableVertexArray(bool enable)
{
	if(_VertexArrayEnabled != enable)
	{
		if(enable)
			glEnableClientState(GL_VERTEX_ARRAY);
		else
			glDisableClientState(GL_VERTEX_ARRAY);
		_VertexArrayEnabled= enable;
	}
}
// ***************************************************************************
void			CDriverGLStates::enableNormalArray(bool enable)
{
	if(_NormalArrayEnabled != enable)
	{
		if(enable)
			glEnableClientState(GL_NORMAL_ARRAY);
		else
			glDisableClientState(GL_NORMAL_ARRAY);
		_NormalArrayEnabled= enable;
	}
}
// ***************************************************************************
void			CDriverGLStates::enableWeightArray(bool enable)
{
	if(_WeightArrayEnabled != enable)
	{
		if(enable)
			glEnableClientState(GL_VERTEX_WEIGHTING_EXT);
		else
			glDisableClientState(GL_VERTEX_WEIGHTING_EXT);
		_WeightArrayEnabled= enable;
	}
}
// ***************************************************************************
void			CDriverGLStates::enableColorArray(bool enable)
{
	if(_ColorArrayEnabled != enable)
	{
		if(enable)
			glEnableClientState(GL_COLOR_ARRAY);
		else
			glDisableClientState(GL_COLOR_ARRAY);
		_ColorArrayEnabled= enable;
	}
}


// ***************************************************************************
void			CDriverGLStates::enableSecondaryColorArray(bool enable)
{
	if(_SecondaryColorArrayEnabled != enable)
	{
		if(enable)
			glEnableClientState(GL_SECONDARY_COLOR_ARRAY_EXT);
		else
			glDisableClientState(GL_SECONDARY_COLOR_ARRAY_EXT);
		_SecondaryColorArrayEnabled= enable;
	}
}

// ***************************************************************************
void			CDriverGLStates::clientActiveTextureARB(uint stage)
{
	if( _CurrentClientActiveTextureARB != stage )
	{
		glClientActiveTextureARB(GL_TEXTURE0_ARB+stage);
		_CurrentClientActiveTextureARB= stage;
	}
}

// ***************************************************************************
void			CDriverGLStates::enableTexCoordArray(bool enable)
{
	if(_TexCoordArrayEnabled[_CurrentClientActiveTextureARB] != enable)
	{
		if(enable)
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		else
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		_TexCoordArrayEnabled[_CurrentClientActiveTextureARB]= enable;
	}
}
// ***************************************************************************
void			CDriverGLStates::enableVertexAttribArray(uint glIndex, bool enable)
{
	if(_VertexAttribArrayEnabled[glIndex] != enable)
	{
		if(enable)
			glEnableClientState(glIndex+GL_VERTEX_ATTRIB_ARRAY0_NV);
		else
			glDisableClientState(glIndex+GL_VERTEX_ATTRIB_ARRAY0_NV);
		_VertexAttribArrayEnabled[glIndex]= enable;
	}
}


} // NL3D
