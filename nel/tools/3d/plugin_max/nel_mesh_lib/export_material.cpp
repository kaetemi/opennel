/** \file export_material.cpp
 * Export from 3dsmax to NeL
 *
 * $Id: export_material.cpp,v 1.25 2002/01/04 18:27:30 corvazier Exp $
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

#include "stdafx.h"
#include "export_nel.h"
#include "../tile_utility/tile_utility.h"
#include <3d/texture_file.h>
#include <3d/texture_cube.h>

#include <vector>
#include <string>

using namespace NLMISC;
using namespace NL3D;
using namespace std;

// Name of animatable values
#define BMTEX_CROP_APPLY "apply"
#define BMTEX_CROP_U_NAME "clipu"
#define BMTEX_CROP_V_NAME "clipv"
#define BMTEX_CROP_W_NAME "clipw"
#define BMTEX_CROP_H_NAME "cliph"

#define MAT_SLOTS 2

#define SHADER_NORMAL 1
#define SHADER_BUMP 2
#define SHADER_USER_COLOR 3
#define SHADER_LIGHTMAP 4
#define SHADER_SPECULAR 5
#define SHADER_WATER 6

/** Test wether the given max material is a water material. A water object should only have one material, and must have planar, convex geometry.
  * Morevover, the mapping should only have scale and offsets, no rotation
  */
bool CExportNel::hasWaterMaterial(INode& node, TimeValue time)
{
	// Get primary material pointer of the node
	Mtl* pNodeMat = node.GetMtl();
	// If NULL, no material at all at this node
	if (pNodeMat == NULL) return false;
	if (pNodeMat->NumSubMtls() != 0) return 0; // subMaterials not supported for water

	int bWater = 0; // false
	CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "bWater", (ParamType2)TYPE_BOOL, &bWater, time);
	return bWater != 0;
}





// Build an array of NeL material corresponding with max material at this node. Return the number of material exported.
// Fill an array to remap the 3ds vertexMap channels for each materials. maxBaseBuild.RemapChannel.size() must be == to materials.size(). 
// maxBaseBuild.RemapChannel[mat].size() is the final count of NeL vertexMap channels used for the material n° mat.
// For each NeL channel of a material, copy the 3ds channel maxBaseBuild.RemapChannel[nelChannel]._IndexInMaxMaterial using the transformation matrix
// maxBaseBuild.RemapChannel[nelChannel]._UVMatrix.
// maxBaseBuild.NeedVertexColor will be true if at least one material need vertex color. Forced to true if lightmaps are used.
// maxBaseBuild.AlphaVertex[mat] will be true if the material use per vertex alpha.
// maxBaseBuild.AlphaVertexChannel[mat] will be the channel to use to get the alpha if the material use per vertex alpha.
// This method append the node material to the vector passed.
void CExportNel::buildMaterials (std::vector<NL3D::CMaterial>& materials, CMaxMeshBaseBuild& maxBaseBuild, INode& node, 
								TimeValue time, bool absolutePath)
{
	// Material count
	maxBaseBuild.FirstMaterial=materials.size();
	int nMaterialCount=0;

	// Get primary material pointer of the node
	Mtl* pNodeMat=node.GetMtl();

	// If NULL, no material at all at this node
	if (pNodeMat!=NULL)
	{
		// Number of sub material at in this material
		nMaterialCount=pNodeMat->NumSubMtls();

		// If it is a multisub object, export all its sub materials
		if (nMaterialCount>0)
		{
			// Resize the destination array
			materials.resize (materials.size()+nMaterialCount);

			// Resize the vertMap remap table
			maxBaseBuild.MaterialInfo.resize (nMaterialCount);

			// Export all the sub materials
			for (int nSub=0; nSub<nMaterialCount; nSub++)
			{
				// Get a pointer on the sub material
				Mtl* pSub=pNodeMat->GetSubMtl(nSub);

				// Should not be NULL
				nlassert (pSub);

				// Export it
				buildAMaterial (materials[maxBaseBuild.FirstMaterial+nSub], maxBaseBuild.MaterialInfo[nSub], *pSub, time, absolutePath);

				// Need vertex color ?
				maxBaseBuild.NeedVertexColor |= maxBaseBuild.MaterialInfo[nSub].AlphaVertex | maxBaseBuild.MaterialInfo[nSub].ColorVertex;

				// UV flag
				maxBaseBuild.MappingChannelUsed |= maxBaseBuild.MaterialInfo[nSub].MappingChannelUsed;
			}
		}
		// Else export only this material, so, count is 1
		else
		{
			// Only one material
			nMaterialCount=1;

			// Resize the destination array
			materials.resize (materials.size()+1);

			// Resize the vertMap remap table
			maxBaseBuild.MaterialInfo.resize (1);

			// Export the main material
			buildAMaterial (materials[maxBaseBuild.FirstMaterial], maxBaseBuild.MaterialInfo[0], *pNodeMat, time, absolutePath);

			// Need vertex color ?
			maxBaseBuild.NeedVertexColor |= maxBaseBuild.MaterialInfo[0].AlphaVertex | maxBaseBuild.MaterialInfo[0].ColorVertex;

			// UV flag
			maxBaseBuild.MappingChannelUsed |= maxBaseBuild.MaterialInfo[0].MappingChannelUsed;
		}
	}

	// If no material exported
	if (nMaterialCount==0)
	{
		// Insert at least a material
		materials.resize (materials.size()+1);
		nMaterialCount=1;

		// Resize the vertMap remap table
		maxBaseBuild.MaterialInfo.resize (1);

		// Init the first material
		materials[maxBaseBuild.FirstMaterial].initLighted();

		// Default mat
		materials[maxBaseBuild.FirstMaterial].setLighting(true, true);

		// Export the main material
		maxBaseBuild.MaterialInfo[0].MaterialName = "Default";
	}

	// Return the count of material
	maxBaseBuild.NumMaterials=nMaterialCount;
}

