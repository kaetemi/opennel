/** \file driver_direct3d_texture.cpp
 * Direct 3d driver implementation
 *
 * $Id: driver_direct3d_texture.cpp,v 1.1 2004/03/19 10:11:36 corvazier Exp $
 *
 * \todo manage better the init/release system (if a throw occurs in the init, we must release correctly the driver)
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

#include "stddirect3d.h"

#include "3d/vertex_buffer.h"
#include "3d/light.h"
#include "3d/index_buffer.h"
#include "nel/misc/rect.h"
#include "nel/misc/di_event_emitter.h"
#include "nel/misc/mouse_device.h"
#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "nel/3d/u_driver.h"

#include "driver_direct3d.h"

using namespace std;
using namespace NLMISC;

namespace NL3D 
{

std::vector<uint8> CDriverD3D::_TempBuffer;

extern uint textureCount = 0;

// ***************************************************************************

CTextureDrvInfosD3D::CTextureDrvInfosD3D(IDriver *drv, ItTexDrvInfoPtrMap it, CDriverD3D *drvGl, bool renderTarget) : ITextureDrvInfos(drv, it)
{
	Texture = NULL;
	Texture2d = NULL;
	TextureCube = NULL;
	TextureMemory = 0;
	RenderTarget = renderTarget;

	// Nb: at Driver dtor, all tex infos are deleted, so _Driver is always valid.
	_Driver= drvGl;
}

// ***************************************************************************

CTextureDrvInfosD3D::~CTextureDrvInfosD3D()
{
	if (Texture)
	{
		Texture->Release();
		Texture = NULL;
		Texture2d = NULL;
		TextureCube = NULL;
		textureCount--;
	}

	// release profiling texture mem.
	_Driver->_AllocatedTextureMemory-= TextureMemory;
}

// ***************************************************************************

bool CDriverD3D::setupTexture (ITexture& tex)
{
	bool nTmp;
	return setupTextureInternal (tex, true, nTmp, false, false);
}

// ***************************************************************************

const D3DTEXTUREADDRESS RemapTextureAdressTypeNeL2D3D[ITexture::WrapModeCount]=
{
	D3DTADDRESS_WRAP,	// Repeat
	D3DTADDRESS_CLAMP,	// Clamp
};

// ***************************************************************************

const D3DTEXTUREFILTERTYPE RemapMagTextureFilterTypeNeL2D3D[ITexture::MagFilterCount]=
{
	D3DTEXF_POINT,	// Nearest
	D3DTEXF_LINEAR,	// Linear
};

// ***************************************************************************

const D3DTEXTUREFILTERTYPE RemapMinTextureFilterTypeNeL2D3D[ITexture::MinFilterCount]=
{
	D3DTEXF_POINT,	// NearestMipMapOff
	D3DTEXF_POINT,	// NearestMipMapNearest
	D3DTEXF_POINT,	// NearestMipMapLinear
	D3DTEXF_LINEAR,	// LinearMipMapOff
	D3DTEXF_LINEAR,	// LinearMipMapNearest
	D3DTEXF_LINEAR,	// LinearMipMapLinear
};

// ***************************************************************************

const D3DTEXTUREFILTERTYPE RemapMipTextureFilterTypeNeL2D3D[ITexture::MinFilterCount]=
{
	D3DTEXF_NONE,	// NearestMipMapOff
	D3DTEXF_POINT,	// NearestMipMapNearest
	D3DTEXF_LINEAR,	// NearestMipMapLinear
	D3DTEXF_NONE,	// LinearMipMapOff
	D3DTEXF_POINT,	// LinearMipMapNearest
	D3DTEXF_LINEAR,	// LinearMipMapLinear
};

// ***************************************************************************

const D3DFORMAT RemapTextureFormatTypeNeL2D3D[CBitmap::ModeCount]=
{
	D3DFMT_A8R8G8B8,	// RGBA
	D3DFMT_L16,			// Luminance
	D3DFMT_A8,			// Alpha
	D3DFMT_A8L8,		// AlphaLuminance
	D3DFMT_DXT1,		// DXTC1
	D3DFMT_DXT1,		// DXTC1Alpha
	D3DFMT_DXT3,		// DXTC3
	D3DFMT_DXT5,		// DXTC5
	D3DFMT_V8U8,		// DsDt
};

// ***************************************************************************

const bool RemapTextureFormatCompressedTypeNeL2D3D[CBitmap::ModeCount]=
{
	false,	// RGBA
	false,	// Luminance
	false,	// Alpha
	false,	// AlphaLuminance
	true,	// DXTC1
	true,	// DXTC1Alpha
	true,	// DXTC3
	true,	// DXTC5
	false,	// DsDt
};

// ***************************************************************************

const D3DCUBEMAP_FACES RemapCubeFaceTypeNeL2D3D[6]=
{
	D3DCUBEMAP_FACE_POSITIVE_X,	// positive_x
	D3DCUBEMAP_FACE_NEGATIVE_X,	// negative_x
	D3DCUBEMAP_FACE_POSITIVE_Z,	// positive_y
	D3DCUBEMAP_FACE_NEGATIVE_Z,	// negative_y
	D3DCUBEMAP_FACE_POSITIVE_Y,	// positive_z
	D3DCUBEMAP_FACE_NEGATIVE_Y,	// negative_z
};

// ***************************************************************************

D3DFORMAT CDriverD3D::getD3DDestTextureFormat (ITexture& tex)
{
	ITexture::TUploadFormat texfmt= tex.getUploadFormat();

	// If auto, retrieve the pixel format of the bitmap.
	if(texfmt == ITexture::Auto)
	{
		switch(tex.getPixelFormat())
		{
			case CBitmap::RGBA: 
				if(_ForceDXTCCompression && tex.allowDegradation() )
					texfmt= ITexture::DXTC5;
				else
					texfmt= ITexture::RGBA8888;
				break;
			case CBitmap::DXTC1: 
				texfmt= ITexture::DXTC1; 
				break;
			case CBitmap::DXTC1Alpha: 
				texfmt= ITexture::DXTC1Alpha; 
				break;
			case CBitmap::DXTC3: 
				texfmt= ITexture::DXTC3; 
				break;
			case CBitmap::DXTC5: 
				texfmt= ITexture::DXTC5; 
				break;
			case CBitmap::Luminance: 
				texfmt= ITexture::Luminance; 
				break;
			case CBitmap::Alpha: 
				texfmt= ITexture::Alpha; 
				break;
			case CBitmap::AlphaLuminance: 
				texfmt= ITexture::AlphaLuminance; 
				break;
			case CBitmap::DsDt: 
				texfmt= ITexture::DsDt; 
				break;
			default: texfmt= ITexture::RGBA8888; break;
		}
	}

	// Get standard prefered tex format.
	nlassert (texfmt<ITexture::UploadFormatCount);
	return _PreferedTextureFormat[texfmt];
}

// ***************************************************************************

uint getPixelFormatSize (D3DFORMAT destFormat)
{
	uint bits;
	switch (destFormat)
	{
	case D3DFMT_R8G8B8: bits=32; break;
	case D3DFMT_A8R8G8B8: bits=32; break;
	case D3DFMT_X8R8G8B8: bits=32; break;
	case D3DFMT_R5G6B5: bits=16; break;
	case D3DFMT_X1R5G5B5: bits=16; break;
	case D3DFMT_A1R5G5B5: bits=16; break;
	case D3DFMT_A4R4G4B4: bits=16; break;
	case D3DFMT_R3G3B2: bits=8; break;
	case D3DFMT_A8: bits=8; break;
	case D3DFMT_A8R3G3B2: bits=16; break;
	case D3DFMT_X4R4G4B4: bits=16; break;
	case D3DFMT_A2B10G10R10: bits=32; break;
	case D3DFMT_A8B8G8R8: bits=32; break;
	case D3DFMT_X8B8G8R8: bits=32; break;
	case D3DFMT_G16R16: bits=32; break;
	case D3DFMT_A2R10G10B10: bits=32; break;
	case D3DFMT_A16B16G16R16: bits=64; break;
	case D3DFMT_A8P8: bits=16; break;
	case D3DFMT_P8: bits=8; break;
	case D3DFMT_L8: bits=9; break;
	case D3DFMT_L16: bits=16; break;
	case D3DFMT_A8L8: bits=16; break;
	case D3DFMT_A4L4: bits=8; break;
	case D3DFMT_V8U8: bits=16; break;
	case D3DFMT_Q8W8V8U8: bits=32; break;
	case D3DFMT_V16U16: bits=32; break;
	case D3DFMT_Q16W16V16U16: bits=64; break;
	case D3DFMT_CxV8U8: bits=16; break;
	case D3DFMT_L6V5U5: bits=16; break;
	case D3DFMT_X8L8V8U8: bits=32; break;
	case D3DFMT_A2W10V10U10: bits=32; break;
	case D3DFMT_G8R8_G8B8: bits=24; break;
	case D3DFMT_R8G8_B8G8: bits=24; break;
	case D3DFMT_DXT1: bits=4; break;
	case D3DFMT_DXT2: bits=8; break;
	case D3DFMT_DXT3: bits=8; break;
	case D3DFMT_DXT4: bits=8; break;
	case D3DFMT_DXT5: bits=8; break;
	default: nlstop; break;	// unkown pixel format
	}
	return bits;
}

// ***************************************************************************

uint32 CDriverD3D::computeTextureMemoryUsage (uint width, uint height, uint levels, D3DFORMAT destFormat, bool cube)
{
	// Get bit per pixel
	uint bits = getPixelFormatSize (destFormat);
	uint32 size = 0;
	while (levels>0)
	{
		size += (width * height * bits) >> 3;
		width = max((uint)(width>>1), 1U);
		height = max((uint)(height>>1), 1U);
		levels--;
	}
	if (cube)
		size *= 6;
	return size;
}

// ***************************************************************************

// nbPixels is pixel count, not a byte count !
inline void copyRGBA2BGRA (uint32 *dest, const uint32 *src, uint nbPixels)
{
	while (nbPixels != 0)
	{
		register uint32 color = *src;
		*dest = (color & 0xff00ff00) | ((color&0xff)<<16) | ((color&0xff0000)>>16);
		dest++;
		src++;
		nbPixels--;
	}
}

// ***************************************************************************

bool CDriverD3D::generateD3DTexture (ITexture& tex, bool textureDegradation, D3DFORMAT &destFormat, D3DFORMAT &srcFormat, uint &firstMipMap, bool &cube, bool renderTarget)
{
	// Regenerate all the texture.
	tex.generate();

	// Is a cube texture ?
	cube = tex.isTextureCube();

	// D3D infos
	CTextureDrvInfosD3D*	d3dtext;
	d3dtext= getTextureD3D(tex);

	// For each face
	UINT width;
	UINT height;
	UINT levels;
	bool srcFormatCompressed;
	const uint faceCount = cube?6:1;
	uint face;
	for (face=0; face<faceCount; face++)
	{
		// Get the texture pointer
		ITexture *texture = cube?(static_cast<CTextureCube*>(&tex)->getTexture((CTextureCube::TFace)face)):&tex;

		// getD3DTextureFormat choose a d3d dest and a src format. src format should be the tex one.
		srcFormatCompressed = RemapTextureFormatCompressedTypeNeL2D3D[texture->getPixelFormat()];
		srcFormat = RemapTextureFormatTypeNeL2D3D[texture->getPixelFormat()];
		destFormat = getD3DDestTextureFormat (tex);

		// Get new texture format
		width = texture->getWidth();
		height = texture->getHeight();
		levels = texture->getMipMapCount();
		firstMipMap = 0;

		// Texture degradation
		if (_ForceTextureResizePower>0 && texture->allowDegradation() && (!srcFormatCompressed || levels>1) && textureDegradation)
		{
			// New size
			width = max(1U, (UINT)(texture->getWidth(0) >> _ForceTextureResizePower));
			height = max(1U, (UINT)(texture->getHeight(0) >> _ForceTextureResizePower));
			levels = (levels>_ForceTextureResizePower)?levels-_ForceTextureResizePower:1;

			// Modify source texture
			if (srcFormatCompressed)
			{
				// Skip a compressed mipmap
				firstMipMap = texture->getMipMapCount()-levels;
			}
			else
			{
				// Resample the non-compressed texture
				texture->resample(width, height);
			}
		}

		// Generate mipmap
		if (!srcFormatCompressed)
		{
			if (texture->mipMapOn())
			{
				texture->buildMipMaps();
				levels= texture->getMipMapCount();
			}
			else
				levels= 1;
		}
	}

	// Got an old texture ?
	if (d3dtext->Texture &&  
							(	
								(d3dtext->Width != width) ||
								(d3dtext->Height != height) ||
								(d3dtext->Levels != levels) ||
								(d3dtext->DestFormat != destFormat) ||
								(d3dtext->IsCube != cube) ||
								(d3dtext->RenderTarget != renderTarget)
							)
		)
	{
		// Delete this texture
		if (d3dtext->Texture)
		{
			d3dtext->Texture->Release();
			d3dtext->Texture = NULL;
			d3dtext->Texture2d = NULL;
			d3dtext->TextureCube = NULL;
		}

		// profiling: count TextureMemory usage.
		_AllocatedTextureMemory -= d3dtext->TextureMemory;
		d3dtext->TextureMemory = 0;
	}

	// Texture must be created ?
	if (d3dtext->Texture == NULL)
	{
		// profiling: count TextureMemory usage.
		d3dtext->TextureMemory = computeTextureMemoryUsage (width, height, levels, destFormat, cube);
		_AllocatedTextureMemory += d3dtext->TextureMemory;

		// Create the texture
		bool createSuccess;

		if (cube)
		{
			createSuccess = _DeviceInterface->CreateCubeTexture(width, levels, renderTarget?D3DUSAGE_RENDERTARGET:0, destFormat, renderTarget?D3DPOOL_DEFAULT:D3DPOOL_MANAGED, &(d3dtext->TextureCube), NULL) == D3D_OK;
			d3dtext->Texture = d3dtext->TextureCube;
		}
		else
		{
			createSuccess = _DeviceInterface->CreateTexture (width, height, levels, renderTarget?D3DUSAGE_RENDERTARGET:0, destFormat, renderTarget?D3DPOOL_DEFAULT:D3DPOOL_MANAGED, &(d3dtext->Texture2d), NULL) == D3D_OK;
			d3dtext->Texture = d3dtext->Texture2d;
		}

		if (!createSuccess)
			return false;

		textureCount++;

		// Copy parameters
		d3dtext->Width = width;
		d3dtext->Height = height;
		d3dtext->Levels = levels;
		d3dtext->DestFormat = destFormat;
		d3dtext->SrcCompressed = srcFormatCompressed;
		d3dtext->IsCube = cube;
	}

	return true;
}

// ***************************************************************************

bool CDriverD3D::setupTextureEx (ITexture& tex, bool bUpload, bool &bAllUploaded, bool bMustRecreateSharedTexture)
{
	return setupTextureInternal (tex, bUpload, bAllUploaded, bMustRecreateSharedTexture, false);
}

// ***************************************************************************

bool CDriverD3D::setupTextureInternal (ITexture& tex, bool bUpload, bool &bAllUploaded, bool bMustRecreateSharedTexture, bool renderTarget)
{
	bAllUploaded = false;
	
	if(tex.isTextureCube() && (!_TextureCubeSupported))
		return true;

	// 0. Create/Retrieve the driver texture.
	//=======================================
	bool mustCreate = false;
	if ( !tex.TextureDrvShare )
	{
		// insert into driver list. (so it is deleted when driver is deleted).
		ItTexDrvSharePtrList	it= _TexDrvShares.insert(_TexDrvShares.end());
		// create and set iterator, for future deletion.
		*it= tex.TextureDrvShare= new CTextureDrvShare(this, it);

		// Must (re)-create the texture.
		mustCreate = true;
	}

	// Does the texture has been touched ?
	if ( (!tex.touched()) && (!mustCreate) )
		return true; // Do not do anything


	// 1. If modified, may (re)load texture part or all of the texture.
	//=================================================================


	bool	mustLoadAll= false;
	bool	mustLoadPart= false;


	// A. Share mgt.
	//==============
	if(tex.supportSharing())
	{
		// Try to get the shared texture.

		// Create the shared Name.
		std::string	name;
		getTextureShareName (tex, name);
		if (name == "ma_mairie2_wi.tga@Fmt:0@MMp:On")
			int toto=0;
		if (name == "ma_loupiotte.tga@Fmt:0@MMp:On")
			int toto=0;

		// insert or get the texture.
		{
			CSynchronized<TTexDrvInfoPtrMap>::CAccessor access(&_SyncTexDrvInfos);
			TTexDrvInfoPtrMap &rTexDrvInfos = access.value();

			ItTexDrvInfoPtrMap	itTex;
			itTex= rTexDrvInfos.find(name);

			// texture not found?
			if( itTex==rTexDrvInfos.end() )
			{
				// insert into driver map. (so it is deleted when driver is deleted).
				itTex= (rTexDrvInfos.insert(make_pair(name, (ITextureDrvInfos*)NULL))).first;
				// create and set iterator, for future deletion.
				itTex->second= tex.TextureDrvShare->DrvTexture= new CTextureDrvInfosD3D(this, itTex, this, renderTarget);

				// need to load ALL this texture.
				mustLoadAll= true;
			}
			else
			{
				tex.TextureDrvShare->DrvTexture= itTex->second;

				if(bMustRecreateSharedTexture)
					// reload this shared texture (user request)
					mustLoadAll= true;
				else
					// Do not need to reload this texture, even if the format/mipmap has changed, since we found this 
					// couple in the map.
					mustLoadAll= false;
			}
		}
		// Do not test if part of texture may need to be computed, because Rect invalidation is incompatible 
		// with texture sharing.
	}
	else
	{
		// If texture not already created.
		if(!tex.TextureDrvShare->DrvTexture)
		{
			// Must create it. Create auto a D3D id (in constructor).
			// Do not insert into the map. This un-shared texture will be deleted at deletion of the texture.
			// Inform ITextureDrvInfos by passing NULL _Driver.
			tex.TextureDrvShare->DrvTexture= new CTextureDrvInfosD3D(NULL, ItTexDrvInfoPtrMap(), this, renderTarget);

			// need to load ALL this texture.
			mustLoadAll= true;
		}
		else if(tex.isAllInvalidated())
			mustLoadAll= true;
		else if(tex.touched())
			mustLoadPart= true;
	}

	// B. Setup texture.
	//==================
	if(mustLoadAll || mustLoadPart)
	{
		CTextureDrvInfosD3D*	d3dtext;
		d3dtext= getTextureD3D(tex);

		// a. Load All the texture case.
		//==============================
		if (mustLoadAll)
		{
			D3DFORMAT srcFormat;
			D3DFORMAT destFormat;
			uint firstMipMap;
			bool cube;
			if (generateD3DTexture (tex, true, destFormat, srcFormat, firstMipMap, cube, renderTarget))
			{
				if((tex.getSize()>0) || cube)
				{
					// Can support multi part upload ?
					if ((destFormat != srcFormat) && d3dtext->SrcCompressed)
					{
						// No, force upload
						bUpload = true; // Force all upload
					}

					// Must upload the texture ?
					if (bUpload)
					{
						// Update the pixels
						uint i;
						UINT destLevel = 0;
						uint blockCount = d3dtext->Height>>(d3dtext->SrcCompressed?2:0);
						uint lineWidth = d3dtext->Width;
						const uint pixelSize = CBitmap::bitPerPixels[tex.getPixelFormat()];
						for(i=firstMipMap;i<d3dtext->Levels;i++, destLevel++)
						{
							// Size of a line
							const uint blockSize = ((lineWidth*pixelSize)>>3)<<(d3dtext->SrcCompressed?2:0);

							// Several faces for cube texture
							const uint numFaces = d3dtext->IsCube?6:1;
							uint face;
							for (face=0; face<numFaces; face++)
							{
								// Get the cube texture
								ITexture *texture = d3dtext->IsCube?(static_cast<CTextureCube*>(&tex)->getTexture((CTextureCube::TFace)face)):&tex;
								if (texture)
								{
									// Same format ?
									if (destFormat == srcFormat)
									{
										// Lock the surface level
										D3DLOCKED_RECT rect;
										bool locked = false;
										if (!d3dtext->IsCube)
											locked = d3dtext->Texture2d->LockRect (destLevel, &rect, NULL, 0) == D3D_OK;
										else
											locked = d3dtext->TextureCube->LockRect (RemapCubeFaceTypeNeL2D3D[face], destLevel, &rect, NULL, 0) == D3D_OK;

										// Rect locked ?
										if (locked)
										{
											uint block;
											for (block=0; block<blockCount; block++)
											{
												// Copy the block
												const uint8 *src = &(texture->getPixels(i)[block*blockSize]);
												uint8 *dest = ((uint8*)rect.pBits)+block*rect.Pitch;
												if (destFormat == D3DFMT_A8R8G8B8)
													copyRGBA2BGRA ((uint32*)dest, (const uint32*)src, blockSize>>2);
												else
													memcpy (dest, src, blockSize);
											}

											// Unlock
											if (!d3dtext->IsCube)
												d3dtext->Texture2d->UnlockRect (destLevel);
											else
												d3dtext->TextureCube->UnlockRect (RemapCubeFaceTypeNeL2D3D[face], destLevel);
										}
									}
									else
									{
										// Convert the surface using a D3DX method
										IDirect3DSurface9 *pDestSurface;
										bool surfaceOk = false;
										if (!d3dtext->IsCube)
											surfaceOk = d3dtext->Texture2d->GetSurfaceLevel(destLevel, &pDestSurface) == D3D_OK;
										else
											surfaceOk = d3dtext->TextureCube->GetCubeMapSurface(RemapCubeFaceTypeNeL2D3D[face], destLevel, &pDestSurface) == D3D_OK;

										// Surface has been retrieved ?
										if (surfaceOk)
										{
											if (srcFormat == D3DFMT_A8R8G8B8)
											{
												const uint8 *src = &(tex.getPixels(i)[0]);

												// Fill the temp buffer with BGRA info
												_TempBuffer.resize (blockCount*blockSize);
												uint8 *dest = &(_TempBuffer[0]);
												uint line;
												for (line=0; line<blockCount; line++)
												{
													copyRGBA2BGRA ((uint32*)dest, (uint32*)(src+line*blockSize), lineWidth);
													dest += blockSize;
												}

												// Upload the texture part
												RECT srcRect;
												srcRect.top = 0;
												srcRect.bottom = blockCount;
												srcRect.left = 0;
												srcRect.right = lineWidth;
												D3DXLoadSurfaceFromMemory (pDestSurface, NULL, NULL, &(_TempBuffer[0]), srcFormat,
													blockSize, NULL, &srcRect, D3DX_FILTER_NONE, 0);
											}
											else
											{
												RECT rect;
												rect.top = 0;
												rect.bottom = blockCount;
												rect.left = 0;
												rect.right = lineWidth;
												D3DXLoadSurfaceFromMemory (pDestSurface, NULL, NULL, &(texture->getPixels(i)[0]), srcFormat,
													blockSize, NULL, &rect, D3DX_FILTER_NONE, 0);
											}
										}
									}
								}
							}

							// Next level
							lineWidth = max((UINT)(lineWidth>>1), (UINT)(d3dtext->SrcCompressed?pixelSize>>1:1));
							blockCount = max((UINT)(blockCount>>1), 1U);
						}

						// Upload now
						if (!d3dtext->IsCube)
							d3dtext->Texture->PreLoad();
						else
							d3dtext->Texture->PreLoad();
						bAllUploaded = true;
					}
				}
			}
		}
		// b. Load part of the texture case.
		//==================================
		// \todo yoyo: TODO_DXTC
		// Replace parts of a compressed image. Maybe don't work with the actual system of invalidateRect()...
		else if (mustLoadPart && !d3dtext->SrcCompressed)
		{
			D3DFORMAT destFormat;
			D3DFORMAT srcFormat;
			uint firstMipMap;
			bool cube;
			if (CDriverD3D::generateD3DTexture (tex, false, destFormat, srcFormat, firstMipMap, cube, renderTarget))
			{
				// No degradation in load part
				nlassert (firstMipMap == 0);

				if(tex.getSize()>0 && bUpload)
				{
					// For all rect, update the texture/mipmap.
					//===============================================
					list<NLMISC::CRect>::iterator	itRect;
					for(itRect=tex._ListInvalidRect.begin(); itRect!=tex._ListInvalidRect.end(); itRect++)
					{
						CRect &rect = *itRect;
						sint	x0= rect.X;
						sint	y0= rect.Y;
						sint	x1= rect.X+rect.Width;
						sint	y1= rect.Y+rect.Height;

						// Update the pixels
						uint i;
						UINT destLevel = 0;
						for(i=firstMipMap;i<d3dtext->Levels;i++, destLevel++)
						{
							CRect rectDest;
							rectDest.X = x0;
							rectDest.Y = y0;
							rectDest.Width = x1-x0;
							rectDest.Height = y1-y0;
							uploadTextureInternal (tex, rectDest, destLevel, i, destFormat, srcFormat);

							// floor .
							x0= x0/2;
							y0= y0/2;
							// ceil.
							x1= (x1+1)/2;
							y1= (y1+1)/2;
						}
					}
				}
			}
		}

		// Release, if wanted.
		if(tex.getReleasable())
			tex.release();

		// Basic parameters.
		//==================
		d3dtext->WrapS = RemapTextureAdressTypeNeL2D3D[tex.getWrapS()];
		d3dtext->WrapT = RemapTextureAdressTypeNeL2D3D[tex.getWrapT()];
		d3dtext->MagFilter = RemapMagTextureFilterTypeNeL2D3D[tex.getMagFilter()];
		d3dtext->MinFilter = RemapMinTextureFilterTypeNeL2D3D[tex.getMinFilter()];
		d3dtext->MipFilter = RemapMipTextureFilterTypeNeL2D3D[tex.getMinFilter()];
	}

	// The texture is correctly setuped.
	tex.clearTouched();
	return true;
}

// ***************************************************************************

bool CDriverD3D::uploadTexture (ITexture& tex, CRect& rect, uint8 nNumMipMap)
{
	if (tex.TextureDrvShare == NULL)
		return false; // Texture not created
	if (tex.TextureDrvShare->DrvTexture == NULL)
		return false; // Texture not created
	if (tex.isTextureCube())
		return false;

	CTextureDrvInfosD3D*	d3dtext = getTextureD3D(tex);
	nlassert (rect.X < (sint)d3dtext->Width);
	nlassert (rect.Y < (sint)d3dtext->Height);
	nlassert (rect.X + rect.Width <= (sint)d3dtext->Width);
	nlassert (rect.Y + rect.Height <= (sint)d3dtext->Height);

	nlassert(nNumMipMap<(uint8)tex.getMipMapCount());

	// validate rect.
	sint x0 = rect.X;
	sint y0 = rect.Y;
	sint x1 = rect.X+rect.Width;
	sint y1 = rect.Y+rect.Height;
	sint w = tex.getWidth (nNumMipMap);
	sint h = tex.getHeight (nNumMipMap);
	clamp (x0, 0, w);
	clamp (y0, 0, h);
	clamp (x1, x0, w);
	clamp (y1, y0, h);
	rect.X = x0;
	rect.Y = y0;
	rect.Width = x1 - x0;
	rect.Height = y1 - y0;

	// Texture format
	D3DFORMAT srcFormat = RemapTextureFormatTypeNeL2D3D[tex.getPixelFormat()];
	D3DFORMAT destFormat = getD3DDestTextureFormat (tex);

	return uploadTextureInternal (tex, rect, nNumMipMap, nNumMipMap, destFormat, srcFormat);
}

// ***************************************************************************

bool CDriverD3D::uploadTextureInternal (ITexture& tex, CRect& rect, uint8 destMipmap, uint8 srcMipmap, 
										D3DFORMAT destFormat, D3DFORMAT srcFormat)
{
	// The D3D texture
	CTextureDrvInfosD3D*	d3dtext = getTextureD3D(tex);

	// The pixel size
	const uint pixelSize = CBitmap::bitPerPixels[tex.getPixelFormat()];

	// The line width of that mipmap level
	uint lineWidth = max(d3dtext->Width>>srcMipmap, d3dtext->SrcCompressed?pixelSize>>1:1);

	const sint	x0= rect.X;
	const sint	y0= rect.Y;
	const sint	x1= rect.X+rect.Width;
	const sint	y1= rect.Y+rect.Height;

	// Size of a line
	const uint lineSize = ((lineWidth*pixelSize)>>3)<<(d3dtext->SrcCompressed?2:0);
	const uint offsetX = (x0*pixelSize)>>3;

	// Block of line (for compressed textures)
	uint lineStart = y0;
	uint lineEnd = y1;

	// Pitch for compressed texture
	if (d3dtext->SrcCompressed)
	{
		lineStart >>= 2;
		lineEnd >>= 2;
	}

	// Same format ?
	if (destFormat == srcFormat)
	{
		// Lock the surface level
		D3DLOCKED_RECT rect;
		RECT region;
		region.left = x0;
		region.right = x1;
		region.top = y0;
		region.bottom = y1;

		const dataToCopy = (((x1-x0)*pixelSize)>>3)<<(d3dtext->SrcCompressed?2:0);
		if (d3dtext->Texture2d->LockRect (destMipmap, &rect, &region, 0) == D3D_OK)
		{
			uint line;
			for (line=lineStart; line<lineEnd; line++)
			{
				// Copy the line
				const uint8 *src = &(tex.getPixels(srcMipmap)[line*lineSize+offsetX]);
				uint8 *dest = ((uint8*)rect.pBits)+(line-lineStart)*rect.Pitch;
				if (destFormat == D3DFMT_A8R8G8B8)
					copyRGBA2BGRA ((uint32*)dest, (const uint32*)src, dataToCopy>>2);
				else
					memcpy (dest, src, dataToCopy);
			}
			d3dtext->Texture2d->UnlockRect (destMipmap);
		}
		else
			return false;
	}
	else
	{
		// Convert the surface using a D3DX method
		IDirect3DSurface9 *pDestSurface;

		// Surface has been retrieved ?
		if (d3dtext->Texture2d->GetSurfaceLevel(destMipmap, &pDestSurface) == D3D_OK)
		{
			RECT destRect;
			destRect.top = y0;
			destRect.bottom = y1;
			destRect.left = x0;
			destRect.right = x1;

			if (srcFormat == D3DFMT_A8R8G8B8)
			{
				const uint8 *src = &(tex.getPixels(srcMipmap)[0]);

				// Fill the temp buffer with BGRA info
				const lineWidth = x1-x0; 
				_TempBuffer.resize (((y1-y0)*lineWidth)<<2);
				uint8 *dest = &(_TempBuffer[0]);
				uint line;
				for (line=y0; line<(uint)y1; line++)
				{
					copyRGBA2BGRA ((uint32*)dest, (uint32*)(src+line*lineSize+(x0<<2)), lineWidth);
					dest += lineWidth<<2;
				}

				// Upload the texture part
				RECT srcRect;
				srcRect.top = 0;
				srcRect.bottom = y1-y0;
				srcRect.left = 0;
				srcRect.right = lineWidth;
				if (D3DXLoadSurfaceFromMemory (pDestSurface, NULL, &destRect, &(_TempBuffer[0]), srcFormat,
					lineWidth<<2, NULL, &srcRect, D3DX_FILTER_NONE, 0) != D3D_OK)
					return false;
			}
			else
			{
				RECT srcRect;
				srcRect.top = y0;
				srcRect.bottom = y1;
				srcRect.left = x0;
				srcRect.right = x1;
				const uint8 *src = &(tex.getPixels(srcMipmap)[0]);
				if (D3DXLoadSurfaceFromMemory (pDestSurface, NULL, &destRect, src, srcFormat,
					lineSize, NULL, &srcRect, D3DX_FILTER_NONE, 0) != D3D_OK)
					return false;
			}
		}
	}

	return true;
}

// ***************************************************************************

bool CDriverD3D::isTextureExist(const ITexture&tex)
{
	bool result;

	// Create the shared Name.
	std::string	name;
	getTextureShareName (tex, name);

	{
		CSynchronized<TTexDrvInfoPtrMap>::CAccessor access(&_SyncTexDrvInfos);
		TTexDrvInfoPtrMap &rTexDrvInfos = access.value();
		result = (rTexDrvInfos.find(name) != rTexDrvInfos.end());
	}
	return result;
}

// ***************************************************************************

void CDriverD3D::swapTextureHandle(ITexture &tex0, ITexture &tex1)
{
	// ensure creation of both texture
	setupTexture(tex0);
	setupTexture(tex1);

	// avoid any problem, disable all textures
	for(sint stage=0; stage<inlGetNumTextStages() ; stage++)
	{
		setTexture (stage, (LPDIRECT3DBASETEXTURE9)NULL);
	}

	// get the handle.
	CTextureDrvInfosD3D	*t0= getTextureD3D(tex0);
	CTextureDrvInfosD3D	*t1= getTextureD3D(tex1);

	/* Swap contents. Can't swap directly the pointers cause would have to change all CTextureDrvShare which point on
		Can't do swap(*t0, *t1), because must keep the correct _DriverIterator
	*/
	swap(t0->Texture, t1->Texture);
	swap(t0->Texture2d, t1->Texture2d);
	swap(t0->TextureCube, t1->TextureCube);
	swap(t0->DestFormat, t1->DestFormat);
	swap(t0->Width, t1->Width);
	swap(t0->Height, t1->Height);
	swap(t0->SrcCompressed, t1->SrcCompressed);
	swap(t0->IsCube, t1->IsCube);
	swap(t0->Levels, t1->Levels);
	swap(t0->TextureMemory, t1->TextureMemory);
	swap(t0->WrapS, t1->WrapS);
	swap(t0->WrapT, t1->WrapT);
	swap(t0->MagFilter, t1->MagFilter);
	swap(t0->MinFilter, t1->MinFilter);
	swap(t0->MipFilter, t1->MipFilter);
}

