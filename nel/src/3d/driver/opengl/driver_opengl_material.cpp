/** \file driver_opengl_material.cpp
 * OpenGL driver implementation : setupMaterial
 *
 * $Id: driver_opengl_material.cpp,v 1.55 2002/01/15 15:28:58 berenguier Exp $
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

#include "stdopengl.h"


namespace NL3D {

static void convBlend(CMaterial::TBlend blend, GLenum& glenum)
{
	switch(blend)
	{
		case CMaterial::one:		glenum=GL_ONE; break;
		case CMaterial::zero:		glenum=GL_ZERO; break;
		case CMaterial::srcalpha:	glenum=GL_SRC_ALPHA; break;
		case CMaterial::invsrcalpha:glenum=GL_ONE_MINUS_SRC_ALPHA; break;
		case CMaterial::srccolor:	glenum=GL_SRC_COLOR; break;
		case CMaterial::invsrccolor:glenum=GL_ONE_MINUS_SRC_COLOR; break;
		default: nlstop;
	}
}

static void convZFunction(CMaterial::ZFunc zfunc, GLenum& glenum)
{
	switch(zfunc)
	{
		case CMaterial::lessequal:	glenum=GL_LEQUAL; break;
		case CMaterial::less:		glenum=GL_LESS; break;
		case CMaterial::always:		glenum=GL_ALWAYS; break;
		case CMaterial::never:		glenum=GL_NEVER; break;
		case CMaterial::equal:		glenum=GL_EQUAL; break;
		case CMaterial::notequal:	glenum=GL_NOTEQUAL; break;
		case CMaterial::greater:	glenum=GL_GREATER; break;
		case CMaterial::greaterequal:	glenum=GL_GEQUAL; break;
		default: nlstop;
	}
}

static void	convColor(CRGBA col, GLfloat glcol[4])
{
	static	const float	OO255= 1.0f/255;
	glcol[0]= col.R*OO255;
	glcol[1]= col.G*OO255;
	glcol[2]= col.B*OO255;
	glcol[3]= col.A*OO255;
}

static inline void convTexAddr(ITexture *tex, CMaterial::TTexAddressingMode mode, GLenum &glenum)
{	
	nlassert(mode < CMaterial::TexAddrCount);
	static const GLenum glTex2dAddrModesNV[] =
	{
		GL_NONE, GL_TEXTURE_2D, GL_PASS_THROUGH_NV, GL_CULL_FRAGMENT_NV,
		GL_OFFSET_TEXTURE_2D_NV, GL_OFFSET_TEXTURE_2D_SCALE_NV,
		GL_DEPENDENT_AR_TEXTURE_2D_NV, GL_DEPENDENT_GB_TEXTURE_2D_NV,
		GL_DOT_PRODUCT_NV, GL_DOT_PRODUCT_TEXTURE_2D_NV, GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV,
		GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV, GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV,
		GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV, GL_DOT_PRODUCT_DEPTH_REPLACE_NV
	};


	static const GLenum glTexCubedAddrModesNV[] =
	{
		GL_NONE, GL_TEXTURE_CUBE_MAP_ARB, GL_PASS_THROUGH_NV, GL_CULL_FRAGMENT_NV,
		GL_OFFSET_TEXTURE_2D_NV, GL_OFFSET_TEXTURE_2D_SCALE_NV,
		GL_DEPENDENT_AR_TEXTURE_2D_NV, GL_DEPENDENT_GB_TEXTURE_2D_NV,
		GL_DOT_PRODUCT_NV, GL_DOT_PRODUCT_TEXTURE_2D_NV, GL_DOT_PRODUCT_TEXTURE_CUBE_MAP_NV,
		GL_DOT_PRODUCT_REFLECT_CUBE_MAP_NV, GL_DOT_PRODUCT_CONST_EYE_REFLECT_CUBE_MAP_NV,
		GL_DOT_PRODUCT_DIFFUSE_CUBE_MAP_NV, GL_DOT_PRODUCT_DEPTH_REPLACE_NV
	};

	
	if (!tex || !tex->isTextureCube())
	{	
		glenum = glTex2dAddrModesNV[(uint) mode];
	}
	else
	{
		glenum = glTexCubedAddrModesNV[(uint) mode];
	}
}


// --------------------------------------------------

bool CDriverGL::setupMaterial(CMaterial& mat)
{
	
	CShaderGL*	pShader;
	GLenum		glenum;
	uint32		touched=mat.getTouched();
	sint		stage;


	// profile.
	_NbSetupMaterialCall++;


	// 0. Setup / Bind Textures.
	//==========================
	// Must setup textures each frame. (need to test if touched).
	// Must separate texture setup and texture activation in 2 "for"...
	// because setupTexture() may disable all stage.
	for(stage=0 ; stage<getNbTextureStages() ; stage++)
	{
		ITexture	*text= mat.getTexture(stage);
		if (text != NULL && !setupTexture(*text))
			return(false);
	}
	// Here, for Lightmap materials, setup the lightmaps.
	if(mat.getShader()==CMaterial::LightMap)
	{
		for(stage=0 ; stage<(sint)mat._LightMaps.size() ; stage++)
		{
			ITexture	*text= mat._LightMaps[stage].Texture;
			if (text != NULL && !setupTexture(*text))
				return(false);
		}
	}
	

	// Activate the textures.
	// Do not do it for Lightmap, because done in multipass in a very special fashion.
	// This avoid the useless multiple change of texture states per lightmapped object.
	if(mat.getShader()!=CMaterial::LightMap)
	{
		for(stage=0 ; stage<getNbTextureStages() ; stage++)
		{
			ITexture	*text= mat.getTexture(stage);

			// activate the texture, or disable texturing if NULL.
			activateTexture(stage,text);

			// If texture not NULL, Change texture env fonction.
			//==================================================
			if(text)
			{
				CMaterial::CTexEnv	&env= mat._TexEnvs[stage];

				// Activate the env for this stage.
				// NB: Thoses calls use caching.
				activateTexEnvMode(stage, env);
				activateTexEnvColor(stage, env);

				// Activate texture generation mapping
				_DriverGLStates.activeTextureARB(stage);
				if (mat.getTexCoordGen (stage))
				{
					// Enable it
					_DriverGLStates.enableTexGen (stage, true);

					// Cubic or normal ?
					if (text->isTextureCube ())
						_DriverGLStates.setTexGenMode (stage, GL_REFLECTION_MAP_ARB);
					else
						_DriverGLStates.setTexGenMode (stage, GL_SPHERE_MAP);
				}
				else
				{
					_DriverGLStates.enableTexGen (stage, false);
				}
			}
		}				
	}


		

	// 1. Retrieve/Create driver shader.
	//==================================
	if (!mat.pShader)
	{
		// insert into driver list. (so it is deleted when driver is deleted).
		ItShaderPtrList		it= _Shaders.insert(_Shaders.end());
		// create and set iterator, for future deletion.
		*it= mat.pShader= new CShaderGL(this, it);

		// Must create all OpenGL shader states.
		touched= IDRV_TOUCHED_ALL;
	}
	pShader=static_cast<CShaderGL*>((IShader*)(mat.pShader));


	// 2. Setup modified fields of material.
	//=====================================
	if( touched ) 
	{
		/* Exception: if only Textures are modified in the material, no need to "Bind OpenGL States", or even to test
			for change, because textures are activated alone, see above.
			No problem with delete/new problem (see below), because in this case, IDRV_TOUCHED_ALL is set (see above).
		*/
		// If any flag is set (but a flag of texture)
		if( touched & (~_MaterialAllTextureTouchedFlag) )
		{
			// Convert Material to driver shader.
			if (touched & IDRV_TOUCHED_BLENDFUNC)
			{
				convBlend( mat.getSrcBlend(),glenum );
				pShader->SrcBlend=glenum;
				convBlend( mat.getDstBlend(),glenum );
				pShader->DstBlend=glenum;
			}
			if (touched & IDRV_TOUCHED_ZFUNC)
			{
				convZFunction( mat.getZFunc(),glenum);
				pShader->ZComp= glenum;
			}
			if (touched & IDRV_TOUCHED_LIGHTING)
			{
				if(! (mat.getFlags()&IDRV_MAT_DEFMAT) )
				{
					convColor(mat.getEmissive(), pShader->Emissive);
					convColor(mat.getAmbient(), pShader->Ambient);
					convColor(mat.getDiffuse(), pShader->Diffuse);
					convColor(mat.getSpecular(), pShader->Specular);
					pShader->PackedEmissive= mat.getEmissive().getPacked();
					pShader->PackedAmbient= mat.getAmbient().getPacked();
					pShader->PackedDiffuse= mat.getDiffuse().getPacked();
					pShader->PackedSpecular= mat.getSpecular().getPacked();
				}
			}


			// Since modified, must rebind all openGL states. And do this also for the delete/new problem.
			/* If an old material is deleted, _CurrentMaterial is invalid. But this is grave only if a new 
				material is created, with the same pointer (bad luck). Since an newly allocated material always 
				pass here before use, we are sure to avoid any problems.
			*/
			_CurrentMaterial= NULL;
		}

		// Optimize: reset all flags at the end.
		mat.clearTouched(0xFFFFFFFF);
	}
	

	// 3. Bind OpenGL States.
	//=======================
	if (_CurrentMaterial!=&mat)
	{
		// Bind Blend Part.
		//=================
		_DriverGLStates.enableBlend(mat.getFlags()&IDRV_MAT_BLEND);
		if(mat.getFlags()&IDRV_MAT_BLEND)
			_DriverGLStates.blendFunc(pShader->SrcBlend, pShader->DstBlend);

		// Double Sided Part.
		//===================
		// NB: inverse state: DoubleSided <=> !CullFace.
		uint32	twoSided= mat.getFlags()&IDRV_MAT_DOUBLE_SIDED;
		_DriverGLStates.enableCullFace( twoSided==0 );


		// Alpha Test Part.
		//=================
		uint32	alphaTest= mat.getFlags()&IDRV_MAT_ALPHA_TEST;
		_DriverGLStates.enableAlphaTest(alphaTest);
		if(alphaTest)
		{
			// setup alphaTest threshold.
			_DriverGLStates.alphaFunc(mat.getAlphaTestThreshold());
		}


		// Bind ZBuffer Part.
		//===================
		_DriverGLStates.enableZWrite(mat.getFlags()&IDRV_MAT_ZWRITE);
		_DriverGLStates.depthFunc(pShader->ZComp);
		_DriverGLStates.setDepthRange (mat.getZBias () * _OODeltaZ);


		// Color-Lighting Part.
		//=====================

		// Light Part.
		_DriverGLStates.enableLighting(mat.getFlags()&IDRV_MAT_LIGHTING);
		if(mat.getFlags()&IDRV_MAT_LIGHTING)
		{
			if(mat.getFlags()&IDRV_MAT_DEFMAT)
			{
				static const uint32			packedOne= (CRGBA(255,255,255,255)).getPacked();
				static const uint32			packedZero= (CRGBA(0,0,0,255)).getPacked();
				static const GLfloat		one[4]= {1,1,1,1};
				static const GLfloat		zero[4]= {0,0,0,1};
				_DriverGLStates.setEmissive(packedZero, zero);
				_DriverGLStates.setAmbient(packedOne, one);
				_DriverGLStates.setDiffuse(packedOne, one);
				_DriverGLStates.setSpecular(packedZero, zero);
				_DriverGLStates.setVertexColorLighted(false);
			}
			else
			{
				_DriverGLStates.setEmissive(pShader->PackedEmissive, pShader->Emissive);
				_DriverGLStates.setAmbient(pShader->PackedAmbient, pShader->Ambient);
				_DriverGLStates.setDiffuse(pShader->PackedDiffuse, pShader->Diffuse);
				_DriverGLStates.setSpecular(pShader->PackedSpecular, pShader->Specular);
				_DriverGLStates.setShininess(mat.getShininess());
				_DriverGLStates.setVertexColorLighted(mat.isLightedVertexColor ());
			}
		}
		else
		{
			// Color unlit part.
			CRGBA	col= mat.getColor();
			glColor4ub(col.R, col.G, col.B, col.A);
			_DriverGLStates.setVertexColorLighted(false);
		}
		

		// Texture addressing modes (support only via NVTextureShader for now)
		//===================================================================				
		if (_Extensions.NVTextureShader)
		{
			if ( // supported only with normal shader
				mat.getShader() == CMaterial::Normal 
				&& (mat.getFlags() & IDRV_MAT_TEX_ADDR)
			   )
			{		
				enableNVTextureShader(true);

				GLenum glAddrMode;
				for (stage = 0; stage < getNbTextureStages(); ++stage)
				{										
					convTexAddr(mat.getTexture(stage), (CMaterial::TTexAddressingMode) (mat._TexAddrMode[stage]), glAddrMode);

					if (glAddrMode != _CurrentTexAddrMode[stage]) // addressing mode different from the one in the device?
				
					{
						_DriverGLStates.activeTextureARB(stage);
						glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, glAddrMode);				
						_CurrentTexAddrMode[stage] = glAddrMode;					
					}
				}
				
								
			}
			else
			{
				enableNVTextureShader(false);
			}
		}

		_CurrentMaterial=&mat;
	}



	// Textures user matrix
	//=====================================
	if (
		(_UserTexMatEnabled != 0 && (mat.getFlags() & IDRV_MAT_USER_TEX_MAT_ALL) == 0)		
		|| (mat.getFlags() & IDRV_MAT_USER_TEX_MAT_ALL) != 0
	   )
	{
		glMatrixMode(GL_TEXTURE);
		// for each stage, setup the texture matrix if needed
		uint newMask = (mat.getFlags() & IDRV_MAT_USER_TEX_MAT_ALL) >> IDRV_MAT_USER_TEX_FIRST_BIT;
		uint shiftMask = 1;
		for (uint k = 0; k < (uint) getNbTextureStages(); ++k)
		{
			if (newMask & shiftMask) // user matrix for this stage
			{		
				_DriverGLStates.activeTextureARB(k);
				glLoadMatrixf(mat.getUserTexMat(k).get());
				_UserTexMatEnabled |= shiftMask;
			}
			else
			{
				/// check if matrix disabled
				if (
					(newMask & shiftMask) != (_UserTexMatEnabled & shiftMask)
				   )
				{
					_DriverGLStates.activeTextureARB(k);
					glLoadIdentity();
					_UserTexMatEnabled &= ~shiftMask;
				}				
			}
			shiftMask <<= 1;
		}
		glMatrixMode(GL_MODELVIEW);
	}



	return true;
}


