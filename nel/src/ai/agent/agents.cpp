/** \file agents.cpp
 *
 * $Id: agents.cpp,v 1.9 2001/01/23 16:39:20 chafik Exp $
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
#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/agent/agent_mailer.h"
#include "nel/ai/agent/agent_digital.h"
namespace NLAIAGENT
{

	IAgent::IAgent(const IAgent &a) : IAgentComposite(a)
	{			
	}

	IAgent::IAgent(IBasicAgent *parent): IAgentComposite(parent)
	{			
	}

	IAgent::IAgent(IBasicAgent *parent,IMailBox *m): IAgentComposite(parent,m)
	{			
	}

/*	const IObjectIA::CProcessResult &IAgent::run();
	IObjectIA *IAgent::run(const IMessageBase &);*/
		
	void IAgent::onKill(IConnectIA *a)
	{
		eraseFromList<IBasicAgent *>(&_AgentList,(IBasicAgent *)a);
		removeConnection(*a);
	}

	const NLAIC::IBasicType *IAgent::clone() const
	{		
		NLAIC::IBasicInterface *m = new IAgent(*this);
		return m;
	}		

	const NLAIC::IBasicType *IAgent::newInstance() const
	{	
		NLAIC::IBasicInterface *m;
		if(getParent() != NULL) m = new	IAgent((IBasicAgent *)getParent());
		else m = new IAgent(NULL);
		return m;
	}
	
	void IAgent::getDebugString(char *t) const
	{
		sprintf(t,"class IAgent<%4x>",this);
	}

	bool IAgent::isEqual(const IBasicObjectIA &a) const
	{
		return true;
	}

        /*const NLAIC::CIdentType &IAgent::getType() const;		
	void IAgent::save(NLMISC::IStream &os);
	void IAgent::load(NLMISC::IStream &is);*/

	IAgent::~IAgent()
	{	
		Kill();	
	}			
	
	void IAgent::save(NLMISC::IStream &os)
	{				
		IBasicAgent::save(os);
		IAgentComposite::save(os);
	}

	void IAgent::load(NLMISC::IStream &is)
	{	
		IBasicAgent::load(is);
		IAgentComposite::load(is);		
	}		

	void IAgent::runChildren()	// Se charge de l'activation des fils
	{
		std::list<IBasicAgent *>::iterator i_agl = _AgentList.begin();
		while ( i_agl != _AgentList.end() )
		{									
			(*i_agl)->run();
			i_agl++;
		}
	}		
	
	void IAgent::processMessages()
	{
		while(getMail()->getMessageCount())
		{
			const IMessageBase &msg = getMail()->getMessage();				
			IObjectIA *o = IBasicAgent::run( msg );
			getMail()->popMessage();
		}
	}

	const IObjectIA::CProcessResult &IAgent::run()
	{
		setState(processBuzzy,NULL); 

		runChildren();		// Gestion des fils
		getMail()->run();	// Execution de la boite aux lettres

		processMessages();	// Traitement de ses propres messages

		setState(processIdle,NULL);
		return getState();  
	}

	/*IObjectIA *IAgent::run(const IMessageBase &msg)
	{
		return NULL;
	}*/


	/*void IAgent::run(const IMessageBase *msg)
	{
		char buffer[512];
		char buffer2[512];
		msg->getDebugString(buffer);
		getDebugString(buffer2);
		TRACE("AGENT\n %s\nTRAITE MESSAGE\n%s\n", buffer2, buffer);

	}*/

	const NLAIC::CIdentType &IAgent::getType() const
	{		
		return IdAgent;
	}


