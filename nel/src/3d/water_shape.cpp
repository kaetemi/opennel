/** \file water_shape.cpp
 * <File description>
 *
 * $Id: water_shape.cpp,v 1.6 2001/11/16 16:48:08 vizerie Exp $
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

#include "3d/water_shape.h"
#include "3d/water_model.h"
#include "3d/vertex_buffer.h"
#include "3d/texture_bump.h"
#include "3d/texture_blend.h"
#include "3d/scene.h"
	#include "3d/water_pool_manager.h"
#include <memory>


namespace NL3D {




// globals
static std::auto_ptr<CVertexProgram> WaterVP(NULL);
const char *WaterVpCode = "!!VP1.0\n\
					  ADD R1, c[7], -v[0];			#r1 = eye - vertex							\n\
					  DP3 R2, R1, R1;				#r1 = eye - vertex, r2 = (eye - vertex)²	\n\
					  MAX R2, R2, c[18];            # avoid imprecision around 0				\n\
					  RSQ R2, R2.x;					#r1 = eye - vertex, r2 = 1/d(eye, vertex)	\n\
					  RCP R3, R2.x;																\n\
					  MUL R3, c[6], R3;															\n\
					  ADD R3, c[17], -R3;														\n\
					  MAX R3, c[5],	R3;															\n\
					  MUL R0,   R3, v[8];			#attenuate normal with distance             \n\
					  MUL R4.z,   R3, v[0];			#attenuate normal with distance				\n\
					  MOV R4.xyw, v[0];															\n\
					  MOV R0.z,  c[4].x;			#set normal z to 1							\n\
					  DP3 R3.x, R0, R0;															\n\
					  RSQ R3.x,  R3.x;				#normalize normal in R3						\n\
					  MUL R0,  R0, R3.x;														\n\
					  DP4 o[HPOS].x, c[0], R4;	#transform vertex in view space					\n\
					  DP4 o[HPOS].y, c[1], R4;													\n\
					  DP4 o[HPOS].z, c[2], R4;													\n\
					  DP4 o[HPOS].w, c[3], R4;													\n\
					  MUL R3, v[0], c[12];			#compute bump 0 uv's						\n\
					  ADD o[TEX0].xy, R3, c[11];												\n\
					  MUL R3, v[0], c[14];			#compute bump 1 uv's						\n\
					  ADD o[TEX1].xy, R3, c[13];												\n\
					  MUL R1, R1, R2.x;		        #normalize r1, r1 = (eye - vertex).normed   \n\
					  DP3 R2.x, R1, R0;\n\
					  MUL R0, R0, R2.x;\n\
					  ADD R2, R0, R0;\n\
					  ADD R0, R2, -R1;				#compute reflection vector \n\
					  MUL o[TEX2].xy, R0, c[8];\n\
					  END\
					  ";


const char *WaterPlusAlphaVpCode = "!!VP1.0\n\
					  ADD R1, c[7], -v[0];			#r1 = eye - vertex							\n\
					  DP3 R2, R1, R1;				#r1 = eye - vertex, r2 = (eye - vertex)²	\n\
					  MAX R2, R2, c[18];            # avoid imprecision around 0				\n\
					  RSQ R2, R2.x;					#r1 = eye - vertex, r2 = 1/d(eye, vertex)	\n\
					  RCP R3, R2.x;																\n\
					  MUL R3, c[6], R3;															\n\
					  ADD R3, c[17], -R3;														\n\
					  MAX R3, c[5],	R3;															\n\
					  MUL R0,   R3, v[8];			#attenuate normal with distance             \n\
					  MUL R4.z,   R3, v[0];			#attenuate normal with distance				\n\
					  MOV R4.xyw, v[0];															\n\
					  MOV R0.z,  c[4].x;			#set normal z to 1							\n\
					  DP3 R3.x, R0, R0;															\n\
					  RSQ R3.x,  R3.x;				#normalize normal in R3						\n\
					  MUL R0,  R0, R3.x;														\n\
					  DP4 o[HPOS].x, c[0], R4;	#transform vertex in view space					\n\
					  DP4 o[HPOS].y, c[1], R4;													\n\
					  DP4 o[HPOS].z, c[2], R4;													\n\
					  DP4 o[HPOS].w, c[3], R4;													\n\
					  MUL R3, v[0], c[12];			#compute bump 0 uv's						\n\
					  ADD o[TEX0].xy, R3, c[11];												\n\
					  MUL R3, v[0], c[14];			#compute bump 1 uv's						\n\
					  ADD o[TEX1].xy, R3, c[13];												\n\
					  DP4 o[TEX3].x, R4, c[15]; #compute uv for diffuse texture				\n\
					  DP4 o[TEX3].y, R4, c[16];												\n\
					  MUL R1, R1, R2.x;		        #normalize r1, r1 = (eye - vertex).normed   \n\
					  DP3 R2.x, R1, R0;\n\
					  MUL R0, R0, R2.x;\n\
					  ADD R2, R0, R0;\n\
					  ADD R0, R2, -R1;				#compute reflection vector \n\
					  MUL o[TEX2].xy, R0, c[8];\n\
					  END\
					  ";



// temporary code for gfx card that have less than 4 stage. We keep the envmap only
const char *WaterVpCode2Stages = "!!VP1.0\n\
					  ADD R1, c[7], -v[0];			#r1 = eye - vertex							\n\
					  DP3 R2, R1, R1;				#r1 = eye - vertex, r2 = (eye - vertex)²	\n\
					  MAX R2, R2, c[18];            # avoid imprecision around 0				\n\
					  RSQ R2, R2.x;					#r1 = eye - vertex, r2 = 1/d(eye, vertex)	\n\
					  RCP R3, R2.x;																\n\
					  MUL R3, c[6], R3;															\n\
					  ADD R3, c[17], -R3;														\n\
					  MAX R3, c[5],	R3;															\n\
					  MUL R0,   R3, v[8];			#attenuate normal with distance             \n\
					  MUL R4.z,   R3, v[0];			#attenuate normal with distance				\n\
					  MOV R4.xyw, v[0];															\n\
					  MOV R0.z,  c[4].x;			#set normal z to 1							\n\
					  DP3 R3.x, R0, R0;															\n\
					  RSQ R3.x,  R3.x;				#normalize normal in R3						\n\
					  MUL R0,  R0, R3.x;														\n\
					  DP4 o[HPOS].x, c[0], R4;	#transform vertex in view space					\n\
					  DP4 o[HPOS].y, c[1], R4;													\n\
					  DP4 o[HPOS].z, c[2], R4;													\n\
					  DP4 o[HPOS].w, c[3], R4;													\n\
					  MUL R1, R1, R2.x;		        #normalize r1, r1 = (eye - vertex).normed   \n\
					  DP3 R2.x, R1, R0;\n\
					  MUL R0, R0, R2.x;\n\
					  ADD R2, R0, R0;\n\
					  ADD R0, R2, -R1;				#compute reflection vector \n\
					  MUL o[TEX0].xy, R0, c[8];\n\
					  END\
					  ";

const char *WaterVpCode2StagesAlpha = "!!VP1.0\n\
					  ADD R1, c[7], -v[0];			#r1 = eye - vertex							\n\
					  DP3 R2, R1, R1;				#r1 = eye - vertex, r2 = (eye - vertex)²	\n\
					  MAX R2, R2, c[18];            # avoid imprecision around 0				\n\
					  RSQ R2, R2.x;					#r1 = eye - vertex, r2 = 1/d(eye, vertex)	\n\
					  RCP R3, R2.x;																\n\
					  MUL R3, c[6], R3;															\n\
					  ADD R3, c[17], -R3;														\n\
					  MAX R3, c[5],	R3;															\n\
					  MUL R0,   R3, v[8];			#attenuate normal with distance             \n\
					  MUL R4.z,   R3, v[0];			#attenuate normal with distance				\n\
					  MOV R4.xyw, v[0];															\n\
					  MOV R0.z,  c[4].x;			#set normal z to 1							\n\
					  DP3 R3.x, R0, R0;															\n\
					  RSQ R3.x,  R3.x;				#normalize normal in R3						\n\
					  MUL R0,  R0, R3.x;														\n\
					  DP4 o[HPOS].x, c[0], R4;	#transform vertex in view space					\n\
					  DP4 o[HPOS].y, c[1], R4;													\n\
					  DP4 o[HPOS].z, c[2], R4;													\n\
					  DP4 o[HPOS].w, c[3], R4;													\n\
					  DP4 o[TEX1].x, v[0], c[15];\n\
					  DP4 o[TEX1].y, v[0], c[16];\n\
					  MUL R1, R1, R2.x;		        #normalize r1, r1 = (eye - vertex).normed   \n\
					  DP3 R2.x, R1, R0;\n\
					  MUL R0, R0, R2.x;\n\
					  ADD R2, R0, R0;\n\
					  ADD R0, R2, -R1;				#compute reflection vector \n\
					  MUL o[TEX0].xy, R0, c[8];\n\
					  END\
					  ";


// static members

uint32									CWaterShape::_XScreenGridSize = 50;
uint32									CWaterShape::_YScreenGridSize = 50;
uint32									CWaterShape::_XGridBorder = 4;
uint32									CWaterShape::_YGridBorder = 4;
uint32									CWaterShape::_MaxGridSize;
CVertexBuffer							CWaterShape::_VB;
std::vector<uint32>						CWaterShape::_IBUpDown;
std::vector<uint32>						CWaterShape::_IBDownUp;
//NLMISC::CSmartPtr<IDriver>				CWaterShape::_Driver;
bool									CWaterShape::_GridSizeTouched = true;
std::auto_ptr<CVertexProgram>			CWaterShape::_VertexProgram;
std::auto_ptr<CVertexProgram>			CWaterShape::_VertexProgramAlpha;
std::auto_ptr<CVertexProgram>			CWaterShape::_VertexProgram2Stages;
std::auto_ptr<CVertexProgram>			CWaterShape::_VertexProgram2StagesAlpha;








/*
 * Constructor
 */
