/** \file fast_mem.cpp
 * Fast memory copy and precache
 *
 * $Id$
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#include "stdmisc.h"

#include "nel/misc/fast_mem.h"
#include "nel/misc/system_info.h"


namespace NLMISC
{

#ifdef NL_OS_WINDOWS


// ***************************************************************************
#ifdef NL_OS_WINDOWS
#pragma managed(push, off)
#endif
void		*CFastMem::memcpySSE(void *dest, const void *src, size_t nbytes)
{
	_asm 
	{
			mov esi, src 
			mov edi, dest 
			mov ebx, nbytes 

			// edx takes number of bytes%64
			mov	edx, ebx
			and edx, 63

			// ebx takes number of bytes/64
			shr	ebx, 6
			jz	byteCopy


	loop4k: // flush 4k into temporary buffer 
			push esi 
			mov ecx, ebx
			// copy per block of 64 bytes. Must not override 64*64= 4096 bytes.
			cmp ecx, 64
			jle	skipMiniMize
			mov	ecx, 64
	skipMiniMize:
			// eax takes the number of 64bytes packet for this block.
			mov eax, ecx

	loopMemToL1: 
			prefetchnta 64[ESI] // Prefetch next loop, non-temporal 
			prefetchnta 96[ESI] 

			movq mm1,  0[ESI] // Read in source data 
			movq mm2,  8[ESI] 
			movq mm3, 16[ESI] 
			movq mm4, 24[ESI] 
			movq mm5, 32[ESI] 
			movq mm6, 40[ESI] 
			movq mm7, 48[ESI] 
			movq mm0, 56[ESI] 

			add esi, 64 
			dec ecx 
			jnz loopMemToL1 

			pop esi // Now copy from L1 to system memory 
			mov ecx, eax

	loopL1ToMem: 
			movq mm1, 0[ESI] // Read in source data from L1 
			movq mm2, 8[ESI] 
			movq mm3, 16[ESI] 
			movq mm4, 24[ESI] 
			movq mm5, 32[ESI] 
			movq mm6, 40[ESI] 
			movq mm7, 48[ESI] 
			movq mm0, 56[ESI] 

			movntq 0[EDI], mm1 // Non-temporal stores 
			movntq 8[EDI], mm2 
			movntq 16[EDI], mm3 
			movntq 24[EDI], mm4 
			movntq 32[EDI], mm5 
			movntq 40[EDI], mm6 
			movntq 48[EDI], mm7 
			movntq 56[EDI], mm0 

			add esi, 64 
			add edi, 64 
			dec ecx 
			jnz loopL1ToMem

			// Do next 4k block 
			sub ebx, eax
			jnz loop4k 

			emms

	byteCopy:
			// Do last bytes with std cpy
			mov	ecx, edx
			rep movsb
	}
	return dest;
}
#ifdef NL_OS_WINDOWS
#pragma managed(pop)
#endif

// ***************************************************************************
#ifdef NL_OS_WINDOWS
#pragma managed(push, off)
#endif
void		CFastMem::precacheSSE(const void *src, uint nbytes)
{
	_asm 
	{ 
			mov esi, src 
			mov ecx, nbytes
			// 64 bytes per pass
			shr ecx, 6 
			jz endLabel

	loopMemToL1: 
			prefetchnta 64[ESI] // Prefetch next loop, non-temporal 
			prefetchnta 96[ESI] 

			movq mm1,  0[ESI] // Read in source data 
			movq mm2,  8[ESI] 
			movq mm3, 16[ESI] 
			movq mm4, 24[ESI] 
			movq mm5, 32[ESI] 
			movq mm6, 40[ESI] 
			movq mm7, 48[ESI] 
			movq mm0, 56[ESI]

			add esi, 64 
			dec ecx 
			jnz loopMemToL1 

			emms

	endLabel:
	}
}
#ifdef NL_OS_WINDOWS
#pragma managed(pop)
#endif

// ***************************************************************************
#ifdef NL_OS_WINDOWS
#pragma managed(push, off)
#endif
void		CFastMem::precacheMMX(const void *src, uint nbytes)
{
	_asm 
	{ 
			mov esi, src 
			mov ecx, nbytes
			// 64 bytes per pass
			shr ecx, 6 
			jz endLabel

	loopMemToL1: 
			movq mm1,  0[ESI] // Read in source data 
			movq mm2,  8[ESI] 
			movq mm3, 16[ESI] 
			movq mm4, 24[ESI] 
			movq mm5, 32[ESI] 
			movq mm6, 40[ESI] 
			movq mm7, 48[ESI] 
			movq mm0, 56[ESI]

			add esi, 64 
			dec ecx 
			jnz loopMemToL1 

			emms

	endLabel:
	}
}
#ifdef NL_OS_WINDOWS
#pragma managed(pop)
#endif

// ***************************************************************************
void		CFastMem::precache(const void *src, uint nbytes)
{
	if(NLMISC::CSystemInfo::hasSSE())
		precacheSSE(src, nbytes);
	else if(NLMISC::CSystemInfo::hasMMX())
		precacheMMX(src, nbytes);
}


#else


// ***************************************************************************
void		*CFastMem::memcpySSE(void *dst, const void *src, size_t nbytes)
{
	// Use std memcpy.
	return memcpy(dst, src, nbytes);
}
void		CFastMem::precacheSSE(const void *src, uint nbytes)
{
	// no-op.
}
void		CFastMem::precacheMMX(const void *src, uint nbytes)
{
	// no-op.
}
void		CFastMem::precache(const void *src, uint nbytes)
{
	// no-op.
}

#endif

typedef void  *(*memcpyPtr)(void *dts, const void *src, size_t nbytes);

static memcpyPtr findBestmemcpy ()
{
#ifdef NL_OS_WINDOWS
	if (CSystemInfo::hasSSE ())
		return CFastMem::memcpySSE;
	else
		return ::memcpy;
#else // NL_OS_WINDOWS
	return ::memcpy;
#endif // NL_OS_WINDOWS
}

void  *(*CFastMem::memcpy)(void *dts, const void *src, size_t nbytes) = findBestmemcpy ();

} // NLMISC