// ***************************************************************************

uint CDriverD3D::getTextureHandle(const ITexture &tex)
{
	// If DrvShare not setuped
	if(!tex.TextureDrvShare)
		return 0;

	CTextureDrvInfosD3D	*d3dtext= getTextureD3D(const_cast<ITexture&>(tex));

	// If DrvInfo not setuped
	if(!d3dtext)
		return 0;

	return (uint)(d3dtext->Texture);
}

// ***************************************************************************

bool CDriverD3D::setRenderTarget (ITexture *tex, uint32 x, uint32 y, uint32 width, uint32 height, uint32 mipmapLevel, uint32 cubeFace)
{
	if (tex == NULL)
	{
		_InvertCullMode = false;
		if (_BackBuffer)
		{
			setRenderTarget (_BackBuffer, NULL, 0, 0);
			_BackBuffer->Release();
			_BackBuffer = NULL;
		}
	}
	else
	{
		_InvertCullMode = true;
		if (!_BackBuffer)
		{
			updateRenderVariables ();
			_DeviceInterface->GetRenderTarget (0, &_BackBuffer);
		}

		// Alread setuped ?
		if (tex->TextureDrvShare)
		{
			// Get texture info
			CTextureDrvInfosD3D *d3dtext = getTextureD3D(*tex);

			// Renderable ? Erase and rebuild
			if (!d3dtext->RenderTarget)
				// Touch the texture
				tex->touch();
		}

		// Setup the new texture
		bool nTmp;
		if (!setupTextureInternal (*tex, false, nTmp, false, true))
			return false;

		// Get texture info
		CTextureDrvInfosD3D *d3dtext = getTextureD3D(*tex);

		// Setup texture for texture rendering
		IDirect3DSurface9 *pDestSurface;
		bool surfaceOk = false;
		if (!d3dtext->IsCube)
			surfaceOk = d3dtext->Texture2d->GetSurfaceLevel(mipmapLevel, &pDestSurface) == D3D_OK;
		else
			surfaceOk = d3dtext->TextureCube->GetCubeMapSurface(RemapCubeFaceTypeNeL2D3D[cubeFace], mipmapLevel, &pDestSurface) == D3D_OK;
		if (surfaceOk)
			setRenderTarget (pDestSurface, tex, (uint8)mipmapLevel, (uint8)cubeFace);
		else
			return false;
	}
	
	// Handle backside
	if (!_DoubleSided)
		setRenderState (D3DRS_CULLMODE, _InvertCullMode?D3DCULL_CCW:D3DCULL_CW);

	return true;
}

// ***************************************************************************

bool CDriverD3D::copyTargetToTexture (ITexture *tex, uint32 offsetx, uint32 offsety, uint32 x, uint32 y, uint32 width, 
												uint32 height, uint32 mipmapLevel)
{
	return false;
}

// ***************************************************************************

bool CDriverD3D::getRenderTargetSize (uint32 &width, uint32 &height)
{
	// Target is the frame buffer ?
	if (_RenderTarget.Texture)
	{
		CTextureDrvInfosD3D*	d3dtext = getTextureD3D(*_RenderTarget.Texture);
		width = max ((uint32)1, (uint32)(d3dtext->Width >> (_RenderTarget.Level)));
		height = max ((uint32)1, (uint32)(d3dtext->Height >> (_RenderTarget.Level)));
		return true;
	}
	else
	{
		getWindowSize (width, height);
		return true;
	}
}

// ***************************************************************************

} // NL3D