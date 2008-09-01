/**
 * \file welcome_service_itf.h
 * \brief WARNING : this is a generated file, don't change it !
 * \date 2005-10-03 05:15GMT
 * \author Nevrax Ltd. (Nevrax Ltd.)
 * Collection of the interfaces used by the welcome service module.
 * 
 * $Id$
 */

/* 
 * Copyright (C) 2005  Nevrax Ltd. (Nevrax Ltd.)
 * 
 * This file is part of NEVRAX NeL Network Services.
 * NEVRAX NeL Network Services is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either
 * version 2 of the License, or (at your option) any later version.
 * 
 * NEVRAX NeL Network Services is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA.
 */

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !         
/////////////////////////////////////////////////////////////////
#ifndef WS_WELCOME_SERVICE_ITF_H
#define WS_WELCOME_SERVICE_ITF_H
#include <nel/misc/types_nl.h>

// User header
#include "user_role.h"
#include <nel/net/login_cookie.h>

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/log.h>
#include <nel/net/message.h>
#include <nel/net/module.h>
#include <nel/net/module_builder_parts.h>
#include <nel/net/module_message.h>
#include <nel/net/module_gateway.h>

// STL includes
#include <string>

namespace WS {

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !         
/////////////////////////////////////////////////////////////////
/* 
 * \brief CWelcomeServiceSkel
 * \date 2005-10-03 05:15GMT
 * \author Nevrax Ltd. (Nevrax Ltd.)
 * Interface for messages going from the global ring session manager
 * and login service to the shard's welcome service. This is a
 * subsection of the WelcomeService module (CWelcomeServiceMod).
 * Skeleton for implementing the receiving end of this interface into
 * a module.
 */
class CWelcomeServiceSkel
{
public:
	// The interceptor type.
	typedef NLNET::CInterceptorForwarder<CWelcomeServiceSkel> TInterceptor;
	
protected:
	CWelcomeServiceSkel()
	{
		// Do early run time check for message table.
		getMessageHandlers();
	}
	virtual ~CWelcomeServiceSkel() { }
	
	void init(NLNET::IModule *module)
	{
		_Interceptor.init(this, module);
	}
	
	// Unused interceptors.
	std::string fwdBuildModuleManifest() const { return std::string(); }
	void fwdOnModuleUp(NLNET::IModuleProxy *moduleProxy) { };
	void fwdOnModuleDown(NLNET::IModuleProxy *moduleProxy) { };
	void fwdOnModuleSecurityChange(NLNET::IModuleProxy *moduleProxy) { };
	
	// Process module message interceptor.
	bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	
private:
	typedef void (CWelcomeServiceSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	typedef std::map<std::string, TMessageHandler> TMessageHandlerMap;
	
	const TMessageHandlerMap &getMessageHandlers() const;
	
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !         
	/////////////////////////////////////////////////////////////////
	void welcomeUser_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);
	void disconnectUser_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);
	
	// Declare one interceptor member of the skeleton.
	TInterceptor _Interceptor;
	
	// Declare the interceptor forwarder as friend of this class.
	friend class NLNET::CInterceptorForwarder<CWelcomeServiceSkel>;
	
public:
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !         
	/////////////////////////////////////////////////////////////////
	/// Ask the welcome service to welcome a character.
	virtual void welcomeUser(NLNET::IModuleProxy *sender, uint32 charId, const std::string &userName, const NLNET::CLoginCookie &cookie, const std::string &priviledge, const std::string &exPriviledge, TUserRole mode, uint32 instanceId) =0;
	/// Ask the welcome service to disconnect a user.
	virtual void disconnectUser(NLNET::IModuleProxy *sender, uint32 userId) =0;
	
}; /* class CWelcomeServiceSkel */

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !         
/////////////////////////////////////////////////////////////////
/* 
 * \brief CWelcomeServiceProxy
 * \date 2005-10-03 05:15GMT
 * \author Nevrax Ltd. (Nevrax Ltd.)
 * Interface for messages going from the global ring session manager
 * and login service to the shard's welcome service. This is a
 * subsection of the WelcomeService module (CWelcomeServiceMod).
 * Proxy for sending messages trough the proxy of a module that
 * implemented this interface. If the target module is local, the
 * messages will be relayed synchronously to the local module.
 */
class CWelcomeServiceProxy
{
	/// Smart pointer on the module proxy
	NLNET::TModuleProxyPtr _ModuleProxy;
	
