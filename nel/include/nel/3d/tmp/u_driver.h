/** \file u_driver.h
 * <File description>
 *
 * $Id: u_driver.h,v 1.8 2001/03/29 09:54:04 berenguier Exp $
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

#ifndef NL_U_DRIVER_H
#define NL_U_DRIVER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/tmp/viewport.h"
#include "nel/3d/tmp/frustum.h"
#include "nel/misc/geom_ext.h"
#include "nel/misc/matrix.h"
#include "nel/misc/rgba.h"
#include "nel/misc/rect.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/event_server.h"
#include "nel/misc/event_listener.h"


#ifdef NL_OS_WINDOWS
// automatically add the freetype library. MUST fo it because of link problems.
#pragma comment(lib, "freetype.lib")
#endif


namespace NL3D
{


using NLMISC::CVector;
using NLMISC::CMatrix;
using NLMISC::CRGBA;
using NLMISC::CBitmap;


class UScene;
class UCamera;
class UTextureFile;
class UTextureRaw;
class UMaterial;
class UTextContext;
class UAnimationSet;
class UPlayListManager;


// ***************************************************************************
/**
 * Game Interface for window Driver, first object to create.
 * From UDriver, you can create Scene, and render basic primitives (slow!!)
 *
 * All function calls are invalid before init() is called!!
 *
 * NB: there is ONE FontManager per UDriver.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UDriver
{
public:

	/// A Graphic Mode descriptor.
	class CMode 
	{
	public:
		bool				Windowed;
		uint16				Width;
		uint16				Height;
		uint8				Depth;

							CMode(void) 
							{ 
								Windowed=false;
								Width=0;
								Height=0;
								Depth=0;
							}
							CMode(uint16 w, uint16 h, uint8 d, bool windowed= true)
							{
								Windowed=windowed;
								Width=w;
								Height=h;
								Depth=d;
							}
	};

	typedef std::vector<CMode> TModeList;


	/// Message Box enums.
	enum TMessageBoxId { okId=0, yesId, noId, abortId, retryId, cancelId, ignoreId, idCount };
	enum TMessageBoxType { okType=0, okCancelType, yesNoType, abortRetryIgnoreType, yesNoCancelType, retryCancelType, typeCount };
	enum TMessageBoxIcon { noIcon=0, handIcon, questionIcon, exclamationIcon, asteriskIcon, warningIcon, errorIcon, informationIcon, stopIcon, iconCount };

	/// Polygon Mode.
	enum TPolygonMode { Filled=0, Line, Point };


public:
	/// The EventServer of this driver. Init after setDisplay()!!
	NLMISC::CEventServer			EventServer;
	/// The AsyncListener of this driver. Init after setDisplay()!!
	NLMISC::CEventListenerAsync		AsyncListener;


public:

	/// \name Object
	// @{
	UDriver();
	virtual	~UDriver();
	// @}




	/// \name Window / driver management.
	// @{

	/// Enum All Fullscreen modes.
	virtual TModeList		enumModes()=0;

	/// create the window. call activate().
	virtual	void			setDisplay(const CMode &mode) =0;
	/// Release the window. All components are released (Texture, materials, scene, textcontexts).
	virtual	void			release() =0;

	/// Before rendering via a driver in a thread, must activate() (per thread).
	virtual bool			activate(void)=0;
	/// Return true if driver is still active. Return false else. If he user close the window, must return false.
	virtual bool			isActive()=0;

	// @}



	/// \name Buffers.
	// @{
	/// This clear only the RGBA back buffer
	virtual	void			clearRGBABuffer(CRGBA col= CRGBA(255,255,255,255)) =0;
	/// This clear only the Zbuffer
	virtual	void			clearZBuffer() =0;
	/// This clear the buffers (ALL the buffer :) )
	virtual	void			clearBuffers(CRGBA col= CRGBA(255,255,255,255)) =0;
	/// This swap the back and front buffer (ALL the buffer :) ).
	virtual	void			swapBuffers() =0;

	// @}



	/// \name Fog support.
	// @{
	virtual	bool			fogEnabled()=0;
	virtual	void			enableFog(bool enable)=0;
	/// setup fog parameters. fog must enabled to see result. start and end are in [0,1] range.
	virtual	void			setupFog(float start, float end, CRGBA color)=0;
	// @}



	/// \name Scene gestion.
	// @{
	/// Create a new scene.
	virtual	UScene			*createScene() =0;
	/// Delete a scene.
	virtual	void			deleteScene(UScene	*scene) =0;
	// @}


	/// \name Animation gestion.
	// @{
	/// Create a new AnimationSet, load it from a file. Use CPath to search the animation set.  exception EPathNotFound if not found.
	virtual	UAnimationSet	*createAnimationSet(const std::string &animationSetFile) =0;
	/// Delete a AnimationSet. NB: actually, this animation set is internally deleted only when no more UPlayList use it.
	virtual	void			deleteAnimationSet(UAnimationSet *animationSet) =0;
	/// Create a new PlayListManager.
	virtual	UPlayListManager	*createPlayListManager() =0;
	/// Delete a PlayListManager.
	virtual	void				deletePlayListManager(UPlayListManager *playListManager) =0;
	// @}


	/// \name Components gestion for Interface 2D/3D.
	// @{


	/// create a new TextContext, for a given font.
	virtual	UTextContext	*createTextContext(const std::string fontFileName, const std::string fontExFileName = "") =0;
	/// delete a TextContext.
	virtual	void			deleteTextContext(UTextContext	*textContext) =0;
	/// Set the maxMemory used for the FontManager
	virtual	void			setFontManagerMaxMemory(uint maxMem)=0;
	/// get cahce information.
	virtual		std::string getFontManagerCacheInformation() const =0;


	/** Create a new texture file, searching in CPath.
	 * \param file filename, local to CPath paths.
	 */
	virtual	UTextureFile	*createTextureFile(const std::string &file) =0;
	/// Delete a texture file. This one will be really deleted in memory when no material point to it.
	virtual	void			deleteTextureFile(UTextureFile *textfile) =0;
	/// Create a new Raw texture, to be filled by user.
	virtual	UTextureRaw		*createTextureRaw() =0;
	/// Delete a Raw texture. This one will be really deleted in memory when no material point to it.
	virtual	void			deleteTextureRaw(UTextureRaw *textraw) =0;
	/// Create a new Material, to be filled by user.
	virtual	UMaterial		*createMaterial() =0;
	/// Delete a Material.
	virtual	void			deleteMaterial(UMaterial *mat) =0;
	// @}


	/// \name Matrix context for Interface 2D/3D.
	/** UScene ignore those function (use camera parameters instead, and UScene viewport), and do not disturb 
	 * this active Matrix context. (after a scene rendering, the Matrix context for this interface is restored).
	 * Remarks are nearly same for UTextContext, except for UTextContext::render3D() (see doc):
	 *		- UTextContext use the setuped viewport of UDriver Matrix context.
	 *		- UTextContext use its own Matrix2D setup (own Frustum and own ViewAMtrix/ ModelMatrix).
	 *		- UTextContext restore ALL the matrix context, after any rendering function.
	 */
	// @{

	/** Set the active viewport for rendering. Default to fullscreen.
	 */
	virtual	void			setViewport(const CViewport &)=0;
	virtual	CViewport		getViewport()=0;
	/** Set the active Frustum for rendering. 
	 */
	virtual	void			setFrustum(const CFrustum &frust) =0;
	virtual	CFrustum		getFrustum() =0;
	/** Set the active ViewMatrix for rendering.
	 * NB: this is the view matrix, which is the inverse of camera matrix.
	 */
	virtual	void			setViewMatrix(const CMatrix &mat) =0;
	virtual	CMatrix			getViewMatrix() =0;
	/** Set the active ModelMatrix for rendering. NB: UScene ignore this function (use camera parameters instead).
	 */
	virtual	void			setModelMatrix(const CMatrix &mat) =0;
	virtual	CMatrix			getModelMatrix() =0;


	/** Tool function: Setup frustum/viewmatrix/modelmatrix for 2D.
	 * ModelMatrix is setup to identity. ViewMatrix is setup so that (x,y) of vectors maps to x,y screen.
	 */
	virtual	void			setMatrixMode2D(const CFrustum &frust) =0;
	/// Tool function: same as setMatrixMode2D(), using a CFrustum(0,1,0,1,-1,1,false).
	void					setMatrixMode2D11();
	/// Tool function: same as setMatrixMode2D(), using a CFrustum(0,4/3,0,1,-1,1,false).
	void					setMatrixMode2D43();
	/** Tool function: Setup frustum/viewmatrix/modelmatrix for 3D, using parameters of a UCamera.
	 * ModelMatrix setuped to identity. ViewMatrix setuped to the inverse of camera 's LocalMatrix.
	 * Frustum setuped to UCamera frustum.
	 */
	virtual	void			setMatrixMode3D(UCamera &camera) =0;

	// @}


	/// \name Interface 2D/3D.
	/** All of those render primitives are unlit! You must use UScene to render lighted meshes.
	 * NB: If you set a texture to your material, the primitives are textured, even if no Uvs are provided. \n
	 * NB: All rendering are done in current viewport / current matrix context.
	 */
	// @{

	/// Draw the Line, taking color from material.
	virtual	void			drawLine(const NLMISC::CLine &tri, UMaterial &mat) =0;
	/// Draw the Line, taking color from primitive.
	virtual	void			drawLine(const NLMISC::CLineColor &tri, UMaterial &mat) =0;
	/// Draw the Line, taking color from material. With UV for texture.
	virtual	void			drawLine(const NLMISC::CLineUV &tri, UMaterial &mat) =0;
	/// Draw the Line, taking color from primitive. With UV for texture.
	virtual	void			drawLine(const NLMISC::CLineColorUV &tri, UMaterial &mat) =0;

	/// Draw the Triangle, taking color from material.
	virtual	void			drawTriangle(const NLMISC::CTriangle &tri, UMaterial &mat) =0;
	/// Draw the Triangle, taking color from primitive.
	virtual	void			drawTriangle(const NLMISC::CTriangleColor &tri, UMaterial &mat) =0;
	/// Draw the Triangle, taking color from material. With UV for texture.
	virtual	void			drawTriangle(const NLMISC::CTriangleUV &tri, UMaterial &mat) =0;
	/// Draw the Triangle, taking color from primitive. With UV for texture.
	virtual	void			drawTriangle(const NLMISC::CTriangleColorUV &tri, UMaterial &mat) =0;

	/// Draw the Quad, taking color from material.
	virtual	void			drawQuad(const NLMISC::CQuad &tri, UMaterial &mat) =0;
	/// Draw the Quad, taking color from primitive.
	virtual	void			drawQuad(const NLMISC::CQuadColor &tri, UMaterial &mat) =0;
	/// Draw the Quad, taking color from material. With UV for texture.
	virtual	void			drawQuad(const NLMISC::CQuadUV &tri, UMaterial &mat) =0;
	/// Draw the Quad, taking color from primitive. With UV for texture.
	virtual	void			drawQuad(const NLMISC::CQuadColorUV &tri, UMaterial &mat) =0;

	// @}


	/// \name Tools for Interface 2D.
	/** For all those function, setMatrixMode2D*() should have been called (else strange results!!).
	 */
	// @{

	/// Draw a bitmap 2D. Warning: this is slow...
	virtual	void			drawBitmap (float x, float y, float width, float height, class UTexture& texture, bool blend=true, CRGBA col= CRGBA(255,255,255,255)) =0;
	/// Draw a line in 2D. Warning: this is slow...
	virtual	void			drawLine (float x0, float y0, float x1, float y1, CRGBA col= CRGBA(255,255,255,255)) =0;
	/// Draw a Triangle in 2D. Warning: this is slow...
	virtual	void			drawTriangle (float x0, float y0, float x1, float y1, float x2, float y2, CRGBA col) =0;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawQuad (float x0, float y0, float x1, float y1, CRGBA col) =0;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawQuad (float xcenter, float ycenter, float radius, CRGBA col) =0;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawWiredQuad (float x0, float y0, float x1, float y1, CRGBA col) =0;
	/// Draw a Quad in 2D. Warning: this is slow...
	virtual	void			drawWiredQuad (float xcenter, float ycenter, float radius, CRGBA col) =0;

	// @}



	/// \name Driver information/Queries
	// @{
	/**
	  * Get the driver version. Not the same than interface version. Incremented at each implementation change.
	  *
	  * \see InterfaceVersion
	  */
	virtual uint32			getImplementationVersion () const = 0;

	/**
	  * Get driver informations.
	  * get the nel name of the driver (ex: "Opengl 1.2 NeL Driver")
	  */
	virtual const char*		getDriverInformation () = 0;

	/**
	  * Get videocard informations.
	  * get the official name of the driver
	  */
	virtual const char*		getVideocardInformation () = 0;

	/// Get the number of texture stage avaliable, for multitexturing (Normal material shaders). Valid only after setDisplay().
	virtual	sint			getNbTextureStages()=0;

	/// Get the width and the height of the window
	virtual void			getWindowSize (uint32 &width, uint32 &height) = 0;

	/** get the RGBA back buffer
	  *
	  * \param bitmap the buffer will be written in this bitmap
	  */
	virtual void			getBuffer (CBitmap &bitmap) = 0;

	/** get the ZBuffer (back buffer).
	  *
	  * \param zbuffer the returned array of Z. size of getWindowSize() .
	  */
	virtual void			getZBuffer (std::vector<float>  &zbuffer) = 0;

	/** get a part of the RGBA back buffer
	  * NB: 0,0 is the bottom left corner of the screen.
	  *
	  * \param bitmap the buffer will be written in this bitmap
	  * \param rect the in/out (wanted/clipped) part of Color buffer to retrieve.
	  */
	virtual void			getBufferPart (CBitmap &bitmap, NLMISC::CRect &rect) = 0;

	/** get a part of the ZBuffer (back buffer).
	  * NB: 0,0 is the bottom left corner of the screen.
	  *
	  * \param zbuffer the returned array of Z. size of rec.Width*rec.Height.
	  * \param rect the in/out (wanted/clipped) part of ZBuffer to retrieve.
	  */
	virtual void			getZBufferPart (std::vector<float>  &zbuffer, NLMISC::CRect &rect) = 0;

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
	virtual TMessageBoxId	systemMessageBox (const char* message, const char* title, TMessageBoxType type=okType, TMessageBoxIcon icon=noIcon) =0;


	/// show cursor if b is true, or hide it if b is false
	virtual void			showCursor (bool b) = 0;

	/// x and y must be between 0.0 and 1.0
	virtual void			setMousePos (float x, float y) = 0;

	/// If true, capture the mouse to force it to stay under the window.
	virtual void			setCapture (bool b) = 0;

	/** Set the global polygon mode. Can be filled, line or point. The implementation driver must
	  * call IDriver::setPolygonMode and active this mode.
	  *
	  * \param polygon mode choose in this driver.
	  * \see getPolygonMode(), TPolygonMode
	  */
	virtual void			setPolygonMode (TPolygonMode mode) =0;

	/** Get the global polygon mode.
	  *
	  * \param polygon mode choose in this driver.
	  * \see setPolygonMode(), TPolygonMode
	  */
	virtual TPolygonMode 	getPolygonMode () =0;

	// @}


public:

	/**
	 *	This is the static function which build a UDriver, the root for all 3D functions.
	 */
	static	UDriver			*createDriver();

};


} // NL3D


#endif // NL_U_DRIVER_H

/* End of u_driver.h */
