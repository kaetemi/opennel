/** \file codage.cpp
 *
 * $Id: codage.cpp,v 1.17 2001/06/12 09:44:11 chafik Exp $
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

#include "nel/ai/script/compilateur.h"
#include "nel/ai/script/constraint.h"
#ifdef NL_DEBUG 
#ifdef NL_OS_WINDOWS
#include "windows.h"
#endif
#endif

#define PROFILE
#ifdef PROFILE
#include "nel/misc/time_nl.h"
#endif

namespace NLAISCRIPT
{	
	NLAIC::IIO	*CCodeBrancheRun::InputOutput = NULL;	

	void CCodeBrancheRun::save(NLMISC::IStream &os)
	{
		if(_TableCode != NULL)
		{
			sint32 count = _Count;
			os.serial( count );
			for(int i = 0 ;i < _Count; i ++)
			{		
				char text[1024*8];
				sprintf(text,"Opcode '%s' No: %d",(const char *)_TableCode[i]->getType(),i);
				count = i;
				std::string x( text );
				os.serial(x );
				os.serial( (NLAIC::CIdentType &)  _TableCode[i]->getType() );
				os.serial( *_TableCode[i] );
			}
		}
	}

	void CCodeBrancheRun::load(NLMISC::IStream &is)
	{
		NLAIC::CIdentTypeAlloc id;		
		del();

		sint32 count;
		is.serial( count );
		_Count = count;
		_TableCode = new IOpCode * [_Count];
		for(int i = 0 ;i < _Count; i ++)
		{
			std::string s;
			is.serial( s );
#ifdef NL_DEBUG			
			const char *ss = s.data();
#endif
			
			is.serial( id );
			_TableCode[i] = (IOpCode *)id.allocClass();
			_TableCode[i]->load(is);			
		}

	}

	const NLAIC::IBasicType *CCodeBrancheRun::newInstance() const		
	{
		NLAIC::IBasicType *x = new CCodeBrancheRun(1,CHaltOpCode());
		return x;
	}

	void CCodeBrancheRun::getDebugString(std::string &) const
	{		
	}

	const NLAIAGENT::IObjectIA::CProcessResult &CCodeBrancheRun::run()
	{
		_RunState.ResultState = NLAIAGENT::processBuzzy;
		_RunState.Result = NULL;
		CStackPointer	stack,heap;
		CCodeContext p(stack,heap,this,NULL,InputOutput);
		
		NLAIAGENT::TProcessStatement i = NLAIAGENT::processIdle;		
		CVarPStack::_LocalTableRef = &heap[0];
		stack --;

		run(p);

		stack -= (int)stack; 
		heap -= (int)heap; 
		return _RunState;
	}	

	const NLAIAGENT::IObjectIA::CProcessResult &CCodeBrancheRun::run(NLAIAGENT::IObjectIA &self)
	{
		_RunState.ResultState = NLAIAGENT::processBuzzy;
		_RunState.Result = NULL;
		CStackPointer	stack,heap;
		CCodeContext p(stack,heap,this,&self,InputOutput);	
		CVarPStack::_LocalTableRef = &heap[0];
		stack --;

		run(p);

		stack -= (int)stack; 
		heap -= (int)heap; 
		return _RunState;
	}



#ifdef NL_DEBUG 
	bool NL_AI_DEBUG_SERVER = 0;
#endif


#ifdef PROFILE
	NLMISC::TTicks TimeCompile = 0;
	NLMISC::TTicks NbCompile = 0;

#endif


	const NLAIAGENT::IObjectIA::CProcessResult &CCodeBrancheRun::run(CCodeContext &p)
	{		
		NLAIAGENT::TProcessStatement i = NLAIAGENT::processIdle;

#ifdef PROFILE
		NLMISC::TTime time = NLMISC::CTime::getPerformanceTime();
#endif
		
		while(i != NLAIAGENT::processEnd)
		{	
			i = runOpCode(p);
		}

#ifdef PROFILE
		time = NLMISC::CTime::getPerformanceTime() - time;
		//if(time)
		{
			TimeCompile += time;
			NbCompile ++;			
		}
#endif
		_Ip = 0;
		_RunState.ResultState = NLAIAGENT::processIdle;
		return _RunState;
	}
	
	NLAIAGENT::TProcessStatement CCodeBrancheRun::runOpCode(CCodeContext &p)
	{		
		IOpCode &op = nextCode();

#ifdef NL_DEBUG
		if(NL_AI_DEBUG_SERVER)
		{
			std::string chaine;
			op.getDebugResult(chaine,p);
			chaine += "\n";
#ifdef NL_OS_WINDOWS
			OutputDebugString(chaine.c_str());
#endif
		}
#endif		

		return op.runOpCode(p);
	}

	CCodeBrancheRun::CCodeBrancheRun(int N,const IOpCode &op):_TableCode(NULL),_Ip(0),_Count(N)
	{		
		_TableCode = new IOpCode * [N];
		for(int i = 0 ;i < _Count; i ++) 
		{
			_TableCode[i] = (IOpCode *)op.clone();			
		}

	}

	CCodeBrancheRun::CCodeBrancheRun(int N):_TableCode(NULL),_Ip(0),_Count(N)
	{		
		_TableCode = new IOpCode * [N];
		for(int i = 0 ;i < _Count; i ++) 
		{
			_TableCode[i] = NULL;			
		}

	}

	void CCodeBrancheRun::getDebugResult(std::string &str,CCodeContext &P) const
	{		
		_TableCode[_Ip]->getDebugResult(str,P);
	}
	
	const NLAIC::CIdentType &CCodeBrancheRun::getType() const
	{		
		return IdCodeBrancheRun;
	}


	void CBagOfCode::setConstraintIndex(int i,CCodeBrancheRun *c)
	{
		std::list<IConstraint *>::iterator it = _Constraint.begin();
		while(it != _Constraint.end())
		{
			(*it++)->addIndex(i,c);
		}
	}
}
