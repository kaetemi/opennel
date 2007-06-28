/** \file frustum.cpp
 * TODO: File description
 *
 * $Id$
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

#include "std3d.h"

#include "nel/3d/frustum.h"
#include "nel/misc/matrix.h"
#include <cmath>


using namespace NLMISC;

namespace NL3D
{


void			CFrustum::init(float left, float right, float bottom, float top, float znear, float zfar, bool perspective)
{
	NL3D_MEM_FRUSTRUM
	Left= left;
	Right= right;
	Bottom=	bottom;
	Top= top;
	Near= znear;
	Far= zfar;
	Perspective= perspective;
}

void			CFrustum::init(float width, float height, float znear, float zfar, bool perspective)
{
	NL3D_MEM_FRUSTRUM
	init(-width/2, width/2, -height/2, height/2, znear, zfar, perspective);
}
void			CFrustum::initPerspective(float fov, float aspectRatio, float znear, float zfar)
{
	NL3D_MEM_FRUSTRUM
	float	w,h;
	w= 2*znear*(float)tan(fov/2);
	h= aspectRatio != 0.f ? w/aspectRatio : 0.f;
	init(w,h,znear,zfar,true);
}
void			CFrustum::getValues(float &left, float &right, float &bottom, float &top, float &znear, float &zfar) const
{
	NL3D_MEM_FRUSTRUM
	left= Left;
	right= Right;
	bottom=	Bottom;
	top= Top;
	znear= Near;
	zfar= Far;
}


// ***************************************************************************
CVector			CFrustum::project(const CVector &vec) const
{
	NL3D_MEM_FRUSTRUM
	CVector		ret;
	float		decalX, decalY;
	float		w, h;
	float		OOw, OOh;

	// Fast transform to openGL like axis.
	CVector		pt;
	pt.x= vec.x;
	pt.y= vec.z;
	pt.z= -vec.y;

	decalX= (Right+Left);
	decalY= (Top+Bottom);
	w= Right-Left;
	h= Top-Bottom;
	OOw= 1.0f/w;
	OOh= 1.0f/h;

	// project to -1..+1.
	if(Perspective)
	{
		ret.x= (2*Near*pt.x + decalX*pt.z)*OOw;
		ret.x/= -pt.z;
		ret.y= (2*Near*pt.y + decalY*pt.z)*OOh;
		ret.y/= -pt.z;
	}
	else
	{
		ret.x= (2*pt.x-decalX)*OOw;
		ret.y= (2*pt.y-decalY)*OOh;
	}


	// Map it to 0..1.
	ret.x= 0.5f*(ret.x+1);
	ret.y= 0.5f*(ret.y+1);
	ret.z= 0;

	return ret;
}


// ***************************************************************************
CVector			CFrustum::projectZ(const CVector &vec) const
{
	NL3D_MEM_FRUSTRUM
		CVector		ret;
	float		decalX, decalY;
	float		w, h;
	float		OOw, OOh;
	
	// Fast transform to openGL like axis.
	CVector		pt;
	pt.x= vec.x;
	pt.y= vec.z;
	pt.z= -vec.y;
	
	decalX= (Right+Left);
	decalY= (Top+Bottom);
	w= Right-Left;
	h= Top-Bottom;
	OOw= 1.0f/w;
	OOh= 1.0f/h;
	
	// project to -1..+1.
	if(Perspective)
	{
		ret.x= (2*Near*pt.x + decalX*pt.z)*OOw;
		ret.x/= -pt.z;
		ret.y= (2*Near*pt.y + decalY*pt.z)*OOh;
		ret.y/= -pt.z;
	}
	else
	{
		ret.x= (2*pt.x-decalX)*OOw;
		ret.y= (2*pt.y-decalY)*OOh;
	}
	
	
	// Map it to 0..1.
	ret.x= 0.5f*(ret.x+1);
	ret.y= 0.5f*(ret.y+1);
	ret.z= pt.z;
	
	return ret;
}


// ***************************************************************************
CVector			CFrustum::unProject(const CVector &vec) const
{
	NL3D_MEM_FRUSTRUM
	CVector		ret;
	float		decalX, decalY;
	float		w, h;

	decalX= (Right+Left);
	decalY= (Top+Bottom);
	w= Right-Left;
	h= Top-Bottom;

	// vec is a vector in a left hand axis.
	CVector		pt;
	pt.x= vec.x;
	pt.y= vec.y;
	pt.z= vec.z;
	
	// Map it to -1..1
	pt.x= 2*(pt.x-0.5f);
	pt.y= 2*(pt.y-0.5f);

	// Map Z to Near..Far.
	// Z IN is 1/Z, and is in 0..1.
	// inverse to 1..0.
	pt.z= 1-pt.z;
	// Map ret.z to 1/Far..1/Near.
	pt.z= 1/Far+(1/Near-1/Far)*pt.z;
	// Inverse, so ret.z E Near..Far.
	pt.z= 1/pt.z;
	// Actually, pt.z==w, homogenous coordinate.


	// unproject
	if(Perspective)
	{
		// w of homogenous coordinate.
		float	Wh;
		float	Zin;
		Wh= pt.z;
		Zin= -pt.z;

		// unproject.  (Projection is: x'= x/w.  y'= y/w).
		pt.x= pt.x*Wh;
		pt.y= pt.y*Wh;
		ret.x= (pt.x*w-decalX*Zin)/(2*Near);
		ret.y= (pt.y*h-decalY*Zin)/(2*Near);
		ret.z= Zin;
	}
	else
	{
		// NOT DONE YET.
		nlstop;
		/*ret.x= (pt.x*w+decalX)/2;
		ret.y= (pt.y*h+decalY)/2;
		*/
	}

	// Fast transform from openGL like axis.
	pt =ret;
	ret.x= pt.x;
	ret.y= -pt.z;
	ret.z= pt.y;

	return ret;
}


// ***************************************************************************
CVector			CFrustum::unProjectZ(const CVector &vec) const
{
	NL3D_MEM_FRUSTRUM
	CVector		ret;
	float		decalX, decalY;
	float		w, h;

	decalX= (Right+Left);
	decalY= (Top+Bottom);
	w= Right-Left;
	h= Top-Bottom;

	// vec is a vector in a left hand axis.
	CVector		pt;
	pt.x= vec.x;
	pt.y= vec.y;
	pt.z= vec.z;
	
	// Map it to -1..1
	pt.x= 2*(pt.x-0.5f);
	pt.y= 2*(pt.y-0.5f);

	// unproject
	if(Perspective)
	{
		// w of homogenous coordinate.
		float	Wh;
		float	Zin;
		Wh= pt.z;
		Zin= -pt.z;

		// unproject.  (Projection is: x'= x/w.  y'= y/w).
		pt.x= pt.x*Wh;
		pt.y= pt.y*Wh;
		ret.x= (pt.x*w-decalX*Zin)/(2*Near);
		ret.y= (pt.y*h-decalY*Zin)/(2*Near);
		ret.z= Zin;
	}
	else
	{
		// NOT DONE YET.
		//nlstop;
		/*ret.x= (pt.x*w+decalX)/2;
		ret.y= (pt.y*h+decalY)/2;
		*/
		// Yoyo: crash avoid for lem
		ret= vec;
	}

	// Fast transform from openGL like axis.
	pt =ret;
	ret.x= pt.x;
	ret.y= -pt.z;
	ret.z= pt.y;

	return ret;
}

// ***************************************************************************
float		CFrustum::getAspectRatio() const
{
	float	w= Right-Left;
	float	h= Top-Bottom;
	if(h!=0)
		return w/h;
	else
		return 1.f;
}


} // NL3D