CWaterShape::CWaterShape() :  _WaterPoolID(0), _TransitionRatio(0.6f), _WaveHeightFactor(3)
{
	_DefaultPos.setValue(NLMISC::CVector::Null);
	_DefaultScale.setValue(NLMISC::CVector(1, 1, 1));
	_DefaultRotQuat.setValue(CQuat::Identity);

	for (sint k = 0; k < 2; ++k)
	{
		_HeightMapScale[k].set(1, 1);
		_HeightMapSpeed[k].set(0, 0);
	}
	_ColorMapMatColumn0.set(1, 0);
	_ColorMapMatColumn1.set(0, 1);
	_ColorMapMatPos.set(0, 0);
}

CWaterShape::~CWaterShape()
{
	if (
		(_EnvMap[0] && dynamic_cast<CTextureBlend *>((ITexture *) _EnvMap[0]))
		|| (_EnvMap[1] && dynamic_cast<CTextureBlend *>((ITexture *) _EnvMap[1]))
		)
	{
		GetWaterPoolManager().unRegisterWaterShape(this);		
	}
}


void CWaterShape::initVertexProgram()
{	
	static bool created = false;
	if (!created)
	{
		_VertexProgram		= std::auto_ptr<CVertexProgram>(new CVertexProgram(WaterVpCode));	
		_VertexProgramAlpha = std::auto_ptr<CVertexProgram>(new CVertexProgram(WaterPlusAlphaVpCode));	
		_VertexProgram2Stages = std::auto_ptr<CVertexProgram>(new CVertexProgram(WaterVpCode2Stages));
		_VertexProgram2StagesAlpha = std::auto_ptr<CVertexProgram>(new CVertexProgram(WaterVpCode2StagesAlpha));
		created = true;
	}
}



