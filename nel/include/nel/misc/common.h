/** \file common.h
 * common algorithms, constants and functions
 *
 * $Id: common.h,v 1.30 2001/06/21 12:35:16 lecroart Exp $
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

#ifndef	NL_COMMON_H
#define	NL_COMMON_H

#include "nel/misc/types_nl.h"

#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>

/// This namespace contains all miscellaneous class used by other module
namespace	NLMISC
{

// Windows posix function remapping
#ifdef NL_OS_WINDOWS
#define vsnprintf _vsnprintf
#endif // NL_OS_WINDOWS



/**
 * \def MaxCStringSize
 *
 * The maximum size allowed for C string (zero terminated string) buffer.
 * This value is used when we have to create a standard C string buffer and we don't know exactly the final size of the string.
 */
const int MaxCStringSize = 1024*64;


/** Pi constant in double format.
 */
const double Pi = 3.1415926535897932384626433832795;


/**
 * \def NLMISC_CONVERT_VARGS(dest,format)
 *
 * This macro converts variable arguments into C string (zero terminated string).
 * This function takes care to avoid buffer overflow.
 *
 * Example:
 *\code
	void MyFunction(const char *format, ...)
	{
		string str;
		NLMISC_CONVERT_VARGS (str, format, NLMISC::MaxCStringSize);
		// str contains the result of the conversion
	}
 *\endcode
 *
 * \param _dest \c string or \c char* that contains the result of the convertion
 * \param _format format of the string, it must be the last argument before the \c '...'
 * \param _size size of the buffer that will contain the C string
 */
#define NLMISC_CONVERT_VARGS(_dest,_format,_size) \
char _cstring[_size]; \
va_list _args; \
va_start (_args, _format); \
int res = vsnprintf (_cstring, _size-1, _format, _args); \
if (res == -1 || res == _size-1) \
{ \
	_cstring[_size-1] = '\0'; \
} \
va_end (_args); \
_dest = _cstring



/** sMart sprintf function. This function do a sprintf and add a zero at the end of the buffer
 * if there no enough room in the buffer.
 *
 * \param buffer a C string
 * \param count Size of the buffer
 * \param format of the string, it must be the last argument before the \c '...'
 */
sint smprintf( char *buffer, size_t count, const char *format, ... );


/** Return a float random inside the interval [0,mod]
 */
inline float frand(float mod)
{
	double	r = (double) rand();
	r/= (double) RAND_MAX;
	return (float)(r * mod);
}


/** Return -1 if f<0, 0 if f==0, 1 if f>1
 */
inline sint fsgn(double f)
{
	if(f<0)
		return -1;
	else if(f>0)
		return 1;
	else
		return 0;
}


/** Return the square of a number
 */
template<class T>	inline T sqr(const T &v)
{
	return v * v;
}


/** Force v to be inside the interval [min,max]. Warning: implicit cast are made if T,U or V are different.
 */
template<class T, class U, class V>	inline void clamp(T &v, const U &min, const V &max)
{
	v = (v < min) ? min : v;
	v = (v > max) ? max : v;
}


/** MIN/MAX extended functions.
 */
template<class T>	inline T minof(const T& a,  const T& b,  const T& c)
	{return std::min(std::min(a,b),c);}
template<class T>	inline T minof(const T& a,  const T& b,  const T& c,  const T& d)
	{return std::min(minof(a,b,c),d);}
template<class T>	inline T minof(const T& a,  const T& b,  const T& c,  const T& d,  const T& e)
	{return std::min(minof(a,b,c,d),e);}
template<class T>	inline T maxof(const T& a,  const T& b,  const T& c)
	{return std::max(std::max(a,b),c);}
template<class T>	inline T maxof(const T& a,  const T& b,  const T& c,  const T& d)
	{return std::max(maxof(a,b,c),d);}
template<class T>	inline T maxof(const T& a,  const T& b,  const T& c,  const T& d,  const T& e)
	{return std::max(maxof(a,b,c,d),e);}


/** \c contReset take a container like std::vector or std::deque and put his size to 0 like \c clear() but free all buffers.
 * This function is useful because \c resize(), \c clear(), \c erase() or \c reserve() methods never realloc when the array size come down.
 * \param a is the container to reset.
 */
template<class T>	inline void contReset (T& a)
{
	a.~T();
	new (&a) T;
}

/** Return the value maximized to the next power of 2 of v.
 * Example:
 *   raiseToNextPowerOf2(8) is 8
 *   raiseToNextPowerOf2(5) is 8
 */
uint			raiseToNextPowerOf2 (uint v);

/** Return the power of 2 of v.
 * Example:
 *   getPowerOf2(8) is 3
 *   getPowerOf2(5) is 3
 */
uint			getPowerOf2 (uint v);

/** Return \c true if the value is a power of 2.
 */
bool			isPowerOf2 (sint32 v);


/** Convert a string in lower case.
 * \param a string to transform to lower case
 */
std::string		&strlwr ( std::string &str );

/** Convert a string in lower case.
 * \param a pointer to char to transform to lower case
 */
char			*strlwr ( char *str );

/** Convert a string in upper case.
 * \param a string to transform to upper case
 */
std::string		&strupr ( std::string &str );

/** Convert a string in upper case.
 * \param a pointer to char to transform to upper case
 */
char			*strupr ( char *str );

/**
 * Base class for all NeL exception.
 * It enables to construct simple string at the ctor.
 */
class Exception : public std::exception
{
protected:
	std::string	_Reason;
public:
	Exception();
	Exception(const std::string &reason);
	Exception(const char *format, ...);
	virtual const char	*what() const throw();
};


/**
 * Portable Sleep() function that suspends the execution of the calling thread for a number of milliseconds.
 * Note: the resolution of the timer is system-dependant and may be more than 1 millisecond.
 */
void nlSleep( uint32 ms );


/// Returns Process Id (note: on Linux, Process Id is the same as the Thread Id)
#ifdef NL_OS_WINDOWS
#	define getpid _getpid
#endif

/// Returns Thread Id (note: on Linux, Process Id is the same as the Thread Id)
uint getThreadId();

/// Returns a readable string from a vector of bytes. unprintable char are replaced by '?'
std::string stringFromVector( const std::vector<uint8>& v );


/** Returns a string corresponding to the class T in string format
 * Example:
 *  string num = toString (1234); // num = "1234";
 */
template<class T> std::string toString (const T &t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}

/// Convert a string into an sint64 (same as atoi() function but for 64 bits intergers)
sint64 atoiInt64 (const char *ident, sint64 base = 10);

/// Convert an sint64 into a string (same as itoa() function but for 64 bits intergers)
void itoaInt64 (sint64 number, char *str, sint64 base = 10);


}	// NLMISC

#endif	// NL_COMMON_H
