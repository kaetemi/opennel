/** \file driver_direct3d.h
 * Direct 3d driver implementation
 *
 * $Id: driver_direct3d.h,v 1.2 2004/03/19 17:50:47 berenguier Exp $
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

#ifndef NL_DRIVER_DIRECT3D_H
#define NL_DRIVER_DIRECT3D_H


#include "nel/misc/types_nl.h"

// NeL includes
#include "nel/misc/matrix.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/misc/event_emitter.h"
#include "nel/misc/bit_set.h"
#include "nel/misc/heap_memory.h"
#include "nel/misc/event_emitter_multi.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/win_event_emitter.h"
#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "3d/driver.h"
#include "3d/material.h"
#include "3d/shader.h"
#include "3d/vertex_buffer.h"
#include "3d/ptr_set.h"
#include "3d/texture_cube.h"
#include "3d/vertex_program_parse.h"

// *** DEBUG MACRO

// Define this to activate the debug mode (default is undefined)
#define NL_DEBUG_D3D

// Define this to enable the render state caching (default is defined)
#define NL_D3D_USE_RENDER_STATE_CACHE

// Define this to disable hardware vertex program (default is undefined)
// #define NL_DISABLE_HARDWARE_VERTEX_PROGAM

// Define this to disable hardware pixel shaders program (default is undefined)
// #define NL_DISABLE_HARDWARE_PIXEL_SHADER

// Define this to disable hardware vertex array AGP (default is undefined)
// #define NL_DISABLE_HARDWARE_VERTEX_ARRAY_AGP

// Define this to force the texture stage count (default is undefined)
// #define NL_FORCE_TEXTURE_STAGE_COUNT 2

// Define this to force the use of pixel shader in the normal shaders (default is undefined)
// #define NL_FORCE_PIXEL_SHADER_USE_FOR_NORMAL_SHADERS

// ***************************************************************************

namespace NL3D 
{

using NLMISC::CMatrix;
using NLMISC::CVector;

class	CDriverD3D;

// ***************************************************************************
class CTextureDrvInfosD3D : public ITextureDrvInfos
{
public:
	/*
		ANY DATA ADDED HERE MUST BE SWAPPED IN swapTextureHandle() !!
	*/

	// The texture
	LPDIRECT3DBASETEXTURE9	Texture;
	IDirect3DTexture9		*Texture2d;
	IDirect3DCubeTexture9	*TextureCube;

	// The texture format and size
	D3DFORMAT				DestFormat;
	uint					Width;
	uint					Height;

	// This is the owner driver.
	CDriverD3D				*_Driver;

	// Is the internal format of the texture is a compressed one?
	bool					SrcCompressed;
	bool					IsCube;

	// Is a render target ?
	bool					RenderTarget;

	// Mipmap levels
	uint8					Levels;

	// This is the computed size of what memory this texture take.
	uint32					TextureMemory;

	// The current wrap modes assigned to the texture.
	D3DTEXTUREADDRESS		WrapS;
	D3DTEXTUREADDRESS		WrapT;
	D3DTEXTUREFILTERTYPE	MagFilter;
	D3DTEXTUREFILTERTYPE	MinFilter;
	D3DTEXTUREFILTERTYPE	MipFilter;

	CTextureDrvInfosD3D(IDriver *drv, ItTexDrvInfoPtrMap it, CDriverD3D *drvGl, bool renderTarget);
	~CTextureDrvInfosD3D();
};

// ***************************************************************************

struct CMaterialDrvInfosD3D : public IMaterialDrvInfos
{
public:
	D3DMATERIAL9	Material;
	D3DCOLOR		UnlightedColor;
	BOOL			SpecularEnabled;
	D3DBLEND		SrcBlend;
	D3DBLEND		DstBlend;
	D3DCMPFUNC		ZComp;
	DWORD			AlphaRef;
	D3DTEXTUREOP	ColorOp[IDRV_MAT_MAXTEXTURES];
	DWORD 			ColorArg0[IDRV_MAT_MAXTEXTURES];
	DWORD 			ColorArg1[IDRV_MAT_MAXTEXTURES];
	DWORD 			ColorArg2[IDRV_MAT_MAXTEXTURES];
	D3DTEXTUREOP	AlphaOp[IDRV_MAT_MAXTEXTURES];
	DWORD 			AlphaArg0[IDRV_MAT_MAXTEXTURES];
	DWORD 			AlphaArg1[IDRV_MAT_MAXTEXTURES];		
	DWORD			AlphaArg2[IDRV_MAT_MAXTEXTURES];
	D3DCOLOR		ConstantColor[IDRV_MAT_MAXTEXTURES];
	DWORD			TexGen[IDRV_MAT_MAXTEXTURES];
	IDirect3DPixelShader9	*PixelShader;
	IDirect3DPixelShader9	*PixelShaderUnlightedNoVertexColor;
	bool			ActivateSpecularWorldTexMT[IDRV_MAT_MAXTEXTURES];
	bool			ActivateInvViewModelTexMT[IDRV_MAT_MAXTEXTURES];
	bool			NeedsConstantForDiffuse;	// Must use TFactor if not vertex color in the vertex buffer
	uint8			ConstantIndex;				// Index of the constant color to use (when only one constant color is needed and NeedsConstantForDiffuse == false);

	CMaterialDrvInfosD3D(IDriver *drv, ItMatDrvInfoPtrList it) : IMaterialDrvInfos(drv, it)
	{
		PixelShader = NULL;
		PixelShaderUnlightedNoVertexColor = NULL;
	}
	void buildTexEnv (uint stage, const CMaterial::CTexEnv &env, bool textured);
};

// ***************************************************************************

class CVertexProgamDrvInfosD3D : public IVertexProgramDrvInfos
{
public:

	// The shader
	IDirect3DVertexShader9	*Shader;

	CVertexProgamDrvInfosD3D(IDriver *drv, ItVtxPrgDrvInfoPtrList it);
	~CVertexProgamDrvInfosD3D();
};

// ***************************************************************************

class CVBDrvInfosD3D : public IVBDrvInfos
{
public:
	IDirect3DVertexDeclaration9		*VertexDecl;
	IDirect3DVertexBuffer9			*VertexBuffer;
	bool							UseVertexColor;
	uint8							Stride;

	CVBDrvInfosD3D(IDriver *drv, ItVBDrvInfoPtrList it, CVertexBuffer *vb);
	virtual ~CVBDrvInfosD3D();
	virtual uint8	*lock (uint first, uint last, bool readOnly);
	virtual void	unlock (uint first, uint last);
};

// ***************************************************************************

