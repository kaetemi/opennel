/** \file mesh_geom.h
 * <File description>
 *
 * $Id: mesh_geom.h,v 1.12 2002/07/02 12:27:19 berenguier Exp $
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

#ifndef NL_MESH_GEOM_H
#define NL_MESH_GEOM_H


#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/plane.h"
#include "nel/misc/matrix.h"


namespace NLMISC
{
	class CAABBoxExt;
}

namespace NL3D 
{


class IDriver;
class CTransformShape;
class CMeshBaseInstance;
class CMeshBlockManager;
class CScene;
class CRenderTrav;
using NLMISC::CPlane;
using NLMISC::CMatrix;


// ***************************************************************************
/**
 * A render Context used to render MeshGeom. Contains any usefull information
 */
class CMeshGeomRenderContext
{
public:
	IDriver			*Driver;
	CScene			*Scene;
	CRenderTrav		*RenderTrav;
};


// ***************************************************************************
/**
 * Interface for MeshGeom.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class IMeshGeom : public NLMISC::IStreamable
{
public:

	/// Render Flags, used in render
	enum	TRenderFlag
	{
		RenderOpaqueMaterial= 1,		// set when the mesh geom must render opaque material
		RenderTransparentMaterial= 2,	// set when the mesh geom must render transparent material
		RenderPassOpaque=4,				// set when the current traversal rdrPass is the opaque pass
		RenderGlobalAlpha= 8,			// set when the caller wants to draw material with global alpha
		RenderGADisableZWrite= 16,		// only when globalAlpha is used. set if must disable ZWrite
	};

public:

	/// Constructor
	IMeshGeom();
	/// dtor
	virtual ~IMeshGeom() {}


	/** store usefull information for this meshGeom in the instance. Used for IMeshVertexProgram as example
	 */
	virtual	void	initInstance(CMeshBaseInstance *mbi) =0;


	/** clip this shape with a pyramid.
	 * the pyramid is given in world space.The world matrix of the object is given.
	 * \param pyramid the clipping polytope, planes are normalized.
	 * \param worldMatrix the world matrix of the instance.
	 * \return true if the object is visible, false otherwise. The default behavior is to return true (never clipped).
	 */
	virtual bool	clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix) {return true;}

	/** render() this meshGeom in a driver, with the specified TransformShape instance information.
	 */
	virtual void	render(IDriver *drv, CTransformShape *trans, float polygonCount, uint32 rdrFlags, float globalAlpha) =0;

	/// \name Load balancing methods
	// @{

	/** get an approximation of the number of triangles this instance will render for a fixed distance.
	  *
	  * \param distance is the distance of the shape from the eye.
	  * \return the approximate number of triangles this instance will render at this distance. This
	  * number can be a float. The function MUST be decreasing or constant with the distance but don't 
	  * have to be continus.
	  */
	virtual float	getNumTriangles (float distance) =0;

	/** get the extended axis aligned bounding box of the mesh
	  */
	virtual const NLMISC::CAABBoxExt& getBoundingBox() const =0;

	// @}


	/// \name Mesh Block Render Interface
	/**
	 *	NB: Mesh Block render cannot occurs if the Mesh is Skinned/MeshMorphed.
	 *	NB: Mesh Block render can occurs only in Opaque pass => globalAlpha is not used.
	 */
	// @{

	/** true if this meshGeom support meshBlock rendering.
	 *	eg: return false if skinned/meshMorphed.
	 */
	virtual bool	supportMeshBlockRendering () const =0;

	/** true if the sort criterion must be by material. Else, sort per instance.
	 *
	 */
	virtual bool	sortPerMaterial() const =0;

	/** return the number of renderPasses for this mesh.
	 *
	 */
	virtual uint	getNumRdrPasses() const =0;

	/** The framework call this method when he will render instances of this meshGeom soon.
	 *
	 */
	virtual	void	beginMesh(CMeshGeomRenderContext &rdrCtx) =0;

	/** The framework call this method any time a change of instance occurs.
	 *
	 */
	virtual	void	activeInstance(CMeshGeomRenderContext &rdrCtx, CMeshBaseInstance *inst, float polygonCount) =0;

	/** The framework call this method to render the current renderPass, with the current instance
	 *	NB: if the material is blended, DON'T render it!!
	 */
	virtual	void	renderPass(CMeshGeomRenderContext &rdrCtx, CMeshBaseInstance *inst, float polygonCount, uint rdrPass) =0;

	/** The framework call this method when it has done with this meshGeom
	 *
	 */
	virtual	void	endMesh(CMeshGeomRenderContext &rdrCtx) =0;

	// @}


// *****************
protected:

	/// \name Mesh Block Render methods.
	// @{
	friend class CMeshBlockManager;

	/// This is the head of the list of instances to render in the CMeshBlockManager. -1 if NULL
	sint32			_RootInstanceId;

	// @}


};


} // NL3D


#endif // NL_MESH_GEOM_H

/* End of mesh_geom.h */
