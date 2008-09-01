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

import time, sys, os
from textwrap import TextWrapper

wrapper = TextWrapper()
indent = " * "
wrapper.initial_indent = indent
wrapper.subsequent_indent = indent
gmtime = time.gmtime(time.time())
newline = "\n"

print ""
print "--------------------------------------------------------------"
print " This script generates the interfaces used by NLNET modules."
print "--------------------------------------------------------------"
print ""

_FileDocumentation = [ ]
_FileDocumentation += [ "\\file {FILE_CURRENT}" ]
_FileDocumentation += [ "\\brief WARNING : this is a generated file, don't change it !" ]
_FileDocumentation += [ "\\date {FILE_DATE}" ]
_FileDocumentation += [ "\\author {FILE_AUTHOR} ({FILE_USERNAME})" ]
_FileDocumentation += [ "{FILE_COMMENT}" ]
_FileDocumentation += [ "" ]
_FileDocumentation += [ "$I" + "d$" ]

_ClassDocumentation = [ ]
_ClassDocumentation += [ "\\brief {CLASS_CURRENT}" ]
_ClassDocumentation += [ "\\date {FILE_DATE}" ]
_ClassDocumentation += [ "\\author {FILE_AUTHOR} ({FILE_USERNAME})" ]
_ClassDocumentation += [ "{CLASS_COMMENT}" ]
_ClassDocumentation += [ "{CLASS_EXCOMMENT}" ]

_CopyrightHeader = [ ]
_CopyrightHeader += [ "Copyright (C) {FILE_YEAR}  {FILE_AUTHOR} ({FILE_USERNAME})" ]
_CopyrightHeader += [ "" ]
_CopyrightHeader += [ "This file is part of {FILE_PRODUCT}." ]
_CopyrightHeader += [ "{FILE_PRODUCT} is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version." ]
_CopyrightHeader += [ "" ]
_CopyrightHeader += [ "{FILE_PRODUCT} is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details." ]
_CopyrightHeader += [ "" ]
_CopyrightHeader += [ "You should have received a copy of the GNU General Public License along with {FILE_PRODUCT}; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA." ]

_WarningGeneratedFile = [ ]
_WarningGeneratedFile += ["/////////////////////////////////////////////////////////////////"]
_WarningGeneratedFile += ["// WARNING : this is a generated file, don't change it !         "]
_WarningGeneratedFile += ["/////////////////////////////////////////////////////////////////"]

_HFileHeader1 = [ ]
_HFileHeader1 += [ "#ifndef {FILE_HDEFINE}" ]
_HFileHeader1 += [ "#define {FILE_HDEFINE}" ]
_HFileHeader1 += [ "#include <nel/misc/types_nl.h>" ]
_HFileHeader1 += [ "" ]
_HFileHeader1 += [ "// User header" ]
_HFileHeader3 = [ ]
_HFileHeader3 += [ "" ]
_HFileHeader3 += [ "// NeL includes" ]
_HFileHeader3 += [ "#include <nel/misc/debug.h>" ]
_HFileHeader3 += [ "#include <nel/misc/log.h>" ]
_HFileHeader3 += [ "#include <nel/net/message.h>" ]
_HFileHeader3 += [ "#include <nel/net/module.h>" ]
_HFileHeader3 += [ "#include <nel/net/module_builder_parts.h>" ]
_HFileHeader3 += [ "#include <nel/net/module_message.h>" ]
_HFileHeader3 += [ "#include <nel/net/module_gateway.h>" ]
_HFileHeader3 += [ "" ]
_HFileHeader3 += [ "// STL includes" ]
_HFileHeader3 += [ "#include <string>" ]
_HFileHeader3 += [ "" ]
_HFileHeader3 += [ "namespace {FILE_NAMESPACE} {" ]
_HFileHeader3 += [ "" ]

_HFileFooter = [ ]
_HFileFooter += [ "} /* namespace {FILE_NAMESPACE} */"]
_HFileFooter += [ "" ]
_HFileFooter += [ "#endif /* #ifndef {FILE_HDEFINE} */"]
_HFileFooter += [ "" ]
_HFileFooter += [ "/* end of file */" ]

