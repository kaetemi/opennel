/** \file async_file_manager.cpp
 * <File description>
 *
 * $Id: async_file_manager.cpp,v 1.5 2003/10/20 16:10:17 lecroart Exp $
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
#include "stdmisc.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/async_file_manager.h"


using namespace std;

namespace NLMISC
{

CAsyncFileManager *CAsyncFileManager::_Singleton = NULL;

// ***************************************************************************

CAsyncFileManager::CAsyncFileManager()
{
}

// ***************************************************************************

CAsyncFileManager &CAsyncFileManager::getInstance()
{
	if (_Singleton == NULL)
	{
		_Singleton = new CAsyncFileManager();
	}
	return *_Singleton;
}

// ***************************************************************************

void CAsyncFileManager::terminate ()
{
	if (_Singleton != NULL)
	{
		delete &getInstance();
		_Singleton = NULL;
	}
}


void CAsyncFileManager::addLoadTask(IRunnable *ploadTask)
{
	addTask(ploadTask);
}

bool CAsyncFileManager::cancelLoadTask(const CAsyncFileManager::ICancelCallback &callback)
{
	CSynchronized<list<CWaitingTask> >::CAccessor acces(&_TaskQueue);
	list<CWaitingTask> &rTaskQueue = acces.value ();
	list<CWaitingTask>::iterator it = rTaskQueue.begin();

	while (it != rTaskQueue.end())
	{
		IRunnable *pR = it->Task;

		// check the task with the cancel callback.
		if (callback.callback(pR))
		{
			// Delete the load task
			delete pR;
			rTaskQueue.erase (it);
			return true;
		}
		++it;
	}

	// If not found, the current running task may be the one we want to cancel. Must wait it.
	waitCurrentTaskToComplete ();

	return false;
}

// ***************************************************************************
/*	
void CAsyncFileManager::loadMesh(const std::string& meshName, IShape **ppShp, IDriver *pDriver)
{
	addTask (new CMeshLoad(meshName, ppShp, pDriver));
}
*/
// ***************************************************************************
/*
bool CAsyncFileManager::cancelLoadMesh(const std::string& sMeshName)
{
	CSynchronized<list<IRunnable *> >::CAccessor acces(&_TaskQueue);
	list<IRunnable*> &rTaskQueue = acces.value ();
	list<IRunnable*>::iterator it = rTaskQueue.begin();

	while (it != rTaskQueue.end())
	{
		IRunnable *pR = *it;
		CMeshLoad *pML = dynamic_cast<CMeshLoad*>(pR);
		if (pML != NULL)
		{
			if (pML->MeshName == sMeshName)
			{
				// Delete mesh load task
				delete pML;
				rTaskQueue.erase (it);
				return true;
			}
		}
		++it;
	}
	return false;
}
*/
// ***************************************************************************
/*	
void CAsyncFileManager::loadIG (const std::string& IGName, CInstanceGroup **ppIG)
{
	addTask (new CIGLoad(IGName, ppIG));
}

// ***************************************************************************
	
void CAsyncFileManager::loadIGUser (const std::string& IGName, UInstanceGroup **ppIG)
{
	addTask (new CIGLoadUser(IGName, ppIG));
}
*/
// ***************************************************************************
	
void CAsyncFileManager::loadFile (const std::string& sFileName, uint8 **ppFile)
{
	addTask (new CFileLoad (sFileName, ppFile));
}

// ***************************************************************************

void CAsyncFileManager::loadFiles (const std::vector<std::string> &vFileNames, const std::vector<uint8**> &vPtrs)
{
	addTask (new CMultipleFileLoad (vFileNames, vPtrs));
}

// ***************************************************************************

void CAsyncFileManager::signal (bool *pSgn)
{
	addTask (new CSignal (pSgn));
}

// ***************************************************************************

void CAsyncFileManager::cancelSignal (bool *pSgn)
{
	CSynchronized<list<CWaitingTask> >::CAccessor acces(&_TaskQueue);
	list<CWaitingTask> &rTaskQueue = acces.value ();
	list<CWaitingTask>::iterator it = rTaskQueue.begin();

	while (it != rTaskQueue.end())
	{
		IRunnable *pR = it->Task;
		CSignal *pS = dynamic_cast<CSignal*>(pR);
		if (pS != NULL)
		{
			if (pS->Sgn == pSgn)
			{
				// Delete signal task
				delete pS;
				rTaskQueue.erase (it);
				return;
			}
		}
		++it;
	}
}

// ***************************************************************************
// FileLoad
// ***************************************************************************

// ***************************************************************************
CAsyncFileManager::CFileLoad::CFileLoad (const std::string& sFileName, uint8 **ppFile)
{
	_FileName = sFileName;
	_ppFile = ppFile;
}

// ***************************************************************************
void CAsyncFileManager::CFileLoad::run (void)
{
	FILE *f = fopen (_FileName.c_str(), "rb");
	if (f != NULL)
	{
		uint8 *ptr;
		long filesize=CFile::getFileSize (f);
		//fseek (f, 0, SEEK_END);
		//long filesize = ftell (f);
		//nlSleep(5);
		//fseek (f, 0, SEEK_SET);
		ptr = new uint8[filesize];
		fread (ptr, filesize, 1, f);
		fclose (f);

		*_ppFile = ptr;
	}
	else
	{
		nlwarning ("AFM: Couldn't load '%s'", _FileName.c_str());
		*_ppFile = (uint8*)-1;
	}
}

// ***************************************************************************
void CAsyncFileManager::CFileLoad::getName (std::string &result) const
{
	result = "FileLoad (" + _FileName + ")";
}

// ***************************************************************************
// MultipleFileLoad
// ***************************************************************************

// ***************************************************************************
CAsyncFileManager::CMultipleFileLoad::CMultipleFileLoad (const std::vector<std::string> &vFileNames, 
														 const std::vector<uint8**> &vPtrs)
{
	_FileNames = vFileNames;
	_Ptrs = vPtrs;
}

// ***************************************************************************
void CAsyncFileManager::CMultipleFileLoad::run (void)
{
	for (uint32 i = 0; i < _FileNames.size(); ++i)
	{
		FILE *f = fopen (_FileNames[i].c_str(), "rb");
		if (f != NULL)
		{
			uint8 *ptr;
			long filesize=CFile::getFileSize (f);
			//fseek (f, 0, SEEK_END);
			//long filesize = ftell (f);
			//nlSleep(5);
			//fseek (f, 0, SEEK_SET);
			ptr = new uint8[filesize];
			fread (ptr, filesize, 1, f);
			fclose (f);

			*_Ptrs[i] = ptr;
		}
		else
		{
			nlwarning ("AFM: Couldn't load '%s'", _FileNames[i].c_str());
			*_Ptrs[i] = (uint8*)-1;
		}
	}

}

// ***************************************************************************
void CAsyncFileManager::CMultipleFileLoad::getName (std::string &result) const
{
	result = "MultipleFileLoad (";
	uint i;
	for (i=0; i<_FileNames.size (); i++)
	{
		if (i)
			result += ", ";
		result += _FileNames[i];
	}
	result += ")";
}
// ***************************************************************************
// Signal
// ***************************************************************************

// ***************************************************************************
CAsyncFileManager::CSignal::CSignal (bool *pSgn)
{
	Sgn = pSgn;
	*Sgn = false;
}

// ***************************************************************************
void CAsyncFileManager::CSignal::run (void)
{
	*Sgn = true;
}

// ***************************************************************************
void CAsyncFileManager::CSignal::getName (std::string &result) const
{
	result = "Signal";
}

} // NLMISC

