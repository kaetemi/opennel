/** \file driver_opengl.h
 * OpenGL driver implementation
 *
 * $Id: driver_opengl.h,v 1.108 2002/01/18 10:08:12 berenguier Exp $
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

#ifndef NL_OPENGL_H
#define NL_OPENGL_H


#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#  ifdef min
#    undef min
#  endif
#  ifdef max
#    undef max
#  endif

#else // NL_OS_UNIX

#define	GL_GLEXT_PROTOTYPES
#include <GL/glx.h>

#ifdef XF86VIDMODE
#include <X11/extensions/xf86vmode.h>
#endif //XF86VIDMODE

#endif // NL_OS_UNIX

#include <GL/gl.h>
#include "driver_opengl_extension.h"

#include "3d/driver.h"
#include "3d/material.h"
#include "3d/shader.h"
#include "3d/vertex_buffer.h"
#include "3d/vertex_buffer_hard.h"
#include "nel/misc/matrix.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/misc/event_emitter.h"
#include "nel/misc/bit_set.h"
#include "3d/ptr_set.h"
#include "nel/misc/heap_memory.h"
#include "driver_opengl_states.h"



#ifdef NL_OS_WINDOWS
#include "nel/misc/win_event_emitter.h"
#elif defined (NL_OS_UNIX)
#include "unix_event_emitter.h"
#endif // NL_OS_UNIX


// For optimisation consideration, allow 256 lightmaps at max.
#define	NL3D_DRV_MAX_LIGHTMAP		256


namespace NL3D {

using NLMISC::CMatrix;
using NLMISC::CVector;


class	CDriverGL;


// --------------------------------------------------

class CTextureDrvInfosGL : public ITextureDrvInfos
{
public:
	// The GL Id.
	GLuint					ID;
	// Is the internal format of the texture is a compressed one?
	bool					Compressed;

	// This is the computed size of what memory this texture take.
	uint32					TextureMemory;
	// This is the owner driver.
	CDriverGL				*_Driver;


	// The current wrap modes assigned to the texture.
	ITexture::TWrapMode		WrapS;
	ITexture::TWrapMode		WrapT;
	ITexture::TMagFilter	MagFilter;
	ITexture::TMinFilter	MinFilter;

	// The gl id is auto created here.
	CTextureDrvInfosGL(IDriver *drv, ItTexDrvInfoPtrMap it, CDriverGL *drvGl);
	// The gl id is auto deleted here.
	~CTextureDrvInfosGL();
};



// --------------------------------------------------

class CVBDrvInfosGL : public IVBDrvInfos
{
public:
	// Software Skinning: post-rendered vertices/normales.
	std::vector<CVector>	SoftSkinVertices;
	std::vector<CVector>	SoftSkinNormals;
	// Software Skinning: flags to know what vertex must be computed.
	std::vector<uint8>		SoftSkinFlags;

	CVBDrvInfosGL(IDriver *drv, ItVBDrvInfoPtrList it) : IVBDrvInfos(drv, it) {}
};

// --------------------------------------------------

class CShaderGL : public IShader
{
public:
	GLenum		SrcBlend;
	GLenum		DstBlend;
	GLenum		ZComp;

	GLfloat		Emissive[4];
	GLfloat		Ambient[4];
	GLfloat		Diffuse[4];
	GLfloat		Specular[4];
	// For fast comp.
	uint32		PackedEmissive;
	uint32		PackedAmbient;
	uint32		PackedDiffuse;
	uint32		PackedSpecular;

	CShaderGL(IDriver *drv, ItShaderPtrList it) : IShader(drv, it) {}
};


// --------------------------------------------------


/** Work only if ARRAY_RANGE_NV is enabled. else, only call to ctor/dtor/free() is valid.
 *	any call to allocateVB() will return NULL.
 */
class CVertexArrayRange
{
public:
	CVertexArrayRange()
	{
		_Driver= NULL;
		_VertexArrayPtr= NULL;
		_VertexArraySize= 0;
	}


	void			init(CDriverGL *drv)
	{
		_Driver= drv;
	}

