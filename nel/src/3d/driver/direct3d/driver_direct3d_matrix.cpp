/** \file driver_direct3d_matrix.cpp
 * Direct 3d driver implementation
 *
 * $Id: driver_direct3d_matrix.cpp,v 1.1 2004/03/19 10:11:36 corvazier Exp $
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

#include "3d/vertex_buffer.h"
#include "3d/light.h"
#include "3d/index_buffer.h"
#include "nel/misc/rect.h"
#include "nel/misc/di_event_emitter.h"
#include "nel/misc/mouse_device.h"
#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "nel/3d/u_driver.h"

#include "driver_direct3d.h"

using namespace std;
using namespace NLMISC;

namespace NL3D 
{

// ***************************************************************************

void CDriverD3D::updateMatrices ()
{
	// Update view model matrix
	D3DXMatrixMultiply (&_D3DModelView, &(_MatrixCache[remapMatrixIndex (D3DTS_WORLD)].Matrix), &(_MatrixCache[remapMatrixIndex (D3DTS_VIEW)].Matrix));

	// Update view model projection matrix
	_D3DModelViewProjection = _D3DModelView;
	D3DXMatrixMultiply (&_D3DModelViewProjection, &_D3DModelViewProjection, &(_MatrixCache[remapMatrixIndex (D3DTS_PROJECTION)].Matrix));

	// Update the inverted view model matrix
	D3DXMatrixInverse (&_D3DInvModelView, NULL, &_D3DModelView);

	// Update the normalize state
	setRenderState (D3DRS_NORMALIZENORMALS, (_UserViewMtx.hasScalePart() || _UserModelMtx.hasScalePart() || _ForceNormalize)?TRUE:FALSE);
}

// ***************************************************************************

void CDriverD3D::updateProjectionMatrix ()
{
	float left = _FrustumLeft;
	float right = _FrustumRight;
	float top = _FrustumTop;
	float bottom = _FrustumBottom;

	if (_RenderTarget.Texture)
		swap (bottom, top);

	// Get the render target size
	uint32 clientWidth;
	uint32 clientHeight;
	getRenderTargetSize (clientWidth, clientHeight);

	// In D3D, the center of the first screen pixel is [0.0,0.0]. Is NeL it is [0.5,0.5]
	const float addW = (right-left)/(2*(_Viewport.getWidth() * (float)clientWidth));
	const float addH = (bottom-top)/(2*(_Viewport.getHeight() * (float)clientHeight));
	
	left += addW;
	right += addW;
	top += addH;
	bottom += addH;

	D3DXMATRIX projection;
	if (_FrustumPerspective)
	{
		D3DXMatrixPerspectiveOffCenterLH (&projection, left, right, bottom, top, _FrustumZNear, _FrustumZFar);
	}
	else
	{
		D3DXMatrixOrthoOffCenterLH (&projection, left, right, bottom, top, _FrustumZNear, _FrustumZFar);
	}
	setMatrix (D3DTS_PROJECTION, projection);

	// Backup znear and zfar for zbias setup
	_OODeltaZ = 1 / (_FrustumZFar - _FrustumZNear);

	updateMatrices ();
}

// ***************************************************************************

void CDriverD3D::setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective)
{
	_FrustumLeft = left;
	_FrustumRight = right;
	_FrustumTop = top;
	_FrustumBottom = bottom;
	_FrustumZNear = znear;
	_FrustumZFar = zfar;
	_FrustumPerspective = perspective;
	updateProjectionMatrix ();
}

// ***************************************************************************

void CDriverD3D::setupViewMatrix(const CMatrix& mtx)
{
	// Remeber the view matrix
	_UserViewMtx= mtx;
	_PZBCameraPos= CVector::Null;

	// Set the driver matrix
	D3DXMATRIX view;
	NL_D3D_MATRIX (view, mtx);

	// Pass to directx matrix basis
	swap (view._12, view._13);
	swap (view._22, view._23);
	swap (view._32, view._33);
	swap (view._42, view._43);

	setMatrix (D3DTS_VIEW, view);

	// Set the spacular matrix
	CMatrix specularTex;
	specularTex = mtx;
	specularTex.setPos(CVector(0.0f,0.0f,0.0f));
	specularTex.invert();
	NL_D3D_MATRIX (_D3DSpecularWorldTex, specularTex);

	updateMatrices ();
}

// ***************************************************************************

void CDriverD3D::setupViewMatrixEx(const CMatrix& mtx, const CVector &cameraPos)
{
	// Remeber the view matrix
	_UserViewMtx= mtx;
	_PZBCameraPos= cameraPos;

	// Set the driver matrix
	D3DXMATRIX view;
	NL_D3D_MATRIX (view, mtx);

	// Pass to directx matrix basis
	swap (view._12, view._13);
	swap (view._22, view._23);
	swap (view._32, view._33);
	swap (view._42, view._43);

	// Reset the viewMtx position.
	view._41 = 0;
	view._42 = 0;
	view._43 = 0;

	setMatrix (D3DTS_VIEW, view);

	// Set the spacular matrix
	CMatrix specularTex;
	specularTex = mtx;
	NL_D3D_MATRIX (_D3DSpecularWorldTex, specularTex);
	swap (_D3DSpecularWorldTex._12, _D3DSpecularWorldTex._13);
	swap (_D3DSpecularWorldTex._22, _D3DSpecularWorldTex._23);
	swap (_D3DSpecularWorldTex._32, _D3DSpecularWorldTex._33);
	swap (_D3DSpecularWorldTex._42, _D3DSpecularWorldTex._43);
	_D3DSpecularWorldTex._41 = 0;
	_D3DSpecularWorldTex._42 = 0;
	_D3DSpecularWorldTex._43 = 0;

	D3DXMatrixInverse ( &_D3DSpecularWorldTex, NULL, &_D3DSpecularWorldTex);
	swap (_D3DSpecularWorldTex._12, _D3DSpecularWorldTex._13);
	swap (_D3DSpecularWorldTex._22, _D3DSpecularWorldTex._23);
	swap (_D3DSpecularWorldTex._32, _D3DSpecularWorldTex._33);
	swap (_D3DSpecularWorldTex._42, _D3DSpecularWorldTex._43);

	updateMatrices ();
}

// ***************************************************************************

void CDriverD3D::setupModelMatrix(const CMatrix& mtx)
{
	// Remeber the model matrix
	_UserModelMtx= mtx;

	D3DXMATRIX world;
	NL_D3D_MATRIX (world, mtx);

	// Remove from position the camera position
	world._41 -= _PZBCameraPos.x;
	world._42 -= _PZBCameraPos.y;
	world._43 -= _PZBCameraPos.z;

	setMatrix (D3DTS_WORLD, world);

	updateMatrices ();
}

// ***************************************************************************

CMatrix CDriverD3D::getViewMatrix() const
{
	return _UserViewMtx;
}

// ***************************************************************************

void CDriverD3D::forceNormalize(bool normalize)
{
	_ForceNormalize = normalize;
	updateMatrices ();
}

// ***************************************************************************

bool CDriverD3D::isForceNormalize() const
{
	return _RenderStateCache[D3DRS_NORMALIZENORMALS].Value != FALSE;
}

// ***************************************************************************

void CDriverD3D::setupScissor (const class CScissor& scissor)
{
	nlassert (_DeviceInterface);
	
	// Get viewport
	float x= scissor.X;
	float width= scissor.Width;
	float height= scissor.Height;

	if(x==0 && x==0 && width==1 && height==1)
	{		
		setRenderState (D3DRS_SCISSORTESTENABLE, FALSE);
	}
	else
	{

		float y= scissor.Y;

		if (_HWnd)
		{
			// Get the render target size
			uint32 clientWidth;
			uint32 clientHeight;
			getRenderTargetSize (clientWidth, clientHeight);

			// Setup d3d scissor
			RECT rect;
			rect.left=(int)floor((float)clientWidth * x + 0.5f);
			clamp (rect.left, 0, (int)clientWidth);
			rect.top=(int)floor((float)clientHeight* y + 0.5f);
			clamp (rect.top, 0, (int)clientHeight);

			rect.right=(int)floor((float)clientWidth * (x+width) + 0.5f );
			clamp (rect.right, 0, (int)clientWidth);
			rect.bottom=(int)floor((float)clientHeight* (y+height) + 0.5f );
			clamp (rect.bottom, 0, (int)clientHeight);

			_DeviceInterface->SetScissorRect (&rect);
			setRenderState (D3DRS_SCISSORTESTENABLE, TRUE);
		}
	}

	// Backup the scissor
	_Scissor = scissor;
}

// ***************************************************************************

void CDriverD3D::setupViewport (const class CViewport& viewport)
{
	if (_HWnd == NULL) 
		return;

	// Get the render target size
	uint32 clientWidth;
	uint32 clientHeight;
	getRenderTargetSize (clientWidth, clientHeight);

	// Get viewport
	float x;
	float y;
	float width;
	float height;
	viewport.getValues (x, y, width, height);

	// Get integer values
	int ix=(int)((float)clientWidth*x);
	clamp (ix, 0, (int)clientWidth);
	int iy;
	if (_RenderTarget.Texture)
		iy=(int)((float)clientHeight*y);
	else
		iy=(int)((float)clientHeight*(1.f-(y+height)));
	clamp (iy, 0, (int)clientHeight);
	int iwidth=(int)((float)clientWidth*width);
	clamp (iwidth, 0, (int)clientWidth-ix);
	int iheight=(int)((float)clientHeight*height);
	clamp (iheight, 0, (int)clientHeight-iy);

	// Setup D3D viewport
	D3DVIEWPORT9 viewPort9;
	viewPort9.X = ix;
	viewPort9.Y = iy;
	viewPort9.Width = iwidth;
	viewPort9.Height = iheight;
	viewPort9.MinZ = 0;
	viewPort9.MaxZ = 1;
	_DeviceInterface->SetViewport (&viewPort9);

	// Backup the viewport
	_Viewport = viewport;

	updateProjectionMatrix ();
}

// ***************************************************************************

void CDriverD3D::getViewport(CViewport &viewport)
{
	viewport = _Viewport;
}

// ***************************************************************************


} // NL3D