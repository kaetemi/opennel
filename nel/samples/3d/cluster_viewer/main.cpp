/** \file main.cpp
 * viewer of cluster system
 *
 * $Id: main.cpp,v 1.4 2003/11/07 14:30:15 besson Exp $
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


// ---------------------------------------------------------------------------
// Includes
// ---------------------------------------------------------------------------


#include "nel/misc/types_nl.h"
#include "nel/misc/file.h"
#include "nel/misc/bitmap.h"

#include "3d/driver.h"
#include "3d/nelu.h"
#include "nel/misc/event_server.h"
#include "nel/misc/event_listener.h"
#include "nel/misc/events.h"
#include "nel/misc/path.h"
#include "nel/misc/time_nl.h"
#include "3d/scene_group.h"
#include "3d/transform_shape.h"
#include "3d/event_mouse_listener.h"
#include "3d/text_context.h"

#include <windows.h>

using namespace std;
using namespace NL3D;
using namespace NLMISC;


// ---------------------------------------------------------------------------
struct SDispCS
{
	string Name;
	CInstanceGroup *pIG;
};


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
char *readLine (char*out, char*in)
{
	out[0] = 0;

	while ((*in != '\r') && (*in != '\n') && (*in != 0))
	{
		*out = *in;
		++out;
		++in;
		*out = 0;
	}
	
	while ((*in == '\r') || (*in == '\n'))
		++in;
	return in;
}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
CInstanceGroup* LoadInstanceGroup(const char* sFilename)
{
	CIFile file;
	CInstanceGroup *newIG = new CInstanceGroup;

	if( file.open( CPath::lookup( string(sFilename) ) ) )
	{
		try
		{
			// Serial the skeleton
			newIG->serial (file);
			// All is good
		}
		catch (Exception &)
		{
			// Cannot save the file
			delete newIG;
			return NULL;
		}
	}
	return newIG;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
void LoadSceneScript (const char *ScriptName, CScene* pScene, vector<SDispCS> &DispCS, CVector &CameraStart, 
					  vector<CInstanceGroup*> &vIGs)
{
	char nameIG[256];
	float posx, posy, posz;
	float roti, rotj, rotk;
	
	FILE *f = fopen (ScriptName,"rb");
	fseek (f, 0, SEEK_END);
	int file_size = ftell (f);
	fseek (f, 0, SEEK_SET);
	char *file_buf = (char*)malloc(file_size+1);
	fread (file_buf, 1, file_size, f);
	file_buf[file_size] = 0;
	++file_size;
	fclose (f);
	char *buf_ptr = file_buf;
	int nLastNbPlus = 0;
	vector<CInstanceGroup*> pile;
	pile.clear ();
	pile.push_back (pScene->getGlobalInstanceGroup());
	pile.push_back (pScene->getGlobalInstanceGroup());

	do 
	{
		char Line[256], *line_ptr;
		int nNbPlus = 0;
		line_ptr = &Line[0];
		buf_ptr = readLine (line_ptr, buf_ptr);

		while ((*line_ptr == '\t') || (*line_ptr == ' ') || (*line_ptr == '+'))
		{
			if (*line_ptr == '+') 
				++nNbPlus;
			++line_ptr;
		}

		if (strlen (line_ptr) == 0)
			continue;

		if (*line_ptr == '/')
			continue;

		posx = posy = posz = roti = rotj = rotk = 0.0f;
		sscanf (line_ptr, "%s %f %f %f %f %f %f", nameIG, &posx, &posy, &posz, &roti, &rotj, &rotk);

		if (stricmp (nameIG, "CAMERA_START") == 0)
		{
			CameraStart = CVector(posx, posy, posz);
		}
		else
		{
			if (nLastNbPlus >= nNbPlus)
			for (int i = 0; i < ((nLastNbPlus-nNbPlus)+1); ++i)
				pile.pop_back();
		
			nLastNbPlus = nNbPlus;

			CInstanceGroup *father = pile.back();
		
			CInstanceGroup *ITemp = LoadInstanceGroup (nameIG);
			if (ITemp != NULL)
			{
				SDispCS dcsTemp;
				dcsTemp.Name = "";
				for (sint32 i = 0; i < (1+nNbPlus); ++i)
					dcsTemp.Name += "   ";
				dcsTemp.Name += nameIG;
				dcsTemp.pIG = ITemp;
				DispCS.push_back (dcsTemp);
				
				ITemp->createRoot (*pScene);
				ITemp->setPos (CVector(posx, posy, posz));
				ITemp->setRotQuat (ITemp->getRotQuat() * CQuat(CVector::I, roti));
				ITemp->setRotQuat (ITemp->getRotQuat() * CQuat(CVector::J, rotj));
				ITemp->setRotQuat (ITemp->getRotQuat() * CQuat(CVector::K, rotk));
				ITemp->setClusterSystem (father);
				ITemp->addToScene (*pScene);
				vIGs.push_back (ITemp);
			}
			pile.push_back (ITemp);
		}
	} 
	//while (strlen(buf_ptr) > 0);
	while (buf_ptr < (file_buf+file_size-1));
	free (file_buf);
}


// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int WINAPI WinMain(
  HINSTANCE hInstance,      // handle to current instance
  HINSTANCE hPrevInstance,  // handle to previous instance
  LPSTR lpCmdLine,          // command line
  int nCmdShow              // show state
)
{
	double rGlobalTime = 0;
	double rOldGlobalTime = 0;
	double rDeltaTime = 0;
	double rMoveTime = 0;

	vector<SDispCS> DispCS;

	bool bAutoDetect = true;
	bool bDisplay = true;
	sint32 nSelected = 0;

	SDispCS dcsTemp;

	vector<CInstanceGroup*> vAllIGs;
	// 2 dynamics objects
	CTransformShape *pDynObj_InRoot;
	CTransformShape *pDynObj_InCS;



	CNELU::init (800, 600, CViewport(), 32, true);

	CPath::addSearchPath("shapes/");
	CPath::addSearchPath("groups/");

	CFontManager FontManager;
	CTextContext TextContext;

	TextContext.init (CNELU::Driver, &FontManager);	
	TextContext.setFontGenerator ("fonts/n019003l.pfb");
	TextContext.setHotSpot (CComputedString::TopLeft);
	TextContext.setColor (CRGBA(255,255,255));
	TextContext.setFontSize (20);

	CEvent3dMouseListener MouseListener;
	MouseListener.addToServer (CNELU::EventServer);
	MouseListener.setFrustrum (CNELU::Camera->getFrustum());
	MouseListener.setMouseMode (CEvent3dMouseListener::firstPerson);
	
	CNELU::Camera->setTransformMode (ITransformable::DirectMatrix);
	// Force to automatically find the cluster system
	CNELU::Camera->setClusterSystem ((CInstanceGroup*)-1); 


	CClipTrav *pClipTrav = (CClipTrav*)&(CNELU::Scene->getClipTrav());
	dcsTemp.Name = "Root";
	dcsTemp.pIG = NULL;
	DispCS.push_back (dcsTemp);

	
	// Add all instance that create the scene
	// --------------------------------------	
	// Begining of script reading
	CVector CameraStart;

	LoadSceneScript ("main.cvs", CNELU::Scene, DispCS, CameraStart, vAllIGs);

	CMatrix m = MouseListener.getViewMatrix();
	m.setPos (CameraStart);
	MouseListener.setMatrix (m);
	// End of script reading

	// Put a dynamic object in the root
	pDynObj_InRoot = CNELU::Scene->createInstance ("Sphere01.shape");
	pDynObj_InRoot->setPos (0.0f, 0.0f, 0.0f);
	// Put a dynamic object inside
	pDynObj_InCS = CNELU::Scene->createInstance ("Sphere01.shape");
	pDynObj_InCS->setPos (50.0f, 50.0f, 25.0f);
	// No automatic detection for moving objects - setup in street
	pDynObj_InCS->setClusterSystem (vAllIGs[0]);


	// Main loop
	do
	{
		rGlobalTime = CTime::ticksToSecond(CTime::getPerformanceTime());
		rDeltaTime = rGlobalTime - rOldGlobalTime;
		rOldGlobalTime = rGlobalTime;

		pDynObj_InRoot->setPos	(-20.0f+20.0f*sinf((float)rGlobalTime), 
								-20.0f+20.0f*cosf((float)rGlobalTime*1.2f), 
								20.0f*sinf((float)rGlobalTime*1.1f+0.5f));

		pDynObj_InCS->setPos	(25.0f+5.0f*sinf((float)rGlobalTime*1.4f), 
								-25.0f+5.0f*cosf((float)rGlobalTime*1.3f), 
								25.0f+2.0f*sinf((float)rGlobalTime*1.2f+0.7f));


		CNELU::Scene->animate ((float)rGlobalTime);

		CNELU::EventServer.pump();

		CNELU::clearBuffers (CRGBA(0,0,0));

		CNELU::Scene->render ();

		// -----------------------------------------------------
		// -----------------------------------------------------
		if (bDisplay)
		{
			vector<CCluster*> vCluster;
			DispCS[0].pIG = pClipTrav->RootCluster->Group;
			TextContext.setColor (CRGBA(255,255,255,255));
			if (bAutoDetect)
			{
				TextContext.printfAt (0, 1, "AutoDetect : ON");
				
				pClipTrav->fullSearch (vCluster, pClipTrav->RootCluster->Group, pClipTrav->CamPos);

				for (uint32 i = 0; i < DispCS.size(); ++i)
				{
					TextContext.setColor (CRGBA(255,255,255,255));
					for( uint32 j = 0; j < vCluster.size(); ++j)
					{
						if (DispCS[i].pIG == vCluster[j]->Group)
						{
							TextContext.setColor (CRGBA(255,0,0,255));
							break;
						}
					}

					TextContext.printfAt (0, 1-(i+2)*0.028f, DispCS[i].Name.c_str());
				}

			}
			else
			{
				TextContext.printfAt (0, 1, "AutoDetect : OFF");

				CInstanceGroup *pCurIG = CNELU::Camera->getClusterSystem();
				for (uint32 i = 0; i < DispCS.size(); ++i)
				{
					if (DispCS[i].pIG == pCurIG)
						TextContext.setColor (CRGBA(255,0,0,255));
					else
						TextContext.setColor (CRGBA(255,255,255,255));

					TextContext.printfAt (0, 1-(i+2)*0.028f, DispCS[i].Name.c_str());
				}

				TextContext.setColor (CRGBA(255,255,255,255));

				pClipTrav->Accel.select (pClipTrav->CamPos, pClipTrav->CamPos);
				CQuadGrid<CCluster*>::CIterator itAcc = pClipTrav->Accel.begin();
				while (itAcc != pClipTrav->Accel.end())
				{
					CCluster *pCluster = *itAcc;
					if (pCluster->Group == pClipTrav->Camera->getClusterSystem())
					if (pCluster->isIn (pClipTrav->CamPos))
					{
						vCluster.push_back (pCluster);
					}
					++itAcc;
				}
				if ((vCluster.size() == 0) && (DispCS[0].pIG == pCurIG))
				{
					vCluster.push_back (pClipTrav->RootCluster);
				}

			}			

			TextContext.setColor (CRGBA(255,255,255,255));

			string sAllClusters = "";
			for( uint32 j = 0; j < vCluster.size(); ++j)
			{
				sAllClusters += vCluster[j]->Name;
				if (j < (vCluster.size()-1))
					sAllClusters += ",  ";
			}
			TextContext.printfAt (0, 1-0.028f, sAllClusters.c_str());				


		}

		// -----------------------------------------------------
		// -----------------------------------------------------

		CNELU::Driver->swapBuffers ();

		
		// Keys management
		// ---------------

		if (CNELU::AsyncListener.isKeyDown (KeySHIFT))
			MouseListener.setSpeed (50.0f);
		else
			MouseListener.setSpeed (10.0f);

		CNELU::Camera->setMatrix (MouseListener.getViewMatrix());

		if (CNELU::AsyncListener.isKeyPushed(KeyL))
		{
			CNELU::Driver->setPolygonMode (IDriver::Line);
		}

		if (CNELU::AsyncListener.isKeyPushed (KeyP))
		{
			CNELU::Driver->setPolygonMode (IDriver::Filled);
		}

		if (CNELU::AsyncListener.isKeyPushed (KeyTAB))
			bDisplay = !bDisplay;

		if (CNELU::AsyncListener.isKeyPushed (KeyA))
		{
			if (bAutoDetect)
			{
				bAutoDetect = false;
				CNELU::Camera->setClusterSystem (NULL);
				nSelected = 0;
			}
			else
			{
				bAutoDetect = true;
				CNELU::Camera->setClusterSystem ((CInstanceGroup*)-1);
			}
		}

		if (!bAutoDetect)
		{
			if (CNELU::AsyncListener.isKeyPushed (KeyZ))
			{
				nSelected--;
				if(nSelected == -1)
					nSelected = DispCS.size()-1;
			}
			if (CNELU::AsyncListener.isKeyPushed (KeyS))
			{
				nSelected++;
				if(nSelected == (sint32)DispCS.size())
					nSelected = 0;
			}
			CNELU::Camera->setClusterSystem (DispCS[nSelected].pIG);
		}

	}
	while (!CNELU::AsyncListener.isKeyPushed (KeyESCAPE));
	return 1;
}