// ***************************************************************************
sint			CDriverGL::beginMultiPass(const CMaterial &mat)
{
	// Depending on material type and hardware, return number of pass required to draw this material.
	switch(mat.getShader())
	{
	case CMaterial::LightMap: 
		return  beginLightMapMultiPass(mat);
	case CMaterial::Specular: 
		return  beginSpecularMultiPass(mat);

	// All others materials require just 1 pass.
	default: return 1;
	}
}
// ***************************************************************************
void			CDriverGL::setupPass(const CMaterial &mat, uint pass)
{
	switch(mat.getShader())
	{
	case CMaterial::LightMap: 
		setupLightMapPass(mat, pass);
		break;
	case CMaterial::Specular: 
		setupSpecularPass(mat, pass);
		break;

	// All others materials do not require multi pass.
	default: return;
	}
}


// ***************************************************************************
void			CDriverGL::endMultiPass(const CMaterial &mat)
{
	switch(mat.getShader())
	{
	case CMaterial::LightMap: 
		endLightMapMultiPass(mat);
		break;
	case CMaterial::Specular: 
		endSpecularMultiPass(mat);
		break;

	// All others materials do not require multi pass.
	default: return;
	}
}


// ***************************************************************************
void CDriverGL::computeLightMapInfos (const CMaterial &mat)
{
	static const uint32 RGBMaskPacked = CRGBA(255,255,255,0).getPacked();

	// For optimisation consideration, suppose there is not too much lightmap.
	nlassert(mat._LightMaps.size()<=NL3D_DRV_MAX_LIGHTMAP);

	// Compute number of lightmaps really used (ie factor not NULL), and build the LUT.
	_NLightMaps = 0;
	// For all lightmaps of the material.
	for (uint i = 0; i < mat._LightMaps.size(); ++i)
	{
		// If the lightmap's factor is not null.
		if (mat._LightMaps[i].Factor.getPacked() & RGBMaskPacked)
		{
			_LightMapLUT[_NLightMaps]= i;
			++_NLightMaps;
		}
	}

	// Compute how many pass, according to driver caps.
	_NLightMapPerPass = getNbTextureStages()-1;
	// Can do more than 2 texture stages only if NVTextureEnvCombine4.
	if (!_Extensions.NVTextureEnvCombine4)
		_NLightMapPerPass = 1;

	// Number of pass.
	_NLightMapPass = (_NLightMaps + _NLightMapPerPass-1)/(_NLightMapPerPass);

	// NB: _NLightMaps==0 means there is no lightmaps at all.
}