class CIBDrvInfosD3D : public IIBDrvInfos
{
public:
	IDirect3DIndexBuffer9			*IndexBuffer;

	CIBDrvInfosD3D(IDriver *drv, ItIBDrvInfoPtrList it, CIndexBuffer *vb);
	virtual ~CIBDrvInfosD3D();
	virtual uint32	*lock (uint first, uint last, bool readOnly);
	virtual void	unlock (uint first, uint last);
};

// ***************************************************************************

class CShaderDrvInfosD3D : public IShaderDrvInfos
{
public:
	enum
	{
		MaxShaderTexture=8,
	};


	ID3DXEffect				*Effect;
	bool					Validated;

	// Texture handles
	D3DXHANDLE				TextureHandle[MaxShaderTexture];

	// Color handles
	D3DXHANDLE				ColorHandle[MaxShaderTexture];

	// Factor handles
	D3DXHANDLE				FactorHandle[MaxShaderTexture];

	CShaderDrvInfosD3D(IDriver *drv, ItShaderDrvInfoPtrList it);
	virtual ~CShaderDrvInfosD3D();
};

// ***************************************************************************

class CVertexDeclaration
{
public:
	// The human readable values
	D3DVERTEXELEMENT9				VertexElements[CVertexBuffer::NumValue+1];

	// The driver pointer
	IDirect3DVertexDeclaration9		*VertexDecl;
};

// ***************************************************************************

// Normal shader description
class CNormalShaderDesc
{
public:
	CNormalShaderDesc ()
	{
		memset (this, 0, sizeof(CNormalShaderDesc));
	}
	~CNormalShaderDesc ()
	{
		if (PixelShader)
			PixelShader->Release();
	}
	bool					StageUsed[IDRV_MAT_MAXTEXTURES];
	uint32					TexEnvMode[IDRV_MAT_MAXTEXTURES];

	bool operator==(const CNormalShaderDesc &other) const
	{
		uint i;
		for (i=0; i<IDRV_MAT_MAXTEXTURES; i++)
			if ((StageUsed[i] != other.StageUsed[i]) || (StageUsed[i] && (TexEnvMode[i] != other.TexEnvMode[i])))
				return false;
		return true;
	}

	IDirect3DPixelShader9	*PixelShader;
};

// ***************************************************************************

class CDriverD3D : public IDriver, ID3DXEffectStateManager 
{
public:

	// Some constants
	enum 
	{ 
		MaxLight=8,
		MaxRenderState=256,
		MaxTextureState=36,
		MaxTexture=8,
		MaxSamplerState=16,
		MaxSampler=8,

		MatrixStateRemap = 24,
		MaxMatrixState=32,
		MaxVertexProgramConstantState=96,
		MaxPixelShaderConstantState=96,
	};

	// Prefered pixel formats
	enum
	{
		// Number of pixel format choice for each pixel format
		FinalPixelFormatChoice = 5,
	};

	// Construction / destruction
							CDriverD3D();
	virtual					~CDriverD3D();

	// ***************************************************************************
	// Implementation
	// see nel\src\3d\driver.h
	// ***************************************************************************

	// Mode initialisation, requests
	virtual bool			init (uint windowIcon = 0);
	virtual bool			setDisplay(void* wnd, const GfxMode& mode, bool show) throw(EBadDisplay);
	virtual bool			release();
	// todo hulud d3d switch mode
	virtual bool			setMode(const GfxMode& mode) {return false;};
	virtual bool			getModes(std::vector<GfxMode> &modes);
	virtual bool			getCurrentScreenMode(GfxMode &mode);
	virtual bool			activate();
	virtual bool			isActive ();
	// todo hulud d3d vertex buffer hard
	virtual	bool			initVertexArrayRange(uint agpMem, uint vramMem) {return false;};

	// Windows interface
	virtual void*			getDisplay();
	virtual emptyProc		getWindowProc();
	virtual NLMISC::IEventEmitter	*getEventEmitter();
	virtual void			getWindowSize (uint32 &width, uint32 &height);
	virtual void			getWindowPos (uint32 &x, uint32 &y);
	virtual uint8			getBitPerPixel ();

	// Driver parameters
	virtual void			disableHardwareVertexProgram();
	virtual void			disableHardwareIndexArrayAGP();
	virtual void			disableHardwareVertexArrayAGP();
	// todo hulud d3d vertex buffer hard
	virtual void			disableHardwareTextureShader() {};
	virtual void			forceDXTCCompression(bool dxtcComp);
	virtual void			forceTextureResize(uint divisor);

	// Driver informations
	virtual uint			getNumAdapter() const;
	virtual bool			getAdapter(uint adapter, CAdapter &desc) const;
	virtual bool			setAdapter(uint adapter);
	// todo hulud d3d vertex buffer hard
	virtual uint32			getAvailableVertexAGPMemory ();
	// todo hulud d3d texture
	virtual uint32			getAvailableVertexVRAMMemory ();
	virtual	sint			getNbTextureStages() const;
	virtual	bool			supportVertexBufferHard() const;
	virtual	bool			supportIndexBufferHard() const;
	// todo hulud d3d vertex buffer hard
	virtual	bool			slowUnlockVertexBufferHard() const {return false;};
	virtual	uint			getMaxVerticesByVertexBufferHard() const;
	virtual uint32			getImplementationVersion () const;
	virtual const char*		getDriverInformation ();
	// todo hulud d3d
	virtual const char*		getVideocardInformation () {return "";};
	virtual CVertexBuffer::TVertexColorType getVertexColorFormat() const;

	// Textures
	virtual bool			isTextureExist(const ITexture&tex);
	virtual bool			setupTexture (ITexture& tex);
	virtual bool			setupTextureEx (ITexture& tex, bool bUpload, bool &bAllUploaded, bool bMustRecreateSharedTexture= false);
	virtual bool			uploadTexture (ITexture& tex, NLMISC::CRect& rect, uint8 nNumMipMap);
	// todo hulud d3d texture
	virtual bool			uploadTextureCube (ITexture& tex, NLMISC::CRect& rect, uint8 nNumMipMap, uint8 nNumFace) {return false;};

	// Material
	virtual bool			setupMaterial(CMaterial& mat);