// Build a NeL material corresponding with a max material.
void CExportNel::buildAMaterial (NL3D::CMaterial& material, CMaxMaterialInfo& materialInfo, Mtl& mtl, TimeValue time, bool absolutePath)
{
	// It is a NeL material ?
	if (isClassIdCompatible (mtl, Class_ID(NEL_MTL_A,NEL_MTL_B)))
	{
		// *** Init lighted or unlighted ?

		// Get unlighted flag
		int bUnlighted = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bUnlighted", (ParamType2)TYPE_BOOL, &bUnlighted, 0);

		// Init the material
		if( bUnlighted )
			material.initUnlit ();
		else
			material.initLighted ();

		// *** Choose a shader

		// Get the shader value
		int iShaderType = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "iShaderType", (ParamType2)TYPE_INT, &iShaderType, time);
		switch (iShaderType)
		{
		case SHADER_NORMAL:
		case SHADER_BUMP:
		case SHADER_USER_COLOR:
		case SHADER_LIGHTMAP:
		case SHADER_SPECULAR:
			material.setShader ((CMaterial::TShader)(iShaderType-1));
			break;
		case SHADER_WATER:
			material.setShader (CMaterial::Normal);
			break;
		}

		// Stain glass window flag
		int bStainGlass = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bStainedGlassWindow", (ParamType2)TYPE_BOOL, &bStainGlass, time);		
		material.setStainedGlassWindow (bStainGlass!=0);

		// *** Alpha

		// Alpha blend ?
		int bAlphaBlend = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bAlphaBlend", (ParamType2)TYPE_BOOL, &bAlphaBlend, time);
		
		// Set the blend
		material.setBlend (bAlphaBlend!=0);

		// Blend function
		int blendFunc = 1;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "iBlendSrcFunc", (ParamType2)TYPE_INT, &blendFunc, time);
		material.setSrcBlend ((CMaterial::TBlend)(blendFunc-1));
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "iBlendDestFunc", (ParamType2)TYPE_INT, &blendFunc, time);
		material.setDstBlend ((CMaterial::TBlend)(blendFunc-1));

		// Set the alpha test flag
		int bAlphaTest = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bAlphaTest", (ParamType2)TYPE_BOOL, &bAlphaTest, time);
		material.setAlphaTest (bAlphaTest != 0);

		// Vertex alpha
		int bAlphaVertex = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bAlphaVertex", (ParamType2)TYPE_BOOL, &bAlphaVertex, time);

		// Active vertex alpha if in alpha test or alpha blend mode
		materialInfo.AlphaVertex = (bAlphaBlend || bAlphaTest) && (bAlphaVertex != 0);

		// Get channel to use for alpha vertex
		materialInfo.AlphaVertexChannel = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "iAlphaVertexChannel", (ParamType2)TYPE_INT, &materialInfo.AlphaVertexChannel, time);

		// *** Zbuffer

		// ZBias
		float zBias = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "fZBias", (ParamType2)TYPE_FLOAT, &zBias, time);
		material.setZBias ( zBias );

		// Alpha blend ?
		if (bAlphaBlend)
		{
			// Force z write ?
			int bForceZWrite = 0;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "bForceZWrite", (ParamType2)TYPE_BOOL, &bForceZWrite, time);
			material.setZWrite( bForceZWrite != 0 );
		}
		else
		{
			// Force no z write ?
			int bForceNoZWrite = 0; // false
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "bForceNoZWrite", (ParamType2)TYPE_BOOL, &bForceNoZWrite, time);
			material.setZWrite( bForceNoZWrite == 0 );
		}

		// *** Colors

		// * Vertex color ?

		int bColorVertex = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bColorVertex", (ParamType2)TYPE_BOOL, &bColorVertex, time);
		material.setLightedVertexColor (material.isLighted() && (bColorVertex != 0));

		// Ok, color vertex
		materialInfo.ColorVertex = bColorVertex != 0;


		// * Diffuse

		// Get from max
		Point3 maxDiffuse;
		CRGBA  nelDiffuse;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "diffuse", (ParamType2)TYPE_RGBA, &maxDiffuse, time);

		// Convert to NeL color
		convertColor (nelDiffuse, maxDiffuse);

		// Set the opacity
		float fOp = 0.0f;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "opacity", (ParamType2)TYPE_PCNT_FRAC, &fOp, time);

		// Add alpha to the value
		float fA=(fOp*255.f+0.5f);
		clamp (fA, 0.f, 255.f);
		nelDiffuse.A=(uint8)fA;

		// Set le NeL diffuse color material
		if (bUnlighted)
		{
			material.setColor (nelDiffuse);
		}
		else
		{
			material.setDiffuse (nelDiffuse);
			material.setOpacity (nelDiffuse.A);
		}
		
		// * Self illum color

		CRGBA nelEmissive;
		int bSelfIllumColorOn;
		Point3 maxSelfIllum;
		float fTemp;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "useSelfIllumColor", (ParamType2)TYPE_BOOL, &bSelfIllumColorOn, time);
		if( bSelfIllumColorOn )
		{
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "selfIllumColor", (ParamType2)TYPE_RGBA, &maxSelfIllum, time);
		}
		else
		{
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "selfIllumAmount", (ParamType2)TYPE_PCNT_FRAC, &fTemp, time);
			maxSelfIllum = maxDiffuse * fTemp;
		}
		convertColor( nelEmissive, maxSelfIllum );
		material.setEmissive (nelEmissive);

		// * Ambient

		CRGBA nelAmbient;
		Point3 maxAmbient;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "ambient", (ParamType2)TYPE_RGBA, &maxAmbient, time);
		convertColor (nelAmbient, maxAmbient);
		material.setAmbient (nelAmbient);

		// * Specular

		CRGBA nelSpecular;
		Point3 maxSpecular;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "specular", (ParamType2)TYPE_RGBA, &maxSpecular, time);
		convertColor (nelSpecular, maxSpecular);

		// Get specular level
		float shininess;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "specularLevel", (ParamType2)TYPE_PCNT_FRAC, &shininess, time);
		CRGBAF fColor = nelSpecular;
		fColor *= shininess;
		nelSpecular = fColor;
		material.setSpecular (nelSpecular);

		// * Get specular shininess

		CExportNel::getValueByNameUsingParamBlock2 (mtl, "glossiness", (ParamType2)TYPE_PCNT_FRAC, &shininess, time);
		shininess=(float)pow(2.0, shininess * 10.0) * 4.f;
		material.setShininess (shininess);

		// * Usercolor

		if (iShaderType == SHADER_USER_COLOR)
		{
			CRGBA nelUserColor;
			Point3 maxUserColor;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "cUserColor", (ParamType2)TYPE_RGBA, &maxUserColor, time);
			convertColor (nelUserColor, maxUserColor);
			material.setUserColor (nelUserColor);
		}

		// * Double sided flag
		int bDoubleSided;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "twoSided", (ParamType2)TYPE_BOOL, &bDoubleSided, time);
		material.setDoubleSided ( bDoubleSided!=0 );

		// *** Textures


		/// test wether texture matrix animation should be exported
		int bExportTexMatAnim = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bExportTextureMatrix", (ParamType2)TYPE_BOOL, &bExportTexMatAnim, 0);
		materialInfo.TextureMatrixEnabled = (bExportTexMatAnim != 0);

		// Reset info
		materialInfo.RemapChannel.clear ();

		// For each textures
		uint i;
		for (i=0; i<IDRV_MAT_MAXTEXTURES; i++)
		{
			// Lightmap, only one texture
			if ((iShaderType==SHADER_USER_COLOR) && (i>0)) break;
			if ((iShaderType==SHADER_LIGHTMAP) && (i>0)) break;
			if ((iShaderType==SHADER_SPECULAR) && (i>1)) break;

			// Get the enable flag name
			char enableSlotName[100];
			smprintf (enableSlotName, 100, "bEnableSlot_%d", i+1);

			// Get the enable flag 
			int bEnableSlot = 0;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, enableSlotName, (ParamType2)TYPE_BOOL, &bEnableSlot, time);
			if (bEnableSlot)
			{
				// Get the texture arg name
				char textureName[100];
				smprintf (textureName, 100, "tTexture_%d", i+1);

				// Get the texture pointer
				Texmap *pTexmap = NULL;
				CExportNel::getValueByNameUsingParamBlock2 (mtl, textureName, (ParamType2)TYPE_TEXMAP, &pTexmap, time);
				if (pTexmap)
				{
					// Pointer on the  diffuse texture
					static ITexture* pTexture=NULL;

					// Ok export the texture in NeL format
					CMaterialDesc materialDesc;
					pTexture=buildATexture (*pTexmap, materialDesc, time, absolutePath, (i==1) && (iShaderType==SHADER_SPECULAR) );

					// Get the gen texture coord flag
					char texGenName[100];
					smprintf (texGenName, 100, "bTexGen_%d", i+1);
					int bTexGen = 0;
					CExportNel::getValueByNameUsingParamBlock2 (mtl, texGenName, (ParamType2)TYPE_BOOL, &bTexGen, time);

					// If specular shader, set uv channel to reflexion
					if ( ((i==1) && (iShaderType==SHADER_SPECULAR)) || bTexGen )
					{
						materialDesc._IndexInMaxMaterial = UVGEN_REFLEXION;
					}

					materialInfo.RemapChannel.push_back (materialDesc);
		
					// Add flags if mapping coodinates are used..
					materialInfo.MappingChannelUsed |= (materialDesc._IndexInMaxMaterial>=0)?(1<<i):0;

					// Add the texture if it exist
					material.setTexture (i, pTexture);

					// Envmap gen ?
					material.setTexCoordGen (i, materialDesc._IndexInMaxMaterial<0);

					/// texture matrix animation ?
					if (bExportTexMatAnim != 0)
					{										
						/// and activate flag
						material.enableUserTexMat(i);
						/// setup the uv matrix
						CMatrix uvMat;
						CExportNel::uvMatrix2NelUVMatrix(materialDesc.getUVMatrix(), uvMat);
						material.setUserTexMat(i, uvMat);
					}
				}
			}
		}

		// Export mapping channel 2 if lightmap asked.
		if ( iShaderType==SHADER_LIGHTMAP ) // lightmap enabled ?
		{
			// Get the lightmap UV channel
			int iLightMapChannel = 0;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "iLightMapChannel", (ParamType2)TYPE_INT, &iLightMapChannel, time);

			// Resize it
			uint size = materialInfo.RemapChannel.size();
			materialInfo.RemapChannel.resize( size + 1 );

			// Copy information from channel 0
			if (materialInfo.RemapChannel.size () > 1)
				materialInfo.RemapChannel[size] = materialInfo.RemapChannel[0];

			// Source lightmap mapping channel
			materialInfo.RemapChannel[size]._IndexInMaxMaterial = iLightMapChannel;

			// Backup old mapping channel
			materialInfo.RemapChannel[size]._IndexInMaxMaterialAlternative = materialInfo.RemapChannel[0]._IndexInMaxMaterial;
		}

		// For each slot
		uint stage;
		for (stage=0; stage<std::min ((uint)MAT_SLOTS, (uint)IDRV_MAT_MAXTEXTURES); stage++)
		{
			// Make a post fixe
			char postfixC[10];
			smprintf (postfixC, 10, "_%d", stage+1);
			string postfix = postfixC;

			// Shader normal ?
			if (iShaderType==SHADER_NORMAL)
			{
				// RGB, get the values
				int opRGB = 0;
				int opRGBBlend = 0;
				int opRGBArg0 = 0;
				int opRGBArg1 = 0;
				int opRGBArg0Operand = 0;
				int opRGBArg1Operand = 0;
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iRgbOperation"+postfix).c_str(), (ParamType2)TYPE_INT, &opRGB, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iRgbBlendSource"+postfix).c_str(), (ParamType2)TYPE_INT, &opRGBBlend, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iRgbArg0"+postfix).c_str(), (ParamType2)TYPE_INT, &opRGBArg0, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iRgbArg1"+postfix).c_str(), (ParamType2)TYPE_INT, &opRGBArg1, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iRgbArg0Operand"+postfix).c_str(), (ParamType2)TYPE_INT, &opRGBArg0Operand, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iRgbArg1Operand"+postfix).c_str(), (ParamType2)TYPE_INT, &opRGBArg1Operand, time);

				// Setup the value
				if (opRGB<5)
					material.texEnvOpRGB (stage, (CMaterial::TTexOperator)(opRGB-1));
				else
					material.texEnvOpRGB (stage, (CMaterial::TTexOperator)(opRGBBlend+3));
				material.texEnvArg0RGB (stage, (CMaterial::TTexSource)(opRGBArg0-1), (CMaterial::TTexOperand)(opRGBArg0Operand-1));
				material.texEnvArg1RGB (stage, (CMaterial::TTexSource)(opRGBArg1), (CMaterial::TTexOperand)(opRGBArg1Operand-1));

				// Alpha, get the values
				int opAlpha = 0;
				int opAlphaBlend = 0;
				int opAlphaArg0 = 0;
				int opAlphaArg1 = 0;
				int opAlphaArg0Operand = 0;
				int opAlphaArg1Operand = 0;
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iAlphaOperation"+postfix).c_str(), (ParamType2)TYPE_INT, &opAlpha, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iAlphaBlendSource"+postfix).c_str(), (ParamType2)TYPE_INT, &opAlphaBlend, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iAlphaArg0"+postfix).c_str(), (ParamType2)TYPE_INT, &opAlphaArg0, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iAlphaArg1"+postfix).c_str(), (ParamType2)TYPE_INT, &opAlphaArg1, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iAlphaArg0Operand"+postfix).c_str(), (ParamType2)TYPE_INT, &opAlphaArg0Operand, time);
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iAlphaArg1Operand"+postfix).c_str(), (ParamType2)TYPE_INT, &opAlphaArg1Operand, time);

				// Setup the value
				if (opAlpha<5)
					material.texEnvOpAlpha (stage, (CMaterial::TTexOperator)(opAlpha-1));
				else
					material.texEnvOpAlpha (stage, (CMaterial::TTexOperator)(opAlphaBlend+3));
				material.texEnvArg0Alpha (stage, (CMaterial::TTexSource)(opAlphaArg0-1), (CMaterial::TTexOperand)(opAlphaArg0Operand-1));
				material.texEnvArg1Alpha (stage, (CMaterial::TTexSource)(opAlphaArg1), (CMaterial::TTexOperand)(opAlphaArg1Operand-1));
		
				// Constant color
				Point3 constantColor;
				CRGBA  nelConstantColor;
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("cConstant"+postfix).c_str(), (ParamType2)TYPE_RGBA, &constantColor, time);

				// Convert to NeL color
				convertColor (nelConstantColor, constantColor);
		
				// Constant alpha
				int constantAlpha = 255;
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iConstantAlpha"+postfix).c_str(), (ParamType2)TYPE_INT, &constantAlpha, time);
				nelConstantColor.A = (uint8)constantAlpha;

				// Setup the constant color
				material.texConstantColor(stage, nelConstantColor);

				// Texenv mode
				int texEnvMode = 0;
				CExportNel::getValueByNameUsingParamBlock2 (mtl, ("iTextureShader"+postfix).c_str(), (ParamType2)TYPE_INT, &texEnvMode, time);
				if (texEnvMode>1)
				{
					// Enable and setup it
					material.enableTexAddrMode ();
					material.setTexAddressingMode (stage, (CMaterial::TTexAddressingMode)(texEnvMode-2));
				}
			}
		}

		// Set material name		
		TSTR name=mtl.GetName();
		materialInfo.MaterialName = (const char*)name;
	}
	else
	{
		// Not a nel material, try to get something near...

		// Init the material lighted
		material.initLighted ();

		// *** ***************
		// *** Export Textures
		// *** ***************

		/// TODO: Only one texture for the time. Add multitexture support, and shaders support.

		// Look for a diffuse texmap
		vector<bool> mapEnables;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "mapEnables", (ParamType2)TYPE_BOOL_TAB, &mapEnables, time);

		Texmap *pDifTexmap = NULL;
		Texmap *pOpaTexmap = NULL;
		Texmap *pSpeTexmap = NULL;

		if (mapEnables[ID_DI])
			pDifTexmap = mtl.GetSubTexmap (ID_DI);
		if (mapEnables[ID_OP])
			pOpaTexmap = mtl.GetSubTexmap (ID_OP);
		if (mapEnables[ID_SP])
			pSpeTexmap = mtl.GetSubTexmap (ID_SP);

		// Is there a lightmap handling wanted
		int bLightMap = 0; // false
		int bAlphaTest = 1; // true
		int bForceZWrite = 0; // false
		int bForceNoZWrite = 0; // false

		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bForceZWrite", (ParamType2)TYPE_BOOL, &bForceZWrite, time);
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bForceNoZWrite", (ParamType2)TYPE_BOOL, &bForceNoZWrite, time);

		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bAlphaTest", (ParamType2)TYPE_BOOL, &bAlphaTest, time);

		if( pSpeTexmap != NULL )
		{
			material.setShader (CMaterial::Specular);
		}
		else
		{
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "bLightMap", (ParamType2)TYPE_BOOL, &bLightMap, time);		
			if (bLightMap)
			{
				material.setShader (CMaterial::LightMap);
			}
			else
			{
				material.setShader (CMaterial::Normal);
			}
		}

		int bStainedGlassWindow = 0;
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bStainedGlassWindow", (ParamType2)TYPE_BOOL, &bStainedGlassWindow, time);
		material.setStainedGlassWindow( bStainedGlassWindow!=0 );

		material.setAlphaTest(false);

		// By default set blend to false
		material.setBlend (false);

		// Diffuse texmap is present ?
		if (pDifTexmap)
		{
			// Pointer on the  diffuse texture
			static ITexture* pTexture=NULL;

			// Is it a simple file ?
			if (isClassIdCompatible(*pDifTexmap, Class_ID (BMTEX_CLASS_ID,0)))
			{
				// List of channels used by this texture
				CMaterialDesc materialDesc;
				
				// Ok export the texture in NeL format
				pTexture=buildATexture (*pDifTexmap, materialDesc, time, absolutePath);

				// For this shader, only need a texture channel.
				materialInfo.RemapChannel.push_back (materialDesc);
	
				// Add flags if mapping coodinates are used..
				materialInfo.MappingChannelUsed |= (materialDesc._IndexInMaxMaterial>=0)?1:0;

				// Add the texture if it exist
				material.setTexture(0, pTexture);

				// Active blend if texture in opacity
				if( pOpaTexmap!=NULL )
				{
					if( bAlphaTest )
					{ // If Alpha Test enabled no blend required just check if we are forced to NOT write in the ZBuffer
						material.setAlphaTest(true);
						if( bForceNoZWrite )
							material.setZWrite( false );
						else
							material.setZWrite( true );
					}
					else
					{ // No Alpha Test so we have to blend and check if we are forced to write in the ZBuffer
						material.setBlend( true );
						if( bForceZWrite )
							material.setZWrite( true );
						else
							material.setZWrite( false );
					}
				}
				

				// Export mapping channel 2 if lightmap asked.
				if( bLightMap ) // lightmap enabled ?
				{
					materialInfo.RemapChannel.resize( 2 );
					// Copy information from channel 0
					materialInfo.RemapChannel[1] = materialInfo.RemapChannel[0];
					materialInfo.RemapChannel[1]._IndexInMaxMaterial = 2;
					materialInfo.RemapChannel[1]._IndexInMaxMaterialAlternative = materialInfo.RemapChannel[0]._IndexInMaxMaterial;
				}
			}
		}

		if (pSpeTexmap)
		{
			// Pointer on the  diffuse texture
			static ITexture* pTexture=NULL;
			static CTextureCube* pTextureCube=NULL;

			pTextureCube = new CTextureCube;
			pTextureCube->setWrapS( ITexture::Clamp );
			pTextureCube->setWrapT( ITexture::Clamp );
			pTextureCube->setFilterMode(ITexture::Linear,ITexture::LinearMipMapOff);

			if( isClassIdCompatible(*pSpeTexmap, Class_ID (ACUBIC_CLASS_ID,0)) )
			{
				CTextureCube::TFace tfNewOrder[6] = {	CTextureCube::positive_z, CTextureCube::negative_z,
														CTextureCube::negative_x, CTextureCube::positive_x,
														CTextureCube::negative_y, CTextureCube::positive_y	};
				vector<string> names;
				CExportNel::getValueByNameUsingParamBlock2 (mtl, "bitmapName", (ParamType2)TYPE_STRING_TAB, &names, time);
				for( int i = 0; i< (int)names.size(); ++i )
				{
					CTextureFile *pT = new CTextureFile;

					char sFileName[512];
					strcpy(sFileName, names[i].c_str());
					if (!absolutePath)
					{
						// Decompose bitmap file name
						char sName[256];
						char sExt[256];
						_splitpath (sFileName, NULL, NULL, sName, sExt);
						// Make the final path
						_makepath (sFileName, NULL, NULL, sName, sExt);
					}

					// Set the file name
					pT->setFileName(sFileName);

					pTextureCube->setTexture(tfNewOrder[i], pT);
				}
			}
			else
			if( isClassIdCompatible(*pSpeTexmap, Class_ID (COMPOSITE_CLASS_ID,0)) )
			{
				int nNbSubMap = pSpeTexmap->NumSubTexmaps();
				if( nNbSubMap > 6 )
					nNbSubMap = 6;	
				for( int i = 0; i < nNbSubMap; ++i )
				{
					std::vector<CMaterialDesc> _3dsTexChannel;
					Texmap *pSubMap = pSpeTexmap->GetSubTexmap(i);

					if( pSubMap != NULL )
					if (isClassIdCompatible(*pSubMap, Class_ID (BMTEX_CLASS_ID,0)))
					{					
						CMaterialDesc _3dsTexChannel;
						pTexture = buildATexture (*pSubMap, _3dsTexChannel, time, absolutePath);
						pTextureCube->setTexture((CTextureCube::TFace)i, pTexture);
					}
				}
			}
			else
			// Is it a simple file ?
			if (isClassIdCompatible(*pSpeTexmap, Class_ID (BMTEX_CLASS_ID,0)))
			{
				// List of channels used by this texture
				CMaterialDesc _3dsTexChannel;
				
				// Ok export the texture in NeL format
				pTexture = buildATexture (*pSpeTexmap, _3dsTexChannel, time, absolutePath);
				pTextureCube->setTexture(CTextureCube::positive_x, pTexture);
			}
			// Add the texture if it exist
			material.setTexture(1, pTextureCube);
		}

		// Blend mode
		int opacityType = 0; // 0-filter 1-substractive 2-additive
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "opacityType", (ParamType2)TYPE_INT, &opacityType, time);
		if( opacityType == 0 ) // Filter
			material.setBlendFunc (CMaterial::srcalpha, CMaterial::invsrcalpha);
		else
			material.setBlendFunc (CMaterial::srcalpha, CMaterial::one);

		// Z function by default. TODO.
		material.setZFunc (CMaterial::lessequal);

		// Z bias by default. TODO.
		material.setZBias (0.f);

		// Is the mtl a std material ?
		// The class Id can be the StdMat one, or StdMat2
		// It can be the superClassId if the mtl is derived from StdMat or StdMat2.
		if (
			isClassIdCompatible (mtl, Class_ID(DMTL_CLASS_ID, 0))	||
			isClassIdCompatible (mtl, Class_ID(DMTL2_CLASS_ID, 0))
			)
		{
			// Get a pointer on a stdmat
			//StdMat2* stdmat=(StdMat2*)&mtl;

			// *****************************************
			// *** Colors, self illumination and opacity
			// *****************************************

			// Get the diffuse color of the max material
			//Color color=stdmat->GetDiffuse (time);
			Point3 maxDiffuse;
			CRGBA  nelDiffuse;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "diffuse", (ParamType2)TYPE_RGBA, &maxDiffuse, time);

			// Convert to NeL color
			convertColor (nelDiffuse, maxDiffuse);
			// Get the opacity value from the material
			// float fOp=stdmat->GetOpacity (time);
			float fOp = 0.0f;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "opacity", (ParamType2)TYPE_PCNT_FRAC, &fOp, time);

			// Add alpha to the value
			float fA=(fOp*255.f+0.5f);
			clamp (fA, 0.f, 255.f);
			nelDiffuse.A=(uint8)fA;
			// Set le NeL diffuse color material
			material.setColor (nelDiffuse);

			// Set the blend mode on if opacity is not 1.f
			if( fOp < 0.99f )
			{
				if( bAlphaTest )
				{ // If Alpha Test enabled no blend required just check if we are forced to NOT write in the ZBuffer
					material.setAlphaTest(true);
					if( bForceNoZWrite )
						material.setZWrite( false );
					else
						material.setZWrite( true );
				}
				else
				{ // No Alpha Test so we have to blend and check if we are forced to write in the ZBuffer
					material.setBlend( true );
					if( bForceZWrite )
						material.setZWrite( true );
					else
						material.setZWrite( false );
				}
			}

			// Get colors of 3dsmax material
			CRGBA nelEmissive;
			CRGBA nelAmbient;
			CRGBA nelSpecular;
			//if (stdmat->GetSelfIllumColorOn())
			//	convertColor (emissiveColor, stdmat->GetSelfIllumColor (time));
			//else
			//	convertColor (emissiveColor, stdmat->GetDiffuse (time)*stdmat->GetSelfIllum (time));
			int bSelfIllumColorOn;
			Point3 maxSelfIllum;
			float fTemp;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "useSelfIllumColor", (ParamType2)TYPE_BOOL, &bSelfIllumColorOn, time);
			if( bSelfIllumColorOn )
			{
				CExportNel::getValueByNameUsingParamBlock2 (mtl, "selfIllumColor", (ParamType2)TYPE_RGBA, &maxSelfIllum, time);
			}
			else
			{
				CExportNel::getValueByNameUsingParamBlock2 (mtl, "selfIllumAmount", (ParamType2)TYPE_PCNT_FRAC, &fTemp, time);
				maxSelfIllum = maxDiffuse * fTemp;
			}
			convertColor( nelEmissive, maxSelfIllum );

			Point3 maxAmbient;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "ambient", (ParamType2)TYPE_RGBA, &maxAmbient, time);
			convertColor (nelAmbient, maxAmbient);

			Point3 maxSpecular;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "specular", (ParamType2)TYPE_RGBA, &maxSpecular, time);
			convertColor (nelSpecular, maxSpecular);

			// Specular level
			float shininess; //=stdmat->GetShinStr(time);
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "specularLevel", (ParamType2)TYPE_PCNT_FRAC, &shininess, time);
			CRGBAF fColor = nelSpecular;
			fColor *= shininess;
			nelSpecular = fColor;

			// Shininess
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "glossiness", (ParamType2)TYPE_PCNT_FRAC, &shininess, time);
			//shininess=stdmat->GetShader()->GetGlossiness(time);
			shininess=(float)pow(2.0, shininess * 10.0) * 4.f;

			// Light parameters
			material.setLighting (true, false, nelEmissive, nelAmbient, nelDiffuse, nelSpecular, shininess);

			// Double sided
			int bDoubleSided;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "twoSided", (ParamType2)TYPE_BOOL, &bDoubleSided, time);

			//material.setDoubleSided (stdmat->GetTwoSided()!=FALSE);
			material.setDoubleSided ( bDoubleSided!=0 );
		}

		if( ! bLightMap )
		{
			int bUnlighted = 0; // false
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "bUnlighted", (ParamType2)TYPE_BOOL, &bUnlighted, 0);
			if( bUnlighted )	
			{
				material.setLighting( false );
			}
		}

		// Use alpha vertex ?
		int bAlphaVertex = 0; // false

		// Get the scripted value
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bAlphaVertex", (ParamType2)TYPE_BOOL, &bAlphaVertex, time);

		// Find ?
		if ( bAlphaVertex )
		{
			// Ok, alpha vertex
			materialInfo.AlphaVertex = true;

			if( bAlphaTest )
			{ 
				// If Alpha Test enabled no blend required just check if we are forced to NOT write in the ZBuffer
				material.setAlphaTest(true);
				if( bForceNoZWrite )
					material.setZWrite( false );
				else
					material.setZWrite( true );
			}
			else
			{ // No Alpha Test so we have to blend and check if we are forced to write in the ZBuffer
				material.setBlend( true );
				if( bForceZWrite )
					material.setZWrite( true );
				else
					material.setZWrite( false );
			}

			// Get the channel for alpha vertex
			materialInfo.AlphaVertexChannel = 0;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, "iAlphaVertexChannel", (ParamType2)TYPE_INT, &materialInfo.AlphaVertexChannel, time);
		}

		// Use color vertex ?
		int bColorVertex = 0; // false

		// Get the scripted value
		CExportNel::getValueByNameUsingParamBlock2 (mtl, "bColorVertex", (ParamType2)TYPE_BOOL, &bColorVertex, time);

		// Find ?
		if ( bColorVertex )
		{
			// Ok, color vertex
			materialInfo.ColorVertex = true;

			// Active vertex color in lighted mode
			material.setLightedVertexColor (material.isLighted());
		}

		// Set material name		
		TSTR name=mtl.GetName();
		materialInfo.MaterialName = (const char*)name;
	}
}

