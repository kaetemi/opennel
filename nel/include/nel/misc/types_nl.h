/** \file types_nl.h
 * Basic types, define and class
 *
 * $Id$
 *
 * Available constantes:
 * - NL_OS_WINDOWS		: windows operating system (32bits only)
 * - NL_OS_UNIX			: unix operating system (GNU/Linux and other)
 *
 * - NL_LITTLE_ENDIAN	: x86 processor
 * - NL_BIG_ENDIAN		: other processor
 *
 * - NL_DEBUG			: no optimization, full debug information, all log for the client
 * - NL_RELEASE			: full optimization, no debug information, no log for the client
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

#ifndef NL_TYPES_H
#define NL_TYPES_H


// nelconfig.h inclusion, file generated by autoconf
#ifdef HAVE_NELCONFIG_H
#	include "nelconfig.h"
#endif // HAVE_NELCONFIG_H


// Define NL_STATIC to 1 in your project if you want only one driver inside the exe (no .dll at all)

// Uncomment this define if you want to disable the MENU key on Windows (F10, ALT and ALT+SPACE key doesn't freeze or open the menu)
#define NL_DISABLE_MENU 1

// Uncomment this define if you don't want generation of nel debug file (report_*, exception_catched, breakpointed, ...)
#define NL_NO_DEBUG_FILES 1



#ifdef FINAL_VERSION
	// If the FINAL_VERSION is defined externally, check that the value is 0 or 1
#	if FINAL_VERSION != 1 && FINAL_VERSION != 0
#		error "Bad value for FINAL_VERSION, it must be 0 or 1"
#	endif
#else
	// If you want to compile in final version just put 1 instead of 0
	// WARNING: never comment this #define
#	define FINAL_VERSION 0
#endif // FINAL_VERSION

// Operating systems definition

#ifdef WIN32
#	define NL_OS_WINDOWS
#	define NL_LITTLE_ENDIAN
#	define NL_CPU_INTEL
#   ifndef _WIN32_WINNT
#		define _WIN32_WINNT 0x0400
#   endif
#   ifdef __SGI_STL_STLPORT
#       define NL_COMP_STLPORT
#   endif
#	if _MSC_VER >= 1500
#		define NL_COMP_VC9
#		include <string> // This way we know about _HAS_TR1 :O
#		if defined(_HAS_TR1) && (_HAS_TR1 + 0) // VC9 TR1 feature pack
#			define NL_ISO_STDTR1_AVAILABLE
#			define NL_ISO_STDTR1_HEADER(header) <header>
#		endif
#	elif _MSC_VER >= 1400
#		define NL_COMP_VC8
#	elif _MSC_VER >= 1310
#		define NL_COMP_VC71
#	elif _MSC_VER >= 1300
#		define NL_COMP_VC7
#	elif _MSC_VER >= 1200
#		define NL_COMP_VC6
#		define NL_COMP_NEED_PARAM_ON_METHOD
#	endif
#	ifdef _DEBUG
#		define NL_DEBUG
#		ifndef _STLP_USE_DEBUG_LIB
#			define _STLP_USE_DEBUG_LIB		// we have to put this to include the stlport_debug.lib instead of stlport.lib
#		endif
#	else
#		ifndef NL_RELEASE_DEBUG
#			define NL_RELEASE
#		endif
#	endif
	// define NOMINMAX to be sure that windows includes will not define min max macros, but instead, use the stl template
#	define NOMINMAX
#else
#	ifdef __APPLE__
#		define NL_OS_MAC
#		ifdef __BIG_ENDIAN__
#			define NL_BIG_ENDIAN
#		elif defined(__LITTLE_ENDIAN__)
#			define NL_LITTLE_ENDIAN
#		else
#			error "Cannot detect the endianness of this Mac"
#		endif
#	else
#		ifdef WORDS_BIGENDIAN
#			define NL_BIG_ENDIAN
#		else
#			define NL_LITTLE_ENDIAN
#		endif
#	endif
// these define are set the linux and mac os
#	define NL_OS_UNIX
#	define NL_COMP_GCC
#endif

// Mode checks: NL_DEBUG and NL_DEBUG_FAST are allowed at the same time, but not with any release mode
// (by the way, NL_RELEASE and NL_RELEASE_DEBUG are not allowed at the same time, see above)
#if defined (NL_DEBUG) || defined (NL_DEBUG_FAST)
#	if defined (NL_RELEASE) || defined (NL_RELEASE_DEBUG)
#		error "Error in preprocessor directives for NeL debug mode!"
#	endif
#endif

// gcc 3.4 introduced ISO C++ with tough template rules
//
// NL_ISO_SYNTAX can be used using #if NL_ISO_SYNTAX or #if !NL_ISO_SYNTAX
//
// NL_ISO_TEMPLATE_SPEC can be used in front of an instanciated class-template member data definition,
// because sometimes MSVC++ 6 produces an error C2908 with a definition with template <>.
#if defined(NL_OS_WINDOWS) || (defined(__GNUC__) && ((__GNUC__ < 3) || (__GNUC__ == 3 && __GNUC_MINOR__ <= 3)))
#	define NL_ISO_SYNTAX 0
#	define NL_ISO_TEMPLATE_SPEC
#else
#	define NL_ISO_SYNTAX 1
#	define NL_ISO_TEMPLATE_SPEC template <>
#endif

// gcc 4.1+ provides std::tr1
#if defined(__GNUC__) && ((__GNUC__ > 4) || (__GNUC__ == 4) && (__GNUC_MINOR__ > 1))
#	define NL_ISO_STDTR1_AVAILABLE
#	define NL_ISO_STDTR1_HEADER(header) <tr1/header>
#endif

// Remove stupid Visual C++ warnings

#ifdef NL_OS_WINDOWS
#	pragma warning (disable : 4503)			// STL: Decorated name length exceeded, name was truncated
#	pragma warning (disable : 4786)			// STL: too long identifier
#	pragma warning (disable : 4290)			// throw() not implemented warning
#	pragma warning (disable : 4250)			// inherits via dominance (informational warning).
#	pragma warning (disable : 4390)			// don't warn in empty block "if(exp) ;"
// Debug : Sept 01 2006
#	if defined(NL_COMP_VC8) || defined(NL_COMP_VC9)
#		pragma warning (disable : 4005)			// don't warn on redefinitions caused by xp platform sdk
#		pragma warning (disable : 4996)			// don't warn for deprecated function (sprintf, sscanf in VS8)
#	endif // NL_COMP_VC8 || NL_COMP_VC9 
#endif // NL_OS_WINDOWS


// Standard include

#include <string>
#include <exception>

// Setup extern asm functions.

#ifndef NL_NO_ASM							// If NL_NO_ASM is externely defined, don't override it.
#	ifndef NL_CPU_INTEL						// If not on an Intel compatible plateforme (BeOS, 0x86 Linux, Windows)
#		define NL_NO_ASM						// Don't use extern ASM. Full C++ code.
#	endif // NL_CPU_INTEL
#endif // NL_NO_ASM

// Standard types

/*
 * correct numeric types:	sint8, uint8, sint16, uint16, sint32, uint32, sint64, uint64, sint, uint
 * correct char types:		char, string, ucchar, ucstring
 * correct misc types:		void, bool, float, double
 *
 */