	bool			allocated() const {return _VertexArrayPtr!=NULL;}


	/// allocate a vertex array sapce. false if error. must free before re-allocate.
	bool			allocate(uint32 size, IDriver::TVBHardType vbType);
	/// free this space.
	void			free();


	// Those methods read/write in _Driver->_CurrentVertexArrayRange.
	/// active this VertexArrayRange as the current vertex array range used. no-op if already setup.
	void			enable();
	/// disable this VertexArrayRange. _Driver->_CurrentVertexArrayRange= NULL;
	void			disable();


	/// Allocate a small subset of the memory. NULL if not enough mem.
	void			*allocateVB(uint32 size);
	/// free a VB allocated with allocateVB. No-op if NULL.
	void			freeVB(void	*ptr);


private:
	CDriverGL	*_Driver;
	void		*_VertexArrayPtr;
	uint32		_VertexArraySize;

	// Allocator.
	NLMISC::CHeapMemory		_HeapMemory;

};



/// Work only if ARRAY_RANGE_NV is enabled.
class CVertexBufferHardGL : public IVertexBufferHard
{
public:

	CVertexBufferHardGL();
	virtual	~CVertexBufferHardGL();

	bool				init(CDriverGL *drv, uint16 vertexFormat, const uint8 *typeArray, uint32 numVertices, IDriver::TVBHardType vbType);

	virtual	void		*lock();
	virtual	void		unlock();


	void			enable();
	void			disable();


	// true if a setFence() has been done, without a finishFence().
	bool			isFenceSet() const {return _FenceSet;}
	// if(!isFenceSet()), set the fence, else no-op
	void			setFence();
	// if(isFenceSet()), finish the fence, else no-op
	void			finishFence();


	// For Fence access.
	bool			GPURenderingAfterFence;

public:
	// NB: do not check if format is OK. return invalid result if format is KO.
	/*void				*getVertexCoordPointer()
	{
		nlassert(_VertexPtr);
		return _VertexPtr;
	}
	void				*getNormalCoordPointer()
	{
		nlassert(_VertexPtr);
		return (uint8*)_VertexPtr + getNormalOff();
	}
	void				*getTexCoordPointer(uint stageid)
	{
		nlassert(_VertexPtr);
		return (uint8*)_VertexPtr + getTexCoordOff(stageid);
	}
	void				*getColorPointer()
	{
		nlassert(_VertexPtr);
		return (uint8*)_VertexPtr + getColorOff();
	}
	void				*getSpecularPointer()
	{
		nlassert(_VertexPtr);
		return (uint8*)_VertexPtr + getSpecularOff();
	}
	void				*getWeightPointer(uint wid)
	{
		nlassert(_VertexPtr);
		return (uint8*)_VertexPtr + getWeightOff(wid);
	}
	void				*getPaletteSkinPointer()
	{
		nlassert(_VertexPtr);
		return (uint8*)_VertexPtr + getPaletteSkinOff();
	}*/
	void				*getValueEx (uint value)
	{
		nlassert(_VertexPtr);
		return (uint8*)_VertexPtr + getValueOff (value);
	}

private:
	CDriverGL			*_Driver;
	CVertexArrayRange	*_VertexArrayRange;
	void				*_VertexPtr;

	// The fence inserted in command stream
	GLuint				_Fence;
	// True if a setFence() has been done, without a finishFence().
	bool				_FenceSet;

};



// --------------------------------------------------
/// Info for the last VertexBuffer setuped (iether normal or hard).
class	CVertexBufferInfo
{
public:
	uint16					VertexFormat;
	uint16					VertexSize;
	uint32					NumVertices;
	uint32					NumWeight;	
	CVertexBuffer::TType	Type[CVertexBuffer::NumValue];

	// NB: ptrs are invalid if VertexFormat does not support the compoennt. must test VertexFormat, not the ptr.
	void					*ValuePtr[CVertexBuffer::NumValue];

	void		setupVertexBuffer(CVertexBuffer &vb);
	void		setupVertexBufferHard(CVertexBufferHardGL &vb);
};


