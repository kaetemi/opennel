/** \file seg_remanence_shape.cpp
 *
 * $Id: seg_remanence_shape.cpp,v 1.6 2003/03/26 10:13:19 vizerie Exp $
 */

/* Copyright, 2000, 2001, 2002 Nevrax Ltd.
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

#include "3d/seg_remanence_shape.h"
#include "3d/seg_remanence.h"
#include "3d/driver.h"
#include "3d/scene.h"
//
#include "nel/misc/bsphere.h"



namespace NL3D
{


//===========================================================
CSegRemanenceShape::CSegRemanenceShape() : _GeomTouched(true),
										   _MatTouched(true),
										   _TextureShifting(true),
										   _NumSlices(8),
										   _SliceTime(0.05f),
										   _RollUpRatio(1.f),
										   _AnimatedMat(NULL)
{
	_BBox.setCenter(NLMISC::CVector::Null);
	_BBox.setHalfSize(NLMISC::CVector(3, 3, 3));
	setNumCorners(2);
}

//===========================================================
void CSegRemanenceShape::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion(1);
	f.serial(_NumSlices);
	f.serial(_SliceTime);
	f.serialCont(_Corners);	
	f.serial(_Mat);	
	f.serial(_BBox);
	f.serial(_TextureShifting);
	f.serialPtr(_AnimatedMat);
	if (f.isReading())
	{	
		_GeomTouched = true;
		_MatTouched  = true;
	}
	if (ver >= 1)
	{
		f.serial(_RollUpRatio);
	}
}

//===========================================================
void CSegRemanenceShape::setSliceTime(float sliceTime)
{
	nlassert(sliceTime > 0);
	_SliceTime = sliceTime;	
}

//===========================================================
void CSegRemanenceShape::setCorner(uint corner, const NLMISC::CVector &value)
{
	nlassert(corner < _Corners.size());
	_Corners[corner] = value;
}

//===========================================================
void CSegRemanenceShape::setNumSlices(uint32 numSlices)
{
	nlassert(numSlices >= 2);
	_NumSlices = numSlices;	
	_GeomTouched = true;
}

//===========================================================
NLMISC::CVector CSegRemanenceShape::getCorner(uint corner) const
{
	nlassert(corner < _Corners.size());
	return _Corners[corner];
}

//===========================================================
void CSegRemanenceShape::setNumCorners(uint numCorners)
{
	nlassert(numCorners >= 2);
	_Corners.resize(numCorners);
	std::fill(_Corners.begin(), _Corners.end(), NLMISC::CVector::Null);
	_GeomTouched = true;
}

//===========================================================
void CSegRemanenceShape::render(IDriver *drv, CTransformShape *trans, bool opaquePass)
{
	if ((!opaquePass && _Mat.getBlend())
	    || (opaquePass && !_Mat.getBlend())
	   )
	{
		CSegRemanence *sr = NLMISC::safe_cast<CSegRemanence *>(trans);
		if (!sr->isStarted()) return;		
		setupVBnPB();
		setupMaterial();
		// vertices are rendered in world space
		drv->setupModelMatrix(NLMISC::CMatrix::Identity);		
		//		
		sr->render(drv, _VB, _PB, _Mat);		
	}
}


//===========================================================
void CSegRemanenceShape::flushTextures(IDriver &driver)
{
	_Mat.flushTextures(driver);
}

//===========================================================
CTransformShape *CSegRemanenceShape::createInstance(CScene &scene)
{	
	CSegRemanence *sr = NLMISC::safe_cast<CSegRemanence *>(scene.createModel(NL3D::SegRemanenceShapeId) );
	sr->Shape = this;
	CAnimatedMaterial *aniMat = NULL;
	if (_AnimatedMat)
	{
		aniMat = new CAnimatedMaterial(_AnimatedMat);
		aniMat->setMaterial(&_Mat);		
	}
	sr->setAnimatedMaterial(aniMat);
	sr->setupFromShape();
	// SegRemanence are added to the "Fx" Load Balancing Group.
	sr->setLoadBalancingGroup("Fx");
	return sr;
}


//===========================================================
float CSegRemanenceShape::getNumTriangles(float distance)
{
	return (float) (_NumSlices * 2);
}


//===========================================================
void CSegRemanenceShape::setupVBnPB()
{
	if (!_GeomTouched) return;

	uint numCorners = _Corners.size();
	_VB.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag);
	_VB.setNumVertices(numCorners * (_NumSlices + 1));
	uint k, l;
	
	// set tex coords
	for(l = 0; l < numCorners; ++l)
	{		
		for(k = 0; k <= _NumSlices; ++k)
		{
		
			_VB.setTexCoord((_NumSlices + 1) * l + k, 0, (float) k / _NumSlices, (float) l / (numCorners - 1));
		}
	}
	// create primitive block
	_PB.setNumTri(2 * (numCorners - 1) * _NumSlices);
	//
	for(l = 0; l < numCorners - 1; ++l)
	{
		for(k = 0; k < _NumSlices; ++k)
		{
			_PB.setTri(2 * (l * _NumSlices + k), (_NumSlices + 1) * l + k,  (_NumSlices + 1) * (l + 1) + k + 1, (_NumSlices + 1) * (l + 1) + k);
			_PB.setTri(2 * (l * _NumSlices + k) + 1, (_NumSlices + 1) * l + k, (_NumSlices + 1) * l + k + 1, (_NumSlices + 1) * (l + 1) + k + 1);
		}
	}
	_GeomTouched = false;	
}

//===========================================================
void CSegRemanenceShape::setBBox(const NLMISC::CAABBox &bbox)
{
	_BBox = bbox;
}

//===========================================================
void CSegRemanenceShape::setMaterial(const CMaterial &mat)
{
	_Mat = mat;
	_MatTouched = true;
}

//===========================================================
void CSegRemanenceShape::setTextureShifting(bool on /*=true*/)
{
	_TextureShifting = on;
	_MatTouched = true;
}

