/** \file module_message.h
 * module message definition
 *
 * $Id: module_message.h,v 1.3.6.2 2006/01/11 15:02:09 boucher Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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


#ifndef NL_MODULE_MESSAGE_H
#define NL_MODULE_MESSAGE_H

#include "nel/misc/enum_bitset.h"
#include "nel/net/message.h"
#include "module_common.h"


namespace NLNET
{
	/** Module message header coder/decoder
	 *	Codec for module message header data.
	 */
	class CModuleMessageHeaderCodec
	{
	public:
		enum TMessageType
		{
			/// Standard one way message
			mt_oneway,
			/// Two way request
			mt_twoway_request,
			/// Two way response
			mt_twoway_response,


			/// A special checking value
			mt_num_types,
			/// invalid flag
			mt_invalid = mt_num_types

		};

		static void encode(CMessage &headerMessage, TMessageType msgType, TModuleId senderProxyId, TModuleId addresseeProxyId)
		{
			serial(headerMessage, msgType, senderProxyId, addresseeProxyId);
		}

		static void decode(const CMessage &headerMessage, TMessageType &msgType, TModuleId &senderProxyId, TModuleId &addresseeProxyId)
		{
			serial(const_cast<CMessage&>(headerMessage), msgType, senderProxyId, addresseeProxyId);
		}

	private:
		static void serial(CMessage &headerMessage, TMessageType &msgType, TModuleId &senderProxyId, TModuleId &addresseeProxyId)
		{
			uint8 mt;
			if (headerMessage.isReading())
			{
				headerMessage.serial(mt);
				msgType = CModuleMessageHeaderCodec::TMessageType(mt);
			}
			else
			{
				mt = msgType;
				headerMessage.serial(mt);
			}
			headerMessage.serial(senderProxyId);
			headerMessage.serial(addresseeProxyId);
		}
	};


	/** An utility struct to serial binary buffer.
	 *	WARNING : you must be aware that using binary buffer serialisation
	 *	is not fair with the portability and endiennes problem.
	 *	Use it ONLY when you now what you are doing and when
	 *	bytes ordering is not an issue.
	 */
	struct TBinBuffer
	{
	private:
		uint8			*_Buffer;
		uint32			_BufferSize;
		mutable bool	_Owner;

	public:
		
		/// default constructor, used to read in stream
		TBinBuffer()
			:	_Buffer(NULL),
				_BufferSize(0),
				_Owner(true)
		{
		}

		TBinBuffer(const uint8 *buffer, uint32 bufferSize)
			:	_Buffer(const_cast<uint8*>(buffer)),
				_BufferSize(bufferSize),
				_Owner(false)
		{
		}

		// copy constructor transfere ownership of the buffer (if it is owned)
		TBinBuffer(const TBinBuffer &other)
			:	_Buffer(other._Buffer),
				_BufferSize(other._BufferSize),
				_Owner(other._Owner)
		{
			// remove owning on source
			other._Owner = false;
		}

		~TBinBuffer()
		{
			if (_Owner && _Buffer != NULL)
				delete _Buffer;
		}

		void serial(NLMISC::IStream &s)
		{
			if (s.isReading())
			{
				nlassert(_Buffer == NULL);
				s.serial(_BufferSize);

				_Buffer = new uint8[_BufferSize];

				s.serialBuffer(_Buffer, _BufferSize);
			}
			else
			{
				s.serialBufferWithSize(_Buffer, _BufferSize);
			}
		}

		uint32 getBufferSize() const
		{
			return _BufferSize;
		}

		uint8 *getBuffer() const
		{
			return _Buffer;
		}
			

		/** Release the buffer by returning the pointer to the caller.
		 *	The caller is now owner and responsible of the allocated
		 *	buffer.
		 */
		uint8 *release()
		{
			nlassert(_Owner);
			_Owner = false;
			return _Buffer;
		}
	};


} // namespace NLNET

#endif // NL_MODULE_MESSAGE_H