/**
 * \typedef uint8
 * An unsigned 8 bits integer (use char only as \b character and not as integer)
 **/

/**
 * \typedef sint8
 * An signed 8 bits integer (use char only as \b character and not as integer)
 */

/**
 * \typedef uint16
 * An unsigned 16 bits integer (don't use short)
 **/

/**
 * \typedef sint16
 * An signed 16 bits integer (don't use short)
 */

/**
 * \typedef uint32
 * An unsigned 32 bits integer (don't use int or long)
 **/

/**
 * \typedef sint32
 * An signed 32 bits integer (don't use int or long)
 */

/**
 * \typedef uint64
 * An unsigned 64 bits integer (don't use long long or __int64)
 **/

/**
 * \typedef sint64
 * An signed 64 bits integer (don't use long long or __int64)
 */

/**
 * \typedef uint
 * An unsigned integer, at least 32 bits (used only for internal loops or speedy purpose, processor dependent)
 **/

/**
 * \typedef sint
 * An signed integer at least 32 bits (used only for internal loops or speedy purpose, processor dependent)
 */

/**
 * \def NL_I64
 * Used to display a int64 in a platform independent way with printf like functions.
 \code
 sint64 myint64 = SINT64_CONSTANT(0x123456781234);
 printf("This is a 64 bits int: %"NL_I64"u", myint64);
 \endcode
 */

#ifdef NL_OS_WINDOWS

