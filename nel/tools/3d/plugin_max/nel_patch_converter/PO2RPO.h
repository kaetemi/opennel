/** \file po2rpo.h
 * <File description>
 *
 * $Id: PO2RPO.h,v 1.1 2001/04/26 16:37:31 corvazier Exp $
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


#ifndef __PMESH2RKLPMESH__H
#define __PMESH2RKLPMESH__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "..\nel_export\checkversion.h"

#include "meshadj.h"

#define PO2RPO_CLASS_ID Class_ID(0x43bb65e6, 0x68935530)

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

class PO2RPO : public Modifier {
	public:
		// Parameter block
		IParamBlock2	*pblock;	//ref 0
		static IObjParam *ip;			//Access to the interface
		
		HWND hRollup;

		// From Animatable
		TCHAR *GetObjectName() { return GetString(IDS_CLASS_NAME); }

		//From Modifier
		//TODO: Add the channels that the modifier needs to perform its modification
		ChannelMask ChannelsUsed()  { return (PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|PART_DISPLAY|PART_TOPO|TEXMAP_CHANNEL); }
		//TODO: Add the channels that the modifier actually modifies
		ChannelMask ChannelsChanged() { return (PART_GEOM|SELECT_CHANNEL|PART_SUBSEL_TYPE|PART_DISPLAY|PART_TOPO|TEXMAP_CHANNEL); }
		void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		//TODO: Return the ClassID of the object that the modifier can modify
		Class_ID InputType() {return Class_ID(PATCHOBJ_CLASS_ID,0);}
		Interval LocalValidity(TimeValue t);

		// From BaseObject
		//TODO: Return true if the modifier changes topology
		BOOL ChangeTopology() {return TRUE;}		

		CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;}
		void BeginEditParams(IObjParam *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

		Interval GetValidity(TimeValue t);


		// Automatic texture support
		BOOL HasUVW();
		void SetGenUVW(BOOL sw);
		
		// Loading/Saving
		IOResult Load(ILoad *iload);
		IOResult Save(ISave *isave);

		//From Animatable
		Class_ID ClassID() {return PO2RPO_CLASS_ID;}		
		SClass_ID SuperClassID() { return OSM_CLASS_ID; }
		void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}
		
		RefTargetHandle Clone( RemapDir &remap );
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
			PartID& partID,  RefMessage message);

		int NumSubs() { return 0; }
		TSTR SubAnimName(int i) { return GetString(IDS_PARAMS); }
		Animatable* SubAnim(int i) { return pblock; }
		int NumRefs() { return 1; }
		RefTargetHandle GetReference(int i) { return pblock; }
		void SetReference(int i, RefTargetHandle rtarg) { pblock=(IParamBlock2*)rtarg; }

		int	NumParamBlocks() { return 1; }					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) { return pblock; } // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; } // return id'd ParamBlock
		void DeleteThis() { delete this; }		
		//Constructor/Destructor
		PO2RPO();
		~PO2RPO();		
};


class PO2RPOClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() {return 1;}
	void *			Create(BOOL loading = FALSE) 
	{
		// Check version
		static bool bPassed=false;
		if (!bPassed)
		{
			bPassed=true;
			CheckPluginVersion ("plugins max\\plugins\\nelconvertpatch.dlm");
		}

		return new PO2RPO();
	}
	const TCHAR *	ClassName() {return "NeLConvert";}
	SClass_ID		SuperClassID() {return OSM_CLASS_ID;}
	Class_ID		ClassID() {return PO2RPO_CLASS_ID;}
	const TCHAR* 	Category() {return "NeL Tools";}
	const TCHAR*	InternalName() { return _T("PatchObject To NelPatchObject"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};


#endif // __PMESH2RKLPMESH__H
