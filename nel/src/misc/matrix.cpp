/** \file matrix.cpp
 * <description>
 *
 * $Id: matrix.cpp,v 1.20 2001/01/03 15:26:32 berenguier Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#include "nel/misc/matrix.h"
#include "nel/misc/plane.h"
#include "nel/misc/debug.h"
#include <algorithm>
using namespace std;



namespace	NLMISC
{

// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// State Bits.
#define	MAT_TRANS		1
#define	MAT_ROT			2
#define	MAT_SCALEUNI	4
#define	MAT_SCALEANY	8
#define	MAT_PROJ		16
// Validity bits. These means that the part may be yet identity, but is valid in the floats.
// eg: MAT_VALTRANS has no means if MAT_TRANS is set.
#define	MAT_VALIDTRANS	32
#define	MAT_VALIDROT	64
#define	MAT_VALIDPROJ	128
#define	MAT_VALIDALL	(MAT_VALIDTRANS | MAT_VALIDROT | MAT_VALIDPROJ)
// The identity is nothing.
#define	MAT_IDENTITY	0



// Matrix elements.
#define	a11		M[0]
#define	a21		M[1]
#define	a31		M[2]
#define	a41		M[3]
#define	a12		M[4]
#define	a22		M[5]
#define	a32		M[6]
#define	a42		M[7]
#define	a13		M[8]
#define	a23		M[9]
#define	a33		M[10]
#define	a43		M[11]
#define	a14		M[12]
#define	a24		M[13]
#define	a34		M[14]
#define	a44		M[15]



// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
inline bool	CMatrix::hasRot() const
{
	return (StateBit&(MAT_ROT|MAT_SCALEUNI|MAT_SCALEANY))!=0;
}
inline bool	CMatrix::hasTrans() const
{
	return (StateBit&MAT_TRANS)!=0;
}
inline bool	CMatrix::hasProj() const
{
	return (StateBit&MAT_PROJ)!=0;
}
inline bool	CMatrix::hasAll() const
{
	return (hasRot() && hasTrans() && hasProj());
}


inline void CMatrix::testExpandRot() const
{
	if(hasRot())
		return;
	if(!(StateBit&MAT_VALIDROT))
	{
		CMatrix	*self= const_cast<CMatrix*>(this);
		self->StateBit|=MAT_VALIDROT;
		self->a11= 1; self->a12=0; self->a13=0;
		self->a21= 0; self->a22=1; self->a23=0;
		self->a31= 0; self->a32=0; self->a33=1;
		self->Scale33= 1;
	}
}
inline void CMatrix::testExpandTrans() const
{
	if(hasTrans())
		return;
	if(!(StateBit&MAT_VALIDTRANS))
	{
		CMatrix	*self= const_cast<CMatrix*>(this);
		self->StateBit|=MAT_VALIDTRANS;
		self->a14=0;
		self->a24=0;
		self->a34=0;
	}
}
inline void CMatrix::testExpandProj() const
{
	if(hasProj())
		return;
	if(!(StateBit&MAT_VALIDPROJ))
	{
		CMatrix	*self= const_cast<CMatrix*>(this);
		self->StateBit|=MAT_VALIDPROJ;
		self->a41=0; self->a42=0; self->a43=0; self->a44=1; 
	}
}


// ======================================================================================================
CMatrix::CMatrix(const CMatrix &m)
{
	(*this)= m;
}
// ======================================================================================================
CMatrix		&CMatrix::operator=(const CMatrix &m)
{
	StateBit= m.StateBit & ~MAT_VALIDALL;
	if(hasAll())
	{
		memcpy(M, m.M, 16*sizeof(float));
		Scale33= m.Scale33;
	}
	else
	{
		if(hasRot())
		{
			memcpy(&a11, &m.a11, 3*sizeof(float));
			memcpy(&a12, &m.a12, 3*sizeof(float));
			memcpy(&a13, &m.a13, 3*sizeof(float));
			Scale33= m.Scale33;
		}
		if(hasTrans())
		{
			memcpy(&a14, &m.a14, 3*sizeof(float));
		}
		if(hasProj())
		{
			a41= m.a41;
			a42= m.a42;
			a43= m.a43;
			a44= m.a44;
		}
	}
	return *this;
}


// ======================================================================================================
void		CMatrix::identity()
{
	StateBit= MAT_IDENTITY;
	// For optimisation it would be usefull to keep MAT_VALID states.
	// But this slows identity(), and this may not be interesting...
}
// ======================================================================================================
void		CMatrix::setRot(const CVector &i, const CVector &j, const CVector &k, bool hintNoScale)
{
	StateBit|= MAT_ROT | MAT_SCALEANY;
	if(hintNoScale)
		StateBit&= ~(MAT_SCALEANY|MAT_SCALEUNI);
	a11= i.x; a12= j.x; a13= k.x; 
	a21= i.y; a22= j.y; a23= k.y; 
	a31= i.z; a32= j.z; a33= k.z; 
	Scale33= 1.0f;
}
// ======================================================================================================
void		CMatrix::setRot(const float m33[9], bool hintNoScale)
{
	StateBit|= MAT_ROT | MAT_SCALEANY;
	if(hintNoScale)
		StateBit&= ~(MAT_SCALEANY|MAT_SCALEUNI);
	a11= m33[0]; a12= m33[3]; a13= m33[6]; 
	a21= m33[1]; a22= m33[4]; a23= m33[7]; 
	a31= m33[2]; a32= m33[5]; a33= m33[8]; 
	Scale33= 1.0f;
}
// ======================================================================================================
void		CMatrix::setRot(const CVector &v, TRotOrder ro)
{
	CMatrix		rot;
	rot.identity();
	rot.rotate(v, ro);
	float	m33[9];
	rot.getRot(m33);
	setRot(m33, true);
}

// ======================================================================================================
void		CMatrix::setPos(const CVector &v)
{
	a14= v.x;
	a24= v.y;
	a34= v.z;
	if(a14!=0 || a24!=0 || a34!=0)
		StateBit|= MAT_TRANS;
	else
	{
		// The trans is identity, and is correcly setup!
		StateBit&= ~MAT_TRANS;
		StateBit|= MAT_VALIDTRANS;
	}
}
// ======================================================================================================
void		CMatrix::movePos(const CVector &v)
{
	a14+= v.x;
	a24+= v.y;
	a34+= v.z;
	if(a14!=0 || a24!=0 || a34!=0)
		StateBit|= MAT_TRANS;
	else
	{
		// The trans is identity, and is correcly setup!
		StateBit&= ~MAT_TRANS;
		StateBit|= MAT_VALIDTRANS;
	}
}
// ======================================================================================================
void		CMatrix::setProj(const float proj[4])
{
	a41= proj[0];
	a42= proj[1];
	a43= proj[2];
	a44= proj[3];

	// Check Proj state.
	if(a41!=0 || a42!=0 || a43!=0 || a44!=1)
		StateBit|= MAT_PROJ;
	else
	{
		// The proj is identity, and is correcly setup!
		StateBit&= ~MAT_PROJ;
		StateBit|= MAT_VALIDPROJ;
	}
}
// ======================================================================================================
void		CMatrix::resetProj()
{
	a41= 0;
	a42= 0;
	a43= 0;
	a44= 1;
	// The proj is identity, and is correcly setup!
	StateBit&= ~MAT_PROJ;
	StateBit|= MAT_VALIDPROJ;
}
// ======================================================================================================
void		CMatrix::set(const float m44[16])
{
	StateBit= MAT_IDENTITY;

	StateBit|= MAT_ROT | MAT_SCALEANY;
	memcpy(M, m44, 16*sizeof(float));
	Scale33= 1.0f;

	// Check Trans state.
	if(a14!=0 || a24!=0 || a34!=0)
		StateBit|= MAT_TRANS;
	else
	{
		// The trans is identity, and is correcly setup!
		StateBit&= ~MAT_TRANS;
		StateBit|= MAT_VALIDTRANS;
	}

	// Check Proj state.
	if(a41!=0 || a42!=0 || a43!=0 || a44!=1)
		StateBit|= MAT_PROJ;
	else
	{
		// The proj is identity, and is correcly setup!
		StateBit&= ~MAT_PROJ;
		StateBit|= MAT_VALIDPROJ;
	}
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
void		CMatrix::getRot(CVector &i, CVector &j, CVector &k) const
{
	if(hasRot())
	{
		i.set(a11, a21, a31);
		j.set(a12, a22, a32);
		k.set(a13, a23, a33);
	}
	else
	{
		i.set(1, 0, 0);
		j.set(0, 1, 0);
		k.set(0, 0, 1);
	}
}
// ======================================================================================================
void		CMatrix::getRot(float m33[9]) const
{
	if(hasRot())
	{
		m33[0]= a11;
		m33[1]= a21;
		m33[2]= a31;

		m33[3]= a12;
		m33[4]= a22;
		m33[5]= a32;

		m33[6]= a13;
		m33[7]= a23;
		m33[8]= a33;
	}
	else
	{
		m33[0]= 1;
		m33[1]= 0;
		m33[2]= 0;

		m33[3]= 0;
		m33[4]= 1;
		m33[5]= 0;

		m33[6]= 0;
		m33[7]= 0;
		m33[8]= 1;
	}
}
// ======================================================================================================
void		CMatrix::getPos(CVector &v) const
{
	if(hasTrans())
		v.set(a14, a24, a34);
	else
		v.set(0, 0, 0);
}
// ======================================================================================================
CVector		CMatrix::getPos() const
{
	if(hasTrans())
		return CVector(a14, a24, a34);
	else
		return CVector(0, 0, 0);
}
// ======================================================================================================
void		CMatrix::getProj(float proj[4]) const
{
	if(hasProj())
	{
		proj[0]= a41;
		proj[1]= a42;
		proj[2]= a43;
		proj[3]= a44;
	}
	else
	{
		proj[0]= 0;
		proj[1]= 0;
		proj[2]= 0;
		proj[3]= 1;
	}
}
// ======================================================================================================
CVector		CMatrix::getI() const
{
	if(hasRot())
		return CVector(a11, a21, a31);
	else
		return CVector(1, 0, 0);
}
// ======================================================================================================
CVector		CMatrix::getJ() const
{
	if(hasRot())
		return CVector(a12, a22, a32);
	else
		return CVector(0, 1, 0);
}
// ======================================================================================================
CVector		CMatrix::getK() const
{
	if(hasRot())
		return CVector(a13, a23, a33);
	else
		return CVector(0, 0, 1);
}
// ======================================================================================================
void		CMatrix::get(float m44[16]) const
{
	// TODO_OPTIMIZE_it.
	testExpandRot();
	testExpandTrans();
	testExpandProj();
	memcpy(m44, M, 16*sizeof(float));
}
// ======================================================================================================
const float *CMatrix::get() const
{
	testExpandRot();
	testExpandTrans();
	testExpandProj();
	return M;
}
/*// ======================================================================================================
CVector		CMatrix::toEuler(TRotOrder ro) const
{

}*/


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
void		CMatrix::translate(const CVector &v)
{
	testExpandTrans();

	// SetTrans.
	if( hasRot() )
	{
		a14+= a11*v.x + a12*v.y + a13*v.z;
		a24+= a21*v.x + a22*v.y + a23*v.z;
		a34+= a31*v.x + a32*v.y + a33*v.z;
	}
	else
	{
		a14+= v.x;
		a24+= v.y;
		a34+= v.z;
	}

	// SetProj.
	if( hasProj() )
		a44+= a41*v.x + a42*v.y + a43*v.z;

	// Check Trans.
	if(a14!=0 || a24!=0 || a34!=0)
		StateBit|= MAT_TRANS;
	else
	{
		// The trans is identity, and is correcly setup!
		StateBit&= ~MAT_TRANS;
		StateBit|= MAT_VALIDTRANS;
	}
}
// ======================================================================================================
void		CMatrix::rotateX(float a)
{

	if(a==0)
		return;
	double	ca,sa;
	ca=cos(a);
	sa=sin(a);

	// SetRot.
	if( hasRot() )
	{
		float	b12=a12, b22=a22, b32=a32;
		float	b13=a13, b23=a23, b33=a33;
		a12= (float)(b12*ca + b13*sa);
		a22= (float)(b22*ca + b23*sa);
		a32= (float)(b32*ca + b33*sa);
		a13= (float)(b13*ca - b12*sa);
		a23= (float)(b23*ca - b22*sa);
		a33= (float)(b33*ca - b32*sa);
	}
	else
	{
		testExpandRot();
		a12= 0.0f; a22= (float)ca; a32= (float)sa;
		a13= 0.0f; a23= (float)-sa; a33= (float)ca;
	}

	// SetProj.
	if( hasProj() )
	{
		float	b42=a42, b43=a43;
		a42= (float)(b42*ca + b43*sa);
		a43= (float)(b43*ca - b42*sa);
	}

	// set Rot.
	StateBit|= MAT_ROT;
}
// ======================================================================================================
void		CMatrix::rotateY(float a)
{

	if(a==0)
		return;
	double	ca,sa;
	ca=cos(a);
	sa=sin(a);

	// SetRot.
	if( hasRot() )
	{
		float	b11=a11, b21=a21, b31=a31;
		float	b13=a13, b23=a23, b33=a33;
		a11= (float)(b11*ca - b13*sa);
		a21= (float)(b21*ca - b23*sa);
		a31= (float)(b31*ca - b33*sa);
		a13= (float)(b13*ca + b11*sa);
		a23= (float)(b23*ca + b21*sa);
		a33= (float)(b33*ca + b31*sa);
	}
	else
	{
		testExpandRot();
		a11= (float)ca; a21=0.0f; a31= (float)-sa;
		a13= (float)sa; a23=0.0f; a33= (float)ca;
	}

	// SetProj.
	if( hasProj() )
	{
		float	b41=a41, b43=a43;
		a41= (float)(b41*ca - b43*sa);
		a43= (float)(b43*ca + b41*sa);
	}

	// set Rot.
	StateBit|= MAT_ROT;
}
// ======================================================================================================
void		CMatrix::rotateZ(float a)
{

	if(a==0)
		return;
	double	ca,sa;
	ca=cos(a);
	sa=sin(a);

	// SetRot.
	if( StateBit & (MAT_ROT|MAT_SCALEUNI|MAT_SCALEANY) )
	{
		float	b11=a11, b21=a21, b31=a31;
		float	b12=a12, b22=a22, b32=a32;
		a11= (float)(b11*ca + b12*sa);
		a21= (float)(b21*ca + b22*sa);
		a31= (float)(b31*ca + b32*sa);
		a12= (float)(b12*ca - b11*sa);
		a22= (float)(b22*ca - b21*sa);
		a32= (float)(b32*ca - b31*sa);
	}
	else
	{
		testExpandRot();
		a11= (float)ca; a21= (float)sa; a31=0.0f;
		a12= (float)-sa; a22= (float)ca; a32=0.0f;
	}

	// SetProj.
	if( hasProj() )
	{
		float	b41=a41, b42=a42;
		a41= (float)(b41*ca + b42*sa);
		a42= (float)(b42*ca - b41*sa);
	}

	// set Rot.
	StateBit|= MAT_ROT;
}
// ======================================================================================================
void		CMatrix::rotate(const CVector &v, TRotOrder ro)
{
	CMatrix		rot;
	rot.identity();
	switch(ro)
	{
		case XYZ: rot.rotateX(v.x); rot.rotateY(v.y); rot.rotateZ(v.z); break;
		case XZY: rot.rotateX(v.x); rot.rotateZ(v.z); rot.rotateY(v.y); break;
		case YXZ: rot.rotateY(v.y); rot.rotateX(v.x); rot.rotateZ(v.z); break;
		case YZX: rot.rotateY(v.y); rot.rotateZ(v.z); rot.rotateX(v.x); break;
		case ZXY: rot.rotateZ(v.z); rot.rotateX(v.x); rot.rotateY(v.y); break;
		case ZYX: rot.rotateZ(v.z); rot.rotateY(v.y); rot.rotateX(v.x); break;
	}

	(*this)*= rot;
}
// ======================================================================================================
void		CMatrix::scale(float f)
{

	if(f==1.0f) return;
	if(StateBit & MAT_SCALEANY)
	{
		scale(CVector(f,f,f));
	}
	else
	{
		testExpandRot();
		StateBit|= MAT_SCALEUNI;
		Scale33*=f;
		a11*= f; a12*=f; a13*=f;
		a21*= f; a22*=f; a23*=f;
		a31*= f; a32*=f; a33*=f;

		// SetProj.
		if( hasProj() )
		{
			a41*=f; a42*=f; a43*=f;
		}
	}
}
// ======================================================================================================
void		CMatrix::scale(const CVector &v)
{

	if( v==CVector(1,1,1) ) return;
	if( !(StateBit & MAT_SCALEANY) && v.x==v.y && v.x==v.z)
	{
		scale(v.x);
	}
	else
	{
		testExpandRot();
		StateBit|=MAT_SCALEANY;
		a11*= v.x; a12*=v.y; a13*=v.z;
		a21*= v.x; a22*=v.y; a23*=v.z;
		a31*= v.x; a32*=v.y; a33*=v.z;

		// SetProj.
		if( hasProj() )
		{
			a41*=v.x;
			a42*=v.y;
			a43*=v.z;
		}
	}
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
CMatrix		CMatrix::operator*(const CMatrix &m) const
{
	CMatrix		ret;
	// Do ret= M1*M2, where M1=*this and M2=m.
	ret.identity();
	ret.StateBit= StateBit | m.StateBit;
	ret.StateBit&= ~MAT_VALIDALL;

	// TODO_OPTIMIZE it...
	testExpandRot();
	testExpandTrans();
	testExpandProj();
	m.testExpandRot();
	m.testExpandTrans();
	m.testExpandProj();
	ret.testExpandRot();
	ret.testExpandTrans();
	ret.testExpandProj();


	// Build Rot part.
	//===============
	bool	M1Identity= ! hasRot();
	bool	M2Identity= ! (m.hasRot());
	bool	M1ScaleOnly= ! (StateBit & MAT_ROT);
	bool	M2ScaleOnly= ! (m.StateBit & MAT_ROT);

	// If one of the 3x3 matrix is an identity, just do a copy
	if( M1Identity || M2Identity )
	{
		// Copy the non identity matrix.
		const CMatrix	*c= M2Identity? this : &m;
		ret.a11= c->a11; ret.a12= c->a12; ret.a13= c->a13;
		ret.a21= c->a21; ret.a22= c->a22; ret.a23= c->a23;
		ret.a31= c->a31; ret.a32= c->a32; ret.a33= c->a33;
	}
	// If two 3x3 matrix are just scaleOnly matrix, do a scaleFact.
	else if( M1ScaleOnly && M2ScaleOnly )
	{
		// same process for scaleUni or scaleAny.
		ret.a11= a11*m.a11; ret.a12= 0; ret.a13= 0; 
		ret.a21= 0; ret.a22= a22*m.a22; ret.a23= 0; 
		ret.a31= 0; ret.a32= 0; ret.a33= a33*m.a33;
	}
	// If one of the matrix is a scaleOnly matrix, do a scale*Rot.
	else if( M1ScaleOnly && !M2ScaleOnly )
	{
		ret.a11= a11*m.a11; ret.a12= a11*m.a12; ret.a13= a11*m.a13;
		ret.a21= a22*m.a21; ret.a22= a22*m.a22; ret.a23= a22*m.a23;
		ret.a31= a33*m.a31; ret.a32= a33*m.a32; ret.a33= a33*m.a33;
	}
	else if( !M1ScaleOnly && M2ScaleOnly )
	{
		ret.a11= a11*m.a11; ret.a12= a12*m.a22; ret.a13= a13*m.a33;
		ret.a21= a21*m.a11; ret.a22= a22*m.a22; ret.a23= a23*m.a33;
		ret.a31= a31*m.a11; ret.a32= a32*m.a22; ret.a33= a33*m.a33;
	}
	// Else, general case: blending of two rotations.
	else
	{
		ret.a11= a11*m.a11 + a12*m.a21 + a13*m.a31;
		ret.a12= a11*m.a12 + a12*m.a22 + a13*m.a32;
		ret.a13= a11*m.a13 + a12*m.a23 + a13*m.a33;

		ret.a21= a21*m.a11 + a22*m.a21 + a23*m.a31;
		ret.a22= a21*m.a12 + a22*m.a22 + a23*m.a32;
		ret.a23= a21*m.a13 + a22*m.a23 + a23*m.a33;

		ret.a31= a31*m.a11 + a32*m.a21 + a33*m.a31;
		ret.a32= a31*m.a12 + a32*m.a22 + a33*m.a32;
		ret.a33= a31*m.a13 + a32*m.a23 + a33*m.a33;
	}

	// If M1 has translate and M2 has projective, rotation is modified.
	if( hasTrans() && m.hasProj())
	{
		ret.StateBit|= MAT_ROT|MAT_SCALEANY;

		ret.a11+= a14*m.a41;
		ret.a12+= a14*m.a42;
		ret.a13+= a14*m.a43;

		ret.a21+= a24*m.a41;
		ret.a22+= a24*m.a42;
		ret.a23+= a24*m.a43;

		ret.a31+= a34*m.a41;
		ret.a32+= a34*m.a42;
		ret.a33+= a34*m.a43;
	}
	// Modify Scale.
	if( (ret.StateBit & MAT_SCALEUNI) && !(ret.StateBit & MAT_SCALEANY) )
		ret.Scale33= Scale33*m.Scale33;
	else
		ret.Scale33=1;


	// Build Trans part.
	//=================
	if( ret.StateBit & MAT_TRANS )
	{
		// Compose M2 part.
		if( M1Identity )
		{
			ret.a14= m.a14;
			ret.a24= m.a24;
			ret.a34= m.a34;
		}
		else if (M1ScaleOnly )
		{
			ret.a14= a11*m.a14;
			ret.a24= a22*m.a24;
			ret.a34= a33*m.a34;
		}
		else
		{
			ret.a14= a11*m.a14 + a12*m.a24 + a13*m.a34;
			ret.a24= a21*m.a14 + a22*m.a24 + a23*m.a34;
			ret.a34= a31*m.a14 + a32*m.a24 + a33*m.a34;
		}
		// Compose M1 part.
		if(StateBit & MAT_TRANS)
		{
			if(m.StateBit & MAT_PROJ)
			{
				ret.a14+= a14*m.a44;
				ret.a24+= a24*m.a44;
				ret.a34+= a34*m.a44;
			}
			else
			{
				ret.a14+= a14;
				ret.a24+= a24;
				ret.a34+= a34;
			}
		}
	}
	else
	{
		ret.a14= 0;
		ret.a24= 0;
		ret.a34= 0;
		ret.StateBit|= MAT_VALIDTRANS;
	}


	// Build Proj part.
	//=================
	if( ret.StateBit & MAT_PROJ )
	{
		// optimise nothing... (projection matrix are rare).
		ret.a41= a41*m.a11 + a42*m.a21 + a43*m.a31 + a44*m.a41;
		ret.a42= a41*m.a12 + a42*m.a22 + a43*m.a32 + a44*m.a42;
		ret.a43= a41*m.a13 + a42*m.a23 + a43*m.a33 + a44*m.a43;
		ret.a44= a41*m.a14 + a42*m.a24 + a43*m.a34 + a44*m.a44;
	}
	else
	{
		ret.a41= 0;
		ret.a42= 0;
		ret.a43= 0;
		ret.a44= 1;
	}


	return ret;
}
// ======================================================================================================
CMatrix		&CMatrix::operator*=(const CMatrix &m)
{

	*this= *this*m;

	return *this;
}
// ======================================================================================================
void		CMatrix::invert()
{

	*this= inverted();
}

// ======================================================================================================
void	CMatrix::fastInvert33(CMatrix &ret) const
{
	// Fast invert of 3x3 rot matrix.
	// Work if no scale and if MAT_SCALEUNI. doesn't work if MAT_SCALEANY.

	if(StateBit & MAT_SCALEUNI)
	{
		double	s,S;	// important for precision.
		// Must divide the matrix by 1/Scale 2 times, to set unit, and to have a Scale=1/Scale.
		S=1.0/Scale33;
		ret.Scale33= (float)S;
		s=S*S;
		// The matrix is a base, so just transpose it.
		ret.a11= (float)(a11*s); ret.a12= (float)(a21*s); ret.a13= (float)(a31*s);
		ret.a21= (float)(a12*s); ret.a22= (float)(a22*s); ret.a23= (float)(a32*s);
		ret.a31= (float)(a13*s); ret.a32= (float)(a23*s); ret.a33= (float)(a33*s);
	}
	else
	{
		ret.Scale33=1;
		// The matrix is a base, so just transpose it.
		ret.a11= a11; ret.a12= a21; ret.a13=a31;
		ret.a21= a12; ret.a22= a22; ret.a23=a32;
		ret.a31= a13; ret.a32= a23; ret.a33=a33;
	}

	// 15 cycles if no scale.
	// 35 cycles if scale.
}
// ======================================================================================================
bool	CMatrix::slowInvert33(CMatrix &ret) const
{
	CVector	invi,invj,invk;
	CVector	i,j,k;
	double	s;

	i= getI();
	j= getJ();
	k= getK();
	// Compute cofactors (minors *(-1)^(i+j)).
	invi.x= j.y*k.z - k.y*j.z;
	invi.y= j.z*k.x - k.z*j.x;
	invi.z= j.x*k.y - k.x*j.y;
	invj.x= k.y*i.z - i.y*k.z;
	invj.y= k.z*i.x - i.z*k.x;
	invj.z= k.x*i.y - i.x*k.y;
	invk.x= i.y*j.z - j.y*i.z;
	invk.y= i.z*j.x - j.z*i.x;
	invk.z= i.x*j.y - j.x*i.y;
	// compute determinant.
	s= invi.x*i.x + invj.x*j.x + invk.x*k.x;
	if(s==0)
		return false;
	// Transpose the Comatrice, and divide by determinant.
	s=1.0/s;
	ret.a11= (float)(invi.x*s); ret.a12= (float)(invi.y*s); ret.a13= (float)(invi.z*s);
	ret.a21= (float)(invj.x*s); ret.a22= (float)(invj.y*s); ret.a23= (float)(invj.z*s);
	ret.a31= (float)(invk.x*s); ret.a32= (float)(invk.y*s); ret.a33= (float)(invk.z*s);

	return true;
	// Roundly 82 cycles. (1Div=10 cycles).
}
// ======================================================================================================
bool	CMatrix::slowInvert44(CMatrix &ret) const
{
	sint	i,j;
	double	s;

	// Compute Cofactors
	//==================
	for(i=0;i<=3;i++)
	{
		for(j=0;j<=3;j++)
		{
			sint	l1,l2,l3;
			sint	c1,c2,c3;
			getCofactIndex(i,l1,l2,l3);
			getCofactIndex(j,c1,c2,c3);

			ret.mat(i,j)= 0;
			ret.mat(i,j)+= mat(l1,c1) * mat(l2,c2) * mat(l3,c3);
			ret.mat(i,j)+= mat(l1,c2) * mat(l2,c3) * mat(l3,c1);
			ret.mat(i,j)+= mat(l1,c3) * mat(l2,c1) * mat(l3,c2);

			ret.mat(i,j)-= mat(l1,c1) * mat(l2,c3) * mat(l3,c2);
			ret.mat(i,j)-= mat(l1,c2) * mat(l2,c1) * mat(l3,c3);
			ret.mat(i,j)-= mat(l1,c3) * mat(l2,c2) * mat(l3,c1);

			if( (i+j)&1 )
				ret.mat(i,j)=-ret.mat(i,j);
		}
	}

	// Compute determinant.
	//=====================
	s= ret.mat(0,0) * mat(0,0) + ret.mat(0,1) * mat(0,1) + ret.mat(0,2) * mat(0,2) + ret.mat(0,3) * mat(0,3);
	if(s==0)
		return false;

	// Divide by determinant.
	//=======================
	s=1.0/s;
	for(i=0;i<=3;i++)
	{
		for(j=0;j<=3;j++)
			ret.mat(i,j)= (float)(ret.mat(i,j)*s);
	}

	// Transpose the comatrice.
	//=========================
	for(i=0;i<=3;i++)
	{
		for(j=i+1;j<=3;j++)
		{
			swap(ret.mat(i,j), ret.mat(j,i));
		}
	}

	return true;
}
// ======================================================================================================
CMatrix		CMatrix::inverted() const
{

	CMatrix	ret;

	// TODO_OPTIMIZE it...
	testExpandRot();
	testExpandTrans();
	testExpandProj();

	// Do a conventionnal 44 inversion.
	//=================================
	if(StateBit & MAT_PROJ)
	{
		if(!slowInvert44(ret))
		{
			ret.identity();
			return ret;
		}

		// Well, don't know what happens to matrix, so set all StateBit :).
		ret.StateBit= MAT_TRANS|MAT_ROT|MAT_SCALEANY|MAT_PROJ;

		// Check Trans state.
		if(ret.a14!=0 || ret.a24!=0 || ret.a34!=0)
			ret.StateBit|= MAT_TRANS;
		else
			ret.StateBit&= ~MAT_TRANS;

		// Check Proj state.
		if(ret.a41!=0 || ret.a42!=0 || ret.a43!=0 || ret.a44!=1)
			ret.StateBit|= MAT_PROJ;
		else
			ret.StateBit&= ~MAT_PROJ;
	}

	// Do a speed 34 inversion.
	//=========================
	else
	{
		// Invert the rotation part.
		if(StateBit & MAT_SCALEANY)
		{
			if(!slowInvert33(ret))
			{
				ret.identity();
				return ret;
			}
		}
		else
			fastInvert33(ret);
		// Scale33 is updated in fastInvert33().

		// Invert the translation part.
		if(StateBit & MAT_TRANS)
		{
			// Invert the translation part.
			// This can only work if 4th line is 0 0 0 1.
			// formula: InvVp= InvVi*(-Vp.x) + InvVj*(-Vp.y) + InvVk*(-Vp.z)
			ret.a14= ret.a11*(-a14) + ret.a12*(-a24) + ret.a13*(-a34);
			ret.a24= ret.a21*(-a14) + ret.a22*(-a24) + ret.a23*(-a34);
			ret.a34= ret.a31*(-a14) + ret.a32*(-a24) + ret.a33*(-a34);
		}
		else
		{
			ret.a14= 0;
			ret.a24= 0;
			ret.a34= 0;
		}

		// The projection part is unmodified.
		ret.a41= 0; ret.a42= 0; ret.a43= 0; ret.a44= 1;

		// The matrix inverted keep the same state bits.
		ret.StateBit= StateBit;
	}
	
	
	return ret;
}
// ======================================================================================================
bool		CMatrix::normalize(TRotOrder ro)
{

	CVector	ti,tj,tk;
	ti= getI();
	tj= getJ();
	tk= getK();

	// TODO_OPTIMIZE it...
	testExpandRot();

	// Normalize with help of ro
	switch(ro)
	{
		case XYZ:
			ti.normalize();
			tk= ti^tj;
			tk.normalize();
			tj= tk^ti;
			break;
		case XZY:
			ti.normalize();
			tj= tk^ti;
			tj.normalize();
			tk= ti^tj;
			break;
		case YXZ:
			tj.normalize();
			tk= ti^tj;
			tk.normalize();
			ti= tj^tk;
			break;
		case YZX:
			tj.normalize();
			ti= tj^tk;
			ti.normalize();
			tk= ti^tj;
			break;
		case ZXY:
			tk.normalize();
			tj= tk^ti;
			tj.normalize();
			ti= tj^tk;
			break;
		case ZYX:
			tk.normalize();
			ti= tj^tk;
			ti.normalize();
			tj= tk^ti;
			break;
	}

	// Check, and set result.
	if( ti.isNull() || tj.isNull() || tk.isNull() )
		return false;
	a11= ti.x; a12= tj.x; a13= tk.x; 
	a21= ti.y; a22= tj.y; a23= tk.y; 
	a31= ti.z; a32= tj.z; a33= tk.z; 
	// Scale is reseted.
	StateBit&= ~(MAT_SCALEUNI|MAT_SCALEANY);
	// Rot is setup...
	StateBit|= MAT_ROT;
	Scale33=1;

	return true;
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
CVector		CMatrix::mulVector(const CVector &v) const
{

	CVector	ret;

	if( hasRot() )
	{
		ret.x= a11*v.x + a12*v.y + a13*v.z;
		ret.y= a21*v.x + a22*v.y + a23*v.z;
		ret.z= a31*v.x + a32*v.y + a33*v.z;
		return ret;
	}
	else
		return v;
}

// ======================================================================================================
CVector		CMatrix::mulPoint(const CVector &v) const
{

	CVector	ret;

	if( hasRot() )
	{
		ret.x= a11*v.x + a12*v.y + a13*v.z;
		ret.y= a21*v.x + a22*v.y + a23*v.z;
		ret.z= a31*v.x + a32*v.y + a33*v.z;
	}
	else
	{
		ret= v;
	}
	if( hasTrans() )
	{
		ret.x+= a14;
		ret.y+= a24;
		ret.z+= a34;
	}

	return ret;
}


/*
 * Multiply
 */
CVectorH	CMatrix::operator*(const CVectorH& v) const
{

	CVectorH ret;

	// TODO_OPTIMIZE it...
	testExpandRot();
	testExpandTrans();
	testExpandProj();

	ret.x= a11*v.x + a12*v.y + a13*v.z + a14*v.w;
	ret.y= a21*v.x + a22*v.y + a23*v.z + a24*v.w;
	ret.z= a31*v.x + a32*v.y + a33*v.z + a34*v.w;
	ret.w= a41*v.x + a42*v.y + a43*v.z + a44*v.w;
	return ret;
}


// ======================================================================================================
CPlane		operator*(const CPlane &p, const CMatrix &m)
{
	// TODO_OPTIMIZE it...
	m.testExpandRot();
	m.testExpandTrans();
	m.testExpandProj();


	CPlane	ret;

	if( m.StateBit & (MAT_ROT|MAT_SCALEUNI|MAT_SCALEANY|MAT_PROJ) )
	{
		// Compose with translation too.
		ret.a= p.a*m.a11 + p.b*m.a21 + p.c*m.a31 + p.d*m.a41;
		ret.b= p.a*m.a12 + p.b*m.a22 + p.c*m.a32 + p.d*m.a42;
		ret.c= p.a*m.a13 + p.b*m.a23 + p.c*m.a33 + p.d*m.a43;
		ret.d= p.a*m.a14 + p.b*m.a24 + p.c*m.a34 + p.d*m.a44;
		return ret;
	}
	else if( m.StateBit & MAT_TRANS )
	{

		// Compose just with a translation.
		ret.a= p.a;
		ret.b= p.b;
		ret.c= p.c;
		ret.d= p.a*m.a14 + p.b*m.a24 + p.c*m.a34 + p.d*m.a44;
		return ret;
	}
	else	// Identity!!
		return p;
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
void		CMatrix::serial(IStream &f)
{
	// Use versionning, maybe for futur improvement.
	sint	ver= f.serialVersion(0);

	if(f.isReading())
		identity();
	f.serial(StateBit);
	f.serial(Scale33);
	if( hasRot() )
	{
		f.serial(a11, a12, a13);
		f.serial(a21, a22, a23);
		f.serial(a31, a32, a33);
	}
	if( hasTrans() )
	{
		f.serial(a14, a24, a34);
	}
	if( hasProj() )
	{
		f.serial(a41, a42, a43, a44);
	}
}




}