typedef	signed		__int8		sint8;
typedef	unsigned	__int8		uint8;
typedef	signed		__int16		sint16;
typedef	unsigned	__int16		uint16;
typedef	signed		__int32		sint32;
typedef	unsigned	__int32		uint32;
typedef	signed		__int64		sint64;
typedef	unsigned	__int64		uint64;

typedef				int			sint;			// at least 32bits (depend of processor)
typedef	unsigned	int			uint;			// at least 32bits (depend of processor)

#define	NL_I64 "I64"

#ifndef NL_ISO_STDTR1_AVAILABLE
#	include <hash_map>
#	include <hash_set>
#	if defined(NL_COMP_VC7) || defined(NL_COMP_VC71) || defined(NL_COMP_VC8) || defined(NL_COMP_VC9) // VC7 through 9
#		define CHashMap stdext::hash_map
#		define CHashSet stdext::hash_set
#		define CHashMultiMap stdext::hash_multimap
#	else // MSVC6
#		define CHashMap ::std::hash_map
#		define CHashSet ::std::hash_set
#		define CHashMultiMap ::std::hash_multimap
#	endif
#endif // NL_ISO_STDTR1_AVAILABLE

#elif defined (NL_OS_UNIX)

#include <sys/types.h>
#include <stdint.h>
#include <climits>

typedef	int8_t		sint8;
typedef	u_int8_t	uint8;
typedef	int16_t		sint16;
typedef	u_int16_t	uint16;
typedef	int32_t		sint32;
typedef	u_int32_t	uint32;
typedef	long long int		sint64;
typedef	unsigned long long int	uint64;

typedef				int			sint;			// at least 32bits (depend of processor)
typedef	unsigned	int			uint;			// at least 32bits (depend of processor)

#define	NL_I64 "ll"

#if defined(NL_COMP_GCC) && !defined(NL_ISO_STDTR1_AVAILABLE) // GCC4
#	include <ext/hash_map>
#	include <ext/hash_set>
#	define CHashMap ::__gnu_cxx::hash_map
#	define CHashSet ::__gnu_cxx::hash_set
#	define CHashMultiMap ::__gnu_cxx::hash_multimap

namespace __gnu_cxx {

template<> struct hash<std::string>
{
	size_t operator()(const std::string &s) const
	{
		return __stl_hash_string(s.c_str());
	}
};

template<> struct hash<uint64>
{
	size_t operator()(const uint64 x) const
	{
		return x;
	}
};

} // END NAMESPACE __GNU_CXX

#endif // NL_COMP_GCC && !NL_ISO_STDTR1_AVAILABLE

#endif // NL_OS_UNIX

// use std::tr1 for CHash* classes, if available (gcc 4.1+ and VC9 with TR1 feature pack)
#ifdef NL_ISO_STDTR1_AVAILABLE
#	include NL_ISO_STDTR1_HEADER(unordered_map)
#	include NL_ISO_STDTR1_HEADER(unordered_set)
#	define CHashMap std::tr1::unordered_map
#	define CHashSet std::tr1::unordered_set
#	define CHashMultiMap std::tr1::unordered_multimap
#endif

/**
 * \typedef ucchar
 * An Unicode character (16 bits)
 */
typedef	uint16	ucchar;


// To define a 64bits constant; ie: UINT64_CONSTANT(0x123456781234)
#ifdef NL_OS_WINDOWS
#  if defined(NL_COMP_VC8) || defined(NL_COMP_VC9)
#    define INT64_CONSTANT(c)	(c##LL)
#    define SINT64_CONSTANT(c)	(c##LL)
#    define UINT64_CONSTANT(c)	(c##LL)
#  else
#    define INT64_CONSTANT(c)	(c)
#    define SINT64_CONSTANT(c)	(c)
#    define UINT64_CONSTANT(c)	(c)
#  endif
#else
#  define INT64_CONSTANT(c)		(c##LL)
#  define SINT64_CONSTANT(c)	(c##LL)
#  define UINT64_CONSTANT(c)	(c##ULL)
#endif

// Define a macro to write template function according to compiler weakness
#ifdef NL_COMP_NEED_PARAM_ON_METHOD
 #define NL_TMPL_PARAM_ON_METHOD_1(p1)	<p1>
 #define NL_TMPL_PARAM_ON_METHOD_2(p1, p2)	<p1, p2>
#else
 #define NL_TMPL_PARAM_ON_METHOD_1(p1)
 #define NL_TMPL_PARAM_ON_METHOD_2(p1, p2)	
#endif

#endif // NL_TYPES_H