_CppFileHeader = [ ]
_CppFileHeader += [ "#include <nel/misc/types_nl.h>"]
# _CppFileHeader += [ "#include \"{FILE_NAME}.h\""]
_CppFileHeader += [ "{FILE_HEADERINCLUDE}" ]
_CppFileHeader += [ "" ]
# _CppFileHeader += [ "// Project includes" ]
# _CppFileHeader += [ "" ]
# _CppFileHeader += [ "// NeL includes" ]
# _CppFileHeader += [ "// #include <nel/misc/debug.h>"]
# _CppFileHeader += [ "" ]
# _CppFileHeader += [ "// STL includes" ]
# _CppFileHeader += [ "" ]
# _CppFileHeader += [ "using namespace std;" ]
# _CppFileHeader += [ "using namespace NLMISC;" ]
# _CppFileHeader += [ "using namespace NLNET;" ]
# _CppFileHeader += [ "" ]
_CppFileHeader += [ "namespace {FILE_NAMESPACE} {" ]
_CppFileHeader += [ "" ]

_CppFileFooter = [ ]
_CppFileFooter += [ "} /* namespace {FILE_NAMESPACE} */" ]
_CppFileFooter += [ "" ]
_CppFileFooter += [ "/* end of file */" ]

def writeLines(f, prefix, code):
	for line in code:
		f.write(prefix + line + newline)

class CFile:
	"""File base infos"""
	def __init__(self):
		self.Name = "module_itf_gen"
		self.Path = ""
		self.PathToHeaderInclude = ""
		self.PathToHeader = ""
		self.Comment = "Generated module interface skeleton"
		self.Namespace = "NLNET"
		self.Product = "NEVRAX NEL"
		self.Author = "Jan Boon"
		self.Username = "Kaetemi"
		self.Year = time.strftime("%Y", gmtime);
		self.Date = time.strftime("%Y-%m-%d %H:%MGMT", gmtime);
		self.Header = [ ]
		self.Interfaces = [ ]
	def generateVars(self):
		self.NameCpp = self.Name + ".cpp"
		self.NameH = self.Name + ".h"
		self.HDefine = self.Namespace + "_" + self.Name.upper() + "_H"
		if (self.PathToHeaderInclude == "") and (self.PathToHeader == ""):
			self.HeaderInclude = "#include \"" + self.Name + ".h\""
		else:
			self.HeaderInclude = "#include <" + self.PathToHeader + self.Name + ".h>"
	def buildLine(self, line):
		newline = line.replace("{FILE_NAME}", self.Name)
		newline = newline.replace("{FILE_PATH}", self.Path)
		newline = newline.replace("{FILE_COMMENT}", self.Comment)
		newline = newline.replace("{FILE_NAMESPACE}", self.Namespace)
		newline = newline.replace("{FILE_PRODUCT}", self.Product)
		newline = newline.replace("{FILE_AUTHOR}", self.Author)
		newline = newline.replace("{FILE_USERNAME}", self.Username)
		newline = newline.replace("{FILE_YEAR}", self.Year)
		newline = newline.replace("{FILE_DATE}", self.Date)
		newline = newline.replace("{FILE_NAMECPP}", self.NameCpp)
		newline = newline.replace("{FILE_NAMEH}", self.NameH)
		newline = newline.replace("{FILE_HDEFINE}", self.HDefine)
		newline = newline.replace("{FILE_HEADERINCLUDE}", self.HeaderInclude)
		newline = newline.replace("{FILE_CURRENT}", self.Current)
		return newline
	def printComment(self, f, lines):
		for line in lines:
			if line == "":
				f.write(indent + newline)
			else:
				for subline in wrapper.wrap(self.buildLine(line)):
					f.write(subline + newline)
	def writeFileHeader(self, f):
		print "Writing documentation and copyright header to file"
		f.write("/**" + newline)
		self.printComment(f, _FileDocumentation)
		f.write(" */" + newline)
		f.write(newline)
		f.write("/* " + newline)
		self.printComment(f, _CopyrightHeader)
		f.write(" */" + newline)
		f.write(newline)
	def writeGenericCode(self, f, code):
		for line in code:
			f.write(self.buildLine(line) + newline)
	def writeH(self):
		print "Generating Header file: " + self.Path + self.NameH
		self.Current = self.NameH
		f = open(self.Path + self.PathToHeaderInclude + self.PathToHeader + self.Current , 'w')
		self.writeFileHeader(f)
		self.writeGenericCode(f, _WarningGeneratedFile)
		print "Writing .h header to file"
		self.writeGenericCode(f, _HFileHeader1)
		self.writeGenericCode(f, self.Header)
		self.writeGenericCode(f, _HFileHeader3)
		for module in self.Interfaces:
			module.writeHeaderSkel(f, self)
			module.writeHeaderProxy(f, self)
		print "Writing .h footer to file"
		self.writeGenericCode(f, _HFileFooter)
		f.close()
	def writeCpp(self):
		print "Generating CPP file: " + self.Path + self.NameCpp
		self.Current = self.NameCpp
		f = open(self.Path + self.Current , 'w')
		self.writeFileHeader(f)
		self.writeGenericCode(f, _WarningGeneratedFile)
		print "Writing .cpp header to file"
		self.writeGenericCode(f, _CppFileHeader)
		for module in self.Interfaces:
			module.writeCppSkel(f, self)
			module.writeCppProxy(f, self)
		print "Writing .cpp footer to file"
		self.writeGenericCode(f, _CppFileFooter)
		f.close()
	def writeFiles(self):
		self.generateVars()
		for module in self.Interfaces:
			module.generateVars()
		self.writeH()
		self.writeCpp()
		print ""
		print "--------------------------------------------------------------"
		print " Done!"
		print "--------------------------------------------------------------"