	// Pointer on the local module that implement the interface (if the proxy is for a local module)
	NLNET::TModulePtr _LocalModule;
	// Direct pointer on the server implementation interface for collocated module
	CWelcomeServiceSkel *_LocalModuleSkel;
	
public:
	CWelcomeServiceProxy(NLNET::IModuleProxy *proxy)
	{
		_ModuleProxy = proxy;
		// Initialize collocated servant interface.
		if (proxy->getModuleDistance() == 0)
		{
			_LocalModule = proxy->getLocalModule();
			nlassert(_LocalModule != NULL);
			CWelcomeServiceSkel::TInterceptor *interceptor = NULL;
			interceptor = static_cast<NLNET::CModuleBase *>(_LocalModule.getPtr())->getInterceptor(interceptor);
			nlassert(interceptor != NULL);
			_LocalModuleSkel = interceptor->getParent();
			nlassert(_LocalModuleSkel != NULL);
		}
		else _LocalModuleSkel = 0;
	}
	virtual ~CWelcomeServiceProxy() { }
	
	NLNET::IModuleProxy *getModuleProxy()
	{
		return _ModuleProxy;
	}
	
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !         
	/////////////////////////////////////////////////////////////////
	/// Ask the welcome service to welcome a character.
	void welcomeUser(NLNET::IModule *sender, uint32 charId, const std::string &userName, const NLNET::CLoginCookie &cookie, const std::string &priviledge, const std::string &exPriviledge, TUserRole mode, uint32 instanceId);
	/// Ask the welcome service to disconnect a user.
	void disconnectUser(NLNET::IModule *sender, uint32 userId);
	
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !         
	/////////////////////////////////////////////////////////////////
	/// Ask the welcome service to welcome a character.
	/// Message serializer. Return the message received in reference for easier integration.
	static const NLNET::CMessage &buildMessageFor_welcomeUser(NLNET::CMessage &__message, uint32 charId, const std::string &userName, const NLNET::CLoginCookie &cookie, const std::string &priviledge, const std::string &exPriviledge, TUserRole mode, uint32 instanceId);
	/// Ask the welcome service to disconnect a user.
	/// Message serializer. Return the message received in reference for easier integration.
	static const NLNET::CMessage &buildMessageFor_disconnectUser(NLNET::CMessage &__message, uint32 userId);
	
}; /* class CWelcomeServiceProxy */

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !         
/////////////////////////////////////////////////////////////////
/* 
 * \brief CLoginServiceSkel
 * \date 2005-10-03 05:15GMT
 * \author Nevrax Ltd. (Nevrax Ltd.)
 * Interface for messages going from the shard's welcome service to
 * the global login service. This is a subsection of the LoginService
 * module (CLoginServiceMod).
 * Skeleton for implementing the receiving end of this interface into
 * a module.
 */
class CLoginServiceSkel
{
public:
	// The interceptor type.
	typedef NLNET::CInterceptorForwarder<CLoginServiceSkel> TInterceptor;
	
protected:
	CLoginServiceSkel()
	{
		// Do early run time check for message table.
		getMessageHandlers();
	}
	virtual ~CLoginServiceSkel() { }
	
	void init(NLNET::IModule *module)
	{
		_Interceptor.init(this, module);
	}
	
	// Unused interceptors.
	std::string fwdBuildModuleManifest() const { return std::string(); }
	void fwdOnModuleUp(NLNET::IModuleProxy *moduleProxy) { };
	void fwdOnModuleDown(NLNET::IModuleProxy *moduleProxy) { };
	void fwdOnModuleSecurityChange(NLNET::IModuleProxy *moduleProxy) { };
	
	// Process module message interceptor.
	bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	
private:
	typedef void (CLoginServiceSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	typedef std::map<std::string, TMessageHandler> TMessageHandlerMap;
	
	const TMessageHandlerMap &getMessageHandlers() const;
	
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !         
	/////////////////////////////////////////////////////////////////
	void pendingUserLost_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);
	
	// Declare one interceptor member of the skeleton.
	TInterceptor _Interceptor;
	
	// Declare the interceptor forwarder as friend of this class.
	friend class NLNET::CInterceptorForwarder<CLoginServiceSkel>;
	
public:
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !         
	/////////////////////////////////////////////////////////////////
	/// An awaited user did not connect before the allowed timeout expire.
	virtual void pendingUserLost(NLNET::IModuleProxy *sender, const NLNET::CLoginCookie &cookie) =0;
	
}; /* class CLoginServiceSkel */

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !         
/////////////////////////////////////////////////////////////////
/* 
 * \brief CLoginServiceProxy
 * \date 2005-10-03 05:15GMT
 * \author Nevrax Ltd. (Nevrax Ltd.)
 * Interface for messages going from the shard's welcome service to
 * the global login service. This is a subsection of the LoginService
 * module (CLoginServiceMod).
 * Proxy for sending messages trough the proxy of a module that
 * implemented this interface. If the target module is local, the
 * messages will be relayed synchronously to the local module.
 */
