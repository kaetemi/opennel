/** \file vegetable_blend_layer_model.cpp
 * <File description>
 *
 * $Id: vegetable_blend_layer_model.cpp,v 1.10 2004/03/19 10:11:36 corvazier Exp $
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

#include "std3d.h"

#include "3d/vegetable_blend_layer_model.h"
#include "3d/vegetable_manager.h"
#include "3d/vegetable_sort_block.h"
#include "3d/render_trav.h"
#include "3d/clip_trav.h"
#include "nel/misc/debug.h"
#include "nel/misc/hierarchical_timer.h"
#include "3d/scene.h"


using namespace NLMISC;

namespace NL3D {


// ***************************************************************************
void	CVegetableBlendLayerModel::registerBasic()
{
	CScene::registerModel(VegetableBlendLayerModelId, TransformId, CVegetableBlendLayerModel::creator);
}


// ***************************************************************************
CVegetableBlendLayerModel::CVegetableBlendLayerModel()
{
	VegetableManager= NULL;

	// The model must always be renderer in transparency pass only.
	setTransparency(true);
	setOpacity(false);

	// The model is of course renderable
	CTransform::setIsRenderable(true);
}


// ***************************************************************************
void	CVegetableBlendLayerModel::setWorldPos(const CVector &pos)
{
	// setup directly the local matrix.
	_LocalMatrix.setPos(pos);

	// setup directly the world matrix.
	_WorldMatrix.setPos(pos);
	
}


// ***************************************************************************
void	CVegetableBlendLayerModel::render(IDriver *driver)
{
	H_AUTO( NL3D_Vegetable_Render );

	nlassert(VegetableManager);

	if(SortBlocks.size()==0)
		return;

	// Setup VegetableManager renderState (like pre-setuped material)
	//==================
	VegetableManager->setupRenderStateForBlendLayerModel(driver);


	// Render SortBlocks of this layer
	//==================
	uint	rdrPass= NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED_ZSORT;

	// first time, activate the hard VB.
	bool	precVBHardMode= true;
	CVegetableVBAllocator	*vbAllocator= &VegetableManager->getVBAllocatorForRdrPassAndVBHardMode(rdrPass, 1);
	vbAllocator->activate();

	// profile
	CPrimitiveProfile	ppIn, ppOut;
	driver->profileRenderedPrimitives(ppIn, ppOut);
	uint	precNTriRdr= ppOut.NTriangles;

	// render from back to front the list setuped in CVegetableManager::render()
	for(uint i=0; i<SortBlocks.size();i++)
	{
		CVegetableSortBlock	*ptrSortBlock= SortBlocks[i];

		// change of VertexBuffer (soft / hard) if needed.
		if(ptrSortBlock->ZSortHardMode != precVBHardMode)
		{
			// setup new VB for hardMode.
			CVegetableVBAllocator	*vbAllocator= &VegetableManager->getVBAllocatorForRdrPassAndVBHardMode(rdrPass, ptrSortBlock->ZSortHardMode);
			vbAllocator->activate();
			// prec.
			precVBHardMode= ptrSortBlock->ZSortHardMode;
		}

		// render him. we are sure that size > 0, because tested before.
		driver->activeIndexBuffer(ptrSortBlock->_SortedTriangleArray);
		driver->renderSimpleTriangles(
			ptrSortBlock->_SortedTriangleIndices[ptrSortBlock->_QuadrantId], 
			ptrSortBlock->_NTriangles);
	}

	// add number of triangles rendered with vegetable manager.
	driver->profileRenderedPrimitives(ppIn, ppOut);
	VegetableManager->_NumVegetableFaceRendered+= ppOut.NTriangles-precNTriRdr;


	// refresh list now!
	// We must do it here, because if CVegetableManager::render() is no more called (eg: disabled),
	// then the blend layers models must do nothing.
	SortBlocks.clear();


	// Reset RenderState.
	//==================
	VegetableManager->exitRenderStateForBlendLayerModel(driver);

}


// ***************************************************************************
void	CVegetableBlendLayerModel::traverseRender()
{
	CRenderTrav		&rTrav= getOwnerScene()->getRenderTrav();
	render(rTrav.getDriver());
}


} // NL3D
