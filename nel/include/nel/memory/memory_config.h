/** \file memory_config.h
 * Memory manager configuraition
 *
 * $Id: memory_config.h,v 1.1 2003/07/01 15:33:14 corvazier Exp $
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

#ifndef NL_MEMORY_CONFIG_H
#define NL_MEMORY_CONFIG_H

// Define this to disable debug features (use to trace buffer overflow and add debug informations in memory headers)
#define NL_HEAP_ALLOCATION_NDEBUG

// Define this to activate internal checks (use to debug the heap code)
// #define NL_HEAP_ALLOCATOR_INTERNAL_CHECKS

// Define this to disable small block optimizations
//#define NL_HEAP_NO_SMALL_BLOCK_OPTIMIZATION

// Stop when free a NULL pointer
//#define NL_HEAP_STOP_NULL_FREE

#endif // NL_MEMORY_CONFIG_H

/* End of memory_mutex.h */