class CLoginServiceProxy
{
	/// Smart pointer on the module proxy
	NLNET::TModuleProxyPtr _ModuleProxy;
	
	// Pointer on the local module that implement the interface (if the proxy is for a local module)
	NLNET::TModulePtr _LocalModule;
	// Direct pointer on the server implementation interface for collocated module
	CLoginServiceSkel *_LocalModuleSkel;
	
public:
	CLoginServiceProxy(NLNET::IModuleProxy *proxy)
	{
		_ModuleProxy = proxy;
		// Initialize collocated servant interface.
		if (proxy->getModuleDistance() == 0)
		{
			_LocalModule = proxy->getLocalModule();
			nlassert(_LocalModule != NULL);
			CLoginServiceSkel::TInterceptor *interceptor = NULL;
			interceptor = static_cast<NLNET::CModuleBase *>(_LocalModule.getPtr())->getInterceptor(interceptor);
			nlassert(interceptor != NULL);
			_LocalModuleSkel = interceptor->getParent();
			nlassert(_LocalModuleSkel != NULL);
		}
		else _LocalModuleSkel = 0;
	}
	virtual ~CLoginServiceProxy() { }
	
	NLNET::IModuleProxy *getModuleProxy()
	{
		return _ModuleProxy;
	}
	
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !         
	/////////////////////////////////////////////////////////////////
	/// An awaited user did not connect before the allowed timeout expire.
	void pendingUserLost(NLNET::IModule *sender, const NLNET::CLoginCookie &cookie);
	
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !         
	/////////////////////////////////////////////////////////////////
	/// An awaited user did not connect before the allowed timeout expire.
	/// Message serializer. Return the message received in reference for easier integration.
	static const NLNET::CMessage &buildMessageFor_pendingUserLost(NLNET::CMessage &__message, const NLNET::CLoginCookie &cookie);
	
}; /* class CLoginServiceProxy */

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !         
/////////////////////////////////////////////////////////////////
/* 
 * \brief CWelcomeServiceClientSkel
 * \date 2005-10-03 05:15GMT
 * \author Nevrax Ltd. (Nevrax Ltd.)
 * Interface for messages going from the shard's welcome service to
 * the global ring session manager. This is a subsection of the
 * RingSessionManager module (CRingSessionManagerMod) as well as the
 * LoginService module (CLoginServiceMod).
 * Skeleton for implementing the receiving end of this interface into
 * a module.
 */
class CWelcomeServiceClientSkel
{
public:
	// The interceptor type.
	typedef NLNET::CInterceptorForwarder<CWelcomeServiceClientSkel> TInterceptor;
	
protected:
	CWelcomeServiceClientSkel()
	{
		// Do early run time check for message table.
		getMessageHandlers();
	}
	virtual ~CWelcomeServiceClientSkel() { }
	
	void init(NLNET::IModule *module)
	{
		_Interceptor.init(this, module);
	}
	
	// Unused interceptors.
	std::string fwdBuildModuleManifest() const { return std::string(); }
	void fwdOnModuleUp(NLNET::IModuleProxy *moduleProxy) { };
	void fwdOnModuleDown(NLNET::IModuleProxy *moduleProxy) { };
	void fwdOnModuleSecurityChange(NLNET::IModuleProxy *moduleProxy) { };
	
	// Process module message interceptor.
	bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	
private:
	typedef void (CWelcomeServiceClientSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	typedef std::map<std::string, TMessageHandler> TMessageHandlerMap;
	
	const TMessageHandlerMap &getMessageHandlers() const;
	
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !         
	/////////////////////////////////////////////////////////////////
	void registerWS_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);
	void reportWSOpenState_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);
	void welcomeUserResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);
	void updateConnectedPlayerCount_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);
	
	// Declare one interceptor member of the skeleton.
	TInterceptor _Interceptor;
	
	// Declare the interceptor forwarder as friend of this class.
	friend class NLNET::CInterceptorForwarder<CWelcomeServiceClientSkel>;
	
