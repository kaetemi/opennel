/** \file app_context.cpp
 * Classes for managing NeL context in order to support multi module NeL 
 * application.
 *
 * $Id: app_context.cpp,v 1.5 2006/01/10 17:38:47 boucher Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "stdmisc.h"
#include "nel/misc/app_context.h"
#include "nel/misc/dynloadlib.h"
#include "nel/misc/command.h"

namespace NLMISC
{

//INelContext *NelContext = NULL;
INelContext *INelContext::_NelContext = NULL;

INelContext &INelContext::getInstance()
{
	if (_NelContext == NULL)
	{
		_NelContext = new CApplicationContext;
	}

	return *_NelContext;
}

bool INelContext::isContextInitialised()
{
	return _NelContext != NULL;
}


INelContext::~INelContext()
{
	// unregister still undeleted local command into the global command registry
	ICommand::TCommand::iterator first(ICommand::LocalCommands->begin()), last(ICommand::LocalCommands->end());
	for (; first != last; ++first)
	{
		ICommand *command = first->second;
		CCommandRegistry::getInstance().unregisterCommand(command);
	}

	_NelContext = NULL;
}



void	INelContext::contextReady()
{
	// Register the NeL Context
	nlassert(_NelContext == NULL);
	_NelContext = this;

	// register any pending thinks

	// register local instance counter in the global instance counter manager
	CInstanceCounterLocalManager::getInstance().registerLocalManager();

	// register local commands into the global command registry (except it there is no command at all)
	if (ICommand::LocalCommands != NULL)
	{
		ICommand::TCommand::iterator first(ICommand::LocalCommands->begin()), last(ICommand::LocalCommands->end());
		for (; first != last; ++first)
		{
			CCommandRegistry::getInstance().registerCommand(first->second);
		}
	}
}

CApplicationContext::CApplicationContext()
{
	// init
	ErrorLog = NULL;
	WarningLog = NULL;
	InfoLog = NULL;
	DebugLog = NULL;
	AssertLog = NULL;
	DefaultMemDisplayer = NULL;
	DefaultMsgBoxDisplayer = NULL;
	DebugNeedAssert = false;
	NoAssert = false;
	AlreadyCreateSharedAmongThreads = false;

	contextReady();
}

void *CApplicationContext::getSingletonPointer(const std::string &singletonName)
{
	TSingletonRegistry::iterator it(_SingletonRegistry.find(singletonName));
	if (it != _SingletonRegistry.end())
		return it->second;

//	nlwarning("Can't find singleton '%s'", singletonName.c_str());
	return NULL;
}

void CApplicationContext::setSingletonPointer(const std::string &singletonName, void *ptr)
{
	nlassert(_SingletonRegistry.find(singletonName) == _SingletonRegistry.end());
	_SingletonRegistry[singletonName] = ptr;
}

void CApplicationContext::releaseSingletonPointer(const std::string &singletonName, void *ptr)
{
	nlassert(_SingletonRegistry.find(singletonName) != _SingletonRegistry.end());
	nlassert(_SingletonRegistry.find(singletonName)->second == ptr);
	_SingletonRegistry.erase(singletonName);
}


CLog *CApplicationContext::getErrorLog()
{
	return ErrorLog;
}

void CApplicationContext::setErrorLog(CLog *errorLog)
{
	ErrorLog = errorLog;
}

CLog *CApplicationContext::getWarningLog()
{
	return WarningLog;
}

void CApplicationContext::setWarningLog(CLog *warningLog)
{
	WarningLog = warningLog;
}

CLog *CApplicationContext::getInfoLog()
{
	return InfoLog;
}

void CApplicationContext::setInfoLog(CLog *infoLog)
{
	InfoLog = infoLog;
}

CLog *CApplicationContext::getDebugLog()
{
	return DebugLog;
}

void CApplicationContext::setDebugLog(CLog *debugLog)
{
	DebugLog = debugLog;
}

CLog *CApplicationContext::getAssertLog()
{
	return AssertLog;
}

void CApplicationContext::setAssertLog(CLog *assertLog)
{
	AssertLog = assertLog;
}

CMemDisplayer *CApplicationContext::getDefaultMemDisplayer()
{
	return DefaultMemDisplayer;
}

void CApplicationContext::setDefaultMemDisplayer(CMemDisplayer *memDisplayer)
{
	DefaultMemDisplayer = memDisplayer;
}

CMsgBoxDisplayer *CApplicationContext::getDefaultMsgBoxDisplayer()
{
	return DefaultMsgBoxDisplayer;
}

void CApplicationContext::setDefaultMsgBoxDisplayer(CMsgBoxDisplayer *msgBoxDisplayer)
{
	DefaultMsgBoxDisplayer = msgBoxDisplayer;
}

bool CApplicationContext::getDebugNeedAssert()
{
	return DebugNeedAssert;
}

void CApplicationContext::setDebugNeedAssert(bool needAssert)
{
	DebugNeedAssert = needAssert;
}

bool CApplicationContext::getNoAssert()
{
	return NoAssert;
}

void CApplicationContext::setNoAssert(bool noAssert)
{
	NoAssert = noAssert;
}

bool CApplicationContext::getAlreadyCreateSharedAmongThreads()
{
	return AlreadyCreateSharedAmongThreads;
}

void CApplicationContext::setAlreadyCreateSharedAmongThreads(bool b)
{
	AlreadyCreateSharedAmongThreads = b;
}

CLibraryContext::CLibraryContext(INelContext &applicationContext)
: _ApplicationContext(applicationContext)
{
	contextReady();
}


void *CLibraryContext::getSingletonPointer(const std::string &singletonName)
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	return _ApplicationContext.getSingletonPointer(singletonName);
}

void CLibraryContext::setSingletonPointer(const std::string &singletonName, void *ptr)
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	_ApplicationContext.setSingletonPointer(singletonName, ptr);
}

void CLibraryContext::releaseSingletonPointer(const std::string &singletonName, void *ptr)
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	_ApplicationContext.releaseSingletonPointer(singletonName, ptr);
}


CLog *CLibraryContext::getErrorLog()
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	return _ApplicationContext.getErrorLog();
}

void CLibraryContext::setErrorLog(CLog *errorLog)
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	_ApplicationContext.setErrorLog(errorLog);
}

CLog *CLibraryContext::getWarningLog()
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	return _ApplicationContext.getWarningLog();
}

void CLibraryContext::setWarningLog(CLog *warningLog)
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	_ApplicationContext.setWarningLog(warningLog);
}

CLog *CLibraryContext::getInfoLog()
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	return _ApplicationContext.getInfoLog();
}

void CLibraryContext::setInfoLog(CLog *infoLog)
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	_ApplicationContext.setInfoLog(infoLog);
}

CLog *CLibraryContext::getDebugLog()
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	return _ApplicationContext.getDebugLog();
}

void CLibraryContext::setDebugLog(CLog *debugLog)
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	_ApplicationContext.setDebugLog(debugLog);
}

CLog *CLibraryContext::getAssertLog()
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	return _ApplicationContext.getAssertLog();
}

void CLibraryContext::setAssertLog(CLog *assertLog)
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	_ApplicationContext.setAssertLog(assertLog);
}

CMemDisplayer *CLibraryContext::getDefaultMemDisplayer()
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	return _ApplicationContext.getDefaultMemDisplayer();
}

void CLibraryContext::setDefaultMemDisplayer(CMemDisplayer *memDisplayer)
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	_ApplicationContext.setDefaultMemDisplayer(memDisplayer);
}

CMsgBoxDisplayer *CLibraryContext::getDefaultMsgBoxDisplayer()
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	return _ApplicationContext.getDefaultMsgBoxDisplayer();
}

void CLibraryContext::setDefaultMsgBoxDisplayer(CMsgBoxDisplayer *msgBoxDisplayer)
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	_ApplicationContext.setDefaultMsgBoxDisplayer(msgBoxDisplayer);
}

bool CLibraryContext::getDebugNeedAssert()
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	return _ApplicationContext.getDebugNeedAssert();
}

void CLibraryContext::setDebugNeedAssert(bool needAssert)
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	_ApplicationContext.setDebugNeedAssert(needAssert);
}

bool CLibraryContext::getNoAssert()
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	return _ApplicationContext.getNoAssert();
}



void CLibraryContext::setNoAssert(bool noAssert)
{
//	nlassert(_ApplicationContext != NULL); 

	// just forward the call
	_ApplicationContext.setNoAssert(noAssert);
}

bool CLibraryContext::getAlreadyCreateSharedAmongThreads()
{
	return _ApplicationContext.getAlreadyCreateSharedAmongThreads();
}

void CLibraryContext::setAlreadyCreateSharedAmongThreads(bool b)
{
	_ApplicationContext.setAlreadyCreateSharedAmongThreads(b);
}

void initNelLibrary(NLMISC::CLibrary &lib)
{
	nlassert(lib.isLibraryLoaded());

	TInitLibraryFunc *funptrptr = reinterpret_cast<TInitLibraryFunc*>(lib.getSymbolAddress("libraryEntry"));
	nlassert(funptrptr != NULL);

	TInitLibraryFunc funptr = *funptrptr;

	// call the initialisation function
	funptr(NLMISC::INelContext::getInstance());
}



} // namespace NLMISC
