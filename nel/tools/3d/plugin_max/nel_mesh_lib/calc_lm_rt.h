/** \file calc_lm_rt.h
 * Raytrace module
 *
 * $Id: calc_lm_rt.h,v 1.2 2001/12/11 10:19:55 corvazier Exp $
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

#ifndef NL_CALC_LM_RT_H
#define NL_CALC_LM_RT_H

// ***********************************************************************************************

#include "calc_lm.h"

#include "3d/mesh.h"
#include "3d/quad_grid.h"

#include "nel/misc/vector.h"
#include "nel/misc/triangle.h"

// ***********************************************************************************************
// Light representation for the raytrace
class CRTLight
{

public:
	
	enum EType { RTLightAmbient, RTLightPoint, RTLightDir, RTLightSpot };
	EType Type;
	NLMISC::CVector Position;
	NLMISC::CVector Direction;

	float rSoftShadowRadius;
	float rSoftShadowConeLength;
};

// ***********************************************************************************************
// A ray is a cone coming from the point to evaluate and a cylinder joined to the cone at a given
// distance. The beginnning of the cylinder is the light source.
class CRTRay
{

public:

	void init	(uint32 nNbSide, float rRadius, NLMISC::CVector vVertex, NLMISC::CVector vLightPos,
				float rDistCyl);

	void clip	(NLMISC::CTriangle& t); // Warning : t is transformed with the _InvMat matrix

	float getArea ();

private:

	NLMISC::CMatrix _InvMat;
	float _DistLightVertex, _DistCyl;
	NLMISC::CVector _LightPos;

public:

	// Representation of a convex shape
	struct SConvexShape
	{
		std::vector<NLMISC::CUV> Vertices;
	};

	std::vector<SConvexShape> Shapes;

private:

	bool isShapeMustBeClippedByTriangle (SConvexShape&scs, NLMISC::CUV tri[3]);
	void clipShape (SConvexShape& ShapeIn, NLMISC::CUV Tri[3], std::vector<SConvexShape> &ShapesOut);
	NLMISC::CUV getLineIntersection (const NLMISC::CUV &l1p1, const NLMISC::CUV &l1p2, 
									const NLMISC::CUV &l2p1, const NLMISC::CUV &l2p2);

	void weldVertices (SConvexShape&scs);
};
// ***********************************************************************************************
// An element of the accelerators
struct SGridCell
{
	NL3D::CMesh::CFace* pF;
	NL3D::CMesh::CMeshBuild* pMB;
	NL3D::CMeshBase::CMeshBaseBuild* pMBB;
};

// ***********************************************************************************************
// Light accelerator interface to speed up raytrace
class IRTLightAccel
{
public:
	virtual ~IRTLightAccel() { }

	// Creation
	virtual void		insert (NLMISC::CTriangle &tri, SGridCell &cell) = 0;

	// Selection
	virtual void		select (NLMISC::CVector &v) = 0;
	virtual void		select (NLMISC::CVector &v, float rRadius) = 0;
	virtual SGridCell	getSel() = 0;
	virtual void		nextSel() = 0;
	virtual bool		isEndSel() = 0;
};

// ***********************************************************************************************
class CRTWorld
{
	// The world defined by basics meshes and correspondant max node
	std::vector<NL3D::CMesh::CMeshBuild*>			vMB;
	std::vector<NL3D::CMeshBase::CMeshBaseBuild*>	vMBB;
	std::vector<INode*>								vINode;

	// Lights in the world
	std::vector<IRTLightAccel*>				vLightAccel;
	std::vector<CRTLight>					vLight;

	//vector<SCubeGrid> cgAccel;	// One cube grid by light point or spot
	//vector<SDirGrid> dirAccel;	// One grid by light (directionnal only)
	NLMISC::CVector	GlobalTrans;
public:

	CRTWorld();
	~CRTWorld();
	// Build meshes from lights (if a mesh interact with at least one light except ambient)
	void build	(Interface &ip, std::vector<SLightBuild> &AllLights, NLMISC::CVector &vGlobalTrans, 
				bool bExcludeNonSelected, const std::set<INode*> &excludeNode);

	// Raytrace the vertex vVertex from light nLightNb
	NLMISC::CRGBAF raytrace (NLMISC::CVector &vVertex, sint32 nLightNb, uint8& rtVal, bool bSoftShadow);

private:

	// All we need for the raytrace

	void testCell	(NLMISC::CRGBAF &retValue, SGridCell &cell, NLMISC::CVector &vLightPos, 
					NLMISC::CVector &vVertexPos, uint8& rtVal);

	// All we need for the build

	void getAllSelectedNode (std::vector<NL3D::CMesh::CMeshBuild*> &Meshes,  
							std::vector<NL3D::CMeshBase::CMeshBaseBuild*> &MeshesBase,
							std::vector<INode*> &INodes, Interface& ip, std::vector<SLightBuild> &AllLights, 
							const std::set<INode*> &excludeNode);

	void getAllNodeInScene	(std::vector<NL3D::CMesh::CMeshBuild*> &Meshes, 
							std::vector<NL3D::CMeshBase::CMeshBaseBuild*> &BaseMeshes, 
							std::vector<INode* > &INodes, Interface& ip, std::vector<SLightBuild> &AllLights, 
							const std::set<INode*> &excludeNode, INode* pNode = NULL);

	bool intersectionTriangleSphere (NLMISC::CTriangle &t, NLMISC::CBSphere &s);

	bool intersectionSphereCylinder (NLMISC::CBSphere &s, NLMISC::CVector &cyCenter, 
									NLMISC::CVector &cyDir, float cyRadius);

	bool isInteractionLightMesh (SLightBuild &rSLB, NL3D::CMesh::CMeshBuild &rMB, NL3D::CMeshBase::CMeshBaseBuild &rMBB);

};


// ***********************************************************************************************
// Light accelerator for directionnal lights
class CRTLightAccelDir : public IRTLightAccel
{

public:

	float rMin, rMax;	// distance min and max from the light position to clip all rays 
						// the distance is given in the direction of the light direction

public:

	// Creation
	CRTLightAccelDir();
	virtual ~CRTLightAccelDir();

	void		create (int nSize, float rRadius, NLMISC::CVector &vDirection);
	void		insert (NLMISC::CTriangle &tri, SGridCell &cell);

	// Selection
	void		select (NLMISC::CVector &v);
	void		select (NLMISC::CVector &v, float rRadius);
	SGridCell	getSel ();
	void		nextSel ();
	bool		isEndSel ();

private:

	NL3D::CQuadGrid<SGridCell> grid;
	NLMISC::CMatrix invMat;
	NL3D::CQuadGrid<SGridCell>::CIterator itSel;
	
};

// ***********************************************************************************************
// Light accelerator for point and spot lights
class CRTLightAccelPoint : public IRTLightAccel
{

public :

	// Creation
	CRTLightAccelPoint ();
	virtual ~CRTLightAccelPoint ();
	
	void		create (int nSize);
	void		insert (NLMISC::CTriangle &tri, SGridCell &cell);

	// Selection
	void		select (NLMISC::CVector &v);
	void		select (NLMISC::CVector &v, float rRadius);
	SGridCell	getSel ();
	void		nextSel ();
	bool		isEndSel ();

private:

	enum gridPos { kUp = 0, kDown, kLeft, kRight, kFront, kBack };
	NL3D::CQuadGrid<SGridCell> _Grids[6];

	struct SSelector
	{
		sint32 nSelGrid;
		NL3D::CQuadGrid<SGridCell>::CIterator itSel;
	};

	uint32					_CurSel;
	std::vector<SSelector>	_Selection;

private:

	void project	(NLMISC::CTriangle &tri, NLMISC::CPlane pyr[4], NLMISC::CPlane &gridPlane, 
					sint32 nGridNb, SGridCell &cell);

};

#endif // NL_CALC_LM_RT_H

/* End of calc_lm_rt.h */