void CWaterShape::setupVertexBuffer()
{
	const uint rotLenght = (uint) ::ceilf(::sqrtf((float) ((_XScreenGridSize >> 1) * (_XScreenGridSize >> 1)
								  + (_YScreenGridSize >> 1) * (_YScreenGridSize >> 1))));
	_MaxGridSize = 2 * rotLenght;
	const uint w =  _MaxGridSize + 2 * _XGridBorder;

	_VB.clearValueEx();
	_VB.addValueEx (WATER_VB_POS, CVertexBuffer::Float3);
	_VB.addValueEx (WATER_VB_DX, CVertexBuffer::Float2);	

	_VB.initEx();
	_VB.setNumVertices((w + 1) * 2);
	

	
	uint x;

	// setup each index buffer
	// We need 2 vb, because, each time 2 lines of the vertex buffer are filled, we start at the beginning again
	// So we need 1 vb for triangle drawn up to down, and one other for triangle drawn down to top	

	_IBUpDown.resize(6 * w);	
	for (x = 0; x < w; ++x)
	{
		_IBUpDown [ 6 * x      ] = x;
		_IBUpDown [ 6 * x  + 1 ] = x + 1 + (w + 1);
		_IBUpDown [ 6 * x  + 2 ] = x + 1;

		_IBUpDown [ 6 * x  + 3 ] = x;
		_IBUpDown [ 6 * x  + 4 ] = x + 1 + (w + 1);
		_IBUpDown [ 6 * x  + 5 ] = x     + (w + 1);

	}

	_IBDownUp.resize(6 * w);
	for (x = 0; x < w; ++x)
	{
		_IBDownUp [ 6 * x      ] = x;
		_IBDownUp [ 6 * x  + 1 ] = x + 1;
		_IBDownUp [ 6 * x  + 2 ] = x + 1 + (w + 1);

		_IBDownUp [ 6 * x  + 3 ] = x;
		_IBDownUp [ 6 * x  + 4 ] = x     + (w + 1);
		_IBDownUp [ 6 * x  + 5 ] = x + 1 + (w + 1);

	}

	_GridSizeTouched = false;
}

