/** \file baseai.cpp
 *
 * $Id: baseai.cpp,v 1.39 2003/01/21 11:24:39 chafik Exp $
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
#include "nel/ai/agent/agentexport.h"
#include "nel/ai/agent/agent_object.h"
#include "nel/ai/agent/baseai.h"
#include "nel/ai/agent/agent_method_def.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/agent/msg.h"
#include "nel/ai/agent/agent_local_mailer.h"
#include "nel/ai/agent/performative.h"
#include "nel/ai/script/interpret_methodes.h"

namespace NLAIAGENT
{
	void IConnectIA::save(NLMISC::IStream &os)
	{
		IRefrence::save(os);
		TListIBasicCstItr i = _Connection.begin();
		sint32 size = _Connection.size();
		os.serial( size );
		while(i != _Connection.end())
		{	
			IWordNumRef &r = (IWordNumRef &)((const IWordNumRef&)*((const IConnectIA *)*i++));
			os.serial( (NLAIC::CIdentType &) (r.getType()) );
			r.save(os);
		}

		i = _Connected.begin();
		size = _Connected.size(); 
		os.serial( size );
		while(i != _Connected.end())
		{						
			IWordNumRef &r = (IWordNumRef &)((const IWordNumRef&)*((const IConnectIA *)*i++));
			os.serial( (NLAIC::CIdentType &) (r.getType()) );
			r.save(os);
		}
	}	

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

	bool CIdMethod::operator < (const CIdMethod &a) const
	{
		return Weight < a.Weight;
	}

	const CIdMethod &CIdMethod::operator = (const CIdMethod &a)
	{
		Index = a.Index;
		Weight = a.Weight;
		Method = a.Method;
		if(ReturnType != NULL)
		{
			ReturnType->release();
		}
		ReturnType = a.ReturnType;
		if(ReturnType != NULL) ReturnType->incRef();

		return *this;
	}

	CIdMethod::CIdMethod(const CIdMethod &a)
	{
		Index = a.Index;
		Weight = a.Weight;
		Method = a.Method;		
		ReturnType = a.ReturnType;
		if(ReturnType != NULL) 
			ReturnType->incRef();
	}
	
	CIdMethod::CIdMethod(sint32 i, double w,IObjectIA *m,IObjectIA *r)
	{
		Index = i;
		Weight = w;
		Method = m;
		ReturnType = r;
	}

	CIdMethod::CIdMethod()
	{
		Method = NULL;
		ReturnType = NULL;
	}

	CIdMethod::~CIdMethod()
	{
		if(ReturnType != NULL) 
			ReturnType->release();
	}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////	

	void IObjectIA::init(IObjectIA *)
	{
		
	}

	IBasicObjectIA::~IBasicObjectIA()
	{
		//delete _Name;
	}
	
	IBasicObjectIA::IBasicObjectIA()
	{
	}

	IBasicObjectIA::IBasicObjectIA(const IBasicObjectIA &a): NLAIC::IBasicInterface(a)
	{
	}
		
	bool IBasicObjectIA::operator == (const IBasicObjectIA &a) const
	{						
		if(a.getType() == getType() ) return isEqual(a);			
		return false;
	}		

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

	IObjectIA::CProcessResult IObjectIA::ProcessRun = IObjectIA::CProcessResult();
	IObjectIA::CProcessResult IObjectIA::ProcessNotComplit = IObjectIA::CProcessResult(processNotComplete);	
	TProcessStatement IObjectIA::ProcessIdle = processIdle;
	TProcessStatement IObjectIA::ProcessLocked = processLocked;
	TProcessStatement IObjectIA::ProcessBuzzy = processBuzzy;
	TProcessStatement IObjectIA::ProcessEnd = processEnd;
	TProcessStatement IObjectIA::ProcessError = processError;	

	IObjectIA::IObjectIA(const IObjectIA &a):IBasicObjectIA(a)
	{			
	}

	IObjectIA::IObjectIA()
	{			
	}		

	IObjectIA::~IObjectIA()
	{
	}

	const IObjectIA::CProcessResult &IObjectIA::runStep()
	{
		std::string text;
		text = NLAIC::stringGetBuild("const CProcessResult &IObjectIA::runStep() note implementaited for the '%s' interface",(const char *)getType());
		throw NLAIE::CExceptionNotImplemented(text.c_str());
		return IObjectIA::ProcessNotComplit;
	}

	IObjectIA &IObjectIA::operator = (const IObjectIA &a)
	{
		std::string text;
		text = NLAIC::stringGetBuild("operator <IObjectIA &operator = (const IObjectIA &a)> note implementaited for the '%s' interface",(const char *)getType());
		throw NLAIE::CExceptionNotImplemented(text.c_str());
		return *this;
	}


	sint32 IObjectIA::getStaticMemberSize() const
	{
		std::string text;
		text = NLAIC::stringGetBuild("sint32 IObjectIA::getStaticMemberSize() note implementaited for the '%s' interface",(const char *)getType());
		throw NLAIE::CExceptionNotImplemented(text.c_str());
		return 0;	
	}

	sint32 IObjectIA::getStaticMemberIndex(const IVarName &) const
	{
		return -1;
	}

	const IObjectIA *IObjectIA::getStaticMember(sint32) const
	{
		return NULL;
	}

	bool IObjectIA::setStaticMember(sint32,IObjectIA *)
	{
		std::string text;
		text = NLAIC::stringGetBuild("Function void IObjectIA::setStaticMember(sint32,IObjectIA *) note implementaited for the '%s' interface",(const char *)getType());
		throw NLAIE::CExceptionNotImplemented(text.c_str());
		return false;
	}

	TQueue IObjectIA::isMember(const IVarName *className,const IVarName *methodName,const IObjectIA &param) const
	{		
		static CStringVarName send(_SEND_);
		static CStringVarName constructor(_CONSTRUCTOR_);
		static CStringVarName run(_RUN_);
		static CStringVarName statM("GetStaticMember");

		if(*methodName == send)
		{
			TQueue r;			
			CObjectType *c = new CObjectType(new NLAIC::CIdentType(NLAIC::CIdentType::VoidType));
			r.push(CIdMethod(0,0.0,NULL,c));
			return r;
		}
		else
		if(*methodName == constructor && !((const NLAISCRIPT::CParam &)param).size())
		{
			TQueue r;
			CObjectType *c = new CObjectType(new NLAIC::CIdentType(NLAIC::CIdentType::VoidType));
			r.push(CIdMethod(1,0.0,NULL,c));
			return r;
		}
		else
		if(*methodName == run && !((const NLAISCRIPT::CParam &)param).size())
		{
			TQueue r;
			CObjectType *c = new CObjectType(new NLAIC::CIdentType(NLAIC::CIdentType::VoidType));
			r.push(CIdMethod(2,0.0,NULL,c));
			return r;			
		}
		if(*methodName == statM && ((const NLAISCRIPT::CParam &)param).size())
		{
			TQueue r;
			CObjectType *c = new CObjectType(new NLAIC::CIdentType(*IAgent::IdAgent));
			r.push(CIdMethod(3,0.0,NULL,c));
			return r;			
		}
		return TQueue();
	}

	sint32 IObjectIA::isClassInheritedFrom(const IVarName &) const
	{
		return -1;
	}	

	sint32 IObjectIA::getMethodIndexSize() const
	{
		return 4;
	}

	// Executes a method from its index i and with its parameters
	IObjectIA::CProcessResult IObjectIA::runMethodeMember(sint32 h, sint32 id,IObjectIA *a)
	{
		/*char text[2048*8];
		sprintf(text,"method  runMethodeMember(sint32 h, sint32 id,const IObjectIA &) not implemented for the '%s' interface",(const char *)getType());
		throw NLAIE::CExceptionNotImplemented(text);
		return CProcessResult();*/
		return IObjectIA::runMethodeMember(id,a);
	}

	IObjectIA::CProcessResult IObjectIA::runMethodeMember(sint32 id,IObjectIA *a)
	{
		
		switch(id)
		{
		case 0:
			{
				IMessageBase *msg;

				IPerformative *p;
				msg = (IMessageBase *)((IBaseGroupType *)a)->pop();
				//this->incRef();
				msg->setReceiver(this);
				p = (IPerformative *)((IBaseGroupType *)a)->pop();
				msg->setPerformatif((IMessageBase::TPerformatif)(sint)p->getNumber());
				p->release();
				if(((IBaseGroupType *)a)->size())
				{					
					CStringType *name = (CStringType *)((IBaseGroupType *)a)->pop();
					IObjectIA::CProcessResult r = sendMessage(name->getStr(),msg);
					name->release();
					return r;
					
				}				
				return sendMessage(msg);
			}			
			break;
		case 1:			
			break;
		case 2:
			return run();
			break;

		case 3:
			{
				CProcessResult r;
				NLAIAGENT::IBaseGroupType *param = (NLAIAGENT::IBaseGroupType *)a;
				r.Result =	(IObjectIA *)getStaticMember((sint)((NLAIAGENT::INombreDefine *)param->get())->getNumber());
				r.Result->incRef();
				return r;
			}
		}
		return CProcessResult();
	}

	std::string IObjectIA::getMethodeMemberDebugString(sint32 h, sint32 id) const
	{
		switch(id)
		{
		case 0:
			{
				return std::string("IObjectIA::sendMessage(IMessage)");
			}			
			break;
		case 1:			
			break;
		case 2:			
			return std::string("IObjectIA::run()");
			break;

		case 3:
			{				
				return std::string("IObjectIA::getStaticMember(Integer)");				
			}
		}
		return std::string("Mathod_?????(Param_?????)");
	}

	IObjectIA::CProcessResult IObjectIA::sendMessage(const IVarName &name,IObjectIA *)
	{
		std::string text;
		text = NLAIC::stringGetBuild("method 'sendMessage(%s,const IObjectIA &)' '%s' interface",name.getString(), (const char *)getType());
		throw NLAIE::CExceptionNotImplemented(text.c_str());
		return CProcessResult();
	}

	IObjectIA::CProcessResult IObjectIA::sendMessage(IObjectIA *)
	{
		std::string text;
		text = NLAIC::stringGetBuild("method 'sendMessage(const IObjectIA &)' '%s' interface", (const char *)getType());
		throw NLAIE::CExceptionNotImplemented(text.c_str());
		return CProcessResult();
	}

	TQueue IObjectIA::canProcessMessage(const IVarName &)
	{
		return TQueue();
	}


	bool IObjectIA::isLocal()
	{
		return true;
	}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

	IRefrence::IRefrence():_Parent(NULL)
	{
		_NumRef = new CLocWordNumRef(this);
	}

	IRefrence::IRefrence(const IWordNumRef *parent):_Parent((IWordNumRef *)parent)
	{
		//if(_Parent != NULL) _Parent->incRef();
		_NumRef = new CLocWordNumRef(this);
	}

	IRefrence::IRefrence(NLMISC::IStream &is)
	{
		refLoadStream(is);
	}

	IRefrence::operator const IWordNumRef &() const
	{
		return *_NumRef;
	}

	IRefrence::operator const IWordNumRef *() const 
	{
		return _NumRef;
	}

	IRefrence *IRefrence::getParent() const
	{
		return _Parent != NULL ? (IRefrence *)((const IRefrence *)*_Parent) : NULL;
	}

	void IRefrence::setParent(const IWordNumRef *parent)
	{
		//if(_Parent != NULL) _Parent->release();
		_Parent = (IWordNumRef *)parent;
		//if(_Parent != NULL) _Parent->incRef();		
	}

	const IRefrence *IRefrence::getOwner() const
	{
		const IRefrence *p = getParent();

		if(p == NULL) return this;

		while( true )
		{
			if(p->getParent() == NULL) return p;
		}
	}

	void IRefrence::save(NLMISC::IStream &os)
	{
		IObjectIA::save(os);
		os.serial( (NLAIC::CIdentType &) (_NumRef->getType()) );
		_NumRef->save(os);
		if(_Parent != NULL)
		{
			bool t = true;
			os.serial( t );
			os.serial( (NLAIC::CIdentType &) (_Parent->getType()) );
			_Parent->save(os);
		}
		else
		{
			bool t =  false;
			os.serial( t );	
		}
	}

	void IRefrence::refLoadStream(NLMISC::IStream &is)
	{
		IObjectIA::load(is);
		_NumRef->release();
		NLAIC::CIdentTypeAlloc id;
		is >> id;
		_NumRef = (IWordNumRef *)id.allocClass();
		_NumRef->load(is);
		bool k;
		is.serial( k );
		if ( k ) 
		{
			if(_Parent) _Parent->release();
			is >> id;
			_Parent = (IWordNumRef *)id.allocClass();
			_Parent->load(is);
		}
		else 
		{
			_Parent = NULL;
		}
	}

	void IRefrence::load(NLMISC::IStream &is)
	{
		refLoadStream(is);
	}

	void IRefrence::setNumRef(const IWordNumRef &ref)
	{
		if(_NumRef != NULL) _NumRef->release();
		_NumRef = new CLocWordNumRef(ref.getNumIdent(),this);
	}

	IRefrence::~IRefrence()
	{
		//if(_Parent != NULL) _Parent->release();
		_NumRef->release();
	}

	IRefrence::IRefrence(const IRefrence &A):IObjectIA(A),_Parent(A._Parent)//,_NumRef(A._NumRef)
	{			
		//if(_Parent != NULL) _Parent->incRef();
		_NumRef = new CLocWordNumRef(this);
		setTypeAt(A._NumRef->getNumIdent().getId().getType());
	}		

	const CIdent IRefrence::getIdentType() const
	{
		return CIdent(0,_NumRef->getNumIdent().getId());
	}

	void IRefrence::setTypeAt(uint64 t)
	{
		((CAgentNumber &)(_NumRef->getNumIdent().getId())).setTypeAt(t);
	}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

	IConnectIA::IConnectIA(const IConnectIA &A):IRefrence(A)
	{
	}

	IConnectIA::IConnectIA():IRefrence()
	{
	}

	IConnectIA::IConnectIA(const IWordNumRef *parent):IRefrence(parent)
	{
	}		

	IConnectIA::IConnectIA(NLMISC::IStream &is):IRefrence(is)
	{
		connectLoadStream(is);
	}

	IConnectIA *IConnectIA::getParent() const
	{
		return (IConnectIA *)IRefrence::getParent();
	}	

	void IConnectIA::load(NLMISC::IStream &is)
	{
		connectLoadStream(is);
	}

	void IConnectIA::connectLoadStream(NLMISC::IStream &is)
	{
		IRefrence::refLoadStream(is);
		NLAIC::CIdentTypeAlloc id;
		sint32 i;
		is.serial( i );

		if(i)
		{
			while(i --)
			{					
				is >> id;
				IWordNumRef *num = (IWordNumRef *)id.allocClass();
				num->load(is);
				addInConnectionList((IConnectIA *)((const IRefrence *)*num));
				delete num; 
			}
		}

		
		is.serial(i);
		if(i)
		{
			while(i--)
			{
				is >> id;					
				IWordNumRef *num = (IWordNumRef *)id.allocClass();
				num->load(is);
				addInConnectedList((IConnectIA *)((const IRefrence *)*num));
				delete num;
			}
		}
	}

	void IConnectIA::connect(IConnectIA *b)
	{
		b->addInConnectedList(this);
		addInConnectionList(b);
	}

	void IConnectIA::removeConnection(IConnectIA *Agent)
	{		
		Agent->removeInConnectedList(this);
		removeInConnectionList(Agent);			
	}

	IConnectIA::~IConnectIA()
	{
		Kill();
	}

	void IConnectIA::addInConnectionList(const IConnectIA *a)
	{
		_Connection.insert(a);
	}

	void IConnectIA::removeInConnectionList(IConnectIA *a)
	{

		TListIBasicItr i = _Connection.find(a);
		if(i != _Connection.end())
							_Connection.erase(i);
		/*TListIBasicItr i = _Connection.begin();
		while(i != _Connection.end())
		{				
			if(*i == a)
			{
				_Connection.erase(i);
				return;
			}
			i++;
		}*/
	}

	void IConnectIA::addInConnectedList(const IConnectIA *a)
	{
		_Connected.insert(a);
	}

	void IConnectIA::removeInConnectedList(const IConnectIA *a)
	{
		TListIBasicItr i = _Connected.find(a);
		if(i != _Connected.end())
						_Connected.erase(i);
		/*TListIBasicItr i = _Connected.begin();
		while(i != _Connected.end())
		{			
			const IConnectIA *o = *i;
			if(o == a)
			{					
				_Connected.erase(i);
				return;
			}
			i++;
		}*/
	}

	void IConnectIA::Kill()
	{
		IConnectIA *parent = getParent();
		if ( parent != NULL )
		{
			parent->onKill( this );
		}

		TListIBasicCstItr it = _Connection.begin();

		while(it != _Connection.end())
		{				
			IConnectIA *a = (IConnectIA *)*it ++;			
			if(a != NULL)
			{
				a->removeInConnectedList(this);
			}
		}
		_Connection.clear();

		it = _Connected.begin();
		while(it != _Connected.end())
		{				
			IConnectIA *a = (IConnectIA *)*it ++;			
			if(a != NULL)
			{
				a->removeInConnectionList(this);
				a->onKill(this);				
			}
		}
		_Connected.clear();
	}

	void IConnectIA::onKill(IConnectIA *a)
	{
		IConnectIA *parent = getParent();
		if ( parent == a )
		{
			//parent->release();
			setParent(NULL);
		}
	}

	void IConnectIA::setParent(const IWordNumRef *parent)
	{		
		IRefrence::setParent(parent);
		IConnectIA *p = (IConnectIA *)getParent();

		if(p != NULL) connect(p);
	}
}