	// Buffer
	virtual bool			clear2D(CRGBA rgba);
	virtual bool			clearZBuffer(float zval=1);
	virtual void			setColorMask (bool bRed, bool bGreen, bool bBlue, bool bAlpha);
	virtual bool			swapBuffers();
	virtual void			getBuffer (CBitmap &bitmap) {};
	virtual void			getZBuffer (std::vector<float>  &zbuffer) {};
	virtual void			getBufferPart (CBitmap &bitmap, NLMISC::CRect &rect) {};
	virtual void			getZBufferPart (std::vector<float>  &zbuffer, NLMISC::CRect &rect) {};
	virtual bool			setRenderTarget (ITexture *tex, uint32 x, uint32 y, uint32 width, uint32 height, uint32 mipmapLevel, uint32 cubeFace);
	virtual bool			copyTargetToTexture (ITexture *tex, uint32 offsetx, uint32 offsety, uint32 x, uint32 y, uint32 width, 
													uint32 height, uint32 mipmapLevel);
	virtual bool			getRenderTargetSize (uint32 &width, uint32 &height);
	virtual bool			fillBuffer (CBitmap &bitmap) {return false;};

	// Performances
	virtual void			startSpecularBatch() {};
	virtual void			endSpecularBatch() {};
	virtual void			setSwapVBLInterval(uint interval) {};
	virtual uint			getSwapVBLInterval() {return 0;};
	virtual void			swapTextureHandle(ITexture &tex0, ITexture &tex1);
	virtual	uint			getTextureHandle(const ITexture&tex);

	// Matrix, viewport and frustum
	virtual void			setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective = true);
	virtual void			setupViewMatrix(const CMatrix& mtx);
	virtual void			setupViewMatrixEx(const CMatrix& mtx, const CVector &cameraPos);
	virtual void			setupModelMatrix(const CMatrix& mtx);
	virtual CMatrix			getViewMatrix() const;
	virtual	void			forceNormalize(bool normalize);
	virtual	bool			isForceNormalize() const;
	virtual void			setupScissor (const class CScissor& scissor);
	virtual void			setupViewport (const class CViewport& viewport);
	virtual	void			getViewport(CViewport &viewport);

	// Vertex buffers
	virtual bool			activeVertexBuffer(CVertexBuffer& VB);

	// Index buffers
	virtual bool			activeIndexBuffer(CIndexBuffer& IB);

	// UV
	virtual	void			mapTextureStageToUV(uint stage, uint uv) {};

	// Render
	virtual bool			renderLines(CMaterial& mat, uint32 firstIndex, uint32 nlines);
	virtual bool			renderTriangles(CMaterial& Mat, uint32 firstIndex, uint32 ntris);
	virtual bool			renderSimpleTriangles(uint32 firstTri, uint32 ntris);
	virtual bool			renderRawPoints(CMaterial& Mat, uint32 startIndex, uint32 numPoints);
	virtual bool			renderRawLines(CMaterial& Mat, uint32 startIndex, uint32 numLines);
	virtual bool			renderRawTriangles(CMaterial& Mat, uint32 startIndex, uint32 numTris);
	virtual bool			renderRawQuads(CMaterial& Mat, uint32 startIndex, uint32 numQuads);
	virtual void			setPolygonMode (TPolygonMode mode);
	virtual	void			finish() {};

	// Profile
	virtual	void			profileRenderedPrimitives(CPrimitiveProfile &pIn, CPrimitiveProfile &pOut) {};
	virtual	uint32			profileAllocatedTextureMemory() {return 0;};
	virtual	uint32			profileSetupedMaterials() const {return 0;};
	virtual	uint32			profileSetupedModelMatrix() const {return 0;};
	virtual void			enableUsedTextureMemorySum (bool enable) {};
	virtual uint32			getUsedTextureMemory() const {return 0;};
	virtual	void			startProfileVBHardLock() {};
	virtual	void			endProfileVBHardLock(std::vector<std::string> &result) {};
	virtual	void			profileVBHardAllocation(std::vector<std::string> &result) {};

	// Misc
	virtual TMessageBoxId	systemMessageBox (const char* message, const char* title, TMessageBoxType type=okType, TMessageBoxIcon icon=noIcon);

	// Inputs
	virtual void			showCursor (bool b);
	virtual void			setMousePos(float x, float y);
	virtual void			setCapture (bool b);
	virtual NLMISC::IMouseDevice			*enableLowLevelMouse(bool enable, bool exclusive);
	virtual NLMISC::IKeyboardDevice			*enableLowLevelKeyboard(bool enable);
	virtual NLMISC::IInputDeviceManager		*getLowLevelInputDeviceManager();
	virtual uint							 getDoubleClickDelay(bool hardwareMouse);

	// Lights
	virtual uint			getMaxLight () const {return 0;};
	virtual void			setLight (uint8 num, const CLight& light);
	virtual void			enableLight (uint8 num, bool enable=true);
	virtual void			setPerPixelLightingLight(CRGBA diffuse, CRGBA specular, float shininess) {};
	virtual void			setAmbientColor (CRGBA color) {};

	// Fog
	virtual	bool			fogEnabled();
	virtual	void			enableFog(bool enable);
	virtual	void			setupFog(float start, float end, CRGBA color);
	virtual	float			getFogStart() const;
	virtual	float			getFogEnd() const;
	virtual	CRGBA			getFogColor() const;

	// Texture addressing modes
	virtual bool			supportTextureShaders() const {return false;};
	virtual bool			isWaterShaderSupported() const {return false;};
	virtual bool			isTextureAddrModeSupported(CMaterial::TTexAddressingMode mode) const {return false;};
	virtual void			setMatrix2DForTextureOffsetAddrMode(const uint stage, const float mat[4]) {};

	// EMBM support
	virtual bool			supportEMBM() const	 {return false;};
	virtual bool			isEMBMSupportedAtStage(uint stage) const {return false;};
	virtual void			setEMBMMatrix(const uint stage, const float mat[4]) {};
	virtual bool			supportPerPixelLighting(bool specular) const {return false;};

	// Blend
	virtual	bool			supportBlendConstantColor() const {return false;};
	virtual	void			setBlendConstantColor(NLMISC::CRGBA col) {};
	virtual	NLMISC::CRGBA	getBlendConstantColor() const {return CRGBA::White;};

	// Monitor properties
	virtual bool			setMonitorColorProperties (const CMonitorColorProperties &properties) {return false;};

	// Polygon smoothing
	virtual	void			enablePolygonSmoothing(bool smooth) {};
	virtual	bool			isPolygonSmoothingEnabled() const {return false;};

	// Material multipass
	virtual sint			beginMaterialMultiPass() { return 0; }
	virtual void			setupMaterialPass(uint pass) { 	}
	virtual void			endMaterialMultiPass() {  }

	// Vertex program
	virtual bool			isVertexProgramSupported () const;
	virtual bool			isVertexProgramEmulated () const;
	virtual bool			activeVertexProgram (CVertexProgram *program);
	virtual void			setConstant (uint index, float, float, float, float);
	virtual void			setConstant (uint index, double, double, double, double);
	virtual void			setConstant (uint index, const NLMISC::CVector& value);
	virtual void			setConstant (uint index, const NLMISC::CVectorD& value);
	virtual void			setConstant (uint index, uint num, const float *src);
	virtual void			setConstant (uint index, uint num, const double *src);
	virtual void			setConstantMatrix (uint index, IDriver::TMatrix matrix, IDriver::TTransform transform);
	virtual void			setConstantFog (uint index);
	virtual void			enableVertexProgramDoubleSidedColor(bool doubleSided);
	virtual bool		    supportVertexProgramDoubleSidedColor() const;
	

	virtual bool			supportCloudRenderSinglePass () const { return false; }
	/** Shader implementation
	  * 
	  * Shader can assume this states are setuped:
	  * TextureTransformFlags[n] = DISABLE;
	  * TexCoordIndex[n] = n;
	  * ColorOp[n] = DISABLE;
	  * AlphaOp[n] = DISABLE;
    */
	virtual bool			activeShader(CShader *shd);

