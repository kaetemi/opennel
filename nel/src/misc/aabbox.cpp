/** \file aabbox.cpp
 * <File description>
 *
 * $Id: aabbox.cpp,v 1.11 2004/02/05 20:24:22 berenguier Exp $
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

#include "stdmisc.h"

#include "nel/misc/aabbox.h"
#include "nel/misc/polygon.h"
#include "nel/misc/bsphere.h"
#include "nel/misc/matrix.h"



namespace NLMISC {


// ***************************************************************************
bool	CAABBox::clipFront(const CPlane &p) const
{
	CVector		hswap;

	// The bbox is front of the plane if only one of his vertex is in front.
	if(p*(Center + HalfSize) > 0)	return true;
	if(p*(Center - HalfSize) > 0)	return true;
	hswap.set(-HalfSize.x, HalfSize.y, HalfSize.z);
	if(p*(Center + hswap) > 0)	return true;
	if(p*(Center - hswap) > 0)	return true;
	hswap.set(HalfSize.x, -HalfSize.y, HalfSize.z);
	if(p*(Center + hswap) > 0)	return true;
	if(p*(Center - hswap) > 0)	return true;
	hswap.set(HalfSize.x, HalfSize.y, -HalfSize.z);
	if(p*(Center + hswap) > 0)	return true;
	if(p*(Center - hswap) > 0)	return true;

	return false;
}
// ***************************************************************************
bool	CAABBox::clipBack(const CPlane &p) const
{
	CVector		hswap;

	// The bbox is back of the plane if only one of his vertex is in back.
	if(p*(Center + HalfSize) < 0)	return true;
	if(p*(Center - HalfSize) < 0)	return true;
	hswap.set(-HalfSize.x, HalfSize.y, HalfSize.z);
	if(p*(Center + hswap) < 0)	return true;
	if(p*(Center - hswap) < 0)	return true;
	hswap.set(HalfSize.x, -HalfSize.y, HalfSize.z);
	if(p*(Center + hswap) < 0)	return true;
	if(p*(Center - hswap) < 0)	return true;
	hswap.set(HalfSize.x, HalfSize.y, -HalfSize.z);
	if(p*(Center + hswap) < 0)	return true;
	if(p*(Center - hswap) < 0)	return true;

	return false;
}


// ***************************************************************************
bool			CAABBox::include(const CVector &a) const
{
	if(Center.x+HalfSize.x<a.x)	return false;
	if(Center.x-HalfSize.x>a.x)	return false;
	if(Center.y+HalfSize.y<a.y)	return false;
	if(Center.y-HalfSize.y>a.y)	return false;
	if(Center.z+HalfSize.z<a.z)	return false;
	if(Center.z-HalfSize.z>a.z)	return false;
	return true;
}


// ***************************************************************************
bool			CAABBox::include(const CAABBox &box) const
{
	if(Center.x+HalfSize.x < box.Center.x+box.HalfSize.x)	return false;
	if(Center.x-HalfSize.x > box.Center.x-box.HalfSize.x)	return false;
	if(Center.y+HalfSize.y < box.Center.y+box.HalfSize.y)	return false;
	if(Center.y-HalfSize.y > box.Center.y-box.HalfSize.y)	return false;
	if(Center.z+HalfSize.z < box.Center.z+box.HalfSize.z)	return false;
	if(Center.z-HalfSize.z > box.Center.z-box.HalfSize.z)	return false;
	return true;
}


// ***************************************************************************
bool			CAABBox::intersect(const CAABBox &box) const
{
	CVector	mina = getMin(), maxa = getMax(),
			minb = box.getMin(), maxb = box.getMax();

	return ! ( mina.x > maxb.x ||
			   mina.y > maxb.y ||
			   mina.z > maxb.z ||
			   minb.x > maxa.x ||
			   minb.y > maxa.y ||
			   minb.z > maxa.z);
}

// ***************************************************************************
bool			CAABBox::intersect(const CVector &a, const CVector &b, const CVector &c) const
{
	// Trivial test.
	if(include(a) || include(b) || include(c))
		return true;
	// Else, must test if the polygon intersect the pyamid.
	CPlane		planes[6];
	makePyramid(planes);
	CPolygon	poly(a,b,c);
	poly.clip(planes, 6);
	if(poly.getNumVertices()==0)
		return false;
	return true;
}

// ***************************************************************************
bool			CAABBox::intersect(const CVector &a, const CVector &b) const
{
	// Trivial test.
	if(include(a) || include(b))
		return true;
	// Else, must test if the segment intersect the pyamid.
	CPlane		planes[6];
	makePyramid(planes);
	CVector		p0=a , p1=b;
	// clip the segment against all planes
	for(uint i=0;i<6;i++)
	{
		if(!planes[i].clipSegmentBack(p0, p1))
			return false;
	}
	return true;
}

// ***************************************************************************
bool			CAABBox::clipSegment(CVector &a, CVector &b) const
{
	// Trivial test. If both are in, they are inchanged
	if(include(a) && include(b))
		return true;
	// Else, must clip the segment againts the pyamid.
	CPlane		planes[6];
	makePyramid(planes);
	CVector		p0=a , p1=b;
	// clip the segment against all planes
	for(uint i=0;i<6;i++)
	{
		if(!planes[i].clipSegmentBack(p0, p1))
			return false;
	}
	// get result
	a= p0;
	b= p1;
	return true;
}

// ***************************************************************************
bool			CAABBox::intersect(const CBSphere &s) const
{
	if (Center.x + HalfSize.x < s.Center.x - s.Radius) return false; 
	if (Center.y + HalfSize.y < s.Center.y - s.Radius) return false; 
	if (Center.z + HalfSize.z < s.Center.z - s.Radius) return false; 

	if (Center.x - HalfSize.x > s.Center.x + s.Radius) return false; 
	if (Center.y - HalfSize.y > s.Center.y + s.Radius) return false; 
	if (Center.z - HalfSize.z > s.Center.z + s.Radius) return false; 

	return true;
}



// ***************************************************************************
void			CAABBox::makePyramid(CPlane	planes[6]) const
{
	planes[0].make(CVector(-1,0,0), Center-HalfSize);
	planes[1].make(CVector(+1,0,0), Center+HalfSize);
	planes[2].make(CVector(0,-1,0), Center-HalfSize);
	planes[3].make(CVector(0,+1,0), Center+HalfSize);
	planes[4].make(CVector(0,0,-1), Center-HalfSize);
	planes[5].make(CVector(0,0,+1), Center+HalfSize);
}


// ***************************************************************************
void			CAABBox::serial(NLMISC::IStream &f)
{
	(void)f.serialVersion(0);
	f.serial(Center);
	f.serial(HalfSize);
}


// ***************************************************************************
void	CAABBox::extend(const CVector &v)
{
	CVector		bmin= getMin(), bmax= getMax();

	bmin.minof(bmin, v);
	bmax.maxof(bmax, v);
	setMinMax(bmin, bmax);
}


//==========================================================================
/**
* Compute the union of 2 aabboxes, that is the  aabbox that contains the 2.
* Should end up in NLMISC
*/