class CInterface:
	"""Module skeleton infos"""
	def __init__(self, name, comment):
		self.Name = name # "Module"
		self.Comment = comment
		self.Messages = [ ]
	def generateVars(self):
		self.ProxyClass = "C" + self.Name + "Proxy"
		self.SkelClass = "C" + self.Name + "Skel"
		for message in self.Messages:
			message.generateVars()
	def buildLine(self, file, line):
		newline = file.buildLine(line)
		newline = newline.replace("{CLASS_CURRENT}", self.Current)
		newline = newline.replace("{CLASS_COMMENT}", self.Comment)
		newline = newline.replace("{CLASS_EXCOMMENT}", self.ExComment)
		return newline
	def printComment(self, f, file, lines):
		f.write("/* " + newline)
		for line in lines:
			if line == "":
				f.write(indent + newline)
			else:
				for subline in wrapper.wrap(self.buildLine(file, line)):
					f.write(subline + newline)
		f.write(" */" + newline)
	def writeHeaderSkel(self, f, file):
		print "writeHeaderSkel: " + self.SkelClass
		self.Current = self.SkelClass;
		self.ExComment = "Skeleton for implementing the receiving end of this interface into a module."
		writeLines(f, "", _WarningGeneratedFile)
		self.printComment(f, file, _ClassDocumentation)
		f.write("class " + self.SkelClass + "" + newline)
		f.write("{" + newline)
		f.write("public:" + newline)
		f.write("	// The interceptor type." + newline)
		f.write("	typedef NLNET::CInterceptorForwarder<" + self.SkelClass + "> TInterceptor;" + newline)
		f.write("	" + newline)
		f.write("protected:" + newline)
		f.write("	" + self.SkelClass + "()" + newline)
		f.write("	{" + newline)
		f.write("		// Do early run time check for message table." + newline)
		f.write("		getMessageHandlers();" + newline)
		f.write("	}" + newline)
		f.write("	virtual ~" + self.SkelClass + "() { }" + newline)
		f.write("	" + newline)
		f.write("	void init(NLNET::IModule *module)" + newline)
		f.write("	{" + newline)
		f.write("		_Interceptor.init(this, module);" + newline)
		f.write("	}" + newline)
		f.write("	" + newline)
		f.write("	// Unused interceptors." + newline)
		f.write("	std::string fwdBuildModuleManifest() const { return std::string(); }" + newline)
		f.write("	void fwdOnModuleUp(NLNET::IModuleProxy *moduleProxy) { };" + newline)
		f.write("	void fwdOnModuleDown(NLNET::IModuleProxy *moduleProxy) { };" + newline)
		f.write("	void fwdOnModuleSecurityChange(NLNET::IModuleProxy *moduleProxy) { };" + newline)
		f.write("	" + newline)
		f.write("	// Process module message interceptor." + newline)
		f.write("	bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);" + newline)
		f.write("	" + newline)
		f.write("private:" + newline)
		f.write("	typedef void (" + self.SkelClass + "::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);" + newline)
		f.write("	typedef std::map<std::string, TMessageHandler> TMessageHandlerMap;" + newline)
		f.write("	" + newline)
		f.write("	const TMessageHandlerMap &getMessageHandlers() const;" + newline)
		f.write("	" + newline)
		writeLines(f, "	", _WarningGeneratedFile)
		for message in self.Messages:
			message.writeHeaderSkelParser(f)
		f.write("	" + newline)
		f.write("	// Declare one interceptor member of the skeleton." + newline)
		f.write("	TInterceptor _Interceptor;" + newline)
		f.write("	" + newline)
		f.write("	// Declare the interceptor forwarder as friend of this class." + newline)
		f.write("	friend class NLNET::CInterceptorForwarder<" + self.SkelClass + ">;" + newline)
		f.write("	" + newline)
		f.write("public:" + newline)
		writeLines(f, "	", _WarningGeneratedFile)
		for message in self.Messages:
			message.writeHeaderSkelCallback(f)
		f.write("	" + newline)
		f.write("}; /* class " + self.SkelClass + " */" + newline)
		f.write("" + newline)
	def writeHeaderProxy(self, f, file):
		print "writeHeaderSkel: " + self.ProxyClass
		self.Current = self.ProxyClass;
		self.ExComment = "Proxy for sending messages trough the proxy of a module that implemented this interface. If the target module is local, the messages will be relayed synchronously to the local module."
		writeLines(f, "", _WarningGeneratedFile)
		self.printComment(f, file, _ClassDocumentation)
		f.write("class " + self.ProxyClass + "" + newline)
		f.write("{" + newline)
		f.write("	/// Smart pointer on the module proxy" + newline)
		f.write("	NLNET::TModuleProxyPtr _ModuleProxy;" + newline)
		f.write("	" + newline)
		f.write("	// Pointer on the local module that implement the interface (if the proxy is for a local module)" + newline)
		f.write("	NLNET::TModulePtr _LocalModule;" + newline)
		f.write("	// Direct pointer on the server implementation interface for collocated module" + newline)
		f.write("	" + self.SkelClass + " *_LocalModuleSkel;" + newline)
		f.write("	" + newline)
		f.write("public:" + newline)
		f.write("	" + self.ProxyClass + "(NLNET::IModuleProxy *proxy)" + newline)
		f.write("	{" + newline)
		f.write("		_ModuleProxy = proxy;" + newline)
		f.write("		// Initialize collocated servant interface." + newline)
		f.write("		if (proxy->getModuleDistance() == 0)" + newline)
		f.write("		{" + newline)
		f.write("			_LocalModule = proxy->getLocalModule();" + newline)
		f.write("			nlassert(_LocalModule != NULL);" + newline)
		f.write("			" + self.SkelClass + "::TInterceptor *interceptor = NULL;" + newline)
		f.write("			interceptor = static_cast<NLNET::CModuleBase *>(_LocalModule.getPtr())->getInterceptor(interceptor);" + newline)
		f.write("			nlassert(interceptor != NULL);" + newline)
		f.write("			_LocalModuleSkel = interceptor->getParent();" + newline)
		f.write("			nlassert(_LocalModuleSkel != NULL);" + newline)
		f.write("		}" + newline)
		f.write("		else _LocalModuleSkel = 0;" + newline)
		f.write("	}" + newline)
		f.write("	virtual ~" + self.ProxyClass + "() { }" + newline)
		f.write("	" + newline)
		f.write("	NLNET::IModuleProxy *getModuleProxy()" + newline)
		f.write("	{" + newline)
		f.write("		return _ModuleProxy;" + newline)
		f.write("	}" + newline)
		f.write("	" + newline)
		writeLines(f, "	", _WarningGeneratedFile)
		for message in self.Messages:
			message.writeHeaderProxyFunction(f)
		f.write("	" + newline)
		writeLines(f, "	", _WarningGeneratedFile)
		for message in self.Messages:
			message.writeHeaderProxyBuilder(f)
		f.write("	" + newline)
		f.write("}; /* class " + self.ProxyClass + " */" + newline)
		f.write("" + newline)
	def writeCppSkel(self, f, file):
		print "writeCppSkel: " + self.SkelClass
		self.Current = self.SkelClass;
		writeLines(f, "", _WarningGeneratedFile)
		f.write("const " + self.SkelClass + "::TMessageHandlerMap &" + self.SkelClass + "::getMessageHandlers() const" + newline)
		f.write("{" + newline)
		f.write("	static TMessageHandlerMap handlers;" + newline)
		f.write("	static bool init = false;" + newline)
		f.write("	" + newline)
		f.write("	if (!init)" + newline)
		f.write("	{" + newline)
		f.write("		std::pair < TMessageHandlerMap::iterator, bool > res;" + newline)
		f.write("		" + newline)
		for message in self.Messages:
			message.writeCppSkelHandler(f, self)
		f.write("		init = true;" + newline)
		f.write("	}" + newline)
		f.write("	" + newline)
		f.write("	return handlers;" + newline)
		f.write("}" + newline)
		f.write("" + newline)
		f.write("bool " + self.SkelClass + "::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)" + newline)
		f.write("{" + newline)
		f.write("	const TMessageHandlerMap &mh = getMessageHandlers();" + newline)
		f.write("	" + newline)
		f.write("	TMessageHandlerMap::const_iterator it(mh.find(message.getName()));" + newline)
		f.write("	" + newline)
		f.write("	if (it == mh.end())" + newline)
		f.write("	{" + newline)
		f.write("		return false;" + newline)
		f.write("	}" + newline)
		f.write("	" + newline)
		f.write("	TMessageHandler cmd = it->second;" + newline)
		f.write("	(this->*cmd)(sender, message);" + newline)
		f.write("	" + newline)
		f.write("	return true;" + newline)
		f.write("}" + newline)
		f.write("" + newline)
		for message in self.Messages:
			message.writeCppSkelParser(f, self)
	def writeCppProxy(self, f, file):
		print "writeCppProxy: " + self.ProxyClass
		self.Current = self.ProxyClass;
		writeLines(f, "", _WarningGeneratedFile)
		for message in self.Messages:
			message.writeCppProxyFunction(f, self)
		for message in self.Messages:
			message.writeCppProxyBuilder(f, self)

