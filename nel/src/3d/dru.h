/** \file dru.h
 * Driver Utilities.
 *
 * $Id: dru.h,v 1.6 2002/07/23 07:56:01 besson Exp $
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

#ifndef NL_DRU_H
#define NL_DRU_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "3d/driver.h"
#include "nel/3d/viewport.h"
#include "nel/misc/geom_ext.h"
#include "nel/misc/line.h"


#ifdef NL_OS_WINDOWS
#	if _MSC_VER >= 1300	// visual .NET, use different dll name
// must test debug fast first, because NL_DEBUG_FAST and NL_DEBUG are declared at same time.
#		ifdef NL_DEBUG_FAST
#			define NL3D_DLL_NAME "nldriver_opengl_df.dll"
#		elif defined (NL_DEBUG)
#			define NL3D_DLL_NAME "nldriver_opengl_d.dll"
#		elif defined (NL_RELEASE_DEBUG)
#			define NL3D_DLL_NAME "nldriver_opengl_rd.dll"
#		elif defined (NL_RELEASE)
#			define NL3D_DLL_NAME "nldriver_opengl_r.dll"
#		else
#			error "Unknown dll name"
#		endif
#	else // visual 6 or lesser
// must test debug fast first, because NL_DEBUG_FAST and NL_DEBUG are declared at same time.
#		ifdef NL_DEBUG_FAST
#			define NL3D_DLL_NAME "nel_drv_opengl_win_df.dll"
#		elif defined (NL_DEBUG)
#			define NL3D_DLL_NAME "nel_drv_opengl_win_d.dll"
#		elif defined (NL_RELEASE_DEBUG)
#			define NL3D_DLL_NAME "nel_drv_opengl_win_rd.dll"
#		elif defined (NL_RELEASE)
#			define NL3D_DLL_NAME "nel_drv_opengl_win_r.dll"
#		else
#			error "Unknown dll name"
#		endif
#	endif
#elif defined (NL_OS_UNIX)
#	define NL3D_DLL_NAME "libnel_drv_opengl.so.0"
#else
#	error "Unknown system"
#endif // NL_OS_UNIX

namespace NL3D 
{

/// Exception thrown by CDru::createGlDriver.
struct EDru : public NLMISC::Exception
{
	EDru(const std::string &reason) : Exception(reason) {}
};

struct EDruOpenglDriverNotFound : public EDru
{
	EDruOpenglDriverNotFound() : EDru( NL3D_DLL_NAME " not found" ) {}
};

struct EDruOpenglDriverCorrupted : public EDru
{
	EDruOpenglDriverCorrupted() : EDru( "Can't get NL3D_createIDriverInstance from " NL3D_DLL_NAME " (Bad dll?)" ) {}
};

struct EDruOpenglDriverOldVersion : public EDru
{
	EDruOpenglDriverOldVersion() : EDru( NL3D_DLL_NAME " is a too old version. Ask for a more recent file" ) {}
};

struct EDruOpenglDriverUnknownVersion : public EDru
{
	EDruOpenglDriverUnknownVersion() : EDru( NL3D_DLL_NAME " is more recent than the application" ) {}
};

struct EDruOpenglDriverCantCreateDriver : public EDru
{
	EDruOpenglDriverCantCreateDriver() : EDru( NL3D_DLL_NAME " can't create driver" ) {}
};

/// The driver Utilities class of static.
class	CDRU
{
public:
	
	/// Portable Function which create a GL Driver (using gl dll...).
	static IDriver		*createGlDriver() throw(EDru);

	/// \name 2D render.
	// @{
	/// Draw a bitmap 2D. Warning: this is slow...
	static void			drawBitmap (float x, float y, float width, float height, class ITexture& texture, IDriver& driver, CViewport viewport=CViewport(), bool blend=true);
	/// Draw a line in 2D. Warning: this is slow...
	static void			drawLine (float x0, float y0, float x1, float y1, IDriver& driver, CRGBA col= CRGBA(255,255,255,255), CViewport viewport=CViewport());
	/// Draw a Triangle in 2D. Warning: this is slow...
	static void			drawTriangle (float x0, float y0, float x1, float y1, float x2, float y2, IDriver& driver, CRGBA col, CViewport viewport);
	/// Draw a Quad in 2D. Warning: this is slow...
	static void			drawQuad (float x0, float y0, float x1, float y1, IDriver& driver, CRGBA col, CViewport viewport);
	/**
	  * Draw a color Quad in 2D. Warning: this is slow...
	  *
	  * \param x0 is the x coordinate of the lower left corner.
	  * \param y0 is the y coordinate of the lower left corner.
	  * \param x1 is the x coordinate of the upper right corner.
	  * \param y1 is the y coordinate of the upper right corner.
	  * \param col0 is the color of the lower left corner.
	  * \param col0 is the color of the lower right corner.
	  * \param col0 is the color of the upper right corner.
	  * \param col0 is the color of the upper left corner.
	  * \param driver is the driver to use to render the quad.
	  * \param viewport is the viewport to use to render the quad.
	  */
	static void			drawQuad (float x0, float y0, float x1, float y1, CRGBA col0, CRGBA col1, CRGBA col2, CRGBA col3, IDriver& driver, CViewport viewport);
	/// Draw a Quad in 2D. Warning: this is slow...
	static void			drawQuad (float xcenter, float ycenter, float radius, IDriver& driver, CRGBA col, CViewport viewport);
	/// Draw a Quad in 2D. Warning: this is slow...
	static void			drawWiredQuad (float x0, float y0, float x1, float y1, IDriver& driver, CRGBA col, CViewport viewport);
	/// Draw a Quad in 2D. Warning: this is slow...
	static void			drawWiredQuad (float xcenter, float ycenter, float radius, IDriver& driver, CRGBA col, CViewport viewport);
	
	// @}

	/// \name Easy render (2D or 3D).
	/** Those render methods work in the current driver viewport/frustum/matrixes.
	 * Since no vertex sharing is performed, their use may be slower than direct use of VBuffer/PBlock etc...
	 * Also, A VBuffer and a PBlock is created, and copies are made from the list of primitives to the driver...
	 */
	// @{
	/// Draw the triangles, with Vertex and 1 UV. "mat" should not be a lighted material since no normal is computed.
	static void			drawTrianglesUnlit(const NLMISC::CTriangleUV	*trilist, sint ntris, CMaterial &mat, IDriver& driver);
	/// Draw the triangles, with Vertex and 1 UV. "mat" should not be a lighted material since no normal is computed.
	static void			drawTrianglesUnlit(const std::vector<NLMISC::CTriangleUV> &trilist, CMaterial &mat, IDriver& driver);

	/// Draw the Lines, with Vertex only. "mat" should not be a lighted material since no normal is computed.
	static void			drawLinesUnlit(const NLMISC::CLine	*linelist, sint nlines, CMaterial &mat, IDriver& driver);
	/// Draw the Lines, with Vertex only. "mat" should not be a lighted material since no normal is computed.
	static void			drawLinesUnlit(const std::vector<NLMISC::CLine> &linelist, CMaterial &mat, IDriver& driver);
	/// Draw one line in 3D only, with a specific color.
	static void			drawLine(const CVector &a, const CVector &b, CRGBA color, IDriver& driver);
	// @}

};

} // NL3D


#endif // NL_DRU_H

/* End of dru.h */
