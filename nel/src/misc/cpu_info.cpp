/** \file cpu_info.cpp
 * <File description>
 *
 * $Id: cpu_info.cpp,v 1.2 2001/10/26 08:33:07 vizerie Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "nel/misc/cpu_info.h"

#include <stdio.h>


namespace NLMISC 
{

static bool DetectMMX(void)
{		
	#ifdef NL_OS_WINDOWS		
		if (!CCpuInfo::hasCPUID()) return false; // cpuid not supported ...

		uint32 result = 0;
		__asm
		{
			 mov  eax,1
			 cpuid
			 test edx,0x800000  // bit 23 = MMX instruction set
			 je   noMMX
			 mov result, 1	
			noMMX:
		}

		return result == 1;
 

		printf("mmx detected\n");

	#else
		return false;
	#endif
}


static bool DetectSSE(void)
{	
	#if 0 // not activated for now ...
		#ifdef NL_OS_WINDOWS
			if (!CCpuInfo::hasCPUID()) return false; // cpuid not supported ...

			uint32 result = 0;
			__asm
			{			
				mov eax, 1   // request for feature flags
				cpuid 							
				test EDX, 002000000h   // bit 25 in feature flags equal to 1
				je noSSE
				mov result, 1  // sse detected
			noSSE:
			}


			if (result)
			{
				// check OS support for SSE
				try 
				{
					__asm
					{
						xorps xmm0, xmm0  // Streaming SIMD Extension
					}
				}
				catch(...)
				{
					return false;
				}
			
				return true;
			}
			else
			{
				return false;
			}
		#else
			printf("sse not detected\n");
			return false
		#endif
	#else
		return false;
	#endif
}

bool HasMMX = DetectMMX();
bool HasSSE = DetectSSE();

bool CCpuInfo::hasCPUID(void)
{
	#ifdef NL_OS_WINDOWS
		 uint32 result;
		 __asm
		 {
			 pushad
			 pushfd						
			 //	 If ID bit of EFLAGS can change, then cpuid is available
			 pushfd
			 pop  eax					// Get EFLAG
			 mov  ecx,eax
			 xor  eax,0x200000			// Flip ID bit
			 push eax
			 popfd						// Write EFLAGS
			 pushfd      
			 pop  eax					// read back EFLAG
			 xor  eax,ecx				
			 je   noCpuid				// no flip -> no CPUID instr.
			 
			 popfd						// restore state
			 popad
			 mov  result, 1
			 jmp  CPUIDPresent
		
			noCpuid:
			 popfd					    // restore state
			 popad
			 mov result, 0
			CPUIDPresent:	 
		 }
		 return result == 1;
	#else
		 return false;
	#endif
}
bool CCpuInfo::hasMMX(void) { return HasMMX; }
bool CCpuInfo::hasSSE(void) { return HasSSE; }

} // NLMISC