// --------------------------------------------------

class CDriverGL : public IDriver
{
public:

	// Some constants
	enum { MaxLight=8 };

	// Acces
	uint32					getHwnd ()
	{
#ifdef NL_OS_WINDOWS
		return (uint32)_hWnd;
#else // NL_OS_WINDOWS
		return 0;
#endif // NL_OS_WINDOWS
	}

							CDriverGL();
	virtual					~CDriverGL() { release(); };

	virtual bool			init();

	virtual ModeList		enumModes();

	virtual void			disableHardwareVertexProgram();
	virtual void			disableHardwareVertexArrayAGP();
	virtual void			disableHardwareTextureShader();

	virtual bool			setDisplay(void* wnd, const GfxMode& mode) throw(EBadDisplay);

	virtual void*			getDisplay()
	{
#ifdef NL_OS_WINDOWS
		return (void*)_hWnd;
#else // NL_OS_WINDOWS
		return NULL;
#endif // NL_OS_WINDOWS
	}

	virtual emptyProc		getWindowProc();

	virtual bool			activate();

	virtual	sint			getNbTextureStages() const {return _Extensions.NbTextureStages;}

	virtual bool			isTextureExist(const ITexture&tex);

	virtual NLMISC::IEventEmitter	*getEventEmitter() { return&_EventEmitter; };

	virtual bool			clear2D(CRGBA rgba);

	virtual bool			clearZBuffer(float zval=1);

	virtual bool			setupTexture(ITexture& tex);

	virtual void			forceDXTCCompression(bool dxtcComp);

	virtual void			forceTextureResize(uint divisor);

	virtual bool			setupMaterial(CMaterial& mat);

