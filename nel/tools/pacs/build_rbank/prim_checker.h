/** \file prim_checker.h
 * <File description>
 *
 * $Id: prim_checker.h,v 1.1 2003/08/27 09:23:07 legros Exp $
 */

/* Copyright, 2000-2003 Nevrax Ltd.
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

#ifndef NL_PRIM_CHECKER_H
#define NL_PRIM_CHECKER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"

namespace NLMISC
{
	class CVectorD;
};

namespace NLLIGO
{
	class IPrimitive;
	class CPrimZone;
};

/**
 * A class that reads .primitive files from ligo and that allows
 * to define flags over pacs surfaces with CPrimZone
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CPrimChecker
{
public:

	/// Constructor
	CPrimChecker();


	/// Bits value for landscape
	enum
	{
		Include = 1,
		Exclude = 2,
		ClusterHint = 4
	};


	/// Init CPrimChecker
	bool	init(const std::string &primitivesPath, const std::string &outputDirectory = "./", bool forceRebuild = false);

	/// Reads bits value at a given position
	uint8	get(uint x, uint y)	const	{ return _Grid.get(x, y); }

private:

	/// \name Grid management
	//@{

	/**
	 * A class that allows to store 65536x65536 elements with minimum allocation and dynamic allocation of elements
	 */
	class CGrid
	{
	public:

		/// Constructor
		CGrid()		{ clear(); }

		/// Set bits in grid
		void	set(uint x, uint y, uint8 bits)
		{
			CGridCell	*cell = _Grid[((x&0xff00)>>8) + (y&0xff00)];
			if (cell == NULL)
			{
				cell = new CGridCell();
				_Grid[((x&0xff00)>>8) + (y&0xff00)] = cell;
			}
			cell->set(x, y, bits);
		}

		/// Get bits in grid
		uint8	get(uint x, uint y) const
		{
			CGridCell	*cell = _Grid[((x&0xff00)>>8) + (y&0xff00)];
			return cell != NULL ? cell->get(x, y) : (uint8)0;
		}


		/// Clear grid
		void	clear()
		{
			for (uint i=0; i<256*256; ++i)
				if (_Grid != NULL)	
					delete _Grid[i],
					_Grid[i] = NULL;
		}

		/// Serializes
		void	serial(NLMISC::IStream &f)
		{
			f.serialCheck((uint32)('PCHK'));
			f.serialVersion(0);

			if (f.isReading())
				clear();

			for (uint i=0; i<256*256; ++i)
			{
				bool	present = (_Grid[i] != NULL);
				f.serial(present);

				if (present)
				{
					if (_Grid[i] == NULL)
						_Grid[i] = new CGridCell();
					_Grid[i]->serial(f);
				}
			}
		}

	private:

		/**
		 * A class that allows to store 256x256 elements
		 */
		class CGridCell
		{
		public:

			/// Constructor
			CGridCell()		{ for (uint i=0; i<256*256; ++i)	_Grid[i] = 0; }

			/// Set bits in grid
			void	set(uint x, uint y, uint8 bits)		{ _Grid[(x&0xff) + ((y&0xff)<<8)] |= bits; }

			/// Get bits in grid
			uint8	get(uint x, uint y) const			{ return _Grid[(x&0xff) + ((y&0xff)<<8)]; }


			/// Serializes
			void	serial(NLMISC::IStream &f)			{ f.serialBuffer(_Grid, 256*256); }

		private:
			uint8		_Grid[256*256];
		};

		CGridCell		*_Grid[256*256];
	};

	CGrid		_Grid;

	//@}



	/// Reads a primitive file and renders contained primitives into grid
	void	readFile(const std::string &filename);

	/// Reads a primitive and its sub primitives
	void	readPrimitive(NLLIGO::IPrimitive *primitive);

	/// Renders a primitive
	void	render(NLLIGO::CPrimZone *zone, uint8 bits);

};


#endif // NL_PRIM_CHECKER_H

/* End of prim_checker.h */
