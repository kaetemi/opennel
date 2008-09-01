#!/usr/bin/python
# 
# Copyright (C) 2008  Jan Boon (Kaetemi)
# 
# This file is part of OpenNeL Snowballs.
# OpenNeL Snowballs is free software; you can redistribute it and/or modify 
# it under the terms of the GNU General Public License as published by the 
# Free Software Foundation; either version 2 of the License, or (at your 
# option) any later version.
# 
# OpenNeL Snowballs is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General 
# Public License for more details.
# 
# You should have received a copy of the GNU General Public License along 
# with OpenNeL Snowballs; if not, write to the Free Software Foundation, 
# Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
# 
# $Id$
# 

import sys, os
sys.path.append(os.getcwd() + "/../../tool/module_itf_gen")
from module_itf_gen import *

File = CFile()



File.Name = "welcome_service_itf" # Base name for the files.
File.Path = "" # Path where the files need to be dropped.
File.Comment = "Collection of the interfaces used by the welcome service module." # Description of the interfaces.
File.Namespace = "WS" # The namespace for the classes.
File.Product = "NEVRAX NeL Network Services" # The product name.
File.Author = "Nevrax Ltd." # The author.
File.Username = "Nevrax Ltd." # The username of the author.
File.Year = "2005" # The year shown in the copyright header.
File.Date = "2005-10-03 05:15GMT" # The date of creation shown in the documentation.



File.Header.append("#include \"user_role.h\"")
File.Header.append("#include <nel/net/login_cookie.h>")



WelcomeService = CInterface("WelcomeService", "Interface for messages going from the global ring session manager and login service to the shard's welcome service. This is a subsection of the WelcomeService module (CWelcomeServiceMod).")

WU = CMessage("WU", "welcomeUser", "Ask the welcome service to welcome a character.")
WU.Params.append(CParam("serial", False, False, "uint32", "charId", "Character identifier."))
WU.Params.append(CParam("serial", True, True, "std::string", "userName", "Login name."))
WU.Params.append(CParam("serial", True, True, "NLNET::CLoginCookie", "cookie", "Login cookie."))
WU.Params.append(CParam("serial", True, True, "std::string", "priviledge", "Privilege."))
WU.Params.append(CParam("serial", True, True, "std::string", "exPriviledge", "Extended privilege."))
WU.Params.append(CParam("serial", False, False, "TUserRole", "mode", "Session-specific user privilege (Used for Ryzom Ring, might want to replace this with std::string sessionPrivilege)."))
WU.Params.append(CParam("serial", False, False, "uint32", "instanceId", "Identifies the game session or instance to log into, allows running multiple games on the same service."))
WelcomeService.Messages.append(WU)

DU = CMessage("DU", "disconnectUser", "Ask the welcome service to disconnect a user.")
DU.Params.append(CParam("serial", False, False, "uint32", "userId", "The user id, welcome service knows about this from the login cookie."))
WelcomeService.Messages.append(DU)

File.Interfaces.append(WelcomeService)



LoginService = CInterface("LoginService", "Interface for messages going from the shard's welcome service to the global login service. This is a subsection of the LoginService module (CLoginServiceMod).")

PUL = CMessage("PUL", "pendingUserLost", "An awaited user did not connect before the allowed timeout expire.")
PUL.Params.append(CParam("serial", True, True, "NLNET::CLoginCookie", "cookie", "Login cookie."))
LoginService.Messages.append(PUL)

File.Interfaces.append(LoginService)



WelcomeServiceClient = CInterface("WelcomeServiceClient", "Interface for messages going from the shard's welcome service to the global ring session manager. This is a subsection of the RingSessionManager module (CRingSessionManagerMod) as well as the LoginService module (CLoginServiceMod).")

RWS = CMessage("RWS", "registerWS", "Register the welcome service in the ring session manager. The provided sessionId will be non-zero only for a shard with a fixed sessionId.")
RWS.Params.append(CParam("serial", False, False, "uint32", "shardId", "Unique shard identification number."))
RWS.Params.append(CParam("serial", False, False, "uint32", "fixedSessionId", "Non-zero only if the session id on the shard is fixed (there only is one session on the shard)."))
RWS.Params.append(CParam("serial", False, False, "bool", "isOnline", "If the shard is open or not."))
WelcomeServiceClient.Messages.append(RWS)

RWSOS = CMessage("RWSOS", "reportWSOpenState", "WS report it's current open state.")
RWSOS.Params.append(CParam("serial", False, False, "bool", "isOnline", "If the shard is open or not."))
WelcomeServiceClient.Messages.append(RWSOS)

WUR = CMessage("WUR", "welcomeUserResult", "Return for welcome user.")
WUR.Params.append(CParam("serial", False, False, "uint32", "userId", "The user identification number, known from the login cookie."))
WUR.Params.append(CParam("serial", False, False, "bool", "ok", "k..."))
WUR.Params.append(CParam("serial", True, True, "std::string", "shardAddr", "The shard adress that can be given to the user."))
WUR.Params.append(CParam("serial", True, True, "std::string", "errorMsg", "Empty if there is no error."))
WelcomeServiceClient.Messages.append(WUR)

UCP = CMessage("UCP", "updateConnectedPlayerCount", "Transmits the current player counts.")
UCP.Params.append(CParam("serial", False, False, "uint32", "nbOnlinePlayers", "The number of players connected to the shard."))
UCP.Params.append(CParam("serial", False, False, "uint32", "nbPendingPlayers", "The number of players that the shard is waiting for to connect."))
WelcomeServiceClient.Messages.append(UCP)

File.Interfaces.append(WelcomeServiceClient)



File.writeFiles()

# end of file