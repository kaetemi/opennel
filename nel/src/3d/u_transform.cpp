/** \file u_transform.cpp
 * Interface for transform objects.
 *
 * $Id: u_transform.cpp,v 1.1 2004/05/07 14:41:42 corvazier Exp $
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

#include "nel/3d/u_transform.h"
#include "3d/transform.h"
#include "3d/instance_group_user.h"
#include "nel/misc/hierarchical_timer.h"
#include "scene_group.h"
#include "scene.h"

H_AUTO_DECL( NL3D_Transform_Set_Cluster_System )

#define	NL3D_HAUTO_SET_CLUSTER_SYSTEM		H_AUTO_USE( NL3D_Transform_Set_Cluster_System )

#define NL3D_MEM_TRANSFORM						NL_ALLOC_CONTEXT( 3dTrans )

using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************
void UTransform::setClusterSystem (UInstanceGroup *pIG)
{
	NL3D_HAUTO_SET_CLUSTER_SYSTEM

	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	if (object->getForceClipRoot())
	{
		nlwarning("Transform has been flagged to be glued to the root, and thus can't be clusterized. See UTransform::setForceClipRoot(bool).");
		return;
	}
	if ((pIG == NULL) || (pIG == (UInstanceGroup*)-1))
	{
		if (pIG == NULL)
			object->setClusterSystem (NULL);
		else
			object->setClusterSystem ((CInstanceGroup*)-1);
	}
	else
		object->setClusterSystem (&((CInstanceGroupUser*)pIG)->getInternalIG());
}

// ***************************************************************************
UInstanceGroup *UTransform::getClusterSystem () const
{
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	CInstanceGroup	*ig= object->getClusterSystem();
	if(ig==((CInstanceGroup*)-1))
		return ((UInstanceGroup*)-1);
	else if(ig==NULL)
		return NULL;
	else
		return ig->getUserInterface();
}

// ***************************************************************************
void			UTransform::getLastParentClusters(std::vector<CCluster*> &clusters) const
{
	CTransform	*object = getObjectPtr();
	CScene *scene = object->getOwnerScene();
	// look in the list of parent of the transform object and extract the CCluster parents
	if (scene == NULL)
		return;

	CClipTrav	&clipTrav= scene->getClipTrav();

	uint	num= object->clipGetNumParents();
	for(uint i=0;i<num;i++)
	{
		CCluster *pcluster = dynamic_cast<CCluster*>(object->clipGetParent(i));
		if (pcluster != NULL)
			clusters.push_back(pcluster);
	}

	// If the object is link to a QuadCluster, add the RootCluster to the list
	CTransformShape	*trShp= dynamic_cast<CTransformShape*>( object );
	if( trShp && trShp->isLinkToQuadCluster() )
		clusters.push_back(clipTrav.RootCluster);
}


// ***************************************************************************
void			UTransform::freezeHRC()
{
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	object->freezeHRC();
}

// ***************************************************************************
void			UTransform::unfreezeHRC()
{
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	while (object)
	{
		object->unfreezeHRC();
		object = object->hrcGetParent();
	}
}


// ***************************************************************************
void			UTransform::setLoadBalancingGroup(const std::string &group)
{
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	object->setLoadBalancingGroup(group);
}
// ***************************************************************************
const std::string	&UTransform::getLoadBalancingGroup() const
{
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	return object->getLoadBalancingGroup();
}

// ***************************************************************************
void			UTransform::setMeanColor(NLMISC::CRGBA color)
{
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	object->setMeanColor(color);
}
// ***************************************************************************
NLMISC::CRGBA	UTransform::getMeanColor() const
{
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	return object->getMeanColor();
}

// ***************************************************************************
const CMatrix	&UTransform::getLastWorldMatrixComputed() const
{
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	return object->getWorldMatrix();
}

// ***************************************************************************
void			UTransform::enableCastShadowMap(bool state)
{
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	object->enableCastShadowMap(state);
}

// ***************************************************************************
bool			UTransform::canCastShadowMap() const
{
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	return object->canCastShadowMap();
}

// ***************************************************************************
void			UTransform::enableReceiveShadowMap(bool state)
{
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	object->enableReceiveShadowMap(state);
}

// ***************************************************************************
bool			UTransform::canReceiveShadowMap() const
{
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	return object->canReceiveShadowMap();
}

// ***************************************************************************
void			UTransform::parent(UTransform newFather)
{
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	if (object->getForceClipRoot())
	{
		nlwarning("Transform has been flagged to be glued to the root, can't change parent. See UTransform::setForceClipRoot(bool).");
		return;
	}
	if(!newFather.empty())
	{
		// link me to other.
		CTransform	*other= newFather.getObjectPtr();
		if(other->getOwnerScene()!=object->getOwnerScene())
			nlerror("Try to parent 2 object from 2 differnet scenes!!");
		other->hrcLinkSon( object );
	}
	else
	{
		// link me to Root.
		object->getOwnerScene()->getRoot()->hrcLinkSon( object );
	}
}

// ***************************************************************************
void UTransform::hide()
{
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	nlassert(_Object) ; // object invalid now ...
	object->hide();
}

// ***************************************************************************

void UTransform::show()
{
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	object->show();
}

// ***************************************************************************

void UTransform::setUserClipping(bool enable)
{
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	object->setUserClipping(enable);
}

// ***************************************************************************

bool UTransform::getUserClipping() const
{
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	return object->getUserClipping();
}

// ***************************************************************************

void UTransform::heritVisibility()
{
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	object->heritVisibility();
}

// ***************************************************************************

UTransform::TVisibility UTransform::getVisibility()
{
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	return (UTransform::TVisibility)(uint32)object->getVisibility();
}

// ***************************************************************************

void UTransform::setOrderingLayer(uint layer) 
{ 
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	object->setOrderingLayer(layer); 
}

// ***************************************************************************

uint UTransform::getOrderingLayer() const 
{ 
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	return object->getOrderingLayer(); 
}

// ***************************************************************************

void UTransform::setUserLightable(bool enable) 
{
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	object->setUserLightable(enable);
}

// ***************************************************************************

bool UTransform::getUserLightable() const  
{
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	return  object->getUserLightable();
}

// ***************************************************************************

void UTransform::setLogicInfo(ILogicInfo *logicInfo) 
{
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	object->setLogicInfo(logicInfo);
}

// ***************************************************************************

bool UTransform::getLastWorldVisState() const
{
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	return object->isHrcVisible();
}

// ***************************************************************************

bool UTransform::getLastClippedState() const 
{
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	return object->isClipVisible();
}

// ***************************************************************************

void UTransform::setTransparency(bool v)
{
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	object->setTransparency(v);
}

// ***************************************************************************

void UTransform::setOpacity(bool v)
{
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	object->setOpacity(true);
}

// ***************************************************************************

uint32 UTransform::isOpaque()
{
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	return object->isOpaque();
}

// ***************************************************************************

uint32 UTransform::isTransparent()
{
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	return object->isTransparent();
}

// ***************************************************************************

void UTransform::setForceClipRoot(bool forceClipRoot)
{
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	object->setForceClipRoot(forceClipRoot);
}

// ***************************************************************************

bool UTransform::getForceClipRoot() const
{
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	return object->getForceClipRoot();
}

// ***************************************************************************

void UTransform::setTransparencyPriority(uint8 priority)
{
	NL3D_MEM_TRANSFORM
	nlassert(_Object) ; // object invalid now ...
	CTransform	*object = getObjectPtr();
	object->setTransparencyPriority(priority);
}

// ***************************************************************************

} // NL3D