// ***************************************************************************
sint CDriverGL::beginLightMapMultiPass (const CMaterial &mat)
{
	// One texture stage hardware not supported.
	if (getNbTextureStages()<2)
		return 1;

	// compute how many lightmap and pass we must process.
	computeLightMapInfos (mat);

	// Too be sure, disable vertex coloring / lightmap.
	_DriverGLStates.enableLighting(false);
	// reset VertexColor array if necessary.
	if (_LastVB.VertexFormat & CVertexBuffer::PrimaryColorFlag)
		_DriverGLStates.enableColorArray(false);

	// Manage too if no lightmaps.
	return	std::max (_NLightMapPass, (uint)1);
}
// ***************************************************************************
void			CDriverGL::setupLightMapPass(const CMaterial &mat, uint pass)
{
	// One texture stage hardware not supported.
	if(getNbTextureStages()<2)
		return;

	// No lightmap??, just setup "replace texture" for stage 0.
	if(_NLightMaps==0)
	{
		ITexture	*text= mat.getTexture(0);
		activateTexture(0,text);

		CMaterial::CTexEnv	env;
		env.Env.OpRGB= CMaterial::Replace;
		env.Env.SrcArg0RGB= CMaterial::Texture;
		env.Env.OpArg0RGB= CMaterial::SrcColor;
		activateTexEnvMode(0, env);
		// Since Lighting is disabled, as well as colorArray, must setup alpha.
		glColor4ub(0, 0, 0, 255);

		// Setup gen tex off
		_DriverGLStates.activeTextureARB(0);
		_DriverGLStates.enableTexGen (0, false);

		// And disable other stages.
		for(sint stage=1 ; stage<getNbTextureStages() ; stage++)
		{
			// disable texturing.
			activateTexture(stage, NULL);
		}

		return;
	}

	nlassert(pass<_NLightMapPass);


	// setup Texture Pass.
	//=========================
	uint	lmapId;
	uint	nstages;
	lmapId= pass * _NLightMapPerPass; // Nb lightmaps already processed
	// N lightmaps for this pass, plus the texture.
	nstages= std::min(_NLightMapPerPass, _NLightMaps-lmapId) + 1;
	// setup all stages.
	for(uint stage= 0; stage<(uint)getNbTextureStages(); stage++)
	{
		// if must setup a lightmap stage.
		if(stage<nstages-1)
		{
			// setup lightMap.
			uint	whichLightMap= _LightMapLUT[lmapId];
			// get text and factor.
			ITexture *text	 = mat._LightMaps[whichLightMap].Texture;
			CRGBA lmapFactor = mat._LightMaps[whichLightMap].Factor;
			lmapFactor.A= 255;

			activateTexture(stage,text);

			// If texture not NULL, Change texture env fonction.
			//==================================================
			if(text)
			{
				CMaterial::CTexEnv	env;

				// NB, !_Extensions.NVTextureEnvCombine4, nstages==2, so here always stage==0.
				if (stage==0)
				{
					// do not use consant color to blend lightmap, but incoming diffuse color, for stage0 only.
					// (NB: lighting and vertexcolorArray are disabled here)
					glColor4ub(lmapFactor.R, lmapFactor.G, lmapFactor.B, 255);

					// Leave stage as default env (Modulate with previous)
					activateTexEnvMode(stage, env);

					// Setup gen tex off
					_DriverGLStates.activeTextureARB(stage);
					_DriverGLStates.enableTexGen (stage, false);
				}
				else
				{
					// Here, we are sure that texEnvCombine4 is OK.
					nlassert(_Extensions.NVTextureEnvCombine4);

					// setup constant color with Lightmap factor.
					env.ConstantColor=lmapFactor;
					activateTexEnvColor(stage, env);

					// setup TexEnvCombine4 (ignore alpha part).
					if(_CurrentTexEnvSpecial[stage] != TexEnvSpecialLightMapNV4)
					{
						// TexEnv is special.
						_CurrentTexEnvSpecial[stage] = TexEnvSpecialLightMapNV4;

						// Setup env for texture stage.
						_DriverGLStates.activeTextureARB(stage);

						// What we want to setup is  Texture*Constant + Previous*1.
						glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE4_NV);

						// Operator.
						glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD );
						glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD );
						// Arg0.
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE );
						glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
						// Arg1.
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_CONSTANT_EXT );
						glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
						// Arg2.
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_PREVIOUS_EXT );
						glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR);
						// Arg3.
						glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_ZERO);
						glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_ONE_MINUS_SRC_COLOR);
					}
				}

				// setup UV, with UV1.
				setupUVPtr(stage, _LastVB, 1);
			}

			// Next lightmap.
			lmapId++;
		}
		else if(stage<nstages)
		{
			// optim: do this only for first pass, and last pass only if stage!=nLMapPerPass 
			// (meaning not the same stage as preceding passes).
			if(pass==0 || (pass==_NLightMapPass-1 && stage!=_NLightMapPerPass))
			{
				// setup texture in final stage.
				ITexture	*text= mat.getTexture(0);
				activateTexture(stage,text);

				// activate the texture at last stage.
				// setup default env (modulate). (this may disable possible COMBINE4_NV setup).
				CMaterial::CTexEnv	env;
				// must set replace for alpha part.
				env.Env.OpAlpha= CMaterial::Replace;
				env.Env.SrcArg0Alpha= CMaterial::Texture;
				env.Env.OpArg0Alpha= CMaterial::SrcAlpha;
				activateTexEnvMode(stage, env);

				// Setup gen tex off
				_DriverGLStates.activeTextureARB(stage);
				_DriverGLStates.enableTexGen (stage, false);

				// setup UV, with UV0.
				setupUVPtr(stage, _LastVB, 0);
			}
		}
		else
		{
			// else all other stages are disabled.
			activateTexture(stage,NULL);
		}
	}


	// setup blend / lighting.
	//=========================
	// Blend is different if the material is blended or not
	if( !mat.getBlend() )
	{
		// Not blended, std case.
		if(pass==0)
		{
			// no transparency for first pass.
			_DriverGLStates.enableBlend(false);
		}
		else if(pass==1)
		{
			// setup an Additive transparency (only for pass 1, will be kept for successives pass).
			_DriverGLStates.enableBlend(true);
			_DriverGLStates.blendFunc(GL_ONE, GL_ONE);
		}
	}
	else
	{
		/* 1st pass, std alphaBlend. 2nd pass, add to background. Demo:
			T: texture.
			l0: lightmap (or group of lightmap) of pass 0.
			l1: lightmap (or group of lightmap) of pass 1. (same thing with 2,3 etc....)
			B:	Background.
			A:	Alpha of texture.

			finalResult= T*(l0+l1) * A + B * (1-A).

			We get it in two pass:
				fint=			T*l0 * A + B * (1-A).
				finalResult=	T*l1 * A + fint = T*l1 * A + T*l0 * A + B * (1-A)=
					T* (l0+l1) * A + B * (1-A)
		*/
		if(pass==0)
		{
			// no transparency for first pass.
			_DriverGLStates.enableBlend(true);
			_DriverGLStates.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else if(pass==1)
		{
			// setup an Additive transparency (only for pass 1, will be kept for successives pass).
			_DriverGLStates.enableBlend(true);
			_DriverGLStates.blendFunc(GL_SRC_ALPHA, GL_ONE);
		}
	}

}
// ***************************************************************************
void			CDriverGL::endLightMapMultiPass(const CMaterial &mat)
{
	// special for all stage, normal UV behavior.
	for(sint i=0; i<getNbTextureStages(); i++)
	{
		// normal behavior: each texture has its own UV.
		setupUVPtr(i, _LastVB, i);
	}

	// pop VertexColor array if necessary.
	if (_LastVB.VertexFormat & CVertexBuffer::PrimaryColorFlag)
		_DriverGLStates.enableColorArray(true);

	// nothing to do with blending/lighting, since always setuped in activeMaterial().
	// If material is the same, then it is still a lightmap material (if changed => touched => different!)
	// So no need to reset lighting/blending here.
}

