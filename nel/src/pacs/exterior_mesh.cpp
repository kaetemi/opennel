/** \file exterior_mesh.cpp
 *
 *
 * $Id: exterior_mesh.cpp,v 1.1 2001/07/24 08:44:19 legros Exp $
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

#include <vector>

#include "nel/misc/debug.h"
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"

#include "pacs/exterior_mesh.h"
#include "pacs/local_retriever.h"
#include "pacs/collision_desc.h"


using namespace std;
using namespace NLMISC;

namespace NLPACS
{
	// Functions for vertices comparison.
	// total order relation
	static inline bool	isStrictlyLess(const CVector &a, const CVector &b)
	{
		if (a.x < b.x)	return true;
		if (a.x > b.x)	return false;
		if (a.y < b.y)	return true;
		if (a.y > b.y)	return false;
		if (a.z < b.y)	return true;
		return false;
	}

	static inline bool	isStrictlyGreater(const CVector &a, const CVector &b)
	{
		if (a.x > b.x)	return true;
		if (a.x < b.x)	return false;
		if (a.y > b.y)	return true;
		if (a.y < b.y)	return false;
		if (a.z > b.y)	return true;
		return false;
	}

	CExteriorMesh::CExteriorMesh() { }

	void	CExteriorMesh::setEdges(const vector<CExteriorMesh::CEdge> &edges)
	{
		_Edges = edges;
		_OrderedEdges.clear();

		uint	i;
		for (i=0; i<_Edges.size()-1; )
		{
			_OrderedEdges.resize(_OrderedEdges.size()+1);
			COrderedEdges	&edges = _OrderedEdges.back();
			edges.Start = i;
			if (isStrictlyLess(_Edges[i].Start, _Edges[i+1].Start))
			{
				edges.Forward = true;
				do
				{
					++i;
				}
				while (i<_Edges.size()-1 && isStrictlyLess(_Edges[i].Start, _Edges[i+1].Start));
			}
			else
			{
				edges.Forward = false;
				do
				{
					++i;
				}
				while (i<_Edges.size()-1 && isStrictlyGreater(_Edges[i].Start, _Edges[i+1].Start));
			}
			edges.End = i;
		}
	}
};