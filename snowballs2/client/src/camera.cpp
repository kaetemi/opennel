/** \file camera.cpp
 * Camera interface between the game and NeL
 *
 * $Id$
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

//
// Includes
//

#include <cmath>
#include <nel/misc/vectord.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_skeleton.h>
#include <nel/3d/u_visual_collision_entity.h>
#include <nel/3d/u_visual_collision_manager.h>
#include <nel/3d/u_cloud_scape.h>
#include <nel/3d/viewport.h>

#include "client.h"
#include "entities.h"
#include "mouse_listener.h"
#include "pacs.h"

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NL3D;

//
// Variables
//

// The camera for the whole scene
UCamera					Camera = NULL;
// The collision entity use to snap the camera on the ground
UVisualCollisionEntity	*CamCollisionEntity = NULL;

// The particle system for the snowing effect
static UInstance			Snow = NULL;

// The sky 3D objects
static UScene				*SkyScene = NULL;
static UCamera				SkyCamera = NULL;
static UInstance			Sky = NULL;

static UCloudScape			*Clouds = NULL;

//
// Functions
//

void	initCamera()
{
	// Set up directly the camera
	Camera = Scene->getCam();
	Camera.setTransformMode (UTransformable::DirectMatrix);
	Camera.setPerspective ((float)Pi/2.f, 1.33f, 0.1f, 1000);
	Camera.lookAt (CVector(ConfigFile.getVar("StartPoint").asFloat(0),
							ConfigFile.getVar("StartPoint").asFloat(1),
							ConfigFile.getVar("StartPoint").asFloat(2)),
							CVectorD (0,0,0));

	CamCollisionEntity = VisualCollisionManager->createEntity();
	CamCollisionEntity->setCeilMode(true);

	// Create the snowing particle system
	Snow = Scene->createInstance("snow.ps");
	// And setup it
	Snow.setTransformMode (UTransformable::DirectMatrix);

	//
	// Setup the sky scene
	//

	// -- -- not sure what the sky has to do with the camera

	SkyScene = Driver->createScene(false);

	SkyCamera = SkyScene->getCam ();
	SkyCamera.setTransformMode (UTransformable::DirectMatrix);
	// Set the very same frustum as the main camera
	SkyCamera.setFrustum (Camera.getFrustum ());

	Sky = SkyScene->createInstance("sky.shape");
	Sky.setTransformMode (UTransformable::DirectMatrix);
	Sky.setMatrix(CMatrix::Identity);
}

void releaseCamera()
{
	SkyScene->deleteInstance(Sky);
	Driver->deleteScene(SkyScene);
	Scene->deleteInstance(Snow);
	VisualCollisionManager->deleteEntity(CamCollisionEntity);
}

void updateCamera()
{
	// Set the new position of the snow emitter
	CMatrix	mat = CMatrix::Identity;
	mat.setPos (Camera.getMatrix().getPos()/*+CVector (0.0f, 0.0f, -10.0f)*/);
	Snow.setMatrix(mat);
}

void initSky()
{
	// -- -- or what the clouds have to do with the sky

	SCloudScapeSetup css;
	Clouds = Scene->createCloudScape ();
	Clouds->init (&css);
	Clouds->setQuality (160);
	Clouds->setNbCloudToUpdateIn80ms (1);
}

void releaseSky()
{
	Scene->deleteCloudScape(Clouds);
}

// -- -- random note: update and render makes more sense than animate and update
void animateSky(TTime dt)
{
	Clouds->anim ((double)dt);
}

void updateSky()
{
	CMatrix skyCameraMatrix;
	skyCameraMatrix.identity();
	// 
	skyCameraMatrix= Camera.getMatrix();
	skyCameraMatrix.setPos(CVector::Null);
	SkyCamera.setMatrix(skyCameraMatrix);

	SkyScene->animate (float(NewTime)/1000);
	SkyScene->render ();
	// Must clear ZBuffer For incoming rendering.
	Driver->clearZBuffer();

	Clouds->render ();
}
