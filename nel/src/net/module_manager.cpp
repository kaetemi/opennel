/** \file module_manager.cpp
 * module manager implementation
 *
 * $Id: module_manager.cpp,v 1.2 2005/06/23 17:40:16 boucher Exp $
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


#include "stdnet.h"
#include "nel/misc/app_context.h"
#include "nel/misc/dynloadlib.h"
#include "nel/misc/command.h"
#include "nel/misc/path.h"
#include "nel/misc/twin_map.h"
#include "nel/misc/sstring.h"
#include "nel/net/module_manager.h"

#include "nel/net/module_gateway.h"
#include "nel/net/module.h"
#include "nel/net/module_socket.h"

using namespace std;
using namespace NLMISC;

// for init of the module manager (if not already done by the application)
NLMISC_CATEGORISED_COMMAND(net, initModuleManager, "force the initialisation of the module manager", "")
{
	// not really hard in fact :)
	NLNET::IModuleManager::getInstance();

	log.displayNL("Module manager is now initialised.");

	return true;
}


namespace NLNET
{
	/// Implementation class for module manager
	class CModuleManager : public IModuleManager, public ICommandsHandler
	{
		NLMISC_SAFE_SINGLETON_DECL(CModuleManager);
	public:

		struct TModuleLibraryInfo : public CRefCount
		{
			/// The file name of the library with access path
			std::string					FullPathLibraryName;
			/// The short name of the library, i.e not path, no decoration (.dll or  .so and NeL compilation mode tag)
			std::string					ShortLibraryName;
			/// The list of module factory name that are in use in the library.
			std::vector<std::string>	ModuleFactoryList;
			/// The library handler
			CLibrary					LibraryHandler;
			/// the NeL Module library interface pointer.
			CNelModuleLibrary			*ModuleLibrary;
		};

		typedef	map<std::string, CSmartPtr<TModuleLibraryInfo> >	TModuleLibraryInfos;
		/// Module library registry
		TModuleLibraryInfos		_ModuleLibraryRegistry;

		typedef std::map<std::string, IModuleFactory*>		TModuleFactoryRegistry;
		/// Module factory registry
		TModuleFactoryRegistry	_ModuleFactoryRegistry;

		typedef NLMISC::CTwinMap<std::string, TModulePtr>	TModuleInstances;
		/// Modules instances tracker
		TModuleInstances		_ModuleInstances;

		typedef NLMISC::CTwinMap<TModuleId, TModulePtr>		TModuleIds;
		/// Modules IDs tracker
		TModuleIds				_ModuleIds;

		/// Local module ID generator
		TModuleId				_LastGeneratedId;

		typedef NLMISC::CTwinMap<std::string, TModuleProxyPtr>	TModuleProxyInstances;
		/// Modules proxy instances tracker
		TModuleProxyInstances	_ModuleProxyInstances;

		typedef NLMISC::CTwinMap<TModuleId, TModuleProxyPtr>	TModuleProxyIds;
		/// Modules proxy IDs tracker
		TModuleProxyIds			_ModuleProxyIds;

		typedef map<string, IModuleSocket *>	TModuleSockets;
		/// Module socket registry
		TModuleSockets			_ModuleSocketsRegistry;


		///////////////////////////////////////////////////////////////////
		// Methods 
		///////////////////////////////////////////////////////////////////

		const std::string &getCommandHandlerName() const
		{
			static string moduleManagerName("moduleManager");
			return moduleManagerName;
		}


		CModuleManager() 
			:_LastGeneratedId(0)
		{
			registerCommandsHandler();

			// register local module factory
			TLocalModuleFactoryRegistry &lmfr = TLocalModuleFactoryRegistry::instance();
			addModuleFactoryRegistry(lmfr);
		}

		~CModuleManager()
		{
			// unload any loaded module library
			while (!_ModuleLibraryRegistry.empty())
			{
				TModuleLibraryInfo *mli = _ModuleLibraryRegistry.begin()->second;
				unloadModuleLibrary(mli->ShortLibraryName);
			}

			// delete any lasting modules
			while (!_ModuleInstances.getAToBMap().empty())
			{
				deleteModule(_ModuleInstances.getAToBMap().begin()->second);
			}

			// there should not be proxies or gateway lasting
			nlassert(_ModuleProxyInstances.getAToBMap().empty());
		}

		void releaseInstance()
		{
			delete this;
			_Instance = NULL;
		}



		void addModuleFactoryRegistry(TLocalModuleFactoryRegistry &moduleFactoryRegistry)
		{
			vector<string> moduleList;
			moduleFactoryRegistry.fillFactoryList(moduleList);
			// fill the module factory registry
			vector<string>::iterator first(moduleList.begin()), last(moduleList.end());
			for (; first != last; ++first)
			{
				if (_ModuleFactoryRegistry.find(*first) != _ModuleFactoryRegistry.end())
				{
					// a module class of that name already exist
					nlwarning("CModuleManger : add module factory : module class '%s' that is already registered",
						first->c_str());
				}
				else
				{
					// store the factory
					nlinfo("Adding module '%s' factory", first->c_str());
					_ModuleFactoryRegistry.insert(make_pair(*first, moduleFactoryRegistry.getFactory(*first)));
				}
			}
		}

		virtual bool loadModuleLibrary(const std::string &libraryName)
		{
			// build the short name
			string path = CFile::getPath(libraryName);
			string shortName = CLibrary::cleanLibName(libraryName);

			if (_ModuleLibraryRegistry.find(shortName) != _ModuleLibraryRegistry.end())
			{
				// this lib is already loaded !
				nlwarning("CModuleManager : trying to load library '%s' in '%s'\n"
					"but it is already loaded as '%s', ignoring.",
					shortName.c_str(),
					libraryName.c_str(),
					_ModuleLibraryRegistry[shortName]->FullPathLibraryName.c_str());
				return false;
			}

			// now, load the library
			string fullName = path+"/"+CLibrary::makeLibName(shortName);
			auto_ptr<TModuleLibraryInfo>	mli = auto_ptr<TModuleLibraryInfo>(new TModuleLibraryInfo);
			if (!mli->LibraryHandler.loadLibrary(fullName, false, true, true))
			{
				nlwarning("CModuleManager : failed to load the library '%s' in '%s'",
					shortName.c_str(),
					fullName.c_str());
				return false;
			}
			// the lib is loaded, check that it is a 'pure' nel library
			if (!mli->LibraryHandler.isLibraryPure())
			{
				nlwarning("CModuleManager : the library '%s' is not a pure Nel library",
					shortName.c_str());
				return false;
			}
			// Check that the lib is a pure module library
			CNelModuleLibrary *modLib = dynamic_cast<CNelModuleLibrary *>(mli->LibraryHandler.getNelLibraryInterface());
			if (modLib == NULL)
			{
				nlwarning("CModuleManager : the library '%s' is not a pure Nel Module library",
					shortName.c_str());
				return false;
			}

			// ok, all is fine ! we can store the loaded library info

			mli->FullPathLibraryName = fullName;
			TLocalModuleFactoryRegistry &lmfr = modLib->getLocalModuleFactoryRegistry();
			lmfr.fillFactoryList(mli->ModuleFactoryList);
			mli->ModuleLibrary = modLib;
			mli->ShortLibraryName = shortName;

			pair<TModuleLibraryInfos::iterator, bool> ret = _ModuleLibraryRegistry.insert(make_pair(shortName, mli.release()));
			if (!ret.second)
			{
				nlwarning("CModuleManager : failed to store module library information !");
				return false;
			}

			// fill the module factory registry
			addModuleFactoryRegistry(lmfr);

			return true;
		}

		virtual bool unloadModuleLibrary(const std::string &libraryName)
		{
			string shorName = CLibrary::cleanLibName(libraryName);
			TModuleLibraryInfos::iterator it(_ModuleLibraryRegistry.find(shorName));
			if (it == _ModuleLibraryRegistry.end())
			{
				nlwarning("CModuleManager : failed to unload library '%s' : unknown or not loaded library", shorName.c_str());
				return false;
			}

			// by erasing this entry, we delete the CLibrary object that hold the library
			// module, thus destroying any factory the belong into and deleting the
			// instantiated module !
			// Wow, that's pretty much for a single line ;)
			_ModuleLibraryRegistry.erase(it);

			return true;
		}

		/** Register a module factory in the manager
		 */
