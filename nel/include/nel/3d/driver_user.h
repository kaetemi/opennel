/** \file driver_user.h
 * <File description>
 *
 * $Id: driver_user.h,v 1.2 2001/02/28 16:19:51 berenguier Exp $
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

#ifndef NL_DRIVER_USER_H
#define NL_DRIVER_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/tmp/u_driver.h"
#include "nel/3d/driver.h"
#include "nel/3d/register_3d.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/primitive_block.h"
#include "nel/3d/font_manager.h"
#include "nel/3d/material_user.h"
#include "nel/3d/ptr_set.h"


namespace NL3D 
{


class	CTextureUser;
class	CTextContextUser;
class	CSceneUser;


// ***************************************************************************
/** UDriver implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CDriverUser : public UDriver
{
// **********************
protected:
	struct	CMatrixContext
	{
		CViewport	Viewport;		// Init to fullscreen.
		CFrustum	Frustum;
		CMatrix		ViewMatrix;
		CMatrix		ModelMatrix;
	};


protected:
	IDriver			*_Driver;
	bool			_WindowInit;
	CMatrixContext	_CurrentMatrixContext;
	CFontManager	_FontManager;
	// Components List.
	typedef	CPtrSet<CTextureUser>		TTextureSet;
	typedef	CPtrSet<CMaterialUser>		TMaterialSet;
	typedef	CPtrSet<CTextContextUser>	TTextContextSet;
	typedef	CPtrSet<CSceneUser>			TSceneSet;
	TTextureSet			_Textures;
	TMaterialSet		_Materials;
	TTextContextSet		_TextContexts;
	TSceneSet			_Scenes;


	// For 2D/3D Interface.
	CVertexBuffer		_VBFlat;
	CVertexBuffer		_VBColor;
	CVertexBuffer		_VBUv;
	CVertexBuffer		_VBColorUv;
	CPrimitiveBlock		_PBLine, _PBTri, _PBQuad;
	// For security, texture are initUnlit() at init()/release().
	CMaterialUser		_MatFlat;
	CMaterialUser		_MatText;


	// StaticInit
	static	bool	_StaticInit;

protected:
	void			setupMatrixContext();
	CMaterial		&convMat(UMaterial &mat);


// **********************
public:


	/// \name Object
	// @{
	CDriverUser();
	virtual	~CDriverUser();
	// @}


	/// \name Window / driver management.
	// @{

	/// Enum All Fullscreen modes.
	virtual TModeList		enumModes();

	/// create the window.
	virtual	void			setDisplay(const CMode &mode) ;
	/// Release the window.
	virtual	void			release() ;

	/// Before rendering via a driver in a thread, must activate() (per thread).
	virtual bool			activate(void)
	{
		return _Driver->activate();
	}
	/// Return true if driver is still active. Return false else. If he user close the window, must return false.
	virtual bool			isActive()
	{
		return _Driver->isActive();
	}

	// @}


	/// \name Buffers.
	// @{
	/// This clear the buffers (ALL the buffer :) )
	virtual	void			clearBuffers(CRGBA col= CRGBA(255,255,255,255))
	{
		_Driver->clear2D(col);
		_Driver->clearZBuffer();
	}
	/// This swap the back and front buffer (ALL the buffer :) ).
	virtual	void			swapBuffers()
	{
		_Driver->swapBuffers();
	}

	// @}



	/// \name Fog support.
	// @{
	virtual	bool			fogEnabled()
	{
		return _Driver->fogEnabled();
	}
	virtual	void			enableFog(bool enable)
	{
		_Driver->enableFog(enable);
	}
	/// setup fog parameters. fog must enabled to see result. start and end are in [0,1] range.
	virtual	void			setupFog(float start, float end, CRGBA color)
	{
		_Driver->setupFog(start, end, color);
	}
	// @}



	/// \name Scene gestion.
	// @{
	/// Create a new scene.
	virtual	UScene			*createScene() ;
	/// Delete a scene.
	virtual	void			deleteScene(UScene	*scene) ;
	// @}


	/// \name Components gestion for Interface 2D/3D.
	// @{

	/// create a new TextContext, for a given font.
	virtual	UTextContext	*createTextContext(const std::string fontFileName, const std::string fontExFileName = "") ;
	/// delete a TextContext.
	virtual	void			deleteTextContext(UTextContext	*textContext) ;
	/// Set the maxMemory used for the FontManager
	virtual	void			setFontManagerMaxMemory(uint maxMem) ;
	/// get cahce information.
	virtual		std::string getFontManagerCacheInformation() const ;


	/** Create a new texture file, searching in CPath.
	 * \param file filename, local to CPath paths.
	 */
	virtual	UTextureFile	*createTextureFile(const std::string &file) ;
	/// Delete a texture file. This one will be really deleted in memory when no material point to it.
	virtual	void			deleteTextureFile(UTextureFile *textfile) ;
	/// Create a new Raw texture, to be filled by user.
	virtual	UTextureRaw		*createTextureRaw() ;
	/// Delete a Raw texture. This one will be really deleted in memory when no material point to it.
	virtual	void			deleteTextureRaw(UTextureRaw *textraw) ;
	/// Create a new Material, to be filled by user.
	virtual	UMaterial		*createMaterial() ;
	/// Delete a Material.
	virtual	void			deleteMaterial(UMaterial *mat) ;
	// @}


	/// \name Matrix context for Interface 2D/3D.
	/** UScene ignore those function (use camera parameters instead), and do not disturb this active Matrix context.
	 * (after a scene rendering, the Matrix context for this interface is restored).
	 */
	// @{

	/** Set the active viewport for rendering. Default to fullscreen.
	 */
	virtual	void			setViewport(const CViewport &);
	virtual	CViewport		getViewport();
	/** Set the active Frustum for rendering. 
	 */
	virtual	void			setFrustum(const CFrustum &frust) ;
	virtual	CFrustum		getFrustum() ;
	/** Set the active ViewMatrix for rendering.
	 * NB: this is the view matrix, which is the inverse of camera matrix.
	 */
	virtual	void			setViewMatrix(const CMatrix &mat) ;
	virtual	CMatrix			getViewMatrix() ;
	/** Set the active ModelMatrix for rendering. NB: UScene ignore this function (use camera parameters instead).
	 */
	virtual	void			setModelMatrix(const CMatrix &mat) ;
	virtual	CMatrix			getModelMatrix() ;


	/** Tool function: Setup frustum/viewmatrix/modelmatrix for 2D.
	 * ModelMatrix is setup to identity. ViewMatrix is setup so that (x,y) of vectors maps to x,y screen!!!
	 */
	virtual	void			setMatrixMode2D(const CFrustum &frust) ;
	/** Tool function: Setup frustum/viewmatrix/modelmatrix for 3D, using parameters of a UCamera.
	 * ModelMatrix setuped to identity. ViewMatrix setuped to the inverse of camera 's LocalMatrix.
	 * Frustum setuped to UCamera frustum.
	 */
	virtual	void			setMatrixMode3D(UCamera &camera) ;

	// @}


	/// \name Interface 2D/3D.
	/** All of those render primitives are unlit! You must use UScene to render lighted meshes.
	 * NB: If you set a texture to your material, the primitives are textured, even if no Uvs are provided. \n
	 * NB: All rendering are done in current viewport / current matrix context.
	 */
	// @{

	/// Draw the Line, taking color from material.
	virtual	void			drawLine(const NLMISC::CLine &tri, UMaterial &mat) ;
	/// Draw the Line, taking color from primitive.
	virtual	void			drawLine(const NLMISC::CLineColor &tri, UMaterial &mat) ;
	/// Draw the Line, taking color from material. With UV for texture.
	virtual	void			drawLine(const NLMISC::CLineUV &tri, UMaterial &mat) ;
	/// Draw the Line, taking color from primitive. With UV for texture.
	virtual	void			drawLine(const NLMISC::CLineColorUV &tri, UMaterial &mat) ;

	/// Draw the Triangle, taking color from material.
	virtual	void			drawTriangle(const NLMISC::CTriangle &tri, UMaterial &mat) ;
	/// Draw the Triangle, taking color from primitive.
	virtual	void			drawTriangle(const NLMISC::CTriangleColor &tri, UMaterial &mat) ;
	/// Draw the Triangle, taking color from material. With UV for texture.
	virtual	void			drawTriangle(const NLMISC::CTriangleUV &tri, UMaterial &mat) ;
	/// Draw the Triangle, taking color from primitive. With UV for texture.
	virtual	void			drawTriangle(const NLMISC::CTriangleColorUV &tri, UMaterial &mat) ;

	/// Draw the Quad, taking color from material.
	virtual	void			drawQuad(const NLMISC::CQuad &tri, UMaterial &mat) ;
	/// Draw the Quad, taking color from primitive.
	virtual	void			drawQuad(const NLMISC::CQuadColor &tri, UMaterial &mat) ;
	/// Draw the Quad, taking color from material. With UV for texture.
	virtual	void			drawQuad(const NLMISC::CQuadUV &tri, UMaterial &mat) ;
	/// Draw the Quad, taking color from primitive. With UV for texture.
	virtual	void			drawQuad(const NLMISC::CQuadColorUV &tri, UMaterial &mat) ;

	// @}


	/// \name Tools for Interface 2D.
	/** For all those function, setMatrixMode2D*() should have been called (else strange results!!).
	 */
	// @{

	/// Draw a bitmap 2D. Warning: this is slow...
	virtual	void			drawBitmap (float x, float y, float width, float height, class UTexture& texture, bool blend=true, CRGBA col= CRGBA(255,255,255,255)) ;
	/// Draw a line in 2D. Warning: this is slow...
	virtual	void			drawLine (float x0, float y0, float x1, float y1, CRGBA col= CRGBA(255,255,255,255)) ;
	/// Draw a Triangle in 2D. Warning: this is slow...
	virtual	void			drawTriangle (float x0, float y0, float x1, float y1, float x2, float y2, CRGBA col) ;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawQuad (float x0, float y0, float x1, float y1, CRGBA col) ;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawQuad (float xcenter, float ycenter, float radius, CRGBA col) ;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawWiredQuad (float x0, float y0, float x1, float y1, CRGBA col) ;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawWiredQuad (float xcenter, float ycenter, float radius, CRGBA col) ;

	// @}



	/// \name Driver information/Queries
	// @{
	/**
	  * Get the driver version. Not the same than interface version. Incremented at each implementation change.
	  *
	  * \see InterfaceVersion
	  */
	virtual uint32			getImplementationVersion () const
	{
		return _Driver->getImplementationVersion ();
	}

	/**
	  * Get driver informations.
	  * get the nel name of the driver (ex: "Opengl 1.2 NeL Driver")
	  */
	virtual const char*		getDriverInformation ()
	{
		return _Driver->getDriverInformation();
	}

	/**
	  * Get videocard informations.
	  * get the official name of the driver
	  */
	virtual const char*		getVideocardInformation ()
	{
		return _Driver->getVideocardInformation ();
	}

	/// Get the number of texture stage avaliable, for multitexturing (Normal material shaders). Valid only after setDisplay().
	virtual	sint			getNbTextureStages()
	{
		return _Driver->getNbTextureStages();
	}

	/// Get the width and the height of the window
	virtual void			getWindowSize (uint32 &width, uint32 &height)
	{
		_Driver->getWindowSize (width, height);
	}


	/** get the RGBA back buffer
	  *
	  * \param bitmap the buffer will be written in this bitmap
	  */
	virtual void			getBuffer (CBitmap &bitmap) 
	{
		_Driver->getBuffer (bitmap) ;
	}

	/** get the ZBuffer (back buffer).
	  *
	  * \param zbuffer the returned array of Z. size of getWindowSize() .
	  */
	virtual void			getZBuffer (std::vector<float>  &zbuffer) 
	{
		_Driver->getZBuffer (zbuffer) ;
	}

	/** get a part of the RGBA back buffer
	  * NB: 0,0 is the bottom left corner of the screen.
	  *
	  * \param bitmap the buffer will be written in this bitmap
	  * \param rect the in/out (wanted/clipped) part of Color buffer to retrieve.
	  */
	virtual void			getBufferPart (CBitmap &bitmap, NLMISC::CRect &rect) 
	{
		_Driver->getBufferPart (bitmap, rect) ;
	}

	/** get a part of the ZBuffer (back buffer).
	  * NB: 0,0 is the bottom left corner of the screen.
	  *
	  * \param zbuffer the returned array of Z. size of rec.Width*rec.Height.
	  * \param rect the in/out (wanted/clipped) part of ZBuffer to retrieve.
	  */
	virtual void			getZBufferPart (std::vector<float>  &zbuffer, NLMISC::CRect &rect) 
	{
		_Driver->getZBufferPart (zbuffer, rect) ;
	}

	// @}



	/// \name Misc.
	// @{

	/** Output a system message box and print a message with an icon. This method can be call even if the driver is not initialized.
	  * This method is used to return internal driver problem when string can't be displayed in the driver window.
	  * If the driver can't open a messageBox, it should not override this method and let the IDriver class manage it with the ASCII console.
	  *
	  * \param message This is the message to display in the message box.
	  * \param title This is the title of the message box.
	  * \param type This is the type of the message box, ie number of button and label of buttons.
	  * \param icon This is the icon of the message box should use like warning, error etc...
	  */
	virtual TMessageBoxId	systemMessageBox (const char* message, const char* title, TMessageBoxType type=okType, TMessageBoxIcon icon=noIcon);

	/// show cursor if b is true, or hide it if b is false
	virtual void			showCursor (bool b)
	{
		_Driver->showCursor(b);
	}

	/// x and y must be between 0.0 and 1.0
	virtual void			setMousePos (float x, float y)
	{
		_Driver->setMousePos (x, y);
	}

	/// If true, capture the mouse to force it to stay under the window.
	virtual void			setCapture (bool b)
	{
		_Driver->setCapture (b);
	}

	/** Set the global polygon mode. Can be filled, line or point. The implementation driver must
	  * call IDriver::setPolygonMode and active this mode.
	  *
	  * \param polygon mode choose in this driver.
	  * \see getPolygonMode(), TPolygonMode
	  */
	virtual void			setPolygonMode (TPolygonMode mode)
	{
		IDriver::TPolygonMode	dmode;
		switch(mode)
		{
			case Filled:	dmode= IDriver::Filled; break;
			case Line:		dmode= IDriver::Line; break;
			case Point:		dmode= IDriver::Point; break;
			default: nlstop;
		};
		_Driver->setPolygonMode (dmode);
	}

	/** Get the global polygon mode.
	  *
	  * \param polygon mode choose in this driver.
	  * \see setPolygonMode(), TPolygonMode
	  */
	virtual TPolygonMode 	getPolygonMode () 
	{
		IDriver::TPolygonMode	dmode;
		UDriver::TPolygonMode	umode;
		dmode= _Driver->getPolygonMode();
		switch(dmode)
		{
			case IDriver::Filled:	umode= UDriver::Filled; break;
			case IDriver::Line:		umode= UDriver::Line; break;
			case IDriver::Point:	umode= UDriver::Point; break;
			default: nlstop;
		};

		return umode;
	}

	// @}



public:
	/// \name Accessor for CSeneUser.
	// @{
	IDriver		*getDriver()
	{
		return _Driver;
	}
	void		restoreMatrixContext()
	{
		setupMatrixContext();
	}
	// @}

};


} // NL3D


#endif // NL_DRIVER_USER_H

/* End of driver_user.h */