public:
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !         
	/////////////////////////////////////////////////////////////////
	/// Register the welcome service in the ring session manager. The provided sessionId will be non-zero only for a shard with a fixed sessionId.
	virtual void registerWS(NLNET::IModuleProxy *sender, uint32 shardId, uint32 fixedSessionId, bool isOnline) =0;
	/// WS report it's current open state.
	virtual void reportWSOpenState(NLNET::IModuleProxy *sender, bool isOnline) =0;
	/// Return for welcome user.
	virtual void welcomeUserResult(NLNET::IModuleProxy *sender, uint32 userId, bool ok, const std::string &shardAddr, const std::string &errorMsg) =0;
	/// Transmits the current player counts.
	virtual void updateConnectedPlayerCount(NLNET::IModuleProxy *sender, uint32 nbOnlinePlayers, uint32 nbPendingPlayers) =0;
	
}; /* class CWelcomeServiceClientSkel */

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !         
/////////////////////////////////////////////////////////////////
/* 
 * \brief CWelcomeServiceClientProxy
 * \date 2005-10-03 05:15GMT
 * \author Nevrax Ltd. (Nevrax Ltd.)
 * Interface for messages going from the shard's welcome service to
 * the global ring session manager. This is a subsection of the
 * RingSessionManager module (CRingSessionManagerMod) as well as the
 * LoginService module (CLoginServiceMod).
 * Proxy for sending messages trough the proxy of a module that
 * implemented this interface. If the target module is local, the
 * messages will be relayed synchronously to the local module.
 */
class CWelcomeServiceClientProxy
{
	/// Smart pointer on the module proxy
	NLNET::TModuleProxyPtr _ModuleProxy;
	
	// Pointer on the local module that implement the interface (if the proxy is for a local module)
	NLNET::TModulePtr _LocalModule;
	// Direct pointer on the server implementation interface for collocated module
	CWelcomeServiceClientSkel *_LocalModuleSkel;
	
public:
	CWelcomeServiceClientProxy(NLNET::IModuleProxy *proxy)
	{
		_ModuleProxy = proxy;
		// Initialize collocated servant interface.
		if (proxy->getModuleDistance() == 0)
		{
			_LocalModule = proxy->getLocalModule();
			nlassert(_LocalModule != NULL);
			CWelcomeServiceClientSkel::TInterceptor *interceptor = NULL;
			interceptor = static_cast<NLNET::CModuleBase *>(_LocalModule.getPtr())->getInterceptor(interceptor);
			nlassert(interceptor != NULL);
			_LocalModuleSkel = interceptor->getParent();
			nlassert(_LocalModuleSkel != NULL);
		}
		else _LocalModuleSkel = 0;
	}
	virtual ~CWelcomeServiceClientProxy() { }
	
	NLNET::IModuleProxy *getModuleProxy()
	{
		return _ModuleProxy;
	}
	
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !         
	/////////////////////////////////////////////////////////////////
	/// Register the welcome service in the ring session manager. The provided sessionId will be non-zero only for a shard with a fixed sessionId.
	void registerWS(NLNET::IModule *sender, uint32 shardId, uint32 fixedSessionId, bool isOnline);
	/// WS report it's current open state.
	void reportWSOpenState(NLNET::IModule *sender, bool isOnline);
	/// Return for welcome user.
	void welcomeUserResult(NLNET::IModule *sender, uint32 userId, bool ok, const std::string &shardAddr, const std::string &errorMsg);
	/// Transmits the current player counts.
	void updateConnectedPlayerCount(NLNET::IModule *sender, uint32 nbOnlinePlayers, uint32 nbPendingPlayers);
	
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !         
	/////////////////////////////////////////////////////////////////
	/// Register the welcome service in the ring session manager. The provided sessionId will be non-zero only for a shard with a fixed sessionId.
	/// Message serializer. Return the message received in reference for easier integration.
	static const NLNET::CMessage &buildMessageFor_registerWS(NLNET::CMessage &__message, uint32 shardId, uint32 fixedSessionId, bool isOnline);
	/// WS report it's current open state.
	/// Message serializer. Return the message received in reference for easier integration.
	static const NLNET::CMessage &buildMessageFor_reportWSOpenState(NLNET::CMessage &__message, bool isOnline);
	/// Return for welcome user.
	/// Message serializer. Return the message received in reference for easier integration.
	static const NLNET::CMessage &buildMessageFor_welcomeUserResult(NLNET::CMessage &__message, uint32 userId, bool ok, const std::string &shardAddr, const std::string &errorMsg);
	/// Transmits the current player counts.
	/// Message serializer. Return the message received in reference for easier integration.
	static const NLNET::CMessage &buildMessageFor_updateConnectedPlayerCount(NLNET::CMessage &__message, uint32 nbOnlinePlayers, uint32 nbPendingPlayers);
	
}; /* class CWelcomeServiceClientProxy */

} /* namespace WS */

#endif /* #ifndef WS_WELCOME_SERVICE_ITF_H */

/* end of file */