//		virtual void registerModuleFactory(class IModuleFactory *moduleFactory)
//		{
//			nlstop;
//		}
		/** Unregister a module factory
		*/
		virtual void unregisterModuleFactory(class IModuleFactory *moduleFactory)
		{
			// we need to remove the factory from the registry
			TModuleFactoryRegistry::iterator it(_ModuleFactoryRegistry.find(moduleFactory->getModuleClassName()));

			if (it == _ModuleFactoryRegistry.end() || it->second != moduleFactory)
			{
				nlwarning("The module factory for class '%s' in not registered.", moduleFactory->getModuleClassName().c_str());
				return;
			}

			nlinfo("ModuleManager : unregistering factory for module class '%s'", moduleFactory->getModuleClassName().c_str());
				
			// remove the factory
			_ModuleFactoryRegistry.erase(it);
		}
		

		/** Fill the vector with the list of available module.
		 *	Note that the vector is not cleared before being filled.
		 */
		virtual void getAvailableModuleList(std::vector<std::string> &moduleList)
		{
			TModuleFactoryRegistry::iterator first(_ModuleFactoryRegistry.begin()), last(_ModuleFactoryRegistry.end());
			for (; first != last; ++first)
			{
				moduleList.push_back(first->first);
			}
		}
		
		/** Create a new module instance.
		 *	The method create a module of the specified class with the
		 *	specified local name.
		 *	The class MUST be available in the factory and the
		 *	name MUST be unique OR empty.
		 *	If the name is empty, the method generate a name using
		 *	the module class and a number.
		 */
		virtual IModule *createModule(const std::string &className, const std::string &localName, const std::string &paramString)
		{
			TModuleFactoryRegistry::iterator it(_ModuleFactoryRegistry.find(className));
			if (it == _ModuleFactoryRegistry.end())
			{
				nlwarning("createModule : unknown module class '%s'", className.c_str());
				return NULL;
			}

			string moduleName = localName;
			if (moduleName.empty())
			{
				// we need to generate a name
				uint i=0;
				do 
				{
					moduleName = className+toString(i++);
				} while (_ModuleInstances.getB(moduleName) != NULL);
			}
			else
			{
				// check that the module name is unique
				if (_ModuleInstances.getB(moduleName) != NULL)
				{
					nlwarning("createModule : the name '%s' is already used by another module, can't instantiate the module", moduleName.c_str());
					return NULL;
				}
			}

			IModuleFactory *mf = it->second;
			// sanity check
			nlassert(mf->getModuleClassName() == className);
			auto_ptr<IModule> module = auto_ptr<IModule>(mf->createModule());
			if (module.get() == NULL)
			{
				nlwarning("createModule : factory failed to create a module instance for class '%s'", className.c_str());

				return NULL;
			}

			CModuleBase *modBase = dynamic_cast<CModuleBase*>(module.get());
			if (modBase == NULL)
			{
				nlwarning("Invalid module returned by factory for class '%s'", className.c_str());
				return NULL;
			}

			// init the module basic data
			modBase->_ModuleName = moduleName;
			modBase->_ModuleId = ++_LastGeneratedId;

			// init the module with parameter string
			TModuleInitInfo	mii;
			mii.parseParamList(paramString);
			module->initModule(mii);

			// store the module in the manager
			_ModuleInstances.add(moduleName, module.get());
			_ModuleIds.add(modBase->_ModuleId, module.get());

			// ok, all is fine, return the module
			return module.release();
		}

		void deleteModule(IModule *module)
		{
			nlassert(module != NULL);

			// remove module from trackers
			nlassert(_ModuleInstances.getA(module) != NULL);
			nlassert(_ModuleIds.getA(module) != NULL);

			_ModuleInstances.removeWithB(module);
			_ModuleIds.removeWithB(module);

			// unplug the module if needed
			vector<IModuleSocket *> sockets;
			module->getPluggedSocketList(sockets);
			for (uint i=0; i<sockets.size(); ++i)
			{
				module->unplugModule(sockets[i]);
			}

			// ask the factory to delete the module
			CModuleBase *modBase = dynamic_cast<CModuleBase *>(module);
			nlassert(modBase != NULL);

			modBase->getFactory()->deleteModule(module);
		}


		/** Lookup in the created module for a module having the
		 *	specified local name.
		 */
		virtual IModule *getLocalModule(const std::string &moduleName)
		{
			TModuleInstances::TAToBMap::const_iterator it(_ModuleInstances.getAToBMap().find(moduleName));

			if (it == _ModuleInstances.getAToBMap().end())
				return NULL;
			else
				return it->second;
		}

		virtual void updateModules()
		{
			// module are updated in creation order (i.e in module ID order)
			TModuleIds::TAToBMap::const_iterator first(_ModuleIds.getAToBMap().begin()), last(_ModuleIds.getAToBMap().end());
			for (; first != last; ++first)
			{
				first->second->onModuleUpdate();
			}
		}

		/** Lookup in the created socket for a socket having the
		 *	specified local name.
		 */
		virtual IModuleSocket *getModuleSocket(const std::string &socketName)
		{
			TModuleSockets::iterator it(_ModuleSocketsRegistry.find(socketName));
			if (it == _ModuleSocketsRegistry.end())
				return NULL;
			else
				return it->second;
		}
		/** Register a socket in the manager.
		 */
		virtual void registerModuleSocket(IModuleSocket *moduleSocket)
		{
			nlassert(moduleSocket != NULL);
			TModuleSockets::iterator it(_ModuleSocketsRegistry.find(moduleSocket->getSocketName()));
			nlassert(it == _ModuleSocketsRegistry.end());

			nldebug("Registering module socket '%s'", moduleSocket->getSocketName().c_str());
			_ModuleSocketsRegistry.insert(make_pair(moduleSocket->getSocketName(), moduleSocket));
		}
		/** UrRegister a socket in the manager.
		 */
		virtual void unregisterModuleSocket(IModuleSocket *moduleSocket)
		{
			nlassert(moduleSocket != NULL);
			TModuleSockets::iterator it(_ModuleSocketsRegistry.find(moduleSocket->getSocketName()));
			nlassert(it != _ModuleSocketsRegistry.end());

			nldebug("Unregistering module socket '%s'", moduleSocket->getSocketName().c_str());

			_ModuleSocketsRegistry.erase(it);
		}

		/** Lookup in the created gateway for a gateway having the
		 *	specified local name.
		 */
		virtual IModuleGateway *getModuleGateway(const std::string &gatewayName)
		{
			nlstop;
			return NULL;
		}
		/** Register a gateway in the manager.
		 */
		virtual void registerModuleGateway(IModuleGateway *moduleGateway)
		{
			nlstop;
		}
		/** UrRegister a socket in the manager.
		 */
		virtual void unregisterModuleGateway(IModuleGateway *moduleGateway)
		{
			nlstop;
		}

		/** Get a module proxy with the module ID */
		virtual TModuleProxyPtr getModuleProxy(TModuleId moduleId)
		{
			nlstop;
			return TModuleProxyPtr(NULL);
		}

		/** Called by a module that is begin destroyed.
		 *	This remove module information from the
		 *	the manager
		 */
