/** \file driver_direct3d_vertex_program.cpp
 * Direct 3d driver implementation
 *
 * $Id: driver_direct3d_vertex_program.cpp,v 1.1 2004/03/19 10:11:36 corvazier Exp $
 *
 * \todo manage better the init/release system (if a throw occurs in the init, we must release correctly the driver)
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

#include "stddirect3d.h"

#include "driver_direct3d.h"

using namespace std;
using namespace NLMISC;

namespace NL3D 
{

// ***************************************************************************

CVertexProgamDrvInfosD3D::CVertexProgamDrvInfosD3D(IDriver *drv, ItVtxPrgDrvInfoPtrList it) : IVertexProgramDrvInfos (drv, it)
{
	Shader = NULL;
}

// ***************************************************************************

CVertexProgamDrvInfosD3D::~CVertexProgamDrvInfosD3D()
{
	if (Shader)
		Shader->Release();
}

// ***************************************************************************

bool CDriverD3D::isVertexProgramSupported () const
{
	return _VertexProgram;
}

// ***************************************************************************

bool CDriverD3D::isVertexProgramEmulated () const
{
	// Pure HAL driver, no emulation available
	return false;
}

// ***************************************************************************

static const char *instrToName[] =
{
	"mov  ",
	"mov  ",
	"mul  ",
	"add  ",
	"mad  ",
	"rsq  ",
	"dp3  ",
	"dp4  ",
	"dst  ",
	"lit  ",
	"min  ",
	"max  ",
	"slt  ",
	"sge  ",
	"expp ",
	"log  ",
	"rcp  "
};

// ***************************************************************************

static const char *outputRegisterToName[] =
{
	"Pos",
	"D0",
	"D1",
	"BFC0",
	"BFC1",
	"Fog",
	"Pts",
	"T0",
	"T1",
	"T2",
	"T3",
	"T4",
	"T5",
	"T6",
	"T7"
};

// ***************************************************************************

void dumpWriteMask(uint mask, std::string &out)
{
	if (mask == 0xf)
	{
		out = "";
		return;
	}
	out = ".";
	if (mask & 1) out +="x";
	if (mask & 2) out +="y";
	if (mask & 4) out +="z";
	if (mask & 8) out +="w";
}

// ***************************************************************************

void dumpSwizzle(const CVPSwizzle &swz, std::string &out)
{
	if (swz.isIdentity())
	{
		out = "";
		return;
	}
	out = ".";
	for(uint k = 0; k < 4; ++k)
	{
		switch(swz.Comp[k])
		{
			case CVPSwizzle::X: out += "x"; break;
			case CVPSwizzle::Y: out += "y"; break;
			case CVPSwizzle::Z: out += "z"; break;
			case CVPSwizzle::W: out += "w"; break;
			default:
				nlassert(0);
			break;
		}
		if (swz.isScalar() && k == 0) break;
	}

}

// ***************************************************************************

void dumpOperand(const CVPOperand &op, bool destOperand, std::string &out, set<uint> &inputs)
{
	out = op.Negate ? " -" : " ";
	switch(op.Type)
	{
		case CVPOperand::Variable: out += "r" + NLMISC::toString(op.Value.VariableValue); break;
		case CVPOperand::Constant: 
			out += "c[";
			if (op.Indexed)
			{
				out += "a0.x + ";
			}
			out += NLMISC::toString(op.Value.ConstantValue) + "]"; 
		break;
		case CVPOperand::InputRegister: 
			out += "v" + NLMISC::toString((uint) op.Value.InputRegisterValue); 
			inputs.insert (op.Value.InputRegisterValue);
		break;
		case CVPOperand::OutputRegister:
			nlassert(op.Value.OutputRegisterValue < CVPOperand::OutputRegisterCount);
			out += "o" + std::string(outputRegisterToName[op.Value.OutputRegisterValue]);
		break;
		case CVPOperand::AddressRegister:
			out += "a0.x";
		break;
	}
	std::string suffix;
	if (destOperand)
	{
		// No mask for the fog value
		if (op.Value.OutputRegisterValue != CVPOperand::OFogCoord)
			dumpWriteMask(op.WriteMask, suffix);
	}
	else
	{
		dumpSwizzle(op.Swizzle, suffix);
	}
	out += suffix;
}

// ***************************************************************************

void dumpInstr(const CVPInstruction &instr, std::string &out, set<uint> &inputs)
{
	nlassert(instr.Opcode < CVPInstruction::OpcodeCount);
	out = instrToName[instr.Opcode];
	uint nbOp = instr.getNumUsedSrc();
	std::string destOperand;
	dumpOperand(instr.Dest, true, destOperand, inputs);
	out += destOperand;
	for(uint k = 0; k < nbOp; ++k)
	{
		out += ", ";
		std::string srcOperand;
		dumpOperand(instr.getSrc(k), false, srcOperand, inputs);
		out += srcOperand;
	}
	out +="; \n";
}

// ***************************************************************************

static const char *inputToDecl[CVPOperand::InputRegisterCount] =
{
	"dcl_position v0",
	"dcl_blendweight v1",
	"dcl_normal v2",
	"dcl_color0 v3",
	"dcl_color1 v4",
	"dcl_fog v5",
	"dcl_blendindices v6",
	"",
	"dcl_texcoord0 v8",
	"dcl_texcoord1 v9",
	"dcl_texcoord2 v10",
	"dcl_texcoord3 v11",
	"dcl_texcoord4 v12",
	"dcl_texcoord5 v13",
	"dcl_texcoord6 v14",
	"dcl_texcoord7 v15",
};

// ***************************************************************************

void dump(const CVPParser::TProgram &prg, std::string &dest)
{	
	// Set of input registers used
	set<uint> inputs;

	string program;
	for(uint k = 0; k < prg.size(); ++k)
	{
		std::string instr;
		dumpInstr(prg[k], instr, inputs);
		program += instr;
	}

	// Write the header
	dest = "vs.1.1\n";
	set<uint>::iterator ite = inputs.begin();
	while (ite != inputs.end())
	{
		dest += inputToDecl[*ite] + string("\n");
		ite++;
	}
	dest += program;
}

// ***************************************************************************

bool CDriverD3D::activeVertexProgram (CVertexProgram *program)
{
	if (_DisableHardwareVertexProgram)
		return false;

	// Setup or unsetup ?
	if (program)
	{
		// Program setuped ?
		if (program->_DrvInfo==NULL)
		{
			_VtxPrgDrvInfos.push_front (NULL);
			ItVtxPrgDrvInfoPtrList itTex = _VtxPrgDrvInfos.begin();
			*itTex = new CVertexProgamDrvInfosD3D(this, itTex);

			// Create a driver info structure
			program->_DrvInfo = *itTex;

			/** Check with our parser if the program will works with other implemented extensions, too. (EXT_vertex_shader ..).
			  * There are some incompatibilities.
			  */
			CVPParser parser;
			CVPParser::TProgram parsedProgram;
			std::string errorOutput;
			bool result = parser.parse(program->getProgram().c_str(), parsedProgram, errorOutput);
			if (!result)
			{
				nlwarning("Unable to parse a vertex program :");
				nlwarning(errorOutput.c_str());
				#ifdef NL_DEBUG_D3D
					nlassert(0);
				#endif // NL_DEBUG_D3D
				return false;
			}

			// Dump the vertex program
			std::string dest;
			dump(parsedProgram, dest);