// Get 3ds channels uv used by a texmap and make a good index channel
int CExportNel::getVertMapChannel (Texmap& texmap, Matrix3& channelMatrix, TimeValue time)
{
	// *** Get the channel matrix

	// Set to identity because deafult implementation of texmapGetUVTransform make nothing
	channelMatrix.IdentityMatrix();

	// Get UV channel matrix
	texmap.GetUVTransform(channelMatrix);

	// Return the map channel
	return texmap.GetMapChannel();
}
	
// Build a NeL texture corresponding with a max Texmap.
// Fill an array with the 3ds vertexMap used by this texture. 
// Texture file uses only 1 channel.
ITexture* CExportNel::buildATexture (Texmap& texmap, CMaterialDesc &remap3dsTexChannel, TimeValue time, bool absolutePath, bool forceCubic)
{
	/// TODO: support other texmap than Bitmap

	// By default, not build
	ITexture* pTexture=NULL;

	// Is it a bitmap texture file ?
	if (isClassIdCompatible(texmap, Class_ID (BMTEX_CLASS_ID,0)))
	{
		// Cast the pointer
		BitmapTex* pBitmap=(BitmapTex*)&texmap;

		// File name, maxlen 256 under windows
		char sFileName[512];
		strcpy (sFileName, pBitmap->GetMapName());

		// Let absolute path ?
		if (!absolutePath)
		{
			// Decompose bitmap file name
			char sName[256];
			char sExt[256];
			_splitpath (sFileName, NULL, NULL, sName, sExt);

			// Make the final path
			_makepath (sFileName, NULL, NULL, sName, sExt);
		}

		// Get the apply crop value
		int bApply;
		bool bRes=getValueByNameUsingParamBlock2 (texmap, BMTEX_CROP_APPLY, (ParamType2)TYPE_BOOL, &bApply, time);
		nlassert (bRes);

		// If a crop is applyed
		if (bApply)
		{
			// Get the crop value U
			bRes=getValueByNameUsingParamBlock2 (texmap, BMTEX_CROP_U_NAME, (ParamType2)TYPE_FLOAT, &(remap3dsTexChannel._CropU), time);
			nlassert (bRes);

			// Get the crop value V
			bRes=getValueByNameUsingParamBlock2 (texmap, BMTEX_CROP_V_NAME, (ParamType2)TYPE_FLOAT, &(remap3dsTexChannel._CropV), time);
			nlassert (bRes);

			// Get the crop value W
			bRes=getValueByNameUsingParamBlock2 (texmap, BMTEX_CROP_W_NAME, (ParamType2)TYPE_FLOAT, &(remap3dsTexChannel._CropW), time);
			nlassert (bRes);

			// Get the crop value H
			bRes=getValueByNameUsingParamBlock2 (texmap, BMTEX_CROP_H_NAME, (ParamType2)TYPE_FLOAT, &(remap3dsTexChannel._CropH), time);
			nlassert (bRes);
		}

		// Force cubic
		if (forceCubic)
		{
			// Cube side
			const static CTextureCube::TFace tfNewOrder[6] = {	CTextureCube::positive_z, CTextureCube::negative_z,
														CTextureCube::negative_x, CTextureCube::positive_x,
														CTextureCube::negative_y, CTextureCube::positive_y	};

			// Alloc a cube texture
			CTextureCube* pTextureCube = new CTextureCube;

			// For each side of the cube
			for (uint side=0; side<6; side++)
			{
				// Alloc a file texture
				CTextureFile *pTextureFile=new CTextureFile ();

				// Set the file name
				pTextureFile->setFileName (sFileName);

				// Set the texture in the cube
				pTextureCube->setTexture (tfNewOrder[side], pTextureFile);
			}

			// Ok, good texture
			pTexture=pTextureCube;
		}
		else
		{
			// Alloc a texture
			CTextureFile *pTextureFile=new CTextureFile ();

			// Set the file name
			pTextureFile->setFileName (sFileName);

			// Ok, good texture
			pTexture=pTextureFile;
		}
	}
	else if( isClassIdCompatible(texmap, Class_ID (ACUBIC_CLASS_ID,0)) )
	{
		// Pointer on the  diffuse texture
		CTextureCube* pTextureCube=NULL;

		// Texture cube
		pTextureCube = new CTextureCube;
		pTexture = pTextureCube;

		// Face order
		const static CTextureCube::TFace tfNewOrder[6] = {	CTextureCube::positive_z, CTextureCube::negative_z,
													CTextureCube::negative_x, CTextureCube::positive_x,
													CTextureCube::negative_y, CTextureCube::positive_y	};

		// Vector of bitmap names
		vector<string> names;
		CExportNel::getValueByNameUsingParamBlock2 (texmap, "bitmapName", (ParamType2)TYPE_STRING_TAB, &names, time);

		// For each textures
		for( int i = 0; i< (int)names.size(); ++i )
		{
			// Create a texture file
			CTextureFile *pT = new CTextureFile;

			// Tronc name
			char sFileName[512];
			strcpy(sFileName, names[i].c_str());
			if (!absolutePath)
			{
				// Decompose bitmap file name
				char sName[256];
				char sExt[256];
				_splitpath (sFileName, NULL, NULL, sName, sExt);

				// Make the final path
				_makepath (sFileName, NULL, NULL, sName, sExt);
			}

			// Set the file name
			pT->setFileName(sFileName);

			// Set the texture
			pTextureCube->setTexture(tfNewOrder[i], pT);
		}
	}

	// Get the UVs channel and the channel matrix
	Matrix3	channelMatrix;
	int nChannel=getVertMapChannel (texmap, channelMatrix, time);

	// Add the UVs channel
	remap3dsTexChannel._IndexInMaxMaterial=nChannel;
	remap3dsTexChannel._UVMatrix=channelMatrix;

	// check for tiling
	if (!(texmap.GetTextureTiling() & U_WRAP)) pTexture->setWrapS(ITexture::Clamp);
	if (!(texmap.GetTextureTiling() & V_WRAP)) pTexture->setWrapT(ITexture::Clamp);

	// Return the texture pointer
	return pTexture;
}
