#include "stdafx.h"
#include "nel_patch_paint.h"

#include "paint_to_nel.h"
#include <3d/landscape.h>

/*-------------------------------------------------------------------*/

// Get a patch tile array
std::vector<CTileElement>* CNelPatchChanger::getTileArray (const CNeLZoneIndex &mesh, int patch)
{
	// Find an entry
	CNelPatchMap::iterator ite=_MapNeLPatchInfo.find (CNelPatchKey (mesh.Index, patch));

	// If no entry, add one
	if (ite==_MapNeLPatchInfo.end())
		ite=(_MapNeLPatchInfo.insert (CNelPatchMap::value_type (CNelPatchKey (mesh.Index, patch), CNelPatchValue()))).first;

	// Array doesn't exist ?
	if (ite->second.Tiles.get()==NULL)
	{
		// Get the zone for this mesh
		CZone* zone=_Landscape->getZone (mesh.Index);
		nlassert (zone);

		// Create a new one
		ite->second.Tiles=std::auto_ptr< std::vector<CTileElement> > (new std::vector<CTileElement>);

		// Copy it from the patch
		*ite->second.Tiles=zone->getPatchTexture (patch);
	}

	// Return the array	
	return ite->second.Tiles.get();
}

/*-------------------------------------------------------------------*/

// Get a patch tile array
std::vector<CTileColor>* CNelPatchChanger::getColorArray (const CNeLZoneIndex &mesh, int patch)
{
	// Find an entry
	CNelPatchMap::iterator ite=_MapNeLPatchInfo.find (CNelPatchKey (mesh.Index, patch));

	// If no entry, add one
	if (ite==_MapNeLPatchInfo.end())
		ite=(_MapNeLPatchInfo.insert (CNelPatchMap::value_type (CNelPatchKey (mesh.Index, patch), CNelPatchValue()))).first;

	// Array doesn't exist ?
	if (ite->second.TileColors.get()==NULL)
	{
		// Get the zone for this mesh
		CZone* zone=_Landscape->getZone (mesh.Index);
		nlassert (zone);

		// Create a new one
		ite->second.TileColors=std::auto_ptr< std::vector<CTileColor> > (new std::vector<CTileColor>);

		// Copy it from the patch
		*ite->second.TileColors=zone->getPatchColor (patch);
	}

	// Return the array	
	return ite->second.TileColors.get();
}

/*-------------------------------------------------------------------*/

// Apply changes
void CNelPatchChanger::applyChanges (bool displace)
{
	// If displace, add neighbor
	//if (displace)
	{
		// Find the first entry
		CNelPatchMap::iterator ite=_MapNeLPatchInfo.begin ();

		std::set<std::pair<uint, uint> > setNewPatch;

		while (ite!=_MapNeLPatchInfo.end())
		{
			// *** Get its neighbord

			// Get the zone for this mesh
			const CZone* zone=_Landscape->getZone (ite->first.first);
			nlassert (zone);

			// Get the patch
			const CPatch *patch=zone->getPatch (ite->first.second);
			nlassert (patch);

			// For the 4 edges
			for (uint edge=0; edge<4; edge++)
			{
				// Get the bind info
				CPatch::CBindInfo neighborEdge;
				patch->getBindNeighbor(edge, neighborEdge);

				// Zone loaded
				if (neighborEdge.Zone)
				{
					// Add the patch around
					for (uint i=0; i<(uint)neighborEdge.NPatchs; i++)
					{
						// Add new patch
						setNewPatch.insert (std::pair<uint, uint> (neighborEdge.Zone->getZoneId(), neighborEdge.Next[i]->getPatchId()));
					}
				}
			}

			// Next
			ite++;
		}

		// Invalid the new zones
		std::set<std::pair<uint, uint> >::iterator iteNew=setNewPatch.begin();
		while (iteNew!=setNewPatch.end())
		{
			// Already visited ?
			if (_MapNeLPatchInfo.find (*iteNew)==_MapNeLPatchInfo.end())
			{
				// Get the zone
				CZone* zone=_Landscape->getZone (iteNew->first);

				// Invalide the texture and color
				//zone->changePatchTextureAndColor (iteNew->second, NULL, NULL);

				// Refresh tesselation
				zone->refreshTesselationGeometry (iteNew->second);
			}

			// Next new
			iteNew++;
		}
	}

	// Find the first entry
	CNelPatchMap::iterator ite=_MapNeLPatchInfo.begin ();

	// For all entry
	while (ite!=_MapNeLPatchInfo.end())
	{
		// *** Apply changes

		// Get the zone for this mesh
		CZone* zone=_Landscape->getZone (ite->first.first);
		nlassert (zone);

		// Assign to the NeL patch
		zone->changePatchTextureAndColor (ite->first.second, ite->second.Tiles.get(), ite->second.TileColors.get());

		// Displace ?
		//if (displace)
		{
			// Refresh tesselation
			zone->refreshTesselationGeometry (ite->first.second);
		}

		// Next
		ite++;
	}
}

/*-------------------------------------------------------------------*/

// Clear the container
void CNelPatchChanger::clear ()
{
	_MapNeLPatchInfo.clear();
}
