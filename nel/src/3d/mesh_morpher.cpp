/** \file mesh_morpher.cpp
 * <File description>
 *
 * $Id: mesh_morpher.cpp,v 1.1 2001/10/10 16:07:10 besson Exp $
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

#include "3d/mesh_morpher.h"
#include "3d/vertex_buffer.h"
#include "3d/vertex_buffer_hard.h"


using namespace std;
using namespace NLMISC;


namespace NL3D 
{

// ***************************************************************************
void CBlendShape::serial (NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion (0);

	f.serial (Name);

	f.serialCont (deltaPos);
	f.serialCont (deltaNorm);
	f.serialCont (deltaUV);
	f.serialCont (deltaCol);

	f.serialCont (VertRefs);
}

// ***************************************************************************
CMeshMorpher::CMeshMorpher()
{
	_VBOri = NULL;
	_VBDst = NULL;
	_VBDstHrd = NULL;

	_Vertices = NULL;
	_Normals = NULL;
}

// ***************************************************************************
void CMeshMorpher::init (CVertexBuffer *vbOri, CVertexBuffer *vbDst, IVertexBufferHard *vbDstHrd)
{
	_VBOri = vbOri;
	_VBDst = vbDst;
	_VBDstHrd = vbDstHrd;
}

// ***************************************************************************
void CMeshMorpher::initMRM (CVertexBuffer *vbOri, CVertexBuffer *vbDst, IVertexBufferHard *vbDstHrd,
							std::vector<CVector> *vVertices, std::vector<CVector> *vNormals, bool bSkinApplied)
{
	_VBOri = vbOri;
	_VBDst = vbDst;
	_VBDstHrd = vbDstHrd;

	_Vertices = vVertices;
	_Normals = vNormals;
	_SkinApplied = bSkinApplied;
}

// ***************************************************************************
void CMeshMorpher::update (std::vector<CAnimatedMorph> *pBSFactor)
{
	uint32 i, j;

	static uint32 tmp = 0;
	tmp++;

	if (_VBOri == NULL)
		return;
	if (BlendShapes.size() == 0)
		return;

	if (_VBOri->getNumVertices() != _VBDst->getNumVertices())
	{	// Because the original vertex buffer is not initialized by default
		// we must init it here (if there are some blendshapes)
		*_VBOri = *_VBDst;
	}

	// Does the flags are reserved ?
	if (_Flags.size() != _VBOri->getNumVertices())
	{
		_Flags.resize (_VBOri->getNumVertices());
		for (i = 0; i < _Flags.size(); ++i)
			_Flags[i] = 2; // Modified to update all
	}

	nlassert(_VBOri->getVertexFormat() == _VBDst->getVertexFormat());

	// Cleaning with original vertex buffer
	uint32 VBVertexSize = _VBOri->getVertexSize();
	uint8 *pOri = (uint8*)_VBOri->getVertexCoordPointer ();
	uint8 *pDst = (uint8*)_VBDst->getVertexCoordPointer ();
	
	for (i= 0; i < _Flags.size(); ++i)
	if (_Flags[i] == 2)
	{
		_Flags[i] = 1; // OriginalVBDst

		for(j = 0; j < VBVertexSize; ++j)
			pDst[j+i*VBVertexSize] = pOri[j+i*VBVertexSize];
	}

	// Blending with blendshape
	for (i = 0; i < BlendShapes.size(); ++i)
	{
		CBlendShape &rBS = BlendShapes[i];
		float rFactor = pBSFactor->operator[](i).getFactor()/100.0f;

		if (rFactor > 0.0f)
		for (j = 0; j < rBS.VertRefs.size(); ++j)
		{
			uint32 vp = rBS.VertRefs[j];

			if (_VBDst->getVertexFormat() & CVertexBuffer::PositionFlag)
			if (rBS.deltaPos.size() > 0)
			{
				CVector *pV = (CVector*)_VBDst->getVertexCoordPointer (vp);
				*pV += rBS.deltaPos[j] * rFactor;
			}

			if (_VBDst->getVertexFormat() & CVertexBuffer::NormalFlag)
			if (rBS.deltaNorm.size() > 0)
			{
				CVector *pV = (CVector*)_VBDst->getNormalCoordPointer (vp);
				*pV += rBS.deltaNorm[j] * rFactor;
			}

			if (_VBDst->getVertexFormat() & CVertexBuffer::TexCoord0Flag)
			if (rBS.deltaUV.size() > 0)
			{
				CUV *pUV = (CUV*)_VBDst->getTexCoordPointer (vp);
				*pUV += rBS.deltaUV[j] * rFactor;
			}

			if (_VBDst->getVertexFormat() & CVertexBuffer::PrimaryColorFlag)
			if (rBS.deltaCol.size() > 0)
			{
				CRGBA *pRGBA = (CRGBA*)_VBDst->getColorPointer (vp);
				CRGBAF rgbf(*pRGBA);
				rgbf.R += rBS.deltaCol[j].R * rFactor;
				rgbf.G += rBS.deltaCol[j].G * rFactor;
				rgbf.B += rBS.deltaCol[j].B * rFactor;
				rgbf.A += rBS.deltaCol[j].A * rFactor;
				clamp(rgbf.R, 0.0f, 1.0f);
				clamp(rgbf.G, 0.0f, 1.0f);
				clamp(rgbf.B, 0.0f, 1.0f);
				clamp(rgbf.A, 0.0f, 1.0f);
				*pRGBA = rgbf;
			}
			_Flags[vp] = 2; // Modified
		}
	}

	// Copying to hardware vertex buffer if some
	if (_VBDstHrd != NULL)
	{
		uint8 *pDstHrd = (uint8*)_VBDstHrd->lock();
		for (i = 0; i < _Flags.size(); ++i)
		{
			if (_Flags[i] != 0) // Not OriginalAll ?
			{
				for(j = 0; j < VBVertexSize; ++j)
					pDstHrd[j+i*VBVertexSize] = pDst[j+i*VBVertexSize];
			}
			if (_Flags[i] == 1) // OriginalVBDst ?
				_Flags[i] = 0; // So OriginalAll !
		}
		_VBDstHrd->unlock();
	}
}

// ***************************************************************************
void CMeshMorpher::updateMRM (std::vector<CAnimatedMorph> *pBSFactor)
{
	uint32 i, j;

	static uint32 tmp = 0;
	tmp++;

	if (_VBOri == NULL)
		return;
	if (BlendShapes.size() == 0)
		return;

	if (_VBOri->getNumVertices() != _VBDst->getNumVertices())
	{	// Because the original vertex buffer is not initialized by default
		// we must init it here (if there are some blendshapes)
		*_VBOri = *_VBDst;
	}

	// Does the flags are reserved ?
	if (_Flags.size() != _VBOri->getNumVertices())
	{
		_Flags.resize (_VBOri->getNumVertices());
		for (i = 0; i < _Flags.size(); ++i)
			_Flags[i] = ModifiedUVCol; // Modified to update all
	}

	nlassert(_VBOri->getVertexFormat() == _VBDst->getVertexFormat());

	// Cleaning with original vertex buffer
	uint32 VBVertexSize = _VBOri->getVertexSize();
	uint8 *pOri = (uint8*)_VBOri->getVertexCoordPointer ();
	uint8 *pDst = (uint8*)_VBDst->getVertexCoordPointer ();
	
	for (i= 0; i < _Flags.size(); ++i)
	if ((_Flags[i] == ModifiedPosNorm) || (_Flags[i] == ModifiedUVCol))
	{
		for(j = 0; j < VBVertexSize; ++j)
			pDst[j+i*VBVertexSize] = pOri[j+i*VBVertexSize];
		
		if (_VBDst->getVertexFormat() & CVertexBuffer::PositionFlag)
			if (_Vertices != NULL)
				_Vertices->operator[](i) = ((CVector*)(pOri+i*VBVertexSize))[0];

		if (_VBDst->getVertexFormat() & CVertexBuffer::NormalFlag)
			if (_Normals != NULL)
				_Normals->operator[](i) = ((CVector*)(pOri+i*VBVertexSize))[1];
			
		_Flags[i] = OriginalVBDst;
	}

	// Blending with blendshape
	for (i = 0; i < BlendShapes.size(); ++i)
	{
		CBlendShape &rBS = BlendShapes[i];
		float rFactor = pBSFactor->operator[](i).getFactor()/100.0f;

		if (rFactor > 0.0f)
		for (j = 0; j < rBS.VertRefs.size(); ++j)
		{
			uint32 vp = rBS.VertRefs[j];

			if (_Vertices != NULL)
			{
				if (rBS.deltaPos.size() > 0)
				{
					CVector *pV = &(_Vertices->operator[](vp));
					*pV += rBS.deltaPos[j] * rFactor;
					_Flags[vp] = ModifiedPosNorm;
				}
			}
			else
			{
				if (_VBDst->getVertexFormat() & CVertexBuffer::PositionFlag)
				if (rBS.deltaPos.size() > 0)
				{
					CVector *pV = (CVector*)_VBDst->getVertexCoordPointer (vp);
					*pV += rBS.deltaPos[j] * rFactor;
					_Flags[vp] = ModifiedPosNorm;
				}
			}

			if (_Normals != NULL)
			{
				if (rBS.deltaNorm.size() > 0)
				{
					CVector *pV = &(_Normals->operator[](vp));
					*pV += rBS.deltaNorm[j] * rFactor;
					_Flags[vp] = ModifiedPosNorm;
				}
			}
			else
			{
				if (_VBDst->getVertexFormat() & CVertexBuffer::NormalFlag)
				if (rBS.deltaNorm.size() > 0)
				{
					CVector *pV = (CVector*)_VBDst->getNormalCoordPointer (vp);
					*pV += rBS.deltaNorm[j] * rFactor;
					_Flags[vp] = ModifiedPosNorm;
				}
			}

			if (_VBDst->getVertexFormat() & CVertexBuffer::TexCoord0Flag)
			if (rBS.deltaUV.size() > 0)
			{
				CUV *pUV = (CUV*)_VBDst->getTexCoordPointer (vp);
				*pUV += rBS.deltaUV[j] * rFactor;
				_Flags[vp] = ModifiedUVCol;
			}

			if (_VBDst->getVertexFormat() & CVertexBuffer::PrimaryColorFlag)
			if (rBS.deltaCol.size() > 0)
			{
				CRGBA *pRGBA = (CRGBA*)_VBDst->getColorPointer (vp);
				CRGBAF rgbf(*pRGBA);
				rgbf.R += rBS.deltaCol[j].R * rFactor;
				rgbf.G += rBS.deltaCol[j].G * rFactor;
				rgbf.B += rBS.deltaCol[j].B * rFactor;
				rgbf.A += rBS.deltaCol[j].A * rFactor;
				clamp(rgbf.R, 0.0f, 1.0f);
				clamp(rgbf.G, 0.0f, 1.0f);
				clamp(rgbf.B, 0.0f, 1.0f);
				clamp(rgbf.A, 0.0f, 1.0f);
				*pRGBA = rgbf;
				_Flags[vp] = ModifiedUVCol;
			}
		}
	}

	// If the skin is applied we have to transfert only vertices that have modified UV or Color
	// Because the skinning will transfert vertices Pos and Norm.
	if (_SkinApplied) 
	{
		// Copying to hardware vertex buffer if some
		if (_VBDstHrd != NULL)
		{
			uint8 *pDstHrd = (uint8*)_VBDstHrd->lock();
			for (i = 0; i < _Flags.size(); ++i)
			{
				if (_Flags[i] == ModifiedUVCol) // Modified UV or Color ?
				{
					// We must write the whole vertex because the skinning may not copy 
					// vertex and normal changes into VBHard
					for(j = 0; j < VBVertexSize; ++j)
						pDstHrd[j+i*VBVertexSize] = pDst[j+i*VBVertexSize];
				}
			}
			_VBDstHrd->unlock();
		}
	}
	else
	{
		// Copying to hardware vertex buffer if some
		if (_VBDstHrd != NULL)
		{
			uint8 *pDstHrd = (uint8*)_VBDstHrd->lock();
			for (i = 0; i < _Flags.size(); ++i)
			{
				if (_Flags[i] != OriginalAll) // Not OriginalAll ?
				{
					// We must write the whole vertex because the skinning may not copy 
					// vertex and normal changes into VBHard
					for(j = 0; j < VBVertexSize; ++j)
						pDstHrd[j+i*VBVertexSize] = pDst[j+i*VBVertexSize];
				}
				if (_Flags[i] == OriginalVBDst) // OriginalVBDst ?
					_Flags[i] = OriginalAll; // So OriginalAll !
			}
			_VBDstHrd->unlock();
		}
	}
}

// ***************************************************************************
void CMeshMorpher::serial (NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion (0);

	f.serialCont (BlendShapes);
}


} // NL3D










