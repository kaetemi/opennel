/** \file packed_world.cpp
 * Collision on a set of packed zones (packed representation)
 *
 * $Id:
 */

/* Copyright, 2000-2006 Nevrax Ltd.
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
//
#include "packed_world.h"
//
#include "nel/misc/grid_traversal.h"


using namespace NLMISC;

namespace NL3D
{


//*************************************************************************************************
void CPackedWorld::build(std::vector<TPackedZoneBaseSPtr> &packedZones)
{
	_ZoneGrid.clear();
	_Zones.clear();
	if (packedZones.empty()) return;
	CAABBox box;
	nlassert(packedZones[0])
	box = packedZones[0]->Box;
	for(uint k = 1; k < packedZones.size(); ++k)
	{
		nlassert(packedZones[k]);
		const CAABBox &otherBox = packedZones[k]->Box;
		box.extend(packedZones[k]->Box.getMin());
		box.extend(packedZones[k]->Box.getMax());
	}

	_ZoneMinX = (sint32) floorf(box.getMin().x / 160.f);
	_ZoneMinY = (sint32) floorf(box.getMin().y / 160.f);
	//
	sint32 zoneMaxX = (sint32) floorf(box.getMax().x / 160.f);
	sint32 zoneMaxY = (sint32) floorf(box.getMax().y / 160.f);
	//
	_ZoneGrid.init(zoneMaxX - _ZoneMinX + 1, zoneMaxY - _ZoneMinY + 1);
	_Zones.resize(packedZones.size());
	//
	for(uint k = 0; k < packedZones.size(); ++k)
	{
		CZoneInfo zi;
		zi.Zone = packedZones[k];
		zi.RaytraceCounter = 0;
		_Zones[k] = zi;
		sint zoneMinX = (sint) floorf(packedZones[k]->Box.getMin().x / 160.f) - (sint) _ZoneMinX;
		sint zoneMinY = (sint) floorf(packedZones[k]->Box.getMin().y / 160.f) - (sint) _ZoneMinY;
		sint zoneMaxX = (sint) floorf(packedZones[k]->Box.getMax().x / 160.f)  - (sint) _ZoneMinX;
		sint zoneMaxY = (sint) floorf(packedZones[k]->Box.getMax().y / 160.f)  - (sint) _ZoneMinY;
		for (sint y = zoneMinY; y <= zoneMaxY; ++y)
		{
			if (y < 0) continue;
			if (y >= (sint) _ZoneGrid.getHeight()) break;
			for (sint x = zoneMinX; x <= zoneMaxX; ++x)
			{
				if (x < 0) continue;
				if (x >= (sint) _ZoneGrid.getWidth()) break;
				_ZoneGrid(x, y).IDs.push_back((uint32) k);
			}
		}
	}
	_RaytraceCounter = ~0;
}

//*************************************************************************************************
bool CPackedWorld::raytrace(const NLMISC::CVector &start, const NLMISC::CVector &end, CVector &inter, std::vector<NLMISC::CTriangle> *testedTriangles /*= NULL*/)
{
	if (_ZoneGrid.empty()) return false;
	++_RaytraceCounter;
	float bestDist = FLT_MAX;	
	CVector currEnd = end;
	CVector currInter;
	if (_RaytraceCounter == (uint32) ~0)
	{
		for(uint k = 0; k < _Zones.size(); ++k)
		{
			_Zones[k].RaytraceCounter = 0;
		}
	}
	sint currX, currY;
	CVector2f start2f(start.x / 160.f, start.y / 160.f);
	CVector2f dir2f((end.x - start.x) / 160.f, (end.y - start.y) / 160.f);
	CGridTraversal::startTraverse(start2f, currX, currY);
	do
	{
		sint x = currX - (sint) _ZoneMinX;
		sint y = currY - (sint) _ZoneMinY;
		if (x < 0) continue;
		if (y < 0) continue;
		if (x >= (sint) _ZoneGrid.getWidth()) continue;
		if (y >= (sint) _ZoneGrid.getHeight()) continue;
		std::vector<uint32> &currZoneList = _ZoneGrid(x, y).IDs;
		for(uint k = 0; k < currZoneList.size(); ++k)
		{
			if (_Zones[currZoneList[k]].RaytraceCounter != _RaytraceCounter) // already visited
			{
				if (_Zones[currZoneList[k]].Zone->raytrace(start, currEnd, currInter, testedTriangles))
				{
					float dist = (currInter - start).norm();
					if (dist < bestDist)
					{
						bestDist = dist;
						inter = currInter;
						currEnd = currInter; // during search, just seek hit that are nearest						
					}
				}
				_Zones[currZoneList[k]].RaytraceCounter = _RaytraceCounter;
			}
		}
		if (bestDist != FLT_MAX) return true;
	}
	while (CGridTraversal::traverse(start2f, dir2f, currX, currY));	
	return false;
}

//*************************************************************************************************
void CPackedWorld::getZones(std::vector<TPackedZoneBaseSPtr> &zones)
{
	zones.clear();
	for(uint k = 0; k < _Zones.size(); ++k)
	{
		zones.push_back(_Zones[k].Zone);
	}
}

//*************************************************************************************************
void CPackedWorld::serialZoneNames(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);
	f.serialCheck((uint32) 'OWPA');
	f.serialCont(ZoneNames);
}

//*************************************************************************************************
void CPackedWorld::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{	
	serialZoneNames(f);
	f.serialCont(_Zones);
	f.serial(_ZoneGrid);
	f.serial(_ZoneMinX);
	f.serial(_ZoneMinY);	
}


} // Nl3D