CTransformShape		*CWaterShape::createInstance(CScene &scene)
{
	CWaterModel *wm = NLMISC::safe_cast<CWaterModel *>(scene.createModel(WaterModelClassId) );
	wm->Shape = this;
	// set default pos & scale
	wm->ITransformable::setPos( ((CAnimatedValueVector&)_DefaultPos.getValue()).Value  );
	wm->ITransformable::setScale( ((CAnimatedValueVector&)_DefaultScale.getValue()).Value  );
	wm->ITransformable::setRotQuat( ((CAnimatedValueQuat&)_DefaultRotQuat.getValue()).Value  );
	wm->_Scene = &scene;
	return wm;
}




float CWaterShape::getNumTriangles (float distance)
{
	// TODO
	return 0;
}

void CWaterShape::flushTextures (IDriver &driver)
{
	for (uint k = 0; k < 2; ++k)
	{
		if (_BumpMap[k] != NULL)
			_BumpMap[k]->generate();
	}
}

void	CWaterShape::setScreenGridSize(uint32 x, uint32 y)
{
	nlassert(x > 0 && y > 0);
	_XScreenGridSize = x;	
	_YScreenGridSize = y;
	_GridSizeTouched = true;
}

void		CWaterShape::setGridBorderSize(uint32 x, uint32 y)
{
	_XGridBorder = x;
	_YGridBorder = y;
	_GridSizeTouched = true;
}



void CWaterShape::setShape(const NLMISC::CPolygon2D &poly)
{
	nlassert(poly.Vertices.size() != 0); // empty poly not allowed
	_Poly = poly;
	computeBBox();
}

void CWaterShape::computeBBox()
{
	nlassert(_Poly.Vertices.size() != 0);
	NLMISC::CVector2f min, max;
	min = max = _Poly.Vertices[0];
	for (uint k = 1; k < _Poly.Vertices.size(); ++k)
	{
		min.minof(min, _Poly.Vertices[k]);
		max.maxof(max, _Poly.Vertices[k]);
	}
	_BBox.setMinMax(CVector(min.x, min.y, 0), CVector(max.x, max.y, 0));
}


void				CWaterShape::setHeightMap(uint k, ITexture *hm)
{
	nlassert(k < 2);	
	if (!_BumpMap[k])
	{
		_BumpMap[k] = new CTextureBump;
	}
	((CTextureBump *) (ITexture *) _BumpMap[k])->setHeightMap(hm);
}

ITexture			*CWaterShape::getHeightMap(uint k)
{
	nlassert(k < 2);
	return ((CTextureBump *) (ITexture *) _BumpMap[k] )->getHeightMap();
}

const ITexture		*CWaterShape::getHeightMap(uint k) const
{
	nlassert(k < 2);
	return ((CTextureBump *) (ITexture *) _BumpMap[k] )->getHeightMap();
}