#ifdef NL_DEBUG_D3D
			nlinfo("Assemble Vertex Shader : ");
			int lineBegin = 0;
			int lineEnd;
			while ((lineEnd = dest.find('\n', lineBegin)) != string::npos)
			{
				nlinfo(dest.substr (lineBegin, lineEnd-lineBegin).c_str());
				lineBegin = lineEnd+1;
			}
			nlinfo(dest.substr (lineBegin, lineEnd-lineBegin).c_str());
#endif // NL_DEBUG_D3D

			LPD3DXBUFFER pShader;
			LPD3DXBUFFER pErrorMsgs;
			if (D3DXAssembleShader (dest.c_str(), dest.size(), NULL, NULL, 0, &pShader, &pErrorMsgs) == D3D_OK)
			{
				if (_DeviceInterface->CreateVertexShader((DWORD*)pShader->GetBufferPointer(), &(getVertexProgramD3D(*program)->Shader)) != D3D_OK)
					return false;
			}
			else
			{
				nlwarning ("Can't assemble vertex program:");
				nlwarning ((const char*)pErrorMsgs->GetBufferPointer());
				return false;
			}
		}
	}

	// Set the vertex program
	if (program)
	{
		CVertexProgamDrvInfosD3D *info = static_cast<CVertexProgamDrvInfosD3D *>((IVertexProgramDrvInfos*)program->_DrvInfo);
		setVertexProgram (info->Shader);

		/* D3DRS_FOGSTART and D3DRS_FOGEND must be set with [1, 0] else the fog doesn't work properly on VertexShader and non-VertexShader objects 
		(random fog flicking) with Geforce4 TI 4200 (drivers 53.03 and 45.23). The other cards seam to interpret the "oFog"'s values using D3DRS_FOGSTART, 
		D3DRS_FOGEND.
		 */
		float z = 0;
		float o = 1;
		setRenderState (D3DRS_FOGSTART, *((DWORD*) (&o)));
		setRenderState (D3DRS_FOGEND, *((DWORD*) (&z)));
	}
	else
	{
		setVertexProgram (NULL);

		// Set the old fog range
		setRenderState (D3DRS_FOGSTART, *((DWORD*) (&_FogStart)));
		setRenderState (D3DRS_FOGEND, *((DWORD*) (&_FogEnd)));
	}

	return true;
}