class CMessage:
	"""Message format infos"""
	def __init__(self, shortName, longName, comment):
		self.ShortName = shortName # "MSG"
		self.LongName = longName # "message"
		self.Comment = comment
		self.Params = [ ]
	def generateVars(self):
		for param in self.Params:
			param.generateVars()
		self.DeclarationList = ""
		for param in self.Params:
			self.DeclarationList += param.Declaration + ", "
		self.DeclarationList = self.DeclarationList[:-2]
		self.ParamList = ""
		for param in self.Params:
			self.ParamList += param.Name + ", "
		self.ParamList = self.ParamList[:-2]
	def writeHeaderSkelParser(self, f):
		print "writeHeaderSkelParser: " + self.ShortName + ", " + self.LongName
		f.write("	void " + self.LongName + "_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);" + newline)
	def writeHeaderSkelCallback(self, f):
		print "writeHeaderSkelCallback: " + self.ShortName + ", " + self.LongName + ", " + self.DeclarationList
		f.write("	/// " + self.Comment + newline) # todo: full comments including all param comments!!!!
		if self.DeclarationList == "":
			f.write("	virtual void " + self.LongName + "(NLNET::IModuleProxy *sender) =0;" + newline)
		else:
			f.write("	virtual void " + self.LongName + "(NLNET::IModuleProxy *sender, " + self.DeclarationList + ") =0;" + newline)
	def writeHeaderProxyFunction(self, f):
		print "writeHeaderProxyFunction: " + self.ShortName + ", " + self.LongName
		f.write("	/// " + self.Comment + newline) # todo: full comments including all param comments!!!!
		if self.DeclarationList == "":
			f.write("	void " + self.LongName + "(NLNET::IModule *sender);" + newline)
		else:
			f.write("	void " + self.LongName + "(NLNET::IModule *sender, " + self.DeclarationList + ");" + newline)
	def writeHeaderProxyBuilder(self, f):
		print "writeHeaderProxyBuilder: " + self.ShortName + ", " + self.LongName
		f.write("	/// " + self.Comment + newline) # todo: full comments including all param comments!!!!
		f.write("	/// Message serializer. Return the message received in reference for easier integration." + newline)
		if self.DeclarationList == "":
			f.write("	static const NLNET::CMessage &buildMessageFor_" + self.LongName + "(NLNET::CMessage &__message);" + newline)
		else:
			f.write("	static const NLNET::CMessage &buildMessageFor_" + self.LongName + "(NLNET::CMessage &__message, " + self.DeclarationList + ");" + newline)
	def writeCppSkelHandler(self, f, itf):
		print "writeCppSkelHandler: " + self.ShortName + ", " + self.LongName
		f.write("		res = handlers.insert(std::make_pair(std::string(\"" + self.ShortName + "\"), &" + itf.SkelClass + "::" + self.LongName + "_skel));" + newline)
		f.write("		// If this assert, you have a duplicate message name in your interface definition!" + newline)
		f.write("		nlassert(res.second);" + newline)
		f.write("		" + newline)
	def writeCppSkelParser(self, f, itf):
		print "writeCppSkelParser: " + self.ShortName + ", " + self.LongName
		f.write("void " + itf.SkelClass + "::" + self.LongName + "_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)" + newline)
		f.write("{" + newline)
		f.write("	H_AUTO(" + itf.SkelClass + "_" + self.LongName + "_" + self.ShortName + ");" + newline)
		for param in self.Params:
			f.write("	" + param.Type + " " + param.Name + ";" + newline)
			f.write("	nlRead(__message, " + param.SerialType + ", " + param.SerialReadName + ");" + newline)
		if self.ParamList == "":
			f.write("	" + self.LongName + "(sender);" + newline)
		else:
			f.write("	" + self.LongName + "(sender, " + self.ParamList + ");" + newline)
		f.write("}" + newline)
		f.write("" + newline)
	def writeCppProxyFunction(self, f, itf):
		print "writeCppProxyFunction: " + self.ShortName + ", " + self.LongName
		if self.DeclarationList == "":
			f.write("void " + itf.ProxyClass + "::" + self.LongName + "(NLNET::IModule *sender)" + newline)
		else:
			f.write("void " + itf.ProxyClass + "::" + self.LongName + "(NLNET::IModule *sender, " + self.DeclarationList + ")" + newline)
		f.write("{" + newline)
		f.write("	if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())" + newline)
		f.write("	{" + newline)
		f.write("		// Immediate local synchronous dispatching." + newline)
		if self.ParamList == "":
			f.write("		_LocalModuleSkel->" + self.LongName + "(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender));" + newline)
		else:
			f.write("		_LocalModuleSkel->" + self.LongName + "(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), " + self.ParamList + ");" + newline)
		f.write("	}" + newline)
		f.write("	else" + newline)
		f.write("	{" + newline)
		f.write("		// Send the message for remote dispatching and execution or local queing." + newline)
		f.write("		NLNET::CMessage __message;" + newline)
		if self.ParamList == "":
			f.write("		buildMessageFor_" + self.LongName + "(__message);" + newline)
		else:
			f.write("		buildMessageFor_" + self.LongName + "(__message, " + self.ParamList + ");" + newline)
		f.write("		_ModuleProxy->sendModuleMessage(sender, __message);" + newline)
		f.write("	}" + newline)
		f.write("}" + newline)
		f.write("" + newline)
	def writeCppProxyBuilder(self, f, itf):
		print "writeCppProxyBuilder: " + self.ShortName + ", " + self.LongName
		f.write("// Message serializer. Return the message received in reference for easier integration." + newline)
		if self.DeclarationList == "":
			f.write("const NLNET::CMessage &" + itf.ProxyClass + "::buildMessageFor_" + self.LongName + "(NLNET::CMessage &__message)" + newline)
		else:
			f.write("const NLNET::CMessage &" + itf.ProxyClass + "::buildMessageFor_" + self.LongName + "(NLNET::CMessage &__message, " + self.DeclarationList + ")" + newline)
		f.write("{" + newline)
		f.write("	__message.setType(\"" + self.ShortName + "\");" + newline)
		for param in self.Params:
			f.write("	nlWrite(__message, " + param.SerialType + ", " + param.SerialWriteName + ");" + newline)
		f.write("	return __message;" + newline)
		f.write("}" + newline)
		f.write("" + newline)

class CParam:
	"""Parameter usage infos"""
	def __init__(self, serialType, const, ref, type, name, comment):
		self.SerialType = serialType # "serial"
		self.Const = const;
		self.Ref = ref;
		self.Type = type # "std::string"
		self.Name = name # "userData"
		self.Comment = comment
	def generateVars(self):
		self.Declaration = ""
		if self.Const:
			self.Declaration += "const "
		self.Declaration += self.Type
		if self.Ref:
			self.Declaration += " &"
		else:
			self.Declaration += " "
		self.Declaration += self.Name
		self.SerialReadName = self.Name
		if self.Const:
			if self.Ref:
				self.SerialWriteName = "const_cast<" + self.Type + " &>(" + self.Name + ")"
			else:
				self.SerialWriteName =  "const_cast<" + self.Type + ">(" + self.Name + ")"
		else:
			self.SerialWriteName =  self.Name

# end of file