void CWaterShape::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion(0);
	// serial 'shape' 
	f.serial(_Poly);
	// serial heightMap identifier
	f.serial(_WaterPoolID);
	//serial maps
	ITexture *map = NULL;	
	if (f.isReading())
	{
		f.serialPolyPtr(map); _EnvMap[0] = map;
		f.serialPolyPtr(map); _EnvMap[1] = map;
		f.serialPolyPtr(map); _BumpMap[0] = map;
		f.serialPolyPtr(map); _BumpMap[1] = map;
		f.serialPolyPtr(map); _ColorMap = map;
		computeBBox();
	}
	else
	{
		map = _EnvMap[0]; f.serialPolyPtr(map);
		map = _EnvMap[1]; f.serialPolyPtr(map);
		map = _BumpMap[0]; f.serialPolyPtr(map);
		map = _BumpMap[1]; f.serialPolyPtr(map);
		map = _ColorMap; f.serialPolyPtr(map);	
	}

	f.serial(_HeightMapScale[0], _HeightMapScale[1],
			 _HeightMapSpeed[0], _HeightMapSpeed[1]);

	f.serial(_ColorMapMatColumn0, _ColorMapMatColumn1, _ColorMapMatPos);	

	// serial default tracks
	f.serial(_DefaultPos);
	f.serial(_DefaultScale);
	f.serial(_DefaultRotQuat);

	f.serial(_TransitionRatio);	

	f.serial(_WaveHeightFactor);
	
}


bool CWaterShape::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
{	
	for (uint k = 0; k < pyramid.size(); ++k)
	{
		if (! _BBox.clipBack(pyramid[k] * worldMatrix)) return false;
	}
	return true;
}

void				CWaterShape::setHeightMapScale(uint k, const NLMISC::CVector2f &scale)
{
	nlassert(k < 2);
	_HeightMapScale[k] = scale;
}
NLMISC::CVector2f	CWaterShape::getHeightMapScale(uint k) const
{
	nlassert(k < 2);
	return _HeightMapScale[k];
}
void			    CWaterShape::setHeightMapSpeed(uint k, const NLMISC::CVector2f &speed)
{
	nlassert(k < 2);
	_HeightMapSpeed[k] = speed;
}
NLMISC::CVector2f   CWaterShape::getHeightMapSpeed(uint k) const
{
	nlassert(k < 2);
	return _HeightMapSpeed[k];
}

void	CWaterShape::setColorMapMat(const NLMISC::CVector2f &column0, const NLMISC::CVector2f &column1, const NLMISC::CVector2f &pos)
{
	_ColorMapMatColumn0 = column0;
	_ColorMapMatColumn1 = column1;
	_ColorMapMatPos  = pos;
}

void	CWaterShape::getColorMapMat(NLMISC::CVector2f &column0, NLMISC::CVector2f &column1, NLMISC::CVector2f &pos)
{
	column0 = _ColorMapMatColumn0;
	column1 = _ColorMapMatColumn1;
	pos  = _ColorMapMatPos;
}

void CWaterShape::envMapUpdate()
{
	// if the color map is a blend texture, we MUST be registered to the water pool manager, so that, the
	// setBlend message will be routed to this texture.
	if (
		(_EnvMap[0] && dynamic_cast<CTextureBlend *>((ITexture *) _EnvMap[0]))
		|| (_EnvMap[1] && dynamic_cast<CTextureBlend *>((ITexture *) _EnvMap[1]))
		)
	{
		if (!GetWaterPoolManager().isWaterShapeObserver(this))
		{
			GetWaterPoolManager().registerWaterShape(this);
		}
	}
	else
	{
		if (GetWaterPoolManager().isWaterShapeObserver(this))
		{
			GetWaterPoolManager().unRegisterWaterShape(this);
		}
	}
}

void CWaterShape::setColorMap(ITexture *map)
{ 
	_ColorMap = map; 
	//colorMapUpdate();
}

void CWaterShape::setEnvMap(uint index, ITexture *envMap)
{
	nlassert(index < 2);
	_EnvMap[index] = envMap;	
}


void CWaterShape::getShapeInWorldSpace(NLMISC::CPolygon &poly) const
{
	poly.Vertices.clear();
	// compute the matrix of the object in world space, by using the default tracks
	NLMISC::CMatrix objMat;
	objMat.identity();
	objMat.rotate(((CAnimatedValueQuat *) &_DefaultRotQuat.getValue())->Value);
	objMat.scale(((CAnimatedValueVector *) &_DefaultScale.getValue())->Value);
	objMat.scale(((CAnimatedValueVector *) &_DefaultScale.getValue())->Value);
	objMat.translate(((CAnimatedValueVector *) &_DefaultPos.getValue())->Value);

	for (uint k = 0; k < _Poly.Vertices.size(); ++k)
	{
		poly.Vertices[k] = objMat * NLMISC::CVector(_Poly.Vertices[k].x, _Poly.Vertices[k].y, 0);
	}

}


} // NL3D