// ***************************************************************************

void CDriverD3D::setConstant (uint index, float f0, float f1, float f2, float f3)
{
	const float tabl[4] = {f0, f1, f2, f3};
	setVertexProgramConstant (index, tabl);
}

// ***************************************************************************

void CDriverD3D::setConstant (uint index, double d0, double d1, double d2, double d3)
{
	const float tabl[4] = {(float)d0, (float)d1, (float)d2, (float)d3};
	setVertexProgramConstant (index, tabl);
}

// ***************************************************************************

void CDriverD3D::setConstant (uint index, const NLMISC::CVector& value)
{
	const float tabl[4] = {value.x, value.y, value.z, 0};
	setVertexProgramConstant (index, tabl);
}

// ***************************************************************************

void CDriverD3D::setConstant (uint index, const NLMISC::CVectorD& value)
{
	const float tabl[4] = {(float)value.x, (float)value.y, (float)value.z, 0};
	setVertexProgramConstant (index, tabl);
}

// ***************************************************************************

void CDriverD3D::setConstant (uint index, uint num, const float *src)
{
	uint i;
	for (i=0; i<num; i++)
		setVertexProgramConstant (index+i, src+i*4);
}

// ***************************************************************************

void CDriverD3D::setConstant (uint index, uint num, const double *src)
{
	uint i;
	for (i=0; i<num; i++)
	{
		const float tabl[4] = {(float)src[0], (float)src[1], (float)src[2], (float)src[3]};
		setVertexProgramConstant (index+i, tabl);
		src += 4;
	}
}

// ***************************************************************************

void CDriverD3D::setConstantMatrix (uint index, IDriver::TMatrix matrix, IDriver::TTransform transform)
{
	D3DXMATRIX mat;
	D3DXMATRIX *matPtr;
	switch (matrix)
	{
		case IDriver::ModelView:
			matPtr = &_D3DModelView;
		break;
		case IDriver::Projection:
			matPtr = &(_MatrixCache[remapMatrixIndex (D3DTS_PROJECTION)].Matrix);
		break;
		case IDriver::ModelViewProjection:
			matPtr = &_D3DModelViewProjection;
		break;
	}

	if (transform != IDriver::Identity)
	{
		mat = *matPtr;
		matPtr = &mat;
		switch(transform)
		{
			case IDriver::Inverse:
				D3DXMatrixInverse (&mat, NULL, &mat);
			break;		
			case IDriver::Transpose:
				D3DXMatrixTranspose (&mat, &mat);
			break;
			case IDriver::InverseTranspose:
				D3DXMatrixInverse (&mat, NULL, &mat);
				D3DXMatrixTranspose (&mat, &mat);
			break;
		}
	}

	setConstant (index, matPtr->_11, matPtr->_21, matPtr->_31, matPtr->_41);
	setConstant (index+1, matPtr->_12, matPtr->_22, matPtr->_32, matPtr->_42);
	setConstant (index+2, matPtr->_13, matPtr->_23, matPtr->_33, matPtr->_43);
	setConstant (index+3, matPtr->_14, matPtr->_24, matPtr->_34, matPtr->_44);
}

// ***************************************************************************

void CDriverD3D::setConstantFog (uint index)
{
	/* "oFog" must always be between [1, 0] what ever you set in D3DRS_FOGSTART and D3DRS_FOGEND (1 for no fog, 0 for full fog).
	The Geforce4 TI 4200 (drivers 53.03 and 45.23) doesn't accept other values for "oFog". */
	const float delta = _FogEnd-_FogStart;
	setConstant (index, - _D3DModelView._13/delta, -_D3DModelView._23/delta, -_D3DModelView._33/delta, 1-(_D3DModelView._43-_FogStart)/delta);
}

// ***************************************************************************

void CDriverD3D::enableVertexProgramDoubleSidedColor(bool doubleSided)
{
}

// ***************************************************************************

bool CDriverD3D::supportVertexProgramDoubleSidedColor() const 
{
	// Not supported under D3D
	return false;
}

// ***************************************************************************

void CDriverD3D::disableHardwareVertexProgram()
{
	_DisableHardwareVertexProgram = true;
	_VertexProgram = false;
}

// ***************************************************************************

} // NL3D