	virtual void			setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective = true);

	virtual void			setupViewMatrix(const CMatrix& mtx);

	virtual void			setupModelMatrix(const CMatrix& mtx, uint8 n=0);

	virtual void			multiplyModelMatrix(const CMatrix& mtx, uint8 n=0);

	virtual CMatrix			getViewMatrix() const;

	virtual	void			setupVertexMode(uint vmode)
	{
		_VertexMode= vmode;
	}


	virtual	void			forceNormalize(bool normalize)
	{
		_ForceNormalize= normalize;
		// if ForceNormalize, must enable GLNormalize now.
		if(normalize)
			enableGlNormalize(true);
	}

	virtual	bool			isForceNormalize() const
	{
		return _ForceNormalize;
	}


	virtual	bool			supportVertexBufferHard() const;

	virtual	uint			getMaxVerticesByVertexBufferHard() const;

	virtual	bool			initVertexArrayRange(uint agpMem, uint vramMem);

	virtual	IVertexBufferHard	*createVertexBufferHard(uint16 vertexFormat, const uint8 *typeArray, uint32 numVertices, 
														IDriver::TVBHardType vbType);

	virtual	void			deleteVertexBufferHard(IVertexBufferHard *VB);

	virtual void			activeVertexBufferHard(IVertexBufferHard *VB);


	virtual bool			activeVertexBuffer(CVertexBuffer& VB);

	virtual bool			activeVertexBuffer(CVertexBuffer& VB, uint first, uint end);

	virtual bool			render(CPrimitiveBlock& PB, CMaterial& Mat);

	virtual void			renderTriangles(CMaterial& Mat, uint32 *tri, uint32 ntris);

	virtual void			renderSimpleTriangles(uint32 *tri, uint32 ntris);

	virtual void			renderPoints(CMaterial& Mat, uint32 numPoints) ;
	
	virtual void			renderQuads(CMaterial& Mat, uint32 startIndex, uint32 numQuads) ;

	virtual bool			swapBuffers();

	virtual	void			profileRenderedPrimitives(CPrimitiveProfile &pIn, CPrimitiveProfile &pOut);

	virtual	uint32			profileAllocatedTextureMemory();

	virtual	uint32			profileSetupedMaterials() const;

	virtual	uint32			profileSetupedModelMatrix() const;

	void					enableUsedTextureMemorySum (bool enable);
	
	uint32					getUsedTextureMemory() const;

	virtual uint			getNumMatrix();

	virtual bool			supportPaletteSkinning();

	virtual bool			release();

	virtual TMessageBoxId	systemMessageBox (const char* message, const char* title, TMessageBoxType type=okType, TMessageBoxIcon icon=noIcon);

	virtual void			setupScissor (const class CScissor& scissor);

	virtual void			setupViewport (const class CViewport& viewport);

	virtual uint32			getImplementationVersion () const
	{
		return ReleaseVersion;
	}

	virtual const char*		getDriverInformation ()
	{
		return "Opengl 1.2 NeL Driver";
	}

	virtual const char*		getVideocardInformation ();

	virtual bool			isActive ();

	virtual uint8			getBitPerPixel ();

	virtual void			showCursor (bool b);

	// between 0.0 and 1.0
	virtual void setMousePos(float x, float y);

	virtual void			setCapture (bool b);

	virtual void			getWindowSize (uint32 &width, uint32 &height);

	virtual void			getBuffer (CBitmap &bitmap);

	virtual void			getZBuffer (std::vector<float>  &zbuffer);

	virtual void			getBufferPart (CBitmap &bitmap, NLMISC::CRect &rect);

	virtual void			getZBufferPart (std::vector<float>  &zbuffer, NLMISC::CRect &rect);
		
	virtual void			copyFrameBufferToTexture(ITexture *tex, uint32 level
														, uint32 offsetx, uint32 offsety
													    , uint32 x, uint32 y
														, uint32 width, uint32 height														
													)  ;
	virtual void			setPolygonMode (TPolygonMode mode);

	virtual uint			getMaxLight () const;

	virtual void			setLight (uint8 num, const CLight& light);

	virtual void			enableLight (uint8 num, bool enable=true);

	virtual void			setAmbientColor (CRGBA color);

	/// \name Fog support.
	// @{
	virtual	bool			fogEnabled();
	virtual	void			enableFog(bool enable);
	/// setup fog parameters. fog must enabled to see result. start and end are in [0,1] range.
	virtual	void			setupFog(float start, float end, CRGBA color);
	// @}

	/// \name texture addressing modes
	// @{
	virtual bool supportTextureShaders() const;

	virtual bool isTextureAddrModeSupported(CMaterial::TTexAddressingMode mode) const;

	virtual void setMatrix2DForTextureOffsetAddrMode(const uint stage, const float mat[4]);
	// @}


private:
	friend class					CTextureDrvInfosGL;
	friend class					CVertexProgamDrvInfosGL;


	// For fast vector/point multiplication.
	struct	CMatrix3x4
	{
		// Order them in memory line first, for faster memory access.
		float	a11, a12, a13, a14;
		float	a21, a22, a23, a24;
		float	a31, a32, a33, a34;

		// Copy from a matrix.
		void	set(const CMatrix &mat);
		// mulAddvector. NB: in should be different as v!! (else don't work).
		void	mulAddVector(const CVector &in, float scale, CVector &out)
		{
			out.x+= (a11*in.x + a12*in.y + a13*in.z) * scale;
			out.y+= (a21*in.x + a22*in.y + a23*in.z) * scale;
			out.z+= (a31*in.x + a32*in.y + a33*in.z) * scale;
		}
		// mulAddpoint. NB: in should be different as v!! (else don't work).
		void	mulAddPoint(const CVector &in, float scale, CVector &out)
		{
			out.x+= (a11*in.x + a12*in.y + a13*in.z + a14) * scale;
			out.y+= (a21*in.x + a22*in.y + a23*in.z + a24) * scale;
			out.z+= (a31*in.x + a32*in.y + a33*in.z + a34) * scale;
		}
	};


private:
	// Version of the driver. Not the interface version!! Increment when implementation of the driver change.
	static const uint32		ReleaseVersion;

	bool					_FullScreen;
	bool						_OffScreen;

