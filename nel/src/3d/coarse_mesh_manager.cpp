/** \file coarse_mesh_manager.cpp
 * Management of coarse meshes.
 *
 * $Id: coarse_mesh_manager.cpp,v 1.5 2001/07/11 16:11:28 corvazier Exp $
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

#include "3d/coarse_mesh_manager.h"
#include "3d/mesh.h"
#include "3d/texture_file.h"


namespace NL3D 
{

// ***************************************************************************

void	CCoarseMeshManager::registerBasic()
{
	CMOT::registerModel (CoarseMeshManagerId, TransformId, CCoarseMeshManager::creator);
	CMOT::registerObs (RenderTravId, CoarseMeshManagerId, CCoarseMeshManagerRenderObs::creator);
	CMOT::registerObs (ClipTravId, CoarseMeshManagerId, CCoarseMeshClipObs::creator);
}

// ***************************************************************************

CCoarseMeshManager::CCoarseMeshManager()
{
	// Set as opaque
	setTransparency (false);
	setOpacity (true);

	// ** Init texture

	_Texture=new CTextureFile ();

	// ** Init material

	// Double sided
	_Material.initUnlit ();
	_Material.setDoubleSided (true);
	_Material.setAlphaTest (true);
	_Material.setColor (CRGBA (255, 255, 255));

	// Texture
	_Material.setTexture (0, _Texture);
}

// ***************************************************************************

void CCoarseMeshManager::setTextureFile (const char* file)
{
	_Texture->setFileName (file);
}

// ***************************************************************************

uint64 CCoarseMeshManager::addMesh (const CMeshGeom& geom)
{
	// Get vertex buffer ref
	const CVertexBuffer &vertexBuffer=geom.getVertexBuffer();

	// *** Find the good render pass

	// Normalized number of vertex
	uint32 renderPass = vertexBuffer.getNumVertices () / NL3D_COARSEMESH_VERTEXBUFFER_GRANULARITY;
	if (NL3D_COARSEMESH_VERTEXBUFFER_GRANULARITY_MASK&vertexBuffer.getNumVertices ())
		renderPass++;

	// Look for the good rendering pass
	TRenderingPassMap::iterator ite=_RenderPass.find (renderPass);

	// Not found ?
	if ( ite==_RenderPass.end() )
	{
		// Add the rendering pass
		_RenderPass.insert (TRenderingPassMap::value_type (renderPass, CRenderPass()));
		ite=_RenderPass.find (renderPass);
		ite->second.init (renderPass*NL3D_COARSEMESH_VERTEXBUFFER_GRANULARITY);
	}

	// Ok, add the mesh
	uint32 id=ite->second.addMesh (geom);
	if (id==CRenderPass::CantAddMesh)
		return CantAddCoarseMesh;
	else
		// Return an id
		return buildId (renderPass, id);
}

// ***************************************************************************

void CCoarseMeshManager::removeMesh (uint64 id)
{
	// Get the render pass id
	uint32 renderPass = getRenderPassId (id);

	// Find the render pass
	TRenderingPassMap::iterator ite=_RenderPass.find (renderPass);

	// Not found ?
	nlassert ( ite!=_RenderPass.end() );

	// remove it
	ite->second.removeMesh (getRenderPassMeshId (id));
}

// ***************************************************************************

void CCoarseMeshManager::setMatrixMesh (uint64 id, const CMeshGeom& geom, const CMatrix& matrix)
{
	// Get the render pass id
	uint32 renderPass = getRenderPassId (id);

	// Find the render pass
	TRenderingPassMap::iterator ite=_RenderPass.find (renderPass);

	// Not found ?
	nlassert ( ite!=_RenderPass.end() );

	// remove it
	ite->second.setMatrixMesh (getRenderPassMeshId (id), geom, matrix);
}

// ***************************************************************************

void CCoarseMeshManager::setColor (NLMISC::CRGBA color)
{
	// Set the color
	_Material.setColor (color);
}

// ***************************************************************************

void CCoarseMeshManager::render (IDriver *drv)
{
	// Set Ident matrix
	drv->setupModelMatrix (CMatrix::Identity);

	// Render each render pass
	TRenderingPassMap::iterator ite=_RenderPass.begin ();
	while (ite!=_RenderPass.end())
	{
		// Render the rendering pass
		ite->second.render (drv, _Material);

		// Next render pass
		ite++;
	}
}

// ***************************************************************************

void CCoarseMeshManager::CRenderPass::init (uint blockSize)
{
	// Block size
	VBlockSize = blockSize;

	// Set vertex type
	VBuffer.setVertexFormat (NL3D_COARSEMESH_VERTEX_FORMAT);

	// Reserve the vertex buffer
	VBuffer.reserve (NL3D_COARSEMESH_VERTEXBUFFER_RESERVE*VBlockSize);
}

// ***************************************************************************

uint32 CCoarseMeshManager::CRenderPass::addMesh (const CMeshGeom& geom)
{
	// Is there a free vertex buffer ?
	uint16 vertexBufferId;
	std::list< uint16 >::iterator iteBegin=FreeVBlock.begin();
	if (iteBegin != FreeVBlock.end())
	{
		// Get a free id
		vertexBufferId=*iteBegin;
		FreeVBlock.erase (iteBegin);
	}
	else
	{
		// Number of vertex in the vertex buffer
		uint vertexCount=VBuffer.getNumVertices ();

		// Is the vertex buffer filled ?
		if (VBuffer.capacity () == vertexCount)
		{
			// Double it's capacity
			VBuffer.reserve (vertexCount*2);
		}

		// *** Make a new block

		// Resize the vertex buffer
		VBuffer.setNumVertices (vertexCount+VBlockSize);

		// Set mesh id
		vertexBufferId = vertexCount / VBlockSize;
	}

	// *** Copy the vertices

	// Src vertex buffer
	const CVertexBuffer &vbSrc=geom.getVertexBuffer();

	// Check the vertex format
	nlassert (vbSrc.getVertexFormat() == NL3D_COARSEMESH_VERTEX_FORMAT);

	// Number of source vertex
	uint32 nbVSrc = vbSrc.getNumVertices();
	nlassert (nbVSrc<=VBlockSize);

	// First vertex index
	uint firstVertex=vertexBufferId*VBlockSize;

	// Copy vector
	const void *vSrc = vbSrc.getVertexCoordPointer (0);
	void *vDest = VBuffer.getVertexCoordPointer (firstVertex);
	
	// Copy it
	memcpy (vDest, vSrc, nbVSrc*VBuffer.getVertexSize());

	// *** Setup primitives indexes

	// Try to add the primitive in a primitive blocks
	bool added=false;
	uint16 primitiveBlockId=0;
	std::list< CPrimitiveBlockInfo >::iterator ite=PrimitiveBlockInfoList.begin();
	while (ite!=PrimitiveBlockInfoList.end())
	{
		// Add it
		if ( (ite->addMesh (vertexBufferId, geom, firstVertex)) != CPrimitiveBlockInfo::Failed )
		{
			added=true;
			break;
		}

		// Next primitive block
		ite++;
		primitiveBlockId++;
	}

	// Can't be added ?
	if (ite==PrimitiveBlockInfoList.end())
	{
		// Add a primitive block
		PrimitiveBlockInfoList.push_back (CPrimitiveBlockInfo());
		ite=PrimitiveBlockInfoList.end();
		ite--;

		// Resize it
		ite->init (NL3D_COARSEMESH_PRIMITIVE_BLOCK_SIZE);

		// Add the mesh
		if (ite->addMesh (vertexBufferId, geom, firstVertex)!=CPrimitiveBlockInfo::Failed)
			added=true;
	}

	// Can be added ?
	if (!added)
		// Return error
		return CantAddMesh;

	// Return an id
	return buildId (primitiveBlockId, vertexBufferId);
}

// ***************************************************************************

void CCoarseMeshManager::CRenderPass::removeMesh (uint32 id)
{
	// Get ids
	uint16 primitiveBlockId=getPrimitiveblockId (id);
	uint16 vertexBufferId=getVertexBufferId (id);

	// *** Free the vertex buffer
	FreeVBlock.push_back (vertexBufferId);

	// *** Remove the primitive block

	// Find the primitive block
	std::list< CPrimitiveBlockInfo >::iterator ite=PrimitiveBlockInfoList.begin();
	for (uint i=0; i<primitiveBlockId; i++)
		ite++;

	// Some checks
	nlassert (ite!=PrimitiveBlockInfoList.end());

	// Remove from there
	ite->removeMesh (vertexBufferId);
}

// ***************************************************************************

void CCoarseMeshManager::CRenderPass::setMatrixMesh (uint32 id, const CMeshGeom& geom, const CMatrix& matrix)
{
	// Is there a free vertex buffer ?
	uint16 vertexBufferId=getVertexBufferId (id);

	// *** Transform the vertices

	// Src vertex buffer
	const CVertexBuffer &vbSrc=geom.getVertexBuffer();

	// Check the vertex format
	nlassert (vbSrc.getVertexFormat() == NL3D_COARSEMESH_VERTEX_FORMAT);

	// Number of source vertex
	uint32 nbVSrc = vbSrc.getNumVertices();
	nlassert (nbVSrc<=VBlockSize);
	sint normalOffset = vbSrc.getNormalOff();

	// Vertex size
	uint vtSize=vbSrc.getVertexSize ();

	// Copy vector
	const uint8 *vSrc = (const uint8 *)vbSrc.getVertexCoordPointer (0);
	uint8 *vDest = (uint8 *)VBuffer.getVertexCoordPointer (vertexBufferId*VBlockSize);
	
	// Transform it
	for (uint i=0; i<nbVSrc; i++)
	{
		// Transform position
		*(CVector*)vDest = matrix.mulPoint (*(const CVector*)vSrc);

		// Transform normal
		*(CVector*)(vDest+normalOffset) = matrix.mulVector (*(const CVector*)(vSrc+normalOffset));

		// Next point
		vSrc+=vtSize;
		vDest+=vtSize;
	}
}

// ***************************************************************************

void CCoarseMeshManager::CRenderPass::render (IDriver *drv, CMaterial& mat)
{
	// Active the vertex buffer
	drv->activeVertexBuffer (VBuffer);

	// For each primitive block
	std::list< CPrimitiveBlockInfo >::iterator ite=PrimitiveBlockInfoList.begin();
	while (ite!=PrimitiveBlockInfoList.end())
	{
		// Render it
		drv->render (ite->PrimitiveBlock, mat);

		// Next primitive block
		ite++;
	}
}

// ***************************************************************************

uint CCoarseMeshManager::CRenderPass::CPrimitiveBlockInfo::addMesh (uint16 vertexBufferId, const CMeshGeom& geom, uint32 firstVertexIndex)
{
	// Get num tri in the primitive block
	uint oldNumTri=PrimitiveBlock.getNumTri();

	// Total new tri count
	uint triCount=0;

	// Count number of triangles
	uint numMatrixBlock=geom.getNbMatrixBlock();
	uint matrixBlock;
	for (matrixBlock=0; matrixBlock<numMatrixBlock; matrixBlock++)
	{
		// One render pass
		uint numRenderPass=geom.getNbRdrPass (matrixBlock);
		uint renderPass;
		for (renderPass=0; renderPass<numRenderPass; renderPass++)
		{
			// Get the render pass
			const CPrimitiveBlock &pBlock=geom.getRdrPassPrimitiveBlock (matrixBlock, renderPass);

			// Get num tri in the primitive block
			uint numTri=PrimitiveBlock.getNumTri();

			// Check there is enought room to insert this primitives
			uint wantedNumTri=pBlock.getNumTri ();
			if ( wantedNumTri <= (PrimitiveBlock.capacityTri()-numTri ) )
			{
				triCount+=wantedNumTri;

				// Resize pblock
				PrimitiveBlock.setNumTri (numTri+wantedNumTri);

				// Tri pointer
				const uint32 *pTriSrc=pBlock.getTriPointer ();
				uint32 *pTriDest=PrimitiveBlock.getTriPointer ()+3*numTri;

				// Insert and remap indexes
				uint indexCount=wantedNumTri*3;
				for (uint index=0; index<indexCount; index++)
				{
					// Copy and remap the vertex indexes
					pTriDest[index]=pTriSrc[index]+firstVertexIndex;
				}

			}
		}
	}

	// Add a mesh info in the list
	MeshIdList.push_back (CMeshInfo (oldNumTri, triCount, vertexBufferId));

	// Ok
	return Success;
}

// ***************************************************************************

void CCoarseMeshManager::CRenderPass::CPrimitiveBlockInfo::removeMesh (uint16 vertexBufferId)
{
	// Get mesh info
	std::list< CMeshInfo >::iterator ite=MeshIdList.begin();
	while (ite!=MeshIdList.end())
	{
		// Good geom ?
		if (ite->VertexBufferId == vertexBufferId)
		{
			// Remove indexes of this primitive
			uint numIndexLeft=(PrimitiveBlock.getNumTri()-(ite->Offset+ite->Length))*3;

			// Source triangles
			const uint32 *pTriSrc=PrimitiveBlock.getTriPointer ()+3*(ite->Offset+ite->Length);

			// Destination triangles
			uint32 *pTriDest=PrimitiveBlock.getTriPointer ()+3*ite->Offset;

			// Copy indexes
			for (uint index=0; index<numIndexLeft; index++)
			{
				pTriDest[index]=pTriSrc[index];
			}

			// Resize primitive block
			PrimitiveBlock.setNumTri (PrimitiveBlock.getNumTri()-ite->Length);

			// Backup iterator
			std::list< CMeshInfo >::iterator iteOther=ite;

			// Patch offset for each others meshes in the primitive block
			while (iteOther!=MeshIdList.end())
			{
				// Remap offset
				iteOther->Offset-=ite->Length;

				// Next mesh
				iteOther++;
			}

			// Remove info from list
			MeshIdList.erase (ite);

			// Exit
			break;
		}

		// Next 
		ite++;
	}
	// Check it has been found
	nlassert (ite!=MeshIdList.end());
}

// ***************************************************************************

void CCoarseMeshManager::CRenderPass::CPrimitiveBlockInfo::init (uint size)
{
	// Reserve triangles
	PrimitiveBlock.reserveTri (size);
}

// ***************************************************************************

void	CCoarseMeshManagerRenderObs::traverse(IObs *caller)
{
	CRenderTrav			*trav= (CRenderTrav*)Trav;
	CCoarseMeshManager	*model= (CCoarseMeshManager*)Model;
	IDriver				*drv= trav->getDriver();

	// render the container.
	model->render (drv);
}

// ***************************************************************************

bool	CCoarseMeshClipObs::isRenderable() const
{
	return true;
}

// ***************************************************************************

bool	CCoarseMeshClipObs::clip(IBaseClipObs *caller)
{
	return true;
}

// ***************************************************************************

} // NL3D
