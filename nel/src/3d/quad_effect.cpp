/** \file quad_effect.cpp
 * TODO: File description
 *
 * $Id: quad_effect.cpp,v 1.5 2004/11/15 10:24:47 lecroart Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "3d/quad_effect.h"
#include <algorithm>
#include <deque>

namespace NL3D 
{

// a 2d edge, ordered so that p1 is the highest point of the edge
struct CEdge
{
	CEdge(const NLMISC::CVector2f &p1, const NLMISC::CVector2f &p2)
	{
		if (p1.y < p2.y)
		{
			P1 = p1;
			P2 = p2;
		}
		else
		{
			P2 = p1;
			P1 = p2;
		}
	}

	NLMISC::CVector2f P1, P2;
};

// compares 2 edge, and take the highest
bool operator<(const CEdge &e1, const CEdge &e2) { return e1.P1.y < e2.P1.y; }

typedef std::deque<CEdge> TEdgeList;


void CQuadEffect::makeRasters(const TPoint2DVect &poly
							, float quadWidth, float quadHeight
							, TRasters &dest, float &startY
						   )
{

	dest.clear();
    const float epsilon = 10E-5f;

	sint size = poly.size();
	uint aelSize = 0; // size of active edge list

	sint k; // loop counter



	dest.clear();

	if (!size) return;

	static TEdgeList lel, ael; // the left edge list, and the active edge list
	float highest = poly[0].y;
	lel.clear();
	ael.clear();
	
	/// build a segment list, and go through it;

	for (k = 0; k < size; ++k)
	{

		lel.push_front(
						CEdge(poly[k], poly[k == (size - 1) ? 0 : k + 1])
			          );
		if (poly[k].y < highest) { highest = poly[k].y; }		
	}

	/// sort the segs
	std::sort(lel.begin(), lel.end());

	bool  borderFound;
	float left, right, inter, diff;
	float currY = highest;
	startY = highest;
	
	TEdgeList::iterator elIt; 

	do
	{		
		/// fetch new segment into the ael
		while (size 
			   && lel.begin()->P1.y < (currY +  quadHeight)
			  )
		{
			ael.push_front(lel.front());
			lel.pop_front();
			--size;
			++ aelSize;
		}

		if (aelSize)
		{

			borderFound = false;

			for (elIt = ael.begin(); elIt != ael.end();)
			{
				if (elIt->P2.y <= currY)
				{
					// edge has gone out of active edge list
					elIt = ael.erase(elIt);
					if (! --aelSize) return;
					continue;
				}
				else
				{

					/** edge is in scope. compute its extreme positions
					  * so we need to intersect it with the y = currY and y =  currY + quadHeight lines					  
					  */
										
					/// top of the edge
					if (elIt->P1.y >= currY) 
					{						
						if (!borderFound)
						{
							left = right = elIt->P1.x;
							borderFound = true;
						}
						else
						{
							left = std::min(left, elIt->P1.x);
							right = std::max(right, elIt->P1.x);
						}
					}
					else
					{
						// compute intersection
						diff = elIt->P2.y - elIt->P1.y;
						if (diff > epsilon)
						{
							inter = elIt->P1.x + (elIt->P2.x - elIt->P1.x) * (currY - elIt->P1.y) / diff;
						}
						else
						{
							inter = elIt->P2.x;
						}

						if (!borderFound)
						{
							left = right = inter;
							borderFound = true;
						}
						else
						{
							left = std::min(left, inter);
							right = std::max(right, inter);
						}
					}

					/// bottom of the edge

					if (elIt->P2.y <= currY + quadHeight) 
					{						
						if (!borderFound)
						{
							left = right = elIt->P2.x;
							borderFound = true;
						}
						else
						{
							left = std::min(left, elIt->P2.x);
							right = std::max(right, elIt->P2.x);
						}
					}
					else
					{
						// compute intersection
						diff = elIt->P2.y - elIt->P1.y;
						if (diff > epsilon)
						{
							inter = elIt->P1.x + (elIt->P2.x - elIt->P1.x) * (currY + quadHeight - elIt->P1.y) / diff;
						}
						else
						{
							inter = elIt->P2.x;
						}

						if (!borderFound)
						{
							left = right = inter;
							borderFound = true;
						}
						else
						{
							left = std::min(left, inter);
							right = std::max(right, inter);
						}
					}
					
				}
				++ elIt;				
			}		

			dest.push_back(std::make_pair(left, right));
		}

		currY += quadHeight;

	} while (size || aelSize);	
}

//**

void CQuadEffect::processPoly(const TPoint2DVect &poly
							, float quadWidth, float quadHeight
							, TPoint2DVect &dest
						   )
{
	static TRasters rDest;
	float currY;
	makeRasters(poly, quadWidth, quadHeight, rDest, currY);
	if (dest.size())
	{
		TRasters::const_iterator it, endIt = rDest.end();
		for (it = rDest.begin(); it != endIt; ++it)
		{
			const sint nbQuad = (sint) ceilf( (it->second - it->first) / quadWidth);
			float currX = it->first;
			for (sint k = 0; k < nbQuad; ++k)
			{
				dest.push_back(NLMISC::CVector2f(currX, currY));										  
				currX += quadWidth;
			}
			currY += quadHeight;
		}
	}
}


} // NL3D
