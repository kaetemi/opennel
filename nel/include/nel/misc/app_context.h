/** \file app_context.h
 * Classes for managing NeL context in order to support multi module NeL 
 * application.
 *
 * $Id: app_context.h,v 1.4 2006/01/10 17:38:46 boucher Exp $
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


#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include "nel/misc/types_nl.h"
#include <map>

namespace NLMISC
{
	class CLog;
	class CMemDisplayer;
	class CMsgBoxDisplayer;


	/** Interface definition for nel context.
	 *	Any application wide data can be accessed thru this interface.
	 *
	 *	The NeL context is a mean to allow dynamic library loading in NeL.
	 *	In order to make all NeL application safe, it is mandatory to declare
	 *	a NeL context at startup of any application (first instruction of the
	 *	main() or WinMain() is good practice).
	 *	Note that for NLNET::IService oriented application, service framwork
	 *	already provide the application context.
	 *
	 *	\author Boris 'SoniX' Boucher
	 *  \date 2005
	 */
	class INelContext
	{
	public:

		/// Access to the context singleton
		static INelContext &getInstance();

		static bool isContextInitialised();

		virtual ~INelContext();

		//@name Singleton registry
		//@{
		/** Return the pointer associated to a given singleton name
		 *	If the name is not present, return NULL.
		 */
		virtual void *getSingletonPointer(const std::string &singletonName) =0;
		/** Register a singleton pointer.
		*/
		virtual void setSingletonPointer(const std::string &singletonName, void *ptr) =0;
		/** Release a singleton pointer */
		virtual void releaseSingletonPointer(const std::string &singletonName, void *ptr) =0;
		//@}

		//@name Global debugging object
		//@{
		virtual CLog *getErrorLog() =0;
		virtual void setErrorLog(CLog *errorLog) =0;
		virtual CLog *getWarningLog() =0;
		virtual void setWarningLog(CLog *warningLog) =0;
		virtual CLog *getInfoLog() =0;
		virtual void setInfoLog(CLog *infoLog) =0;
		virtual CLog *getDebugLog() =0;
		virtual void setDebugLog(CLog *debugLog) =0;
		virtual CLog *getAssertLog() =0;
		virtual void setAssertLog(CLog *assertLog) =0;
		virtual CMemDisplayer *getDefaultMemDisplayer() =0;
		virtual void setDefaultMemDisplayer(CMemDisplayer *memDisplayer) =0;
		virtual CMsgBoxDisplayer *getDefaultMsgBoxDisplayer() =0;
		virtual void setDefaultMsgBoxDisplayer(CMsgBoxDisplayer *msgBoxDisplayer) =0;
		virtual bool getDebugNeedAssert() =0;
		virtual void setDebugNeedAssert(bool needAssert) =0;
		virtual bool getNoAssert() =0;
		virtual void setNoAssert(bool noAssert) =0;
		virtual bool getAlreadyCreateSharedAmongThreads() =0;
		virtual void setAlreadyCreateSharedAmongThreads(bool b) =0;
		//@}
	protected:
		/// Called by derived class to finalize initialisation of context
		void	contextReady();

		static INelContext *_NelContext;

	};

	/** This class implement the context interface for the application module
	 *	That mean that this class will really hold the data.
	 *	\author Boris 'SoniX' Boucher
	 *  \date 2005
	 */
	class CApplicationContext : public INelContext
	{
	public:
		CApplicationContext();

		virtual void *getSingletonPointer(const std::string &singletonName);
		virtual void setSingletonPointer(const std::string &singletonName, void *ptr);
		virtual void releaseSingletonPointer(const std::string &singletonName, void *ptr);

		virtual CLog *getErrorLog();
		virtual void setErrorLog(CLog *errorLog);
		virtual CLog *getWarningLog();
		virtual void setWarningLog(CLog *warningLog);
		virtual CLog *getInfoLog();
		virtual void setInfoLog(CLog *infoLog);
		virtual CLog *getDebugLog();
		virtual void setDebugLog(CLog *debugLog);
		virtual CLog *getAssertLog();
		virtual void setAssertLog(CLog *assertLog);
		virtual CMemDisplayer *getDefaultMemDisplayer();
		virtual void setDefaultMemDisplayer(CMemDisplayer *memDisplayer);
		virtual CMsgBoxDisplayer *getDefaultMsgBoxDisplayer();
		virtual void setDefaultMsgBoxDisplayer(CMsgBoxDisplayer *msgBoxDisplayer);
		virtual bool getDebugNeedAssert();
		virtual void setDebugNeedAssert(bool needAssert);
		virtual bool getNoAssert();
		virtual void setNoAssert(bool noAssert);
		virtual bool getAlreadyCreateSharedAmongThreads();
		virtual void setAlreadyCreateSharedAmongThreads(bool b);
	
	private:
		/// Singleton registry
		typedef std::map<std::string, void*>	TSingletonRegistry;
		TSingletonRegistry		_SingletonRegistry;

		CLog *ErrorLog;
		CLog *WarningLog;
		CLog *InfoLog;
		CLog *DebugLog;
		CLog *AssertLog;
		CMemDisplayer *DefaultMemDisplayer;
		CMsgBoxDisplayer *DefaultMsgBoxDisplayer;
		bool DebugNeedAssert;
		bool NoAssert;
		bool AlreadyCreateSharedAmongThreads;
	
	};

	/** This class implement the context interface for the a library module.
	 *	All it contains is forward call to the application context instance.
	 *	\author Boris 'SoniX' Boucher
	 *  \date 2005
	 */
	class CLibraryContext : public INelContext
	{
	public:
		CLibraryContext (INelContext &applicationContext);

		virtual void *getSingletonPointer(const std::string &singletonName);
		virtual void setSingletonPointer(const std::string &singletonName, void *ptr);
		virtual void releaseSingletonPointer(const std::string &singletonName, void *ptr);

		virtual CLog *getErrorLog();
		virtual void setErrorLog(CLog *errorLog);
		virtual CLog *getWarningLog();
		virtual void setWarningLog(CLog *warningLog);
		virtual CLog *getInfoLog();
		virtual void setInfoLog(CLog *infoLog);
		virtual CLog *getDebugLog();
		virtual void setDebugLog(CLog *debugLog);
		virtual CLog *getAssertLog();
		virtual void setAssertLog(CLog *assertLog);
		virtual CMemDisplayer *getDefaultMemDisplayer();
		virtual void setDefaultMemDisplayer(CMemDisplayer *memDisplayer);
		virtual CMsgBoxDisplayer *getDefaultMsgBoxDisplayer();
		virtual void setDefaultMsgBoxDisplayer(CMsgBoxDisplayer *msgBoxDisplayer);
		virtual bool getDebugNeedAssert();
		virtual void setDebugNeedAssert(bool needAssert);
		virtual bool getNoAssert();
		virtual void setNoAssert(bool noAssert);
		virtual bool getAlreadyCreateSharedAmongThreads();
		virtual void setAlreadyCreateSharedAmongThreads(bool b);
	private:
		/// Pointer to the application context.
		INelContext		&_ApplicationContext;
	};


	//@name Singleton utility
	//@{
	/** Some utility macro to build singleton compatible with
	 *	the dynamic loading of library
	 *	This macro must be put inside the singleton class
	 *	definition.
	 *	Warning : this macro change the current access right, it end up with
	 *	private access right.
	 */
