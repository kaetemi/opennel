/** \file agent_operator.h
 * Includes class operator for IObjectIA objects.
 *
 * $Id: agent_operator.h,v 1.7 2001/01/11 17:02:18 chafik Exp $
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
#ifndef NL_AGENT_OPERATOR_H
#define NL_AGENT_OPERATOR_H

//#include "nel/ai/c/abstract_interface.h"
#include "nel/ai/agent/baseai.h"


namespace NLAIAGENT
{	
	/**	
	This is an abstract class to put on the defintion of an operator. Not that an operator here is an object where we can add, sub, mul, dive it with an ather, and
	also the object assum all binary operation. 

	* \author Chafik sameh	 
	* \author Portier Pierre
	* \author Nevrax France
	* \date 2000	
	*/
	class IObjetOp: public IObjectIA
	{
	public:		

		/// \name IObjectIA base class method;
		//@{
		tQueue IObjetOp::isMember(const IVarName *,const IVarName *,const IObjectIA &) const;
		CProcessResult IObjetOp::runMethodeMember(sint32 index, IObjectIA *);
		virtual sint32 getMethodIndexSize() const;
		//@}

		IObjetOp()
		{			
		}		

		IObjetOp(const IObjetOp &a):IObjectIA(a)
		{			
		}		

		///throw (NLAIE::CExceptionNotImplemented)
		virtual IObjetOp &operator += (const IObjetOp &)
		{			
			char text[2048*8];
			sprintf(text,"opertor <IObjectIA &operator += (const IObjectIA &a)> note implemented for the '%s' interface",(const char *)getType());
			throw NLAIE::CExceptionNotImplemented(text);
			return *this;
		}

		virtual IObjetOp &operator -= (const IObjetOp &)/// throw (NLAIE::CExceptionNotImplemented)
		{
			char text[2048*8];
			sprintf(text,"opertor <IObjectIA &operator -= (const IObjectIA &a)> note implemented for the '%s' interface",(const char *)getType());
			throw NLAIE::CExceptionNotImplemented(text);
			return *this;
		}

		virtual IObjetOp &neg()/// throw (NLAIE::CExceptionNotImplemented)
		{
			char text[2048*8];
			sprintf(text,"opertor <IObjectIA &neg()> note implemented for the '%s' interface",(const char *)getType());
			throw NLAIE::CExceptionNotImplemented(text);
			return *this;
		}

		virtual IObjetOp &operator *= (const IObjetOp &)/// throw (NLAIE::CExceptionNotImplemented)
		{
			char text[2048*8];
			sprintf(text,"opertor <IObjectIA &operator *= (const IObjectIA &a)> note implemented for the '%s' interface",(const char *)getType());
			throw NLAIE::CExceptionNotImplemented(text);
			return *this;
		}
		virtual IObjetOp &operator /= (const IObjetOp &)/// throw (NLAIE::CExceptionNotImplemented)
		{
			char text[2048*8];
			sprintf(text,"opertor <IObjectIA &operator /= (const IObjectIA &a)> note implemented for the '%s' interface",(const char *)getType());
			throw NLAIE::CExceptionNotImplemented(text);
			return *this;
		}	

		virtual IObjetOp *operator + (const IObjetOp &) const/// throw (NLAIE::CExceptionNotImplemented) 
		{
			char text[2048*8];
			sprintf(text,"opertor <virtual IObjetOp *operator + (const IObjetOp &a)  const> note implemented for the '%s' interface",(const char *)getType());
			throw NLAIE::CExceptionNotImplemented(text);
			return (IObjetOp *)this;
		}

		virtual IObjetOp *operator - (const IObjetOp &)  const/// throw (NLAIE::CExceptionNotImplemented)
		{
			char text[2048*8];
			sprintf(text,"opertor <virtual IObjetOp *operator - (const IObjetOp &a)  const> note implemented for the '%s' interface",(const char *)getType());
			throw NLAIE::CExceptionNotImplemented(text);
			return (IObjetOp *)this;
		}

		virtual const IObjetOp *operator * (const IObjetOp &)  const ///throw (NLAIE::CExceptionNotImplemented)
		{
			char text[2048*8];
			sprintf(text,"opertor <virtual IObjetOp *operator * (const IObjetOp &a)  const> note implemented for the '%s' interface",(const char *)getType());
			throw NLAIE::CExceptionNotImplemented(text);
			return (IObjetOp *)this;
		}

		virtual const IObjetOp *operator / (const IObjetOp &)  const ///throw (NLAIE::CExceptionNotImplemented)
		{
			char text[2048*8];
			sprintf(text,"opertor <virtual IObjetOp *operator / (const IObjetOp &a)  const> note implemented for the '%s' interface",(const char *)getType());
			throw NLAIE::CExceptionNotImplemented(text);
			return (IObjetOp *)this;
		}
		

		virtual IObjetOp *operator < (IObjetOp &)  const ///throw (NLAIE::CExceptionNotImplemented)
		{
			char text[2048*8];
			sprintf(text,"opertor <virtual IObjetOp *operator < (const IObjetOp &a)  const> note implemented for the '%s' interface",(const char *)getType());
			throw NLAIE::CExceptionNotImplemented(text);
			return (IObjetOp *)this;
		}
		virtual IObjetOp *operator > (IObjetOp &)  const ///throw (NLAIE::CExceptionNotImplemented)
		{
			char text[2048*8];
			sprintf(text,"opertor <virtual IObjetOp *operator > (const IObjetOp &a)  const> note implemented for the '%s' interface",(const char *)getType());
			throw NLAIE::CExceptionNotImplemented(text);
			return (IObjetOp *)this;
		}

		virtual IObjetOp *operator <= (IObjetOp &)  const ///throw (NLAIE::CExceptionNotImplemented)
		{
			char text[2048*8];
			sprintf(text,"opertor <virtual IObjetOp *operator <= (const IObjetOp &a)  const> note implemented for the '%s' interface",(const char *)getType());
			throw NLAIE::CExceptionNotImplemented(text);
			return (IObjetOp *)this;
		}

		virtual IObjetOp *operator >= (IObjetOp &)  const ///throw (NLAIE::CExceptionNotImplemented)
		{
			char text[2048*8];
			sprintf(text,"opertor <virtual IObjetOp *operator >= (const IObjetOp &a)  const> note implemented for the '%s' interface",(const char *)getType());
			throw NLAIE::CExceptionNotImplemented(text);
			return (IObjetOp *)this;
		}
		
		virtual IObjetOp *operator ! ()  const ///throw (NLAIE::CExceptionNotImplemented)
		{
			char text[2048*8];
			sprintf(text,"opertor <virtual IObjetOp *operator ! ()  const> note implemented for the '%s' interface",(const char *)getType());
			throw NLAIE::CExceptionNotImplemented(text);
			return (IObjetOp *)this;
		}
		virtual IObjetOp *operator != (IObjetOp &) const ///throw (NLAIE::CExceptionNotImplemented)
		{
			char text[2048*8];
			sprintf(text,"opertor <virtual IObjetOp *operator != (const IObjetOp &a)  const> note implemented for the '%s' interface",(const char *)getType());
			throw NLAIE::CExceptionNotImplemented(text);
			return (IObjetOp *)this;
		}		

		virtual IObjetOp *operator == (IObjetOp &) const ///throw (NLAIE::CExceptionNotImplemented)
		{
			char text[2048*8];
			sprintf(text,"opertor <virtual IObjetOp *operator == (const IObjetOp &a)  const> note implemented for the '%s' interface",(const char *)getType());
			throw NLAIE::CExceptionNotImplemented(text);
			return (IObjetOp *)this;
		}

		virtual bool isTrue() const = 0;///throw (NLAIE::CExceptionNotImplemented) = 0

		virtual ~IObjetOp()
		{			
		}
	};

	inline bool IObjetOp::isTrue() const ///throw (NLAIE::CExceptionNotImplemented)
	{			
		char text[2048*8];
		sprintf(text,"'bool isTrue() const' note implemented for the '%s' interface",(const char *)getType());
		throw NLAIE::CExceptionNotImplemented(text);
		return false;		
	}
}

#endif