#ifdef NL_OS_WINDOWS

	friend static void GlWndProc(CDriverGL *driver, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	HWND						_hWnd;
	HDC							_hDC;
	PIXELFORMATDESCRIPTOR		_pfd;
    HGLRC						_hRC;
	static uint					_Registered;
	DEVMODE						_OldScreenMode;
	NLMISC::CWinEventEmitter	_EventEmitter;
	bool						_DestroyWindow;

	// Off-screen rendering in Dib section
	HPBUFFERARB					_PBuffer;

#elif defined (NL_OS_UNIX)

	Display						*dpy;
	GLXContext					ctx;
	Window						win;
	Cursor						cursor;
	NLMISC::CUnixEventEmitter	_EventEmitter;

#ifdef XF86VIDMODE
	int						_OldDotClock;   // old dotclock
	XF86VidModeModeLine		_OldScreenMode;	// old modeline
	int						_OldX, _OldY;   //Viewport settings
#endif //XF86VIDMODE

#endif // NL_OS_UNIX

	bool					_Initialized;

	/// \name Driver Caps.
	// @{
	// OpenGL extensions Extensions.
	CGlExtensions			_Extensions;
	// Say if palette skinning can be done in Hardware
	bool					_PaletteSkinHard;	
	// @}


	// Depth of the driver in Bit Per Pixel
	uint8					_Depth;

	// The forceNormalize() state.
	bool					_ForceNormalize;


	// The vertex transform mode.
	uint					_VertexMode;

	// To know if matrix setup has been changed from last activeVertexBuffer() (any call to setupViewMatrix() / setupModelMatrix()).
	bool					_MatrixSetupDirty;

	// To know if view matrix setup has been changed from last activeVertexBuffer() (any call to setupViewMatrix()).
	bool					_ViewMatrixSetupDirty;

	// for each model matrix, a flag to know if setuped.
	NLMISC::CBitSet			_ModelViewMatrixDirty;
	// same flag, but for palette Skinning (because they don't share same setup).
	NLMISC::CBitSet			_ModelViewMatrixDirtyPaletteSkin;

	// Backup znear and zfar
	float					_OODeltaZ;

	// Current (OpenGL basis) View matrix.
	CMatrix					_ViewMtx;
	// Matrix used for specular
	CMatrix					_TexMtx;

	// Current computed (OpenGL basis) ModelView matrix.
	CMatrix					_ModelViewMatrix[MaxModelMatrix];
	// For software skinning.
	CMatrix					_ModelViewMatrixNormal[MaxModelMatrix];
	CMatrix3x4				_ModelViewMatrix3x4[MaxModelMatrix];
	CMatrix3x4				_ModelViewMatrixNormal3x4[MaxModelMatrix];


	// Sofware Skinning.
	uint8					*_CurrentSoftSkinFlags;
	uint8					*_CurrentSoftSkinSrc;
	uint					_CurrentSoftSkinNormalOff;
	uint					_CurrentSoftSkinPaletteSkinOff;
	uint					_CurrentSoftSkinWeightOff;
	uint					_CurrentSoftSkinSrcStride;
	uint					_CurrentSoftSkinFirst;
	uint					_CurrentSoftSkinEnd;
	CVector					*_CurrentSoftSkinVectorDst;
	CVector					*_CurrentSoftSkinNormalDst;

	// Fog.
	bool					_FogEnabled;

	// Num lights return by GL_MAX_LIGHTS
	uint						_MaxDriverLight;
	bool						_LightEnable[MaxLight];				// Light enable.
	uint						_LightMode[MaxLight];				// Light mode.
	CVector						_WorldLightPos[MaxLight];			// World position of the lights.
	CVector						_WorldLightDirection[MaxLight];		// World direction of the lights.

	/// \name Prec settings, for optimisation.
	// @{

	// Special Texture environnements.
	enum	CTexEnvSpecial {
		TexEnvSpecialDisabled= 0, 
		TexEnvSpecialLightMapNV4, 
		TexEnvSpecialSpecularStage0NV4,
		TexEnvSpecialSpecularStage1NV4,
		TexEnvSpecialSpecularStage1NoTextNV4,
	};

	// NB: CRefPtr are not used for mem/spped optimisation. setupMaterial() and setupTexture() reset those states.
	CMaterial*				_CurrentMaterial;
	ITexture*				_CurrentTexture[IDRV_MAT_MAXTEXTURES];
	CMaterial::CTexEnv		_CurrentTexEnv[IDRV_MAT_MAXTEXTURES];
	// Special Texture Environnement.
	CTexEnvSpecial			_CurrentTexEnvSpecial[IDRV_MAT_MAXTEXTURES];
	// Texture addressing mode
	GLenum					_CurrentTexAddrMode[IDRV_MAT_MAXTEXTURES];
	// Reset texture shaders to their initial state if they are used
	void					resetTextureShaders();
	// activation of texture shaders
	bool					_NVTextureShaderEnabled;

	// Prec settings for material.
	CDriverGLStates			_DriverGLStates;
	// Optim: To not test change in Materials states if just texture has changed. Very usefull for landscape.
	uint32					_MaterialAllTextureTouchedFlag;

	// @}

	bool					_CurrentGlNormalize;

private:	

	bool					setupVertexBuffer(CVertexBuffer& VB);
	// Activate Texture Environnement. Do it with caching.
	bool					activateTexture(uint stage, ITexture *tex);
	// NB: this test _CurrentTexEnv[] and _CurrentTexEnvSpecial[].
	void					activateTexEnvMode(uint stage, const CMaterial::CTexEnv  &env);
	void					activateTexEnvColor(uint stage, const CMaterial::CTexEnv  &env);
	// Force Activate Texture Environnement. no caching here. TexEnvSpecial is disabled.
	void					forceActivateTexEnvMode(uint stage, const CMaterial::CTexEnv  &env);
	void					forceActivateTexEnvColor(uint stage, const CMaterial::CTexEnv  &env);

	/// nv texture shaders. Should be used only if this caps is present!
	void					enableNVTextureShader(bool enabled);
	// check nv texture shader consistency
	void			verifyNVTextureShaderConfig();

	// Called by activeVertexBuffer when _ViewMatrixSetupDirty is true to clean the view matrix.
	// set _ViewMatrixSetupDirty to false;
	void					cleanViewMatrix ();

	// According to extensions, retrieve GL tex format of the texture.
	GLint					getGlTextureFormat(ITexture& tex, bool &compressed);


	// Clip the wanted rectangle with window. return true if rect is not NULL.
	bool					clipRect(NLMISC::CRect &rect);


	// software skinning. Use _CurrentSoftSkin* global setup.
	void			computeSoftwareVertexSkinning(uint8 *pSrc, CVector *pVertexDst);
	void			computeSoftwareNormalSkinning(uint8 *pSrc, CVector *pNormalDst);
	void			refreshSoftwareSkinning();


	/// \name Material multipass.
	// @{
	/// init multipass for this material. return number of pass required to render this material.
	sint			beginMultiPass(const CMaterial &mat);
	/// active the ith pass of this material.
	void			setupPass(const CMaterial &mat, uint pass);
	/// end multipass for this material.
	void			endMultiPass(const CMaterial &mat);
	/// LastVB for UV setup.
	CVertexBufferInfo	_LastVB;
	// @}


	/// setup a texture stage with an UV from VB.
	void			setupUVPtr(uint stage, CVertexBufferInfo &VB, uint uvId);


	/// \name Lightmap.
	// @{
	void			computeLightMapInfos(const CMaterial &mat);
	sint			beginLightMapMultiPass(const CMaterial &mat);
	void			setupLightMapPass(const CMaterial &mat, uint pass);
	void			endLightMapMultiPass(const CMaterial &mat);

	/// Temp Variables computed in beginLightMapMultiPass(). Reused in setupLightMapPass().
	uint			_NLightMaps;
	uint			_NLightMapPerPass;
	uint			_NLightMapPass;
	// This array is the LUT from lmapId in [0, _NLightMaps[, to original lightmap id in material.
	std::vector<uint>		_LightMapLUT;

	// @}

	/// \name Specular.
	// @{
	sint			beginSpecularMultiPass(const CMaterial &mat);
	void			setupSpecularPass(const CMaterial &mat, uint pass);
	void			endSpecularMultiPass(const CMaterial &mat);
	// @}


	/// setup GL arrays, with a vb info.
	void			setupGlArrays(CVertexBufferInfo &vb, CVBDrvInfosGL *vbInf, bool skinning, bool paletteSkinning);


	/// Test/activate normalisation of normal.
	void			enableGlNormalize(bool normalize)
	{
		if(_CurrentGlNormalize!=normalize)
		{
			_CurrentGlNormalize= normalize;
			if(normalize)
				glEnable(GL_NORMALIZE);
			else
				glDisable(GL_NORMALIZE);
		}
	}


	/// \name VertexBufferHard 
	// @{
	CPtrSet<CVertexBufferHardGL>	_VertexBufferHardSet;
	friend class					CVertexArrayRange;
	friend class					CVertexBufferHardGL;
	// The CVertexArrayRange activated.
	CVertexArrayRange				*_CurrentVertexArrayRange;
	// The CVertexBufferHardGL activated.
	CVertexBufferHardGL				*_CurrentVertexBufferHard;

	// The AGP VertexArrayRange.
	CVertexArrayRange				_AGPVertexArrayRange;
	// The VRAM VertexArrayRange.
	CVertexArrayRange				_VRAMVertexArrayRange;


	void							resetVertexArrayRange();

	void							fenceOnCurVBHardIfNeeded(CVertexBufferHardGL *newVBHard);


	// @}


	/// \name Profiling
	// @{
	CPrimitiveProfile				_PrimitiveProfileIn;
	CPrimitiveProfile				_PrimitiveProfileOut;
	uint32							_AllocatedTextureMemory;
	uint32							_NbSetupMaterialCall;
	uint32							_NbSetupModelMatrixCall;
	bool							_SumTextureMemoryUsed;
	std::set<ITexture*>				_TextureUsed;
	uint							computeMipMapMemoryUsage(uint w, uint h, GLint glfmt) const;
	// @}

	/// \name Vertex program interface
	// @{

	bool			isVertexProgramSupported () const;
	bool			isVertexProgramEmulated () const;
	bool			activeVertexProgram (CVertexProgram *program);
	void			setConstant (uint index, float, float, float, float);
	void			setConstant (uint index, double, double, double, double);
	void			setConstant (uint indexStart, const NLMISC::CVector* value);
	void			setConstant (uint indexStart, const NLMISC::CVectorD* value);	
	void			setConstant (uint index, uint num, const float *src);	
	void			setConstant (uint index, uint num, const double *src);
	void			setConstantMatrix (uint index, IDriver::TMatrix matrix, IDriver::TTransform transform);	
	void			enableVertexProgramDoubleSidedColor(bool doubleSided);
	
	// @}

	bool			isVertexProgramEnabled () const
	{
		// Extension actived ?
		GLboolean actived=glIsEnabled ((GLenum)GL_VERTEX_PROGRAM_NV);
		return actived!=GL_FALSE;
	}
	// Say if last setupGlArrays() was a VertexProgram setup.
	bool							_LastSetupGLArrayVertexProgram;


	bool							_ForceDXTCCompression;
	/// Divisor for textureResize (power).
	uint							_ForceTextureResizePower;


	// user texture matrix
	NLMISC::CMatrix		_UserTexMat[IDRV_MAT_MAXTEXTURES];
	uint				_UserTexMatEnabled; // bitm ask for user texture coords
	//NLMISC::CMatrix		_UserTexMat[IDRV_MAT_MAXTEXTURES];

	// Static const
	static const uint NumCoordinatesType[CVertexBuffer::NumType];
	static const uint GLType[CVertexBuffer::NumType];
	static const uint GLVertexAttribIndex[CVertexBuffer::NumValue];
	static const uint GLMatrix[IDriver::NumMatrix];
	static const uint GLTransform[IDriver::NumTransform];

};

} // NL3D

#endif // NL_OPENGL_H