private:

	// Hardware render variables, like matrices, render states
	struct CRenderVariable
	{
		CRenderVariable()
		{
			NextModified = NULL;
			Modified = false;
		}

		// Type of render state
		enum
		{
			RenderState = 0,
			TextureState,
			TextureIndexState,
			TexturePtrState,
			VertexProgramPtrState,
			PixelShaderPtrState,
			VertexProgramConstantState,
			PixelShaderConstantState,
			SamplerState,
			MatrixState,
			VBState,
			IBState,
			LightState,
			RenderTargetState,
		}				Type;
		bool			Modified;
		CRenderVariable	*NextModified;
	};

	// Render state
	struct CRenderState : public CRenderVariable
	{
		CRenderState()
		{
			Type = RenderState;
		}
		D3DRENDERSTATETYPE	StateID;
		DWORD				Value;
	};

	// Render texture state
	struct CTextureState : public CRenderVariable
	{
		CTextureState()
		{
			Type = TextureState;
		}
		DWORD						StageID;
		D3DTEXTURESTAGESTATETYPE	StateID;
		DWORD						Value;
	};

	// Render texture index state
	struct CTextureIndexState : public CRenderVariable
	{
		CTextureIndexState()
		{
			Type = TextureIndexState;
		}
		DWORD						StageID;
		DWORD						TexGenMode;
		DWORD						UVChannel;
		bool						TexGen;
	};

	// Render texture
	struct CTexturePtrState : public CRenderVariable
	{
		CTexturePtrState()
		{
			Type = TexturePtrState;
			Texture = NULL;
		}
		DWORD						StageID;
		LPDIRECT3DBASETEXTURE9		Texture;
	};

	// Render texture
	struct CVertexProgramPtrState : public CRenderVariable
	{
		CVertexProgramPtrState()
		{
			Type = VertexProgramPtrState;
			VertexProgram = NULL;
		}
		LPDIRECT3DVERTEXSHADER9		VertexProgram;
	};

	// Render texture
	struct CPixelShaderPtrState : public CRenderVariable
	{
		CPixelShaderPtrState()
		{
			Type = PixelShaderPtrState;
			PixelShader = NULL;
		}
		LPDIRECT3DPIXELSHADER9		PixelShader;
	};

	// Vertex buffer constants state
	struct CVertexProgramConstantState : public CRenderVariable
	{
		CVertexProgramConstantState()
		{
			Type = VertexProgramConstantState;
		}
		enum 
		{
			Float= 0,
			Int,
			Undef,
		}							ValueType;
		uint						StateID;
		DWORD						Values[4];
	};

	// Pixel shader constants state
	struct CPixelShaderConstantState : public CRenderVariable
	{
		CPixelShaderConstantState()
		{
			Type = PixelShaderConstantState;
		}
		enum 
		{
			Float= 0,
			Int,
			Undef,
		}							ValueType;
		uint						StateID;
		DWORD						Values[4];
	};

	// Render sampler state
	struct CSamplerState : public CRenderVariable
	{
		CSamplerState()
		{
			Type = SamplerState;
		}
		DWORD						SamplerID;
		D3DSAMPLERSTATETYPE			StateID;
		DWORD						Value;
	};

	// Render matrix
	struct CMatrixState : public CRenderVariable
	{
		CMatrixState()
		{
			Type = MatrixState;
		}
		D3DTRANSFORMSTATETYPE	TransformType;
		D3DXMATRIX				Matrix;
	};

	// Render vertex buffer
	struct CVBState : public CRenderVariable
	{
		CVBState()
		{
			Type = VBState;
			VertexBuffer = NULL;
		}
		CRefPtr<CVertexBuffer>			VertexBuffer;
	};

	// Render index buffer
	struct CIBState : public CRenderVariable
	{
		CIBState()
		{
			Type = IBState;
			IndexBuffer = NULL;
		}
		CRefPtr<CIndexBuffer>			IndexBuffer;
	};

	// Render vertex buffer
	struct CLightState : public CRenderVariable
	{
		CLightState()
		{
			Type = LightState;
			Enabled = false;
			SettingsTouched = true;
			EnabledTouched = true;
			Light.Type = D3DLIGHT_POINT;
			Light.Range = 1;
			Light.Falloff = 1;
			Light.Position.x = 0;
			Light.Position.y = 0;
			Light.Position.z = 0;
			Light.Direction.x = 1;
			Light.Direction.y = 0;
			Light.Direction.z = 0;
			Light.Attenuation0 = 1;
			Light.Attenuation1 = 1;
			Light.Attenuation2 = 1;
			Light.Theta = 0;
			Light.Phi = 0;
		}
		uint8				LightIndex;
		bool				SettingsTouched;
		bool				EnabledTouched;
		bool				Enabled;
		D3DLIGHT9			Light;
	};

	// Render target
	struct CRenderTargetState : public CRenderVariable
	{
		CRenderTargetState()
		{
			Type = RenderTargetState;
			Target = NULL;
			Texture = NULL;
			Level = 0;
			CubeFace = 0;
		}
		IDirect3DSurface9	*Target;
		ITexture			*Texture;
		uint8				Level;
		uint8				CubeFace;
	};

	// Friends
	friend void D3DWndProc(CDriverD3D *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	friend class CTextureDrvInfosD3D;

	// Reset render states
	void resetRenderVariables();

	// Update all modified render states
	void updateRenderVariables();

	// Touch a render variable
	inline void touchRenderVariable (CRenderVariable *renderVariable)
	{
		// Modified ?
		if (!renderVariable->Modified)
		{
			// Link to modified states
			renderVariable->NextModified = _ModifiedRenderState;
			_ModifiedRenderState = renderVariable;
			renderVariable->Modified = true;
		}
	}

	// Access render states
	inline void setRenderState (D3DRENDERSTATETYPE renderState, DWORD value)
	{
		nlassert (_DeviceInterface);
		nlassert (renderState<MaxRenderState);

		// Ref on the state
		CRenderState &_renderState = _RenderStateCache[renderState];
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		if (_renderState.Value != value)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			_renderState.Value = value;
			touchRenderVariable (&_renderState);
		}
	}

	// Access texture states
	inline void setTextureState (DWORD stage, D3DTEXTURESTAGESTATETYPE textureState, DWORD value)
	{
		nlassert (_DeviceInterface);
		nlassert (stage<MaxTexture);
		nlassert (textureState<MaxTextureState);

		// Ref on the state
		CTextureState &_textureState = _TextureStateCache[stage][textureState];
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		if (_textureState.Value != value)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			_textureState.Value = value;
			touchRenderVariable (&_textureState);
		}
	}

	// Access texture index states
	inline void setTextureIndexMode (DWORD stage, bool texGenEnabled, DWORD value)
	{
		nlassert (_DeviceInterface);
		nlassert (stage<MaxTexture);

		// Ref on the state
		CTextureIndexState &_textureState = _TextureIndexStateCache[stage];
		_textureState.TexGen = texGenEnabled;
		_textureState.TexGenMode = value;
		touchRenderVariable (&_textureState);
	}

	// Access texture index states
	inline void setTextureIndexUV (DWORD stage, DWORD value)
	{
		nlassert (_DeviceInterface);
		nlassert (stage<MaxTexture);

		// Ref on the state
		CTextureIndexState &_textureState = _TextureIndexStateCache[stage];
		_textureState.UVChannel = value;
		touchRenderVariable (&_textureState);
	}

	// Access texture states
	inline void setTexture (DWORD stage, LPDIRECT3DBASETEXTURE9 texture)
	{
		nlassert (_DeviceInterface);
		nlassert (stage<MaxTexture);

		// Ref on the state
		CTexturePtrState &_textureState = _TexturePtrStateCache[stage];
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		if (_textureState.Texture != texture)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			_textureState.Texture = texture;
			touchRenderVariable (&_textureState);
		}
	}

	// Set a NeL texture states
	inline void setTexture (DWORD stage, ITexture *texture)
	{
		nlassert (_DeviceInterface);
		nlassert (stage<MaxTexture);

		// Set the texture parameters
		CTextureDrvInfosD3D *d3dtext = getTextureD3D(*texture);
		setTexture (stage, d3dtext->Texture);
		setSamplerState (stage, D3DSAMP_ADDRESSU, d3dtext->WrapS);
		setSamplerState (stage, D3DSAMP_ADDRESSV, d3dtext->WrapT);
		setSamplerState (stage, D3DSAMP_MAGFILTER, d3dtext->MagFilter);
		setSamplerState (stage, D3DSAMP_MINFILTER, d3dtext->MinFilter);
		setSamplerState (stage, D3DSAMP_MIPFILTER, d3dtext->MipFilter);
	}

	// Access vertex program
	inline void setVertexProgram (LPDIRECT3DVERTEXSHADER9 vertexProgram)
	{
		nlassert (_DeviceInterface);

		// Ref on the state
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		if (_VertexProgramCache.VertexProgram != vertexProgram)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			_VertexProgramCache.VertexProgram = vertexProgram;
			touchRenderVariable (&_VertexProgramCache);
		}
	}

	// Access pixel shader
	inline void setPixelShader (LPDIRECT3DPIXELSHADER9 pixelShader)
	{
		nlassert (_DeviceInterface);

		// Ref on the state
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		if (_PixelShaderCache.PixelShader != pixelShader)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			_PixelShaderCache.PixelShader = pixelShader;
			touchRenderVariable (&_PixelShaderCache);
		}
	}

	// Access vertex program constant
	inline void setVertexProgramConstant (uint index, const float *values)
	{
		nlassert (_DeviceInterface);
		nlassert (index<MaxVertexProgramConstantState);

		// Ref on the state
		CVertexProgramConstantState &state = _VertexProgramConstantCache[index];
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		if ( ((*(float*)(state.Values+0)) != values[0]) ||
			((*(float*)(state.Values+1)) != values[1]) ||
			((*(float*)(state.Values+2)) != values[2]) ||
			((*(float*)(state.Values+3)) != values[3]) || 
			(state.ValueType != CVertexProgramConstantState::Float) )
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			*(float*)(state.Values+0) = values[0];
			*(float*)(state.Values+1) = values[1];
			*(float*)(state.Values+2) = values[2];
			*(float*)(state.Values+3) = values[3];
			state.ValueType = CVertexProgramConstantState::Float;
			touchRenderVariable (&state);
		}
	}

	// Access vertex program constant
	inline void setVertexProgramConstant (uint index, const int *values)
	{
		nlassert (_DeviceInterface);
		nlassert (index<MaxVertexProgramConstantState);

		// Ref on the state
		CVertexProgramConstantState &state = _VertexProgramConstantCache[index];
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		if ( ((*(int*)(state.Values+0)) != values[0]) ||
			((*(int*)(state.Values+1)) != values[1]) ||
			((*(int*)(state.Values+2)) != values[2]) ||
			((*(int*)(state.Values+3)) != values[3]) || 
			(state.ValueType != CVertexProgramConstantState::Int) )
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			*(int*)(state.Values+0) = values[0];
			*(int*)(state.Values+1) = values[1];
			*(int*)(state.Values+2) = values[2];
			*(int*)(state.Values+3) = values[3];
			state.ValueType = CVertexProgramConstantState::Int;
			touchRenderVariable (&state);
		}
	}

	// Access vertex program constant
	inline void setPixelShaderConstant (uint index, const float *values)
	{
		nlassert (_DeviceInterface);
		nlassert (index<MaxPixelShaderConstantState);

		// Ref on the state
		CPixelShaderConstantState &state = _PixelShaderConstantCache[index];
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		if ( ((*(float*)(state.Values+0)) != values[0]) ||
			((*(float*)(state.Values+1)) != values[1]) ||
			((*(float*)(state.Values+2)) != values[2]) ||
			((*(float*)(state.Values+3)) != values[3]) || 
			(state.ValueType != CPixelShaderConstantState::Float) )
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			*(float*)(state.Values+0) = values[0];
			*(float*)(state.Values+1) = values[1];
			*(float*)(state.Values+2) = values[2];
			*(float*)(state.Values+3) = values[3];
			state.ValueType = CPixelShaderConstantState::Float;
			touchRenderVariable (&state);
		}
	}

	// Access vertex program constant
	inline void setPixelShaderConstant (uint index, const int *values)
	{
		nlassert (_DeviceInterface);
		nlassert (index<MaxPixelShaderConstantState);

		// Ref on the state
		CPixelShaderConstantState &state = _PixelShaderConstantCache[index];
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		if ( ((*(int*)(state.Values+0)) != values[0]) ||
			((*(int*)(state.Values+1)) != values[1]) ||
			((*(int*)(state.Values+2)) != values[2]) ||
			((*(int*)(state.Values+3)) != values[3]) || 
			(state.ValueType != CPixelShaderConstantState::Int) )
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			*(int*)(state.Values+0) = values[0];
			*(int*)(state.Values+1) = values[1];
			*(int*)(state.Values+2) = values[2];
			*(int*)(state.Values+3) = values[3];
			state.ValueType = CPixelShaderConstantState::Int;
			touchRenderVariable (&state);
		}
	}

	// Access sampler states
	inline void setSamplerState (DWORD sampler, D3DSAMPLERSTATETYPE samplerState, DWORD value)
	{
		nlassert (_DeviceInterface);
		nlassert (sampler<MaxSampler);
		nlassert (samplerState<MaxSamplerState);

		// Ref on the state
		CSamplerState &_samplerState = _SamplerStateCache[sampler][samplerState];
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		if (_samplerState.Value != value)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			_samplerState.Value = value;
			touchRenderVariable (&_samplerState);
		}
	}

	// Access matrices
	inline uint remapMatrixIndex (D3DTRANSFORMSTATETYPE type)
	{
		if (type>=256)
			return (D3DTRANSFORMSTATETYPE)(MatrixStateRemap + type - 256);
		else
			return (uint)type;
	}
	inline void setMatrix (D3DTRANSFORMSTATETYPE type, const D3DXMATRIX &matrix)
	{
		nlassert (_DeviceInterface);

		// Remap high matrices indexes
		type = (D3DTRANSFORMSTATETYPE)remapMatrixIndex (type);
		nlassert (type<MaxMatrixState);

		CMatrixState &theMatrix = _MatrixCache[type];
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		if ((matrix._11 != theMatrix.Matrix._11) ||
			(matrix._12 != theMatrix.Matrix._12) ||
			(matrix._13 != theMatrix.Matrix._13) ||
			(matrix._14 != theMatrix.Matrix._14) ||
			(matrix._21 != theMatrix.Matrix._21) ||
			(matrix._22 != theMatrix.Matrix._22) ||
			(matrix._23 != theMatrix.Matrix._23) ||
			(matrix._24 != theMatrix.Matrix._24) ||
			(matrix._31 != theMatrix.Matrix._31) ||
			(matrix._32 != theMatrix.Matrix._32) ||
			(matrix._33 != theMatrix.Matrix._33) ||
			(matrix._34 != theMatrix.Matrix._34) ||
			(matrix._41 != theMatrix.Matrix._41) ||
			(matrix._42 != theMatrix.Matrix._42) ||
			(matrix._43 != theMatrix.Matrix._43) ||
			(matrix._44 != theMatrix.Matrix._44))
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			theMatrix.Matrix = matrix;
			touchRenderVariable (&theMatrix);
		}
	}

	// Access texture states
	inline void setRenderTarget (IDirect3DSurface9 *target, ITexture *texture, uint8 level, uint8 cubeFace)
	{
		nlassert (_DeviceInterface);

		// Ref on the state
#ifdef NL_D3D_USE_RENDER_STATE_CACHE
		if (_RenderTarget.Target != target)
#endif // NL_D3D_USE_RENDER_STATE_CACHE
		{
			_RenderTarget.Target = target;
			_RenderTarget.Texture = texture;
			_RenderTarget.Level = level;
			_RenderTarget.CubeFace = cubeFace;

			touchRenderVariable (&_RenderTarget);
		}
	}

	// *** Inline info

	sint			inlGetNumTextStages() const {return _NbNeLTextureStages;}

	// Get the d3dtext mirror of an existing setuped texture.
	static	inline CTextureDrvInfosD3D*	getTextureD3D(ITexture& tex)
	{
		CTextureDrvInfosD3D*	d3dtex;
		d3dtex= (CTextureDrvInfosD3D*)(ITextureDrvInfos*)(tex.TextureDrvShare->DrvTexture);
		return d3dtex;
	}

	// Get the d3dtext mirror of an existing setuped vertex program.
	static	inline CVertexProgamDrvInfosD3D*	getVertexProgramD3D(CVertexProgram& vertexProgram)
	{
		CVertexProgamDrvInfosD3D*	d3dVertexProgram;
		d3dVertexProgram = (CVertexProgamDrvInfosD3D*)(IVertexProgramDrvInfos*)(vertexProgram._DrvInfo);
		return d3dVertexProgram;
	}

	// *** Init helper

	bool isDepthFormatOk(UINT adapter, D3DFORMAT DepthFormat, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat);
	bool isTextureFormatOk(UINT adapter, D3DFORMAT TextureFormat, D3DFORMAT AdapterFormat);

	// *** Texture helper

	D3DFORMAT getD3DDestTextureFormat (ITexture& tex);
	// Generates the texture, degades it, creates / resizes the d3d surface. Don't fill the surface.
	bool generateD3DTexture (ITexture& tex, bool textureDegradation, D3DFORMAT &destFormat, D3DFORMAT &srcFormat, uint &firstMipMap, bool &cube, 
		bool renderTarget);
	// Return the memory used by the surface described iwith the parameters
	uint32 computeTextureMemoryUsage (uint width, uint height, uint levels, D3DFORMAT destFormat, bool cube);
	// Upload a texture part
	bool uploadTextureInternal (ITexture& tex, NLMISC::CRect& rect, uint8 destMipmap, uint8 srcMipmap, D3DFORMAT destFormat, D3DFORMAT srcFormat);
	// Internal setuptexture method
	bool setupTextureInternal (ITexture& tex, bool bUpload, bool &bAllUploaded, bool bMustRecreateSharedTexture, bool renderTarget);

	// *** Matrix helper

	// Update _D3DModelView and _D3DModelViewProjection matrices. Call this if the model, the projection or the view matrix are modified
	void updateMatrices ();
	// Update the projection matrix. Call this if the driver resolution, the viewport or the frustum changes.
	void updateProjectionMatrix ();

	// *** Vertex buffer helper

	// Create a vertex declaration
	bool createVertexDeclaration (uint16 vertexFormat, const uint8 *typeArray,
										IDirect3DVertexDeclaration9 **vertexDecl, 
										uint *stride = NULL);

	// *** Multipass helpers

	void initInternalShaders();
	bool setShaderTexture (uint textureHandle, ITexture *texture);
	
	bool validateShader(CShader *shader);

	void beginMultiPass ()
	{
		if (_CurrentShader)
		{
			// Does the shader validated ?
			validateShader(_CurrentShader);

			// Init default state value
			uint i;
			for (i=0; i<MaxTexture; i++)
			{
				setTextureState (i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
				setTextureIndexMode (i, false, D3DTSS_TCI_PASSTHRU);
				setTextureIndexUV (i, i);
				setTextureState (i, D3DTSS_COLOROP, D3DTOP_DISABLE);
				setTextureState (i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			}

			CShaderDrvInfosD3D *drvInfo = static_cast<CShaderDrvInfosD3D*>((IShaderDrvInfos*)_CurrentShader->_DrvInfo);
			drvInfo->Effect->Begin (&_CurrentShaderPassCount, D3DXFX_DONOTSAVESTATE|D3DXFX_DONOTSAVESHADERSTATE);
		}
		else
			// No shader setuped
			_CurrentShaderPassCount = 1;
	};

	void activePass (uint pass)
	{
		if (_CurrentShader)
		{
			CShaderDrvInfosD3D *drvInfo = static_cast<CShaderDrvInfosD3D*>((IShaderDrvInfos*)_CurrentShader->_DrvInfo);
			drvInfo->Effect->Pass (pass);
		}

		// Update render states
		updateRenderVariables();
	}

	void endMultiPass ()
	{
		if (_CurrentShader)
		{
			CShaderDrvInfosD3D *drvInfo = static_cast<CShaderDrvInfosD3D*>((IShaderDrvInfos*)_CurrentShader->_DrvInfo);
			drvInfo->Effect->End ();
		}
	}

	// Returns true if this material needs to compute the alpha component
	static bool needsAlpha (CMaterial &mat);

	// Returns true if this material needs a constant color for the diffuse component
	static bool needsConstantForDiffuse (CMaterial &mat, bool needAlpha);

	/* Returns true if this normal needs constant. If true, numConstant is the number of needed constants, firstConstant is the first
	constants needed. */ 
	static bool needsConstants (uint &numConstant, uint &firstConstant, CMaterial &mat, bool needAlpha);

	// Build a pixel shader for normal shader
	IDirect3DPixelShader9	*buildPixelShader (const CNormalShaderDesc &normalShaderDesc, bool unlightedNoVertexColor);


	// *** Debug helpers

	void setDebugMaterial();

	// ** From ID3DXEffectStateManager 
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *ppvObj);
	ULONG STDMETHODCALLTYPE AddRef(VOID);
	ULONG STDMETHODCALLTYPE Release(VOID);
	HRESULT STDMETHODCALLTYPE LightEnable(DWORD Index, BOOL Enable);
	HRESULT STDMETHODCALLTYPE SetFVF(DWORD FVF);
	HRESULT STDMETHODCALLTYPE SetLight(DWORD Index, CONST D3DLIGHT9* pLight);
	HRESULT STDMETHODCALLTYPE SetMaterial(CONST D3DMATERIAL9* pMaterial);
	HRESULT STDMETHODCALLTYPE SetNPatchMode(FLOAT nSegments);
	HRESULT STDMETHODCALLTYPE SetPixelShader(LPDIRECT3DPIXELSHADER9 pShader);
	HRESULT STDMETHODCALLTYPE SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT RegisterCount);
	HRESULT STDMETHODCALLTYPE SetPixelShaderConstantF(UINT StartRegister, CONST FLOAT* pConstantData, UINT RegisterCount);
	HRESULT STDMETHODCALLTYPE SetPixelShaderConstantI(UINT StartRegister, CONST INT* pConstantData, UINT RegisterCount);
	HRESULT STDMETHODCALLTYPE SetRenderState(D3DRENDERSTATETYPE State, DWORD Value);
	HRESULT STDMETHODCALLTYPE SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value);
	HRESULT STDMETHODCALLTYPE SetTexture (DWORD Stage, LPDIRECT3DBASETEXTURE9 pTexture);
	HRESULT STDMETHODCALLTYPE SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
	HRESULT STDMETHODCALLTYPE SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix);
	HRESULT STDMETHODCALLTYPE SetVertexShader(LPDIRECT3DVERTEXSHADER9 pShader);
	HRESULT STDMETHODCALLTYPE SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT RegisterCount);
	HRESULT STDMETHODCALLTYPE SetVertexShaderConstantF(UINT StartRegister, CONST FLOAT* pConstantData, UINT RegisterCount);
	HRESULT STDMETHODCALLTYPE SetVertexShaderConstantI(UINT StartRegister, CONST INT* pConstantData, UINT RegisterCount);

	// Windows
	std::string				_WindowClass;
	HWND					_HWnd;
	uint32					_WindowWidth;
	uint32					_WindowHeight;
	sint32					_WindowX;
	sint32					_WindowY;
	sint8					_Depth;
	bool					_DestroyWindow;

	// Directx
	uint32					_Adapter;
	LPDIRECT3D9				_D3D;
	IDirect3DDevice9		*_DeviceInterface;

	// Events
	NLMISC::CEventEmitterMulti	_EventEmitter; // this can contains a win emitter and eventually a direct input emitter	

	// Some matrices (Local -> Model -> World -> Screen)
	CVector					_PZBCameraPos;
	CMatrix					_UserModelMtx;
	CMatrix					_UserViewMtx;
	CViewport				_Viewport;
	CScissor				_Scissor;
	float					_OODeltaZ;	// Backup znear and zfar
	D3DXMATRIX				_D3DSpecularWorldTex;		// World (as in NeL) to model matrix.
	D3DXMATRIX				_D3DModelView;				// Local to world (as in D3D) matrix.
	D3DXMATRIX				_D3DModelViewProjection;	// Local to screen (as in D3D) matrix.
	D3DXMATRIX				_D3DInvModelView;			// World (as in DX) to local matrix.
	bool					_ForceNormalize;
	bool					_FrustumPerspective;
	bool					_InvertCullMode;
	bool					_DoubleSided;
	float					_FrustumLeft;
	float					_FrustumRight;
	float					_FrustumTop;
	float					_FrustumBottom;
	float					_FrustumZNear;
	float					_FrustumZFar;
	float					_FogStart;
	float					_FogEnd;


	// Textures caps
	D3DFORMAT				_PreferedTextureFormat[ITexture::UploadFormatCount];
	uint					_ForceTextureResizePower;
	bool					_ForceDXTCCompression:1;
	bool					_TextureCubeSupported;
	bool					_VertexProgram;
	bool					_PixelShader;
	bool					_DisableHardwareVertexProgram;
	bool					_DisableHardwareVertexArrayAGP;
	bool					_DisableHardwareIndexArrayAGP;
	sint					_NbNeLTextureStages;			// Number of texture stage for NeL (max IDRV_MAT_MAXTEXTURES)
	// sint					_NbTextureStages;				// Number of texture stage available
	uint					_MaxVerticesByVertexBufferHard;

	// Profiling
	uint32					_AllocatedTextureMemory;

	// The render variables
	CRenderState			_RenderStateCache[MaxRenderState];
	CTextureState			_TextureStateCache[MaxTexture][MaxTextureState];
	CTextureIndexState		_TextureIndexStateCache[MaxTexture];
	CTexturePtrState		_TexturePtrStateCache[MaxTexture];
	CSamplerState			_SamplerStateCache[MaxSampler][MaxSamplerState];
	CMatrixState			_MatrixCache[MaxMatrixState];
	CVertexProgramPtrState	_VertexProgramCache;
	CPixelShaderPtrState	_PixelShaderCache;
	CVertexProgramConstantState	_VertexProgramConstantCache[MaxVertexProgramConstantState];
	CPixelShaderConstantState	_PixelShaderConstantCache[MaxPixelShaderConstantState];
	CVBState				_VertexBuffer;
	CIBState				_IndexBuffer;
	CLightState				_LightCache[MaxLight];
	CRenderTargetState		_RenderTarget;

	// The last vertex buffer needs vertex color
	bool					_UseVertexColor;
	bool					_FogEnabled;

	// *** Internal resources

	// Vertex declaration list
	std::list<CVertexDeclaration>	_VertexDeclarationList;

	// Pixel shader list
	std::list<CNormalShaderDesc>	_NormalPixelShaders[2];

	// Quad indexes
	CIndexBuffer			_QuadIndexes;

	// The last setuped shader
	CShader					*_CurrentShader;
	UINT					_CurrentShaderPassCount;
	struct CTextureRef
	{
		CRefPtr<ITexture>		NeLTexture;
		LPDIRECT3DBASETEXTURE9	D3DTexture;
	};
	std::vector<CTextureRef>	_CurrentShaderTextures;


	// The last material setuped
	CMaterial				*_CurrentMaterial;
	CMaterialDrvInfosD3D	*_CurrentMaterialInfo;

	// Optim: To not test change in Materials states if just texture has changed. Very usefull for landscape.
	uint32					_MaterialAllTextureTouchedFlag;

	// The modified render variables list
	CRenderVariable			*_ModifiedRenderState;

	// Internal shaders
	CShader					_ShaderLightmap0;
	CShader					_ShaderLightmap1;
	CShader					_ShaderLightmap2;
	CShader					_ShaderLightmap3;
	CShader					_ShaderLightmap4;
	CShader					_ShaderLightmap0Blend;
	CShader					_ShaderLightmap1Blend;
	CShader					_ShaderLightmap2Blend;
	CShader					_ShaderLightmap3Blend;
	CShader					_ShaderLightmap4Blend;

	// Backup frame buffer
	IDirect3DSurface9		*_BackBuffer;

	// Static bitmap buffer to convert rgba to bgra
	static std::vector<uint8>	_TempBuffer;

	// Version of the driver. Not the interface version!! Increment when implementation of the driver change.
	static const uint32		ReleaseVersion;
};