////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

	IBasicAgent::IBasicAgent(const IBasicAgent &a): IConnectIA(a),_Mail((IMailBox	*)a._Mail->clone())
	{
	}


	IBasicAgent::IBasicAgent(const IWordNumRef *parent): IConnectIA(parent)
	{
		_Mail = new tMailBoxLettre((const IWordNumRef *) *this);
	}		

	IBasicAgent::IBasicAgent(const IWordNumRef *parent,IMailBox	*m): IConnectIA(parent),_Mail(m)
	{			
		_Mail->setParent((const IWordNumRef *) *this);
	}		

	IBasicAgent::~IBasicAgent()
	{
		_Mail->setParent(NULL);
		_Mail->release();			
	}		

	const IObjectIA::CProcessResult &IBasicAgent::getState() const 
	{
		return _RunState;
	}

	void IBasicAgent::setState(TProcessStatement state, IObjectIA *result)
	{
		_RunState.ResultState = state;
		_RunState.Result = result;
	}

	IObjectIA::CProcessResult IBasicAgent::sendMessage(IMessageBase *msg)
	{
		_Mail->addMessage(msg);
		return IObjectIA::ProcessRun;
	}

	IObjectIA::CProcessResult IBasicAgent::sendMessage(IMessageBase *msg, IBasicAgent &receiver)
	{
		receiver.getMail()->addMessage( msg );
		return IObjectIA::ProcessRun;
	}

	IMailBox *IBasicAgent::getMail() const
	{
		return _Mail;
	}

	void IBasicAgent::save(NLMISC::IStream &os)
	{				
		IConnectIA::save(os);
		_Mail->save(os);			
	}

	void IBasicAgent::load(NLMISC::IStream &is)
	{			
		IConnectIA::load(is);
		_Mail->load(is);						
		NLAIC::CIdentTypeAlloc id;			
	}

	void IBasicAgent::addMsgGroup(IBasicMessageGroup &grp)
	{
		getMail()->addGroup( grp );
	}

	void IBasicAgent::removeMsgGroup(IBasicMessageGroup &grp)
	{
		getMail()->removeGroup( grp );
	}

	IObjectIA *IBasicAgent::run(const IMessageBase &msg)
	{
		IMessageBase *returnMsg = NULL;
		switch(msg.getPerformatif())
		{
		case IMessageBase::PUndefine:
			{
				char text[2048*8];
				sprintf(text,"Function IObjectIA *IBasicAgent::run(const IMessageBase &msg) proccess an IMessageBase::PUndefine performatif");
				throw NLAIE::CExceptionNotImplemented(text);
			}
			break;		
		case IMessageBase::PExec:
			returnMsg = runExec(msg);
			if(msg.getContinuation() != NULL) 
			{
				returnMsg->incRef();
				((IObjectIA *)msg.getContinuation())->sendMessage(returnMsg);
			}				
			break;
		case IMessageBase::PAchieve:
			returnMsg = runAchieve(msg);
			if(msg.getContinuation() != NULL)
			{
				returnMsg->incRef();
				((IObjectIA *)msg.getContinuation())->sendMessage(returnMsg);
			}
			break;
		case IMessageBase::PAsk:
			returnMsg = runAsk(msg);
			returnMsg->setPerformatif(IMessageBase::PTell);
			returnMsg->incRef();			
			((IObjectIA *)msg.getSender())->sendMessage(returnMsg);
			if(msg.getContinuation() != NULL)
			{					
				returnMsg->incRef();
				((IObjectIA *)msg.getContinuation())->sendMessage(returnMsg);				
			}			
			break;
		case IMessageBase::PTell:
			returnMsg = runTell(msg);			
			break;
		case IMessageBase::PBreak:
			returnMsg = runBreak(msg);
			break;
		case IMessageBase::PKill:
			returnMsg = runKill(msg);
			break;
		}
		if(returnMsg) returnMsg->release();
		return NULL;
	}
	

	const static sint32 _GetMailer = 0;
	const static sint32 _Father = 1;
	const static sint32 _LastM = 2;

	IBasicAgent::CMethodCall IBasicAgent::_Method[] = 
	{
		IBasicAgent::CMethodCall("GetMailer",_GetMailer),
		IBasicAgent::CMethodCall("Father",_Father)
	};

	sint32 IBasicAgent::getMethodIndexSize() const
	{
		return IObjectIA::getMethodIndexSize() + _LastM;
	}	

	tQueue IBasicAgent::isMember(const IVarName *className,const IVarName *methodName,const IObjectIA &p) const
	{			
		if(className == NULL)
		{
			tQueue a;
			for(int i = 0; i < _LastM; i++)
			{
				if(*methodName == IBasicAgent::_Method[i].MethodName)
				{					
					CObjectType *c = new CObjectType(new NLAIC::CIdentType(CLocalAgentMail::LocalAgentMail));
					a.push(CIdMethod(IBasicAgent::_Method[i].Index + IObjectIA::getMethodIndexSize(),0.0,NULL,c));					
					break;
				}
			}

			if(a.size()) return a;
			else return IConnectIA::isMember(className,methodName,p);
		}
		return IConnectIA::isMember(className,methodName,p);
	}

	IObjectIA::CProcessResult IBasicAgent::runMethodeMember(sint32 h, sint32 index,IObjectIA *p)
	{
		return IConnectIA::runMethodeMember(h,index,p);
	}

	IObjectIA::CProcessResult IBasicAgent::runMethodeMember(sint32 index,IObjectIA *p)
	{
		IBaseGroupType *param = (IBaseGroupType *)p;

		switch(index - IObjectIA::getMethodIndexSize())
		{
		case _GetMailer:
			{
				IObjectIA::CProcessResult a;				
				a.Result = new CLocalAgentMail(this);				
				return a;
			}			

		case _Father:
			{
				IObjectIA::CProcessResult a;				
				IRefrence *father = getParent();
				if ( father )
					a.Result = new CLocalAgentMail( (IBasicAgent *) father );				
				else
				{
					a.Result = &DigitalType::NullOperator;
					a.Result->incRef();
				}
				return a;
			}			

		}
		return IConnectIA::runMethodeMember(index,p);
	}
}
