/** \file register_3d.cpp
 * <File description>
 *
 * $Id: register_3d.cpp,v 1.14 2001/06/19 10:22:33 berenguier Exp $
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


#include "3d/register_3d.h"
#include "3d/texture_font.h"
#include "3d/texture_file.h"
#include "3d/texture_mem.h"
#include "3d/texture_grouped.h"
#include "3d/mesh.h"
#include "3d/skeleton_shape.h"
#include "3d/track_keyframer.h"
#include "3d/ps_util.h"
#include "3d/mesh_mrm.h"


namespace NL3D 
{


//****************************************************************************
void	registerSerial3d()
{
	static bool bInitialized=false;
	if (!bInitialized)
	{
		// Textures.
		NLMISC_REGISTER_CLASS(CTextureFile);
		NLMISC_REGISTER_CLASS(CTextureMem);
		NLMISC_REGISTER_CLASS(CTextureFont);
		NLMISC_REGISTER_CLASS(CTextureGrouped);

		// Track
		NLMISC_REGISTER_CLASS(CTrackKeyFramerTCBFloat);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerTCBVector);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerTCBQuat);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerTCBInt);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerTCBRGBA);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerBezierFloat);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerBezierVector);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerBezierQuat);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerBezierInt);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerBezierRGBA);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerLinearFloat);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerLinearVector);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerLinearQuat);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerLinearInt);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerLinearRGBA);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerConstFloat);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerConstVector);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerConstQuat);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerConstInt);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerConstString);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerConstBool);
		NLMISC_REGISTER_CLASS(CTrackKeyFramerConstRGBA);
		NLMISC_REGISTER_CLASS(CTrackDefaultFloat);
		NLMISC_REGISTER_CLASS(CTrackDefaultVector);
		NLMISC_REGISTER_CLASS(CTrackDefaultQuat);
		NLMISC_REGISTER_CLASS(CTrackDefaultInt);
		NLMISC_REGISTER_CLASS(CTrackDefaultString);
		NLMISC_REGISTER_CLASS(CTrackDefaultBool);
		NLMISC_REGISTER_CLASS(CTrackDefaultRGBA);
		
		// Particle system
		CPSUtil::registerSerialParticleSystem() ;



		// Don't register CTextureCroos,, since local, and not designed to be serialised.

		// Shapes.
		NLMISC_REGISTER_CLASS(CMesh);
		NLMISC_REGISTER_CLASS(CSkeletonShape);
		NLMISC_REGISTER_CLASS(CMeshMRM);

		bInitialized=true;
	}
}


} // NL3D