//		virtual void onModuleDeleted(IModule *module)
//		{
//			// not needed ? to remove
//		}

		virtual IModuleProxy *createModuleProxy(
			IModuleGateway *gateway, 
			const std::string &moduleClassName, 
			const std::string &moduleFullyQualifiedName,
			const TModuleGatewayProxyPtr &foreignGateway,
			TModuleId foreignModuleId)
		{
			auto_ptr<CModuleProxy> modProx = auto_ptr<CModuleProxy>(new CModuleProxy(++_LastGeneratedId, moduleClassName, moduleFullyQualifiedName));
			modProx->_Gateway = gateway;
			modProx->_ForeignGateway = foreignGateway;
			modProx->_ForeignModuleId = foreignModuleId;

			nldebug("Creating module proxy (ID : %u, foreign ID : %u, name : '%s', class : '%s'",
				modProx->getModuleId(),
				modProx->_ForeignModuleId,
				modProx->getModuleName().c_str(),
				modProx->getModuleClassName().c_str());

			_ModuleProxyInstances.add(moduleFullyQualifiedName, TModuleProxyPtr(modProx.get()));
			_ModuleProxyIds.add(modProx->getModuleId(), TModuleProxyPtr(modProx.get()));

			return modProx.release();
		}

		virtual void releaseModuleProxy(TModuleId moduleId)
		{
			nldebug("Releasing module proxy (ID : %u)", moduleId);
			TModuleProxyIds::TAToBMap::const_iterator it(_ModuleProxyIds.getAToBMap().find(moduleId));
			nlassert(it != _ModuleProxyIds.getAToBMap().end());
			CRefPtr<IModuleProxy> sanityCheck(it->second.getPtr());

			// remove the smart ptr, must delete the proxy
			_ModuleProxyInstances.removeWithB(it->second);
			_ModuleProxyIds.removeWithB(it->second);
			
			nlassert(sanityCheck == NULL);
		}


		NLMISC_COMMAND_HANDLER_TABLE_BEGIN(CModuleManager)
			NLMISC_COMMAND_HANDLER_ADD(CModuleManager, dumpModuleManager, "dump various information about module manager state", "");
			NLMISC_COMMAND_HANDLER_ADD(CModuleManager, loadLibrary, "load a pure nel library module (give the path if needed and only the undecorated lib name)", "[path]<undecoratedLibName>");
			NLMISC_COMMAND_HANDLER_ADD(CModuleManager, unloadLibrary, "unload a pure nel library module (give the undecorated name, any path will be removed", "<undecoratedLibName>");
			NLMISC_COMMAND_HANDLER_ADD(CModuleManager, createModule, "create a new module instance", "<moduleClass> <instanceName> [*<moduleArg>]");
			NLMISC_COMMAND_HANDLER_ADD(CModuleManager, deleteModule, "delete a module instance", "<instanceName>");
		NLMISC_COMMAND_HANDLER_TABLE_END

		NLMISC_CLASS_COMMAND_DECL(deleteModule)
		{
			if (args.size() != 1)
				return false;

			TModulePtr const *module = _ModuleInstances.getB(args[0]);
			if (module == NULL)
			{
				log.displayNL("Unknow module '%s'", args[0].c_str());
				return false;
			}

			nlinfo("Deleting module '%s'", args[0].c_str());

			CRefPtr<IModule>	sanityCheck(*module);
			deleteModule(*module);
			if (sanityCheck != NULL)
			{
				log.displayNL("Failed to delete the module instance !");
				return false;
			}
			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(unloadLibrary)
		{
			if (args.size() != 1)
				return false;

			unloadModuleLibrary(args[0]);

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(loadLibrary)
		{
			if (args.size() < 1)
				return false;

			CSString libName = rawCommandString;
			// remove the command name
			libName.strtok(" \t");

			// load the library
			return this->loadModuleLibrary(libName);
		}

		NLMISC_CLASS_COMMAND_DECL(createModule)
		{
			if (args.size() < 2)
				return false;

			CSString moduleArgs = rawCommandString;
			// remove the command name
			moduleArgs.strtok(" \t");
			// retrieve module class
			string moduleClass = moduleArgs.strtok(" \t");
			// retrieve module instance name
			string moduleName = moduleArgs.strtok(" \t");

			nlinfo("Creating module '%s' of class '%s' with params '%s'",
				moduleName.c_str(),
				moduleClass.c_str(),
				moduleArgs.c_str());
			// create the module instance
			IModule *module = createModule(moduleClass, moduleName, moduleArgs);

			return module != NULL;
		}

		NLMISC_CLASS_COMMAND_DECL(dumpModuleManager)
		{
			log.displayNL("Dumping CModuleManager internal states :");
			{
				std::vector<std::string> moduleList;
				TLocalModuleFactoryRegistry::instance().fillFactoryList(moduleList);
				log.displayNL(" List of %u local modules :", moduleList.size());
				for (uint i=0; i<moduleList.size(); ++i)
				{
					if (_ModuleFactoryRegistry.find(moduleList[i]) == _ModuleFactoryRegistry.end())
					{
						log.displayNL("    %s : UNAVAILABLE !", moduleList[i].c_str());
					}
					else
					{
						log.displayNL("    %s : OK", moduleList[i].c_str());
					}
				}
			}
			{
				log.displayNL(" List of %u loaded module libraries :", _ModuleLibraryRegistry.size());
				TModuleLibraryInfos::iterator first(_ModuleLibraryRegistry.begin()), last(_ModuleLibraryRegistry.end());
				for (; first != last; ++first)
				{
					TModuleLibraryInfo &mli = *(first->second);
					log.displayNL("  Library '%s' loaded from '%s', contains %u modules :",
						first->first.c_str(),
						mli.FullPathLibraryName.c_str(),
						mli.ModuleFactoryList.size());
					{
						vector<string>::iterator f2(mli.ModuleFactoryList.begin()), l2(mli.ModuleFactoryList.end());
						for (uint i=0; i<mli.ModuleFactoryList.size(); ++i)
						{
							if (_ModuleFactoryRegistry.find(mli.ModuleFactoryList[i]) == _ModuleFactoryRegistry.end())
							{
								log.displayNL("    %s : UNAVAILABLE !", mli.ModuleFactoryList[i].c_str());
							}
							else
							{
								log.displayNL("    %s : OK", mli.ModuleFactoryList[i].c_str());
							}
						}
					}
				}
			}
			{
				log.displayNL(" List of %u module instances :", _ModuleInstances.getAToBMap().size());
				TModuleInstances::TAToBMap::const_iterator first(_ModuleInstances.getAToBMap().begin()), last(_ModuleInstances.getAToBMap().end());
				for (; first != last; ++first)
				{
					IModule *module = first->second;
					log.displayNL("    ID:%5u : '%s' \tclass '%s'", 
						module->getModuleId(),
						module->getModuleName().c_str(),
						module->getModuleClassName().c_str());
				}
			}
			{
				log.displayNL(" List of %u module proxies :", _ModuleProxyInstances.getAToBMap().size());
				TModuleProxyInstances::TAToBMap::const_iterator first(_ModuleProxyInstances.getAToBMap().begin()), last(_ModuleProxyInstances.getAToBMap().end());
				for (; first != last; ++first)
				{
					const CModuleProxy *modProx = dynamic_cast<const CModuleProxy*>(first->second.getPtr());
					if(modProx != NULL)
					{
						log.displayNL("    ID:%5u (Foreign ID : %u) : '%s' \tclass '%s'", 
							modProx->getModuleId(),
							modProx->_ForeignModuleId,
							modProx->getModuleName().c_str(),
							modProx->getModuleClassName().c_str());
					}
					else
					{
						log.displayNL("    ID:%5u (Unknown proxy type) : '%s' \tclass '%s'",
							modProx->getModuleId(),
							modProx->getModuleName().c_str(),
							modProx->getModuleClassName().c_str());
					}
				}
			}
			return true;
		}

	};

	IModuleManager &IModuleManager::getInstance()
	{
		return CModuleManager::getInstance();
	}

	NLMISC_SAFE_SINGLETON_IMPL(CModuleManager);


	extern void forceGatewayLink();

	void forceLink()
	{
		forceGatewayLink();
	}

} // namespace NLNET
