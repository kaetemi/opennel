/** \file transformable.cpp
 * <File description>
 *
 * $Id: transformable.cpp,v 1.6 2001/03/28 10:33:00 berenguier Exp $
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

#include "nel/3d/transformable.h"
#include "nel/3d/channel_mixer.h"


namespace NL3D 
{


// ***************************************************************************
ITransformable::ITransformable()
{
	// Set number of animated values.
	IAnimatable::resize (AnimValueLast);

	// Deriver note: just copy this line in each ctor.

	// Init default values.
	_Mode= DirectMatrix;
	// matrix init to identity.
	_Pos.Value= CVector::Null;
	_RotEuler.Value= CVector::Null;
	_RotQuat.Value= CQuat::Identity;
	_Scale.Value= CVector(1,1,1);
	_Pivot.Value= CVector::Null;

	_Father= NULL;
	_FatherScaleDate= 0;
	_LocalScaleDate= 0;
}


// ***************************************************************************
IAnimatedValue*		ITransformable::getValue (uint valueId)
{
	// what value ?
	switch (valueId)
	{
	case PosValue:			return &_Pos;
	case RotEulerValue:		return &_RotEuler;
	case RotQuatValue:		return &_RotQuat;
	case ScaleValue:		return &_Scale;
	case PivotValue:		return &_Pivot;
	}

	// No, only ITrnasformable values!
	nlstop;
	// Deriver note: else call BaseClass::getValue(valueId);

	return NULL;
}
// ***************************************************************************
const char 	*ITransformable::getValueName (uint valueId) const
{
	// what value ?
	switch (valueId)
	{
	case PosValue:			return getPosValueName ();
	case RotEulerValue:		return getRotEulerValueName();
	case RotQuatValue:		return getRotQuatValueName();
	case ScaleValue:		return getScaleValueName();
	case PivotValue:		return getPivotValueName();
	}

	// No, only ITrnasformable values!
	nlstop;
	// Deriver note: else call BaseClass::getValueName(valueId);

	return "";
}

// ***************************************************************************
const char	*ITransformable::getPosValueName ()
{
	return "pos";
}
// ***************************************************************************
const char	*ITransformable::getRotEulerValueName()
{
	return "roteuler";
}
// ***************************************************************************
const char	*ITransformable::getRotQuatValueName()
{
	return "rotquat";
}
// ***************************************************************************
const char	*ITransformable::getScaleValueName()
{
	return "scale";
}
// ***************************************************************************
const char	*ITransformable::getPivotValueName()
{
	return "pivot";
}


// ***************************************************************************
void	ITransformable::clearTransformFlags() const
{
	ITransformable	*self= const_cast<ITransformable*>(this);

	// clear my falgs.
	self->clearFlag(PosValue);
	self->clearFlag(RotEulerValue); 
	self->clearFlag(RotQuatValue); 
	self->clearFlag(ScaleValue);
	self->clearFlag(PivotValue);

	// We are OK!
	self->clearFlag(OwnerBit);
}

// ***************************************************************************
bool	ITransformable::needCompute() const
{
	bool	fatherOk;
	bool	fatherScaleTest;
	fatherOk= _Father && _Father->_Mode!=DirectMatrix;
	fatherScaleTest= fatherOk && (_Father->isTouched(OwnerBit) || _Father->_LocalScaleDate>_FatherScaleDate) ;
	// should we update?
	return  isTouched(OwnerBit) || fatherScaleTest;
}


// ***************************************************************************
void	ITransformable::updateMatrix() const
{
	// should we update?
	if(_Mode!=DirectMatrix && needCompute())
	{
		clearTransformFlags();
		// update scale date (so sons are informed of change).
		_LocalScaleDate++;

		// update the matrix.
		_LocalMatrix.identity();

		bool	fatherOk;
		fatherOk= _Father && _Father->_Mode!=DirectMatrix;
		if(fatherOk)
		{
			// copy the scale date to say we are up to date.
			_FatherScaleDate= _Father->_LocalScaleDate;

			// unherit the father scale.
			// T*Sp-1*P
			_LocalMatrix.translate(_Pos.Value);
			CVector		&vs= _Father->_Scale.Value;
			_LocalMatrix.scale(CVector(1.0f/vs.x, 1.0f/vs.y, 1.0f/vs.z));
			_LocalMatrix.translate(_Pivot.Value);
		}
		else
		{
			// father scale will be herited.
			// T*P
			_LocalMatrix.translate(_Pos.Value+_Pivot.Value);
		}

		// R*S*P-1.
		if(_Mode==RotEuler)
			_LocalMatrix.rotate(_RotEuler.Value, _RotOrder);
		else
			_LocalMatrix.rotate(_RotQuat.Value);
		_LocalMatrix.scale(_Scale.Value);
		_LocalMatrix.translate(-_Pivot.Value);
	}
}


// ***************************************************************************
void		ITransformable::lookAt (const CVector& eye, const CVector& target, float roll)
{
	nlassert(_Mode==RotQuat || _Mode==DirectMatrix);

	// Roll matrix
	CMatrix rollMT;
	rollMT.identity();
	if (roll!=0.f)
		rollMT.rotateY (roll);

	// Make the target base
	CVector j=target;
	j-=eye;
	j.normalize();
	CVector i=j^CVector (0,0,1.f);
	CVector k=i^j;
	k.normalize();
	i=j^k;
	i.normalize();

	// Make the target matrix
	CMatrix targetMT;
	targetMT.identity();
	targetMT.setRot (i, j, k);
	targetMT.setPos (eye);

	// Compose matrix
	targetMT*=rollMT;

	// Set the matrix
	if(_Mode==DirectMatrix)
		setMatrix (targetMT);
	else
	{
		// transfrom to quaternion mode.
		setScale(CVector(1,1,1));
		setPivot(CVector::Null);
		setPos(targetMT.getPos());
		setRotQuat(targetMT.getRot());
	}
}


} // NL3D
