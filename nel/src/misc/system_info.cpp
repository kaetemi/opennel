/** \file system_info.cpp
 * <File description>
 *
 * $Id: system_info.cpp,v 1.1 2001/01/08 16:28:13 lecroart Exp $
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

#include <windows.h>
#include <tchar.h>
#include <string>

#include "nel/misc/types_nl.h"

#include "nel/misc/common.h"
#include "nel/misc/system_info.h"

using namespace std;

namespace NLMISC {

string CSystemInfo::getOS ()
{
	string OSString = "Unknown";
#ifdef NL_OS_WINDOWS
	char ver[1024];

	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure,
	// which is supported on Windows 2000.
	//
	// If that fails, try using the OSVERSIONINFO structure.

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
			return "Windows Unknown";
	}

	switch (osvi.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_NT:
		// Test for the product.

		if ( osvi.dwMajorVersion <= 4 )
			OSString = "Microsoft Windows NT ";

		if ( osvi.dwMajorVersion == 5 )
			OSString = "Microsoft Windows 2000 ";

		// Test for workstation versus server.
/* can't access to product type
		if( bOsVersionInfoEx )
		{
			if ( osvi.wProductType == VER_NT_WORKSTATION )
			OSString += "Professional ";

			if ( osvi.wProductType == VER_NT_SERVER )
			OSString += "Server ";
		}
		else
*/		{
			HKEY hKey;
			char szProductType[80];
			DWORD dwBufLen;

			RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\ProductOptions", 0, KEY_QUERY_VALUE, &hKey );
			RegQueryValueEx( hKey, "ProductType", NULL, NULL, (LPBYTE) szProductType, &dwBufLen);
			RegCloseKey( hKey );
			if ( lstrcmpi( "WINNT", szProductType) == 0 )
				OSString += "Workstation ";
			if ( lstrcmpi( "SERVERNT", szProductType) == 0 )
				OSString += "Server ";
		}

		// Display version, service pack (if any), and build number.
		smprintf(ver, 1024, "version %d.%d '%s' (Build %d)", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
		OSString += ver;
		break;

	case VER_PLATFORM_WIN32_WINDOWS:

		if(osvi.dwMinorVersion == 0)
			OSString = "Microsoft Windows 95 ";
		else if (osvi.dwMinorVersion == 10)
			OSString = "Microsoft Windows 98 ";
		else if (osvi.dwMinorVersion == 90)
			OSString = "Microsoft Windows Millenium ";
/* microsoft way
		if ((osvi.dwMajorVersion > 4) || (osvi.dwMajorVersion == 4) && (osvi.dwMinorVersion > 0)))
			OSString += "Microsoft Windows 98 ";
		else
			OSString += "Microsoft Windows 95 ";
*/
		// Display version, service pack (if any), and build number.
		smprintf(ver, 1024, "version %d.%d %s (Build %d)", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
		OSString += ver;
		break;

	case VER_PLATFORM_WIN32s:
		OSString = "Microsoft Win32s";
		break;
	}

#elif defined NL_OS_UNIX

	OSString = "Unix";

#endif	// NL_OS_UNIX

	return OSString;
}

string CSystemInfo::getProc ()
{
	string ProcString = "Unknown";

#ifdef NL_OS_WINDOWS

	LONG result;
	char value[1024];
	DWORD valueSize;
	HKEY hKey;

	result = ::RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Hardware\\Description\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hKey);
	if (result == ERROR_SUCCESS)
	{
		// get processor name
		valueSize = 1024;
		result = ::RegQueryValueEx (hKey, _T("ProcessorNameString"), NULL, NULL, (LPBYTE)&value, &valueSize);
		if (result == ERROR_SUCCESS)
			ProcString = value;
		else
			ProcString = "UnknownProc";

		ProcString += " / ";

		// get processor identifier
		valueSize = 1024;
		result = ::RegQueryValueEx (hKey, _T("Identifier"), NULL, NULL, (LPBYTE)&value, &valueSize);
		if (result == ERROR_SUCCESS)
			ProcString += value;
		else
			ProcString += "UnknownIdentifier";

		ProcString += " / ";

		// get processor vendor
		valueSize = 1024;
		result = ::RegQueryValueEx (hKey, _T("VendorIdentifier"), NULL, NULL, (LPBYTE)&value, &valueSize);
		if (result == ERROR_SUCCESS)
			ProcString += value;
		else
			ProcString += "UnknownVendor";

		ProcString += " / ";
		
		// get processor frequence
		result = ::RegQueryValueEx (hKey, _T("~MHz"), NULL, NULL, (LPBYTE)&value, &valueSize);
		if (result == ERROR_SUCCESS)
		{
			ProcString += itoa (*(int *)value, value, 10);
			ProcString += "MHz";
		}
		else
			ProcString += "UnknownFreq";
	}

	// Make sure to close the reg key

	RegCloseKey (hKey);

#elif defined NL_OS_UNIX


#endif

	return ProcString;
}

string CSystemInfo::getMem ()
{
	string MemString = "Unknown";

#ifdef NL_OS_WINDOWS

	MEMORYSTATUS ms;

	GlobalMemoryStatus (&ms);

	sint extt = 0, extf = 0;
	char *ext2str[] = { "b", "Kb", "Mb", "Gb", "Tb" };

	while (ms.dwTotalPhys > 1024)
	{
		ms.dwTotalPhys /= 1024;
		extt++;
	}
	
	while (ms.dwAvailPhys > 1024)
	{
		ms.dwAvailPhys /= 1024;
		extf++;
	}

	char mem[1024];
	smprintf (mem, 1024, "physical memory: total: %d %s free: %d %s", ms.dwTotalPhys, ext2str[extt], ms.dwAvailPhys, ext2str[extf]);
	MemString = mem;

#elif defined NL_OS_UNIX


#endif

	return MemString;
}



} // NLMISC