//===========================================================
void CSegRemanenceShape::setRollupRatio(float ratio)
{
	nlassert(ratio > 0);
	_RollUpRatio = ratio;
}

//===========================================================
void CSegRemanenceShape::setupMaterial()
{
	if (!_MatTouched) return;	
	_Mat.enableUserTexMat(0);
	if (_Mat.getTexture(0)) _Mat.getTexture(0)->setWrapS(ITexture::Clamp);	
	_Mat.setDoubleSided(true);
	_Mat.setLighting(false); // lighting not supported (the vb has no normals anyway..)
	_MatTouched = false;
}

//===========================================================
void CSegRemanenceShape::setAnimatedMaterial(const std::string &name)
{
	nlassert(!name.empty());
	nlassert(_AnimatedMat == NULL);
	_AnimatedMat  = new CMaterialBase;
	_AnimatedMat->Name = name;
}



//===========================================================
CSegRemanenceShape::CSegRemanenceShape(const CSegRemanenceShape &other) : IShape(other), _AnimatedMat(NULL)
{
	copyFromOther(other);
}

//===========================================================
CSegRemanenceShape &CSegRemanenceShape::operator = (const CSegRemanenceShape &other)
{
	if (&other != this)
	{	
		copyFromOther(other);
		(IShape &) *this = (IShape &) other; // copy base part	
	}
	return *this;
}

//===========================================================
CSegRemanenceShape::~CSegRemanenceShape()
{
	delete _AnimatedMat;
}

//===========================================================
void CSegRemanenceShape::copyFromOther(const CSegRemanenceShape &other)
{
	if (&other == this) return;
	CMaterialBase *otherAnimatedMat = other._AnimatedMat != NULL ? new CMaterialBase(*other._AnimatedMat)
																 : NULL;
	delete _AnimatedMat;
	_AnimatedMat = otherAnimatedMat;

	_GeomTouched	 = other._GeomTouched;
	_MatTouched      = other._MatTouched; 
	_TextureShifting = other._TextureShifting;	
	_NumSlices       = other._NumSlices;
	_SliceTime       = other._SliceTime;
	_Corners		 = other._Corners;	
	_Mat             = other._Mat;
	_VB              = other._VB;
	_PB              = other._PB;
	_BBox			 = other._BBox;	
	_RollUpRatio     = other._RollUpRatio;	
}
	


//===========================================================
bool CSegRemanenceShape::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
{
	// Speed Clip: clip just the sphere.
	NLMISC::CBSphere	localSphere(_BBox.getCenter(), _BBox.getRadius());
	NLMISC::CBSphere	worldSphere;

	// transform the sphere in WorldMatrix (with nearly good scale info).
	localSphere.applyTransform(worldMatrix, worldSphere);

	// if out of only plane, entirely out.
	for(sint i=0;i<(sint)pyramid.size();i++)
	{
		// We are sure that pyramid has normalized plane normals.
		// if SpherMax OUT return false.
		float	d= pyramid[i]*worldSphere.Center;
		if(d>worldSphere.Radius)
			return false;
	}
	return true;
}

}