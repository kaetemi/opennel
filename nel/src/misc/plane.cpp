/** \file plane.cpp
 * class CPlane
 *
 * $Id: plane.cpp,v 1.5 2001/11/14 15:50:58 vizerie Exp $
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

#include "nel/misc/plane.h"


namespace	NLMISC
{


//============================================================
void	CPlane::make(const CVector &normal, const CVector &p)
{
	CVector	v= normal.normed();
	a= v.x;
	b= v.y;
	c= v.z;
	d=-(v*p);		// d=- (ax+by+cz).
}
void	CPlane::make(const CVector &p0, const CVector &p1, const CVector &p2)
{
	CVector v;

	v=(p1-p0)^(p2-p1);
	make(v,p1);
}


//============================================================
bool	CPlane::clipSegmentBack(CVector &p0, CVector &p1) const
{
	float	d0,d1,decal;
	CVector	proj;

	d0= (*this)*p0;
	d1= (*this)*p1;
	if(d0<0 && d1<0)
		return true;
	if(d0>=0 && d1>=0)
		return false;
	// Clip line.
	decal= (0-d0) / (d1-d0);
	proj= p0+ (p1-p0)*decal;
	if(d0>=0)
		p0=proj;
	else
		p1=proj;
	return true;
}
bool	CPlane::clipSegmentFront(CVector &p0, CVector &p1) const
{
	float	d0,d1,decal;
	CVector	proj;

	d0= (*this)*p0;
	d1= (*this)*p1;
	if(d0>=0 && d1>=0)
		return true;
	if(d0<0 && d1<0)
		return false;
	// Clip line.
	decal= (0-d0) / (d1-d0);
	proj= p0+ (p1-p0)*decal;
	if(d0<0)
		p0=proj;
	else
		p1=proj;
	return true;
}


//============================================================
sint	CPlane::clipPolygonBack(CVector in[], CVector out[], sint nIn) const
{
	sint nOut=0,s,p,i;
	if(nIn<=2) return 0;

	s=nIn-1;

	for (i=0;i<nIn;i++)
	{
		p=i;
		if ( (*this)*in[p] < 0 )
		{
			if ( (*this)*in[s] > 0 ) 
				out[nOut++]= intersect(in[s],in[p]);
			out[nOut++]=in[p];
		}
		else
		{
			if ( (*this)*in[s] < 0 ) 
				out[nOut++]= intersect(in[s],in[p]);
		}
		s=p;
	}

	return nOut;
}

//============================================================
sint	CPlane::clipPolygonFront(CVector in[], CVector out[], sint nIn) const
{
	sint nOut=0,s,p,i;
	if(nIn<=2) return 0;

	s=nIn-1;

	for (i=0;i<nIn;i++)
	{
		p=i;
		if ( (*this)*in[p] > 0 )
		{
			if ( (*this)*in[s] < 0 ) 
				out[nOut++]= intersect(in[s],in[p]);
			out[nOut++]=in[p];
		}
		else
		{
			if ( (*this)*in[s] > 0 ) 
				out[nOut++]= intersect(in[s],in[p]);
		}
		s=p;
	}

	return nOut;
}


//============================================================
CPlane  CPlane::inverted() const
{
	return CPlane(-a, -b, -c, -d);
}
		
//============================================================
void	CPlane::invert()
{
	a = -a;
	b = -b;
	c = -c;
	d = -d;
}




}