// ***************************************************************************
sint			CDriverGL::beginSpecularMultiPass(const CMaterial &mat)
{
	_DriverGLStates.activeTextureARB(1);
	glMatrixMode(GL_TEXTURE);
	glLoadMatrixf( _TexMtx.get() );
	glMatrixMode(GL_MODELVIEW);

	if(!_Extensions.ARBTextureCubeMap)
		return 1;

	// One texture stage hardware not supported.
	if(getNbTextureStages()<2)
		return 1;

	if( _Extensions.NVTextureEnvCombine4 ) // NVidia optimization
		return 1;
	else
		return 2;

}
// ***************************************************************************
void			CDriverGL::setupSpecularPass(const CMaterial &mat, uint pass)
{
	// One texture stage hardware not supported.
	if(getNbTextureStages()<2)
		return;

	/// Support NVidia combine 4 extension to do specular map in a single pass
	if( _Extensions.NVTextureEnvCombine4 )
	{	// Ok we can do it in a single pass
		_DriverGLStates.enableBlend(false);

		// Stage 0
		if(_CurrentTexEnvSpecial[0] != TexEnvSpecialSpecularStage0NV4)
		{
			// TexEnv is special.
			_CurrentTexEnvSpecial[0] = TexEnvSpecialSpecularStage0NV4;

			_DriverGLStates.activeTextureARB(0);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE4_NV);
			// Operator.
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD );
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD );
			// Arg0.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR );
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, GL_TEXTURE );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA );
			// Arg1.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PREVIOUS_EXT);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, GL_ZERO );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, GL_ONE_MINUS_SRC_ALPHA );
			// Arg2.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_ZERO );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR );
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, GL_ZERO );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, GL_SRC_ALPHA );
			// Arg3.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_ZERO );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_ALPHA_NV, GL_ZERO );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_ALPHA_NV, GL_SRC_ALPHA );
			// Result RGB : Texture*Diffuse, Alpha : Texture
		}

		// Set Stage 1
		// Special: not the same sepcial env if there is or not texture in stage 0.
		CTexEnvSpecial		newEnvStage1;
		if( mat.getTexture(0) == NULL )
			newEnvStage1= TexEnvSpecialSpecularStage1NoTextNV4;
		else
			newEnvStage1= TexEnvSpecialSpecularStage1NV4;
		// Test if same env as prec.
		if(_CurrentTexEnvSpecial[1] != newEnvStage1)
		{
			// TexEnv is special.
			_CurrentTexEnvSpecial[1] = newEnvStage1;

			_DriverGLStates.activeTextureARB(1);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE4_NV);
			// Operator Add (Arg0*Arg1+Arg2*Arg3)
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD );
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, GL_ADD );
			// Arg0.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, GL_TEXTURE );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR );
			// Arg1.
			if( newEnvStage1 == TexEnvSpecialSpecularStage1NoTextNV4 )
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_ZERO );
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_ONE_MINUS_SRC_COLOR);
			}
			else
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, GL_PREVIOUS_EXT );
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, GL_SRC_ALPHA );
			}
			// Arg2.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, GL_PREVIOUS_EXT );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, GL_SRC_COLOR );
			// Arg3.
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_NV, GL_ZERO );
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_NV, GL_ONE_MINUS_SRC_COLOR);
			// Result : Texture*Previous.Alpha+Previous
		}

		// Setup TexCoord gen for stage1.
		_DriverGLStates.activeTextureARB(1);
		_DriverGLStates.setTextureMode(CDriverGLStates::TextureCubeMap);
		_DriverGLStates.setTexGenMode (1, GL_REFLECTION_MAP_ARB);
		_DriverGLStates.enableTexGen (1, true);
	}
	else
	{ // We have to do it in 2 passes

		if( pass == 0 )
		{ // Just display the texture
			_DriverGLStates.enableBlend(false);
			_DriverGLStates.activeTextureARB(1);
			_DriverGLStates.setTextureMode(CDriverGLStates::TextureDisabled);
		}
		else
		{ // Multiply texture1 by alpha_texture0 and display with add
			_DriverGLStates.enableBlend(true);
			_DriverGLStates.blendFunc(GL_ONE, GL_ONE);

			// Set stage 0
			_DriverGLStates.activeTextureARB(0);
			CMaterial::CTexEnv	env;

			env.Env.OpRGB = CMaterial::Replace;
			env.Env.SrcArg0RGB = CMaterial::Texture;
			env.Env.OpArg0RGB = CMaterial::SrcAlpha;

			activateTexEnvMode(0, env);


			// Set stage 1
			if( mat.getTexture(0) == NULL )
			{
				env.Env.OpRGB = CMaterial::Replace;
				env.Env.SrcArg0RGB = CMaterial::Texture;
				env.Env.OpArg0RGB = CMaterial::SrcColor;
			}
			else
			{
				env.Env.OpRGB = CMaterial::Modulate;
				env.Env.SrcArg0RGB = CMaterial::Texture;
				env.Env.OpArg0RGB = CMaterial::SrcColor;
			
				env.Env.SrcArg1RGB = CMaterial::Previous;
				env.Env.OpArg1RGB = CMaterial::SrcColor;
			}

			activateTexEnvMode(1, env);

			// Set Stage 1
			_DriverGLStates.activeTextureARB(1);
			_DriverGLStates.setTextureMode(CDriverGLStates::TextureCubeMap);
			_DriverGLStates.setTexGenMode (1, GL_REFLECTION_MAP_ARB);
			_DriverGLStates.enableTexGen (1, true);
		}
	}
}
// ***************************************************************************
void			CDriverGL::endSpecularMultiPass(const CMaterial &mat)
{
	// Disable Texture coord generation.
	_DriverGLStates.activeTextureARB(1);
	_DriverGLStates.enableTexGen (1, false);
	//_DriverGLStates.setTextureMode(1, CDriverGLStates::TextureDisabled);

	// Happiness !!! we have already enabled the stage 1
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
}

} // NL3D