CAABBox CAABBox::computeAABBoxUnion(const CAABBox &b1, const CAABBox &b2)
{	
	CAABBox result;
	CVector min, max;
	CVector min1 = b1.getMin() 
		    ,max1 = b1.getMax() 
			,min2 = b2.getMin() 
		    ,max2 = b2.getMax();
	max.maxof(max1, max2);
	min.minof(min1, min2);
	result.setMinMax(min, max);
	return result;
}


//==========================================================================
void	CAABBox::computeIntersection(const CAABBox &b1, const CAABBox &b2)
{
	CVector	min1 = b1.getMin(), max1 = b1.getMax(),
			min2 = b2.getMin(), max2 = b2.getMax();
	CVector	minr, maxr;

	// don't test if intersect or not.
	maxr.minof(max1, max2);
	minr.maxof(min1, min2);

	setMinMax(minr, maxr);
}


//==========================================================================
CAABBox CAABBox::transformAABBox(const CMatrix &mat, const CAABBox &box)
{
	// TODO : optimize this a bit if possible...
	CAABBox result;	

	/* OMG. Old code was false!!  
		if we have ht= M * h
		then CVector(-ht.x, ht.y, ht.z) != M * CVector(-h.x, h.y, h.z) !!!!
	*/
	// compute corners.
	CVector	p[8];
	CVector	min= box.getMin();
	CVector	max= box.getMax();
	p[0].set(min.x, min.y, min.z);
	p[1].set(max.x, min.y, min.z);
	p[2].set(min.x, max.y, min.z);
	p[3].set(max.x, max.y, min.z);
	p[4].set(min.x, min.y, max.z);
	p[5].set(max.x, min.y, max.z);
	p[6].set(min.x, max.y, max.z);
	p[7].set(max.x, max.y, max.z);
	CVector tmp;
	min = max = mat * p[0];
	// transform corners.
	for(uint i=1;i<8;i++)
	{
		tmp= mat * p[i];
		min.minof(min, tmp);
		max.maxof(max, tmp);
	}

	result.setMinMax(min, max);
	
	return result;	
}



// ***************************************************************************
bool	CAABBoxExt::clipFront(const CPlane &p) const
{
	// Assume normalized planes.

	// if( SpherMax OUT )	return false.
	float	d= p*Center;
	if(d<-RadiusMax)
		return false;
	// if( SphereMin IN )	return true;
	if(d>-RadiusMin)
		return true;

	// else, standard clip box.
	return CAABBox::clipFront(p);
}


// ***************************************************************************
bool	CAABBoxExt::clipBack(const CPlane &p) const
{
	// Assume normalized planes.

	// if( SpherMax OUT )	return false.
	float	d= p*Center;
	if(d>RadiusMax)
		return false;
	// if( SphereMin IN )	return true;
	if(d<RadiusMin)
		return true;

	// else, standard clip box.
	return CAABBox::clipBack(p);
}


// ***************************************************************************
void			CAABBoxExt::serial(NLMISC::IStream &f)
{
	CAABBox::serial(f);
	if(f.isReading())
		updateRadius();
}


} // NLMISC