#define NLMISC_SAFE_SINGLETON_DECL(className) \
	private:\
		/* declare private constructors*/ \
		/*className () {}*/\
		className (const className &) {}\
		/* the local static pointer to the singleton instance */ \
		static className	*_Instance; \
	public:\
		static className &getInstance() \
		{ \
			if (_Instance == NULL) \
			{ \
				/* the nel context MUST be initialised */ \
				nlassertex(NLMISC::INelContext::isContextInitialised(), ("You are trying to access a safe singleton without having initialized a NeL context. The simplest correction is to add 'NLMISC::CApplicationContext myApplicationContext;' at the very begining of your application.")); \
				void *ptr = NLMISC::INelContext::getInstance().getSingletonPointer(#className); \
				if (ptr == NULL) \
				{ \
					/* allocate the singleton and register it */ \
					_Instance = new className; \
					NLMISC::INelContext::getInstance().setSingletonPointer(#className, _Instance); \
				} \
				else \
				{ \
					_Instance = reinterpret_cast<className*>(ptr); \
				} \
			} \
			return *_Instance; \
		} \
	private:

	/** The same as above, but generate a getInstance method that
	 *	return a pointer instead of a reference
	 */
#define NLMISC_SAFE_SINGLETON_DECL_PTR(className) \
	private:\
		/* declare private constructors*/ \
		/*className () {}*/\
		className (const className &) {}\
		/* the local static pointer to the singleton instance */ \
		static className	*_Instance; \
	public:\
		static className *getInstance() \
		{ \
			if (_Instance == NULL) \
			{ \
				/* the nel context MUST be initialised */ \
				nlassertex(NLMISC::INelContext::isContextInitialised(), ("You are trying to access a safe singleton without having initialized a NeL context. The simplest correction is to add 'NLMISC::CApplicationContext myApplicationContext;' at the very begining of your application.")); \
				void *ptr = NLMISC::INelContext::getInstance().getSingletonPointer(#className); \
				if (ptr == NULL) \
				{ \
					/* allocate the singleton and register it */ \
					_Instance = new className; \
					NLMISC::INelContext::getInstance().setSingletonPointer(#className, _Instance); \
				} \
				else \
				{ \
					_Instance = reinterpret_cast<className*>(ptr); \
				} \
			} \
			return _Instance; \
		} \
	private:

	 /** This macro is the complement of the previous one.
	 *	It must be put in a cpp file to implement the static 
	 *	property of the singleton.
	 */
#define NLMISC_SAFE_SINGLETON_IMPL(className) className *className::_Instance = NULL;

/// Function type for library entry point
typedef void (*TInitLibraryFunc)(INelContext &applicationContext);

/** An helper macro to build the dll entry point easily.
 */
#define NLMISC_LIBRARY_ENTRY													\
	void libraryEntryImp(NLMISC::INelContext &applicationContext)					\
	{																		\
		nlassert(!NLMISC::INelContext::isContextInitialised() || &applicationContext == &(NLMISC::INelContext::getInstance()));	\
																			\
		if (!NLMISC::INelContext::isContextInitialised())												\
		{																	\
			new CLibraryContext(applicationContext);								\
		}																	\
	}																		\
	extern "C"																\
	{																		\
		NLMISC_LIB_EXPORT TInitLibraryFunc libraryEntry = libraryEntryImp;		\
	}																		\


class CLibrary;
/// helper function to init newly loaded nel library
void initNelLibrary(CLibrary &lib);


} // namespace NLMISC


#endif //APP_CONTEXT_H