#define NL_D3DCOLOR_RGBA(rgba) (D3DCOLOR_ARGB(rgba.A,rgba.R,rgba.G,rgba.B))
#define D3DCOLOR_NL_RGBA(rgba) (NLMISC::CRGBA((uint8)((rgba>>16)&0xff), (uint8)((rgba>>8)&0xff), (uint8)(rgba&0xff), (uint8)((rgba>>24)&0xff)))
#define NL_D3DCOLORVALUE_RGBA(dest,rgba) \
	dest.a=(float)rgba.A/255.f;dest.r=(float)rgba.R/255.f;dest.g=(float)rgba.G/255.f;dest.b=(float)rgba.B/255.f;
#define NL_D3D_MATRIX(d3d_mt,nl_mt) \
	{	const float *nl_mt_ptr = nl_mt.get(); \
	d3d_mt._11 = nl_mt_ptr[0]; \
	d3d_mt._21 = nl_mt_ptr[4]; \
	d3d_mt._31 = nl_mt_ptr[8]; \
	d3d_mt._41 = nl_mt_ptr[12]; \
	d3d_mt._12 = nl_mt_ptr[1]; \
	d3d_mt._22 = nl_mt_ptr[5]; \
	d3d_mt._32 = nl_mt_ptr[9]; \
	d3d_mt._42 = nl_mt_ptr[13]; \
	d3d_mt._13 = nl_mt_ptr[2]; \
	d3d_mt._23 = nl_mt_ptr[6]; \
	d3d_mt._33 = nl_mt_ptr[10]; \
	d3d_mt._43 = nl_mt_ptr[14]; \
	d3d_mt._14 = nl_mt_ptr[3]; \
	d3d_mt._24 = nl_mt_ptr[7]; \
	d3d_mt._34 = nl_mt_ptr[11]; \
	d3d_mt._44 = nl_mt_ptr[15]; }
#define NL_D3DVECTOR_VECTOR(dest,vect) dest.x=(vect).x;dest.y=(vect).y;dest.z=(vect).z;
#define FTODW(f) (*((DWORD*)&(f)))
#define D3DCOLOR_FLOATS(floats,rgba) {floats[0]=(float)((rgba>>16)&0xff) * (1.f/255.f);floats[1]=(float)((rgba>>8)&0xff) * (1.f/255.f);\
	floats[2]=(float)(rgba&0xff) * (1.f/255.f);floats[3]=(float)((rgba>>24)&0xff) * (1.f/255.f);}
#define NL_FLOATS(floats,rgba) {floats[0] = (float)rgba.R * (1.f/255.f);floats[1] = (float)rgba.G * (1.f/255.f);\
	floats[2] = (float)rgba.B * (1.f/255.f);floats[3] = (float)rgba.A * (1.f/255.f);}

// ***************************************************************************

} // NL3D

extern HINSTANCE HInstDLL;

#endif // NL_DRIVER_DIRECT3D_H