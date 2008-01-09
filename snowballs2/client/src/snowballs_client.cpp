/** \file snowballs_client.cpp
 * CSnowballsClient
 * 
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * $Id$
 */

#include <nel/misc/types_nl.h>
#include "snowballs_client.h"
#include "snowballs_config.h"

#include "component_manager.h"

// temp
#include "driver_component.h"
#include "loading_component.h"
#include "hello_world_component.h"

#include <nel/misc/config_file.h>
#include <nel/misc/path.h>
#include <nel/misc/debug.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/i18n.h>

#include <nel/3d/u_driver.h>

using namespace std;
using namespace NLMISC;

namespace SBCLIENT {

CSnowballsClient::CSnowballsClient() 
: _FileDisplayer(NULL), _ComponentManager(NULL), 
  _ConfigFile(NULL), _DriverComponent(NULL), _Driver(NULL), 
  _CurrentState(Invalid), _NextState(Load),
  _HasCore(false), _HasLogin(false), _HasIngame(false),
  _HasOnline(false), _HasOffline(false),
  _PlayOnline(false) // do not change these
{
	// use log.log if NEL_LOG_IN_FILE and SBCLIENT_USE_LOG_LOG defined as 1
	createDebug(NULL, SBCLIENT_USE_LOG_LOG, false);
	srand((uint)CTime::getLocalTime());

#if SBCLIENT_USE_LOG
	// create snowballs_client.log
	// filedisplayer only deletes the 001 etc
	if (CFile::isExists(SBCLIENT_LOG_FILE))
		CFile::deleteFile(SBCLIENT_LOG_FILE);
	// initialize the log file
	_FileDisplayer = new CFileDisplayer();
	_FileDisplayer->setParam(SBCLIENT_LOG_FILE, SBCLIENT_ERASE_LOG);
	DebugLog->addDisplayer(_FileDisplayer);
	InfoLog->addDisplayer(_FileDisplayer);
	WarningLog->addDisplayer(_FileDisplayer);
	AssertLog->addDisplayer(_FileDisplayer);
	ErrorLog->addDisplayer(_FileDisplayer);
#endif	
	nlinfo("Starting Snowballs!");
	// end of debug/log initialization
}

CSnowballsClient::~CSnowballsClient()
{
	disableAll(); // just to be sure

	// begin of debug/log destruction
	nlinfo("See you later!");
#if SBCLIENT_USE_LOG
	nlassert(_FileDisplayer); delete _FileDisplayer;
#endif
}

int CSnowballsClient::run()
{
	// load and unload required components for the current game state
	// and call the componentmanager update until the state changes
SwitchState:
	nlinfo("Switching to the next game state");
	if (_CurrentState == _NextState)
	{
		nlwarning("NextGameState wasn't changed");
	}
	else
	{
		switch(_CurrentState)
		{
		case Game:
			disableOnline();
			disableOffline();
			break;
		}
		switch(_NextState)
		{
		case Load:
			enableCore();
			break;
		case Reset:
			// displayLoadingState("Reset");
			disableAll();
			break;
		case Exit:
			// displayLoadingState("See you later!");
			disableAll();
			break;
		case Login:
			enableCore(); // core is required
			enableLogin(); // login is required
			break;
		case Game:
			enableCore(); // core is required
			disableLogin(); //login can be released
			enableIngame(); // ingame is required
			if (_PlayOnline) enableOnline(); // connection is required
			else enableOffline();
			break;
		}
	}
	_CurrentState = _NextState;
	switch(_CurrentState)
	{
	case Load: // switch to the default state
		_NextState = Login;
		break;
	case Reset: // used to reset everything
		_NextState = Load;
		break;
	case Exit: // exit the loop
		return EXIT_SUCCESS;
	default:
		do { if (!_Driver->isActive()) 
				{ _NextState = Exit; break; }
			_ComponentManager->update();
			_ComponentManager->render();
		} while (_CurrentState == _NextState);
		break;
	}
	goto SwitchState;
}

void CSnowballsClient::enableCore()
{
	if (!_HasCore)
	{
		_HasCore = true;
		// init configuration file
		_ConfigFile = new CConfigFile();
		_ConfigFile->load(SBCLIENT_CONFIG_FILE);

		// set the search paths (kinda important)
		CConfigFile::CVar *var;
		var = &_ConfigFile->getVar(SBCLIENT_NAME "_SearchPaths");
		uint varsize = var->size();
		for (uint i = 0; i < varsize; ++i)
			CPath::addSearchPath(var->asString(i), true, false);
		var = &_ConfigFile->getVar(SBCLIENT_NAME "_RemapExtensions");
		for (uint i = 0; i < varsize; i += 2)
			CPath::remapExtension(var->asString(i), var->asString(i + 1), true);

		// set the language code
		CI18N::load(_ConfigFile->getVar(SBCLIENT_NAME "_LanguageCode").asString());

		// initialize the manager
		_ComponentManager = new CComponentManager(_ConfigFile);

		

		



		// dynamic core, temp
		_LoadingComponent = new CLoadingComponent(
			_ComponentManager, "LoadingScreen", _LoadingScreen);
		_ComponentManager->registerComponent(_LoadingComponent);

		_DriverComponent = new CDriverComponent(
			_ComponentManager, "Graphics", _LoadingScreen);
		_ComponentManager->registerComponent(_DriverComponent);
		_ComponentManager->registerUpdate(_DriverComponent, 100);
		_Driver = _DriverComponent->getDriver();

		_HelloWorldComponent = new CHelloWorldComponent(
			_ComponentManager, "HelloWorld", _LoadingScreen);
		_ComponentManager->registerComponent(_HelloWorldComponent);
		_ComponentManager->registerRender(_HelloWorldComponent, 200);

	}
}

void CSnowballsClient::disableCore()
{
	if (_HasCore)
	{
		// dynamic core, temp
		_ComponentManager->unregisterComponent(_HelloWorldComponent);
		delete _HelloWorldComponent;

		_ComponentManager->unregisterComponent(_DriverComponent);
		delete _DriverComponent;

		_ComponentManager->unregisterComponent(_LoadingComponent);
		delete _LoadingComponent;






		// if they're null, something must've seriously went wrong at some point	
		// release the component manager
		nlassert(_ComponentManager); delete _ComponentManager;

		// release of the language data is done when loading new file
		//CI18N::...

		// release the configuration file
		nlassert(_ConfigFile); delete _ConfigFile;

		// release the search paths etc
		CPath::releaseInstance();

		_HasCore = false;
	}
}

void CSnowballsClient::enableLogin()
{
	if (!_HasLogin)
	{		
		_HasLogin = true;

		// ...
	}
}

void CSnowballsClient::disableLogin()
{
	if (_HasLogin)
	{
		// ...
		
		_HasLogin = false;
	}
}

void CSnowballsClient::enableIngame()
{
	if (!_HasIngame)
	{		
		_HasIngame = true;

		// ...
	}
}

void CSnowballsClient::disableIngame()
{
	if (_HasIngame)
	{
		// ...
		
		_HasIngame = false;
	}
}

void CSnowballsClient::enableOnline()
{
	if (!_HasOnline)
	{		
		_HasOnline = true;

		// ...
	}
}

void CSnowballsClient::disableOnline()
{
	if (_HasOnline)
	{
		// ...
		
		_HasOnline = false;
	}
}

void CSnowballsClient::enableOffline()
{
	if (!_HasOffline)
	{		
		_HasOffline = true;

		// ...
	}
}

void CSnowballsClient::disableOffline()
{
	if (_HasOffline)
	{
		// ...
		
		_HasOffline = false;
	}
}

void CSnowballsClient::disableAll()
{
	disableOnline();
	disableOffline();
	disableIngame();
	disableLogin();
	disableCore();
}

}

#ifdef NL_OS_WINDOWS
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdline, int nCmdShow)
#else
int main(int argc, char **argv)
#endif
{
	SBCLIENT::CSnowballsClient *client = new SBCLIENT::CSnowballsClient();
	int result = client->run(); 
	delete client;
	return result;
}