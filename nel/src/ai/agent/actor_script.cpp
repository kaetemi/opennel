#include "nel/ai/agent/actor_script.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/codage.h"
#include "nel/ai/script/interpret_object_agent.h"
#include "nel/ai/script/interpret_fsm.h"
#include "nel/ai/logic/fsm_seq_script.h"
#include "nel/ai/agent/msg_action.h"
#include "nel/ai/script/interpret_message_action.h"

namespace NLAIAGENT
{
	static CGroupType listBidon;

	/////////////////////////////////////////////////////////////
	// Succes and failure messages declaration

	NLAISCRIPT::COperandSimpleListOr *CActorScript::ParamIdSuccessMsg = NULL;
	NLAISCRIPT::CParam *CActorScript::ParamSuccessMsg = NULL;
	NLAISCRIPT::COperandSimpleListOr *CActorScript::ParamIdFailureMsg = NULL;
	NLAISCRIPT::CParam *CActorScript::ParamFailureMsg = NULL;

	void CActorScript::initClass()
	{
		CActorScript::ParamIdSuccessMsg = new NLAISCRIPT::COperandSimpleListOr(2,
															  new NLAIC::CIdentType(NLAIAGENT::CSuccessMsg::IdSuccessMsg),
															  new NLAIC::CIdentType(NLAISCRIPT::CSuccessMsgClass::IdSuccessMsgClass)	);		
		CActorScript::ParamSuccessMsg = new NLAISCRIPT::CParam(1,ParamIdSuccessMsg);		
		CActorScript::ParamIdFailureMsg = new NLAISCRIPT::COperandSimpleListOr(2,
																  new NLAIC::CIdentType(NLAIAGENT::CFailureMsg::IdFailureMsg),
																  new NLAIC::CIdentType(NLAISCRIPT::CFailureMsgClass::IdFailureMsgClass)	);

		CActorScript::ParamFailureMsg = new NLAISCRIPT::CParam(1,ParamIdFailureMsg);
	}

	void CActorScript::releaseClass()
	{		
		CActorScript::ParamSuccessMsg->release();		
		CActorScript::ParamFailureMsg->release();
	}
	/////////////////////////////////////////////////////////////


	CActorScript::CActorScript(const CActorScript &a) : CAgentScript(a)
	{
		_IsActivated = a._IsActivated;
		_OnActivateIndex = a._OnActivateIndex;
		_OnUnActivateIndex = a._OnUnActivateIndex;
		_TopLevel = a._TopLevel;
		_IsPaused = false;
	}

	CActorScript::CActorScript(IAgentManager *manager, 
							   IBasicAgent *father,
							   std::list<IObjectIA *> &components,	
							   NLAISCRIPT::CAgentClass *actor_class )
	: CAgentScript(manager, father, components, actor_class )
	{	
		_IsActivated = false;
		_OnActivateIndex = -1;
		_OnUnActivateIndex = -1;
		_TopLevel = NULL;
		_IsPaused = false;
	}	

	CActorScript::CActorScript(IAgentManager *manager, bool stay_alive) : CAgentScript( manager )
	{
		_IsActivated = false;
		_OnActivateIndex = -1;
		_OnUnActivateIndex = -1;
		_TopLevel = NULL;
		_IsPaused = false;
	}

	CActorScript::~CActorScript()
	{
		while (  _Launched.size() )
		{
			_Launched.front()->Kill();
			_Launched.front()->release();
			_Launched.pop_front();
		}
	}

	/// Returns true if the actor has a token
	bool CActorScript::isActivated()
	{
		return _IsActivated;
	}

	/// Activates the actor
	void CActorScript::activate()
	{                              
#ifndef NL_DEBUG
	//	const char *
#endif

		if ( !_IsActivated )
		{
			CAgentScript *father = (CAgentScript *) getParent();

			if (  father && ( ( CAgentScript *)father)->getClass()->isClassInheritedFrom( CStringVarName("Fsm") ) != -1 )
			{
				( (CFsmScript *)father)->activate( this );
			}

			// Looks for the function to call at the activation of the state
			static CStringVarName activate_func_name("OnActivate");
			tQueue r = _AgentClass->isMember( NULL, &activate_func_name, NLAISCRIPT::CParam() );
			if ( !r.empty() )
				_OnActivateIndex = r.top().Index;
			else
				onActivate();
			_IsActivated = true;
		}
	}

	/// Unactivates the actor
	void CActorScript::unActivate()
	{
		if ( _IsActivated )
		{
			CAgentScript *father = (CAgentScript *) getParent();
			if (  father && ( ( CAgentScript *)father)->getClass()->isClassInheritedFrom( CStringVarName("Fsm") ) != -1 )
			{
				( (CFsmScript *)father)->unactivate( this );
			}

			static CStringVarName unactivate_func_name("OnUnActivate");
			tQueue r = _AgentClass->isMember( NULL, &unactivate_func_name, NLAISCRIPT::CParam() );
			if ( !r.empty() )
			{	
				_OnUnActivateIndex = r.top().Index;
				const NLAIAGENT::IAgentManager *manager = getAgentManager();
				if ( manager != NULL )
				{
					NLAISCRIPT::CCodeContext *context = (NLAISCRIPT::CCodeContext *) manager->getAgentContext();
					context->Self = this;
					runMethodeMember( _OnUnActivateIndex ,context);
					_OnActivateIndex = -1;
				}
			}
			// Destroys launched childs?
			while ( _Launched.size() )
			{
				( (CActorScript *) _Launched.front() )->cancel();
				_Launched.front()->Kill();
				delete _Launched.front();
				_Launched.pop_front();
			}
			onUnActivate();
			_IsActivated = false;
			_IsPaused = false;
		}
	}

	/// Pauses the actor
	void CActorScript::pause()
	{                              
#ifndef NL_DEBUG
	//	const char *
#endif

		if ( !_IsPaused )
		{
			CAgentScript *father = (CAgentScript *) getParent();

			// Looks for the function to call when the actor is paused
			static CStringVarName activate_func_name("OnPause");
			tQueue r = _AgentClass->isMember( NULL, &activate_func_name, NLAISCRIPT::CParam() );
			if ( !r.empty() )
			{
				if ( getAgentManager() != NULL )
				{
					_OnPauseIndex = r.top().Index;
					NLAISCRIPT::CCodeContext *context = (NLAISCRIPT::CCodeContext *) getAgentManager()->getAgentContext();
					context->Self = this;
					runMethodeMember( _OnPauseIndex ,context);
					_OnPauseIndex = -1;
				}
			}
			else
				onPause();
			_IsPaused = true;
		}
	}

	void CActorScript::onPause()
	{
	}

	/// Restarts the actor
	void CActorScript::restart()
	{                              
#ifndef NL_DEBUG
	//	const char *
#endif

		if ( _IsPaused )
		{
			CAgentScript *father = (CAgentScript *) getParent();

			// Looks for the function to call when the actor is restarted
			static CStringVarName activate_func_name("OnRestart");
			tQueue r = _AgentClass->isMember( NULL, &activate_func_name, NLAISCRIPT::CParam() );
			if ( !r.empty() )
			{
				if ( getAgentManager() != NULL )
				{
					_OnRestartIndex = r.top().Index;
					NLAISCRIPT::CCodeContext *context = (NLAISCRIPT::CCodeContext *) getAgentManager()->getAgentContext();
					context->Self = this;
					runMethodeMember( _OnRestartIndex ,context);
					_OnRestartIndex = -1;
				}
			}
			else
				onRestart();
			_IsPaused = false;
		}
	}

	void CActorScript::onRestart()
	{
	}

	/** Transfers activity to another actor.
		The second arg bool must be set to true for this agent to stay active, false otherwise.
	**/
	void CActorScript::switchActor(CActorScript *receiver, bool stay_active)
	{
		receiver->activate();

		if ( !stay_active && ( receiver != this ) )
			unActivate();
	}
		
	/** Transfers activity to another actor.
		The second arg bool must be set to true for this agent to stay active, false otherwise.
	**/
	void CActorScript::switchActor(std::vector<CActorScript *> &actors, bool stay_active)
	{
		std::vector<CActorScript *>::iterator it_act = actors.begin();
		while ( it_act != actors.end() )
		{
			( *it_act )->activate();
			it_act++;
		}

		// TODO: Envoi de message "activate" 
		if ( !stay_active )
			unActivate();
	}

	/** Transfers activity to another actor.
		The second arg bool must be set to true for this agent to stay active, false otherwise.
	**/
	void CActorScript::switchActor(std::vector<CComponentHandle *> &handles, bool stay_active)
	{
		std::vector<CComponentHandle *>::iterator it_handle = handles.begin();
		while ( it_handle != handles.end() )
		{
			CActorScript *actor = (CActorScript *)( *it_handle )->getValue();
			if ( actor != NULL )
				actor->activate();
			else
			{
				const char *sw_name = (*it_handle)->getCompName()->getString();
				nlwarning("SWITCH: component %s not found.", sw_name);
			}
			it_handle++;
		}
		// TODO: Envoi de message "activate" 
		if ( !stay_active )
			unActivate();
	}


	/// Callback called at the activation of the actor
	void CActorScript::onActivate()
	{
		// Default behaviour: do nothing
	}

	/// Callback called when the agent is unactivated
	void CActorScript::onUnActivate()
	{
		// default behaviour: do nothing
	}

	const NLAIC::IBasicType *CActorScript::clone() const
	{		
		CActorScript *m = new CActorScript(*this);
		return m;
	}		

	const NLAIC::IBasicType *CActorScript::newInstance() const
	{	
		CActorScript *instance;
		if ( _AgentClass )
		{
			instance = (CActorScript *) _AgentClass->buildNewInstance();
		}
		else 
		{			
			instance = new CActorScript(NULL);
		}
		return instance;
	}
	
	void CActorScript::getDebugString(std::string &t) const
	{
		t = "CActorScript ";
		if ( _IsActivated )
			t += "<active>";
		else
			t += "<idle>";
	}

	bool CActorScript::isEqual(const IBasicObjectIA &a) const
	{
		return true;
	}

	IObjectIA::CProcessResult CActorScript::sendMessage(IObjectIA *m)
	{		
		return CAgentScript::sendMessage(m);
	}

	void CActorScript::processMessages()
	{
		//if ( _IsActivated )
		CAgentScript::processMessages();
	}

	const IObjectIA::CProcessResult &CActorScript::run()
	{
		if ( _IsActivated )
		{
			if ( _OnActivateIndex != -1 )
			{
				if ( getAgentManager() != NULL )
				{
					NLAISCRIPT::CCodeContext *context = (NLAISCRIPT::CCodeContext *) getAgentManager()->getAgentContext();
					context->Self = this;
					CProcessResult r = runMethodeMember( _OnActivateIndex ,context);
					if ( r.Result != NULL )
						r.Result->release();
					_OnActivateIndex = -1;
				}
			}
			return CAgentScript::run();
		}
		else
		{
			processMessages();
			return IObjectIA::ProcessRun;
		}
	}

	const NLAIC::CIdentType &CActorScript::getType() const
	{		
		return IdActorScript;
	}

	void CActorScript::save(NLMISC::IStream &os)
	{
		CAgentScript::save(os);
		sint32 b = (_IsActivated == false);
		os.serial( b );
	}

	void CActorScript::load(NLMISC::IStream &is)
	{
		CAgentScript::load(is);
		sint32 b;
		is.serial( b );
		_IsActivated =  !b ;
	}

	sint32 CActorScript::getMethodIndexSize() const
	{
		return CAgentScript::getMethodIndexSize() + fid_last;
	}

//	virtual IObjectIA::CProcessResult runMethodBase(int heritance, int index,IObjectIA *);


	IObjectIA::CProcessResult CActorScript::runMethodBase(int index,int heritance, IObjectIA *params)
	{		
		IObjectIA::CProcessResult r;

		switch ( index )
		{
			case fid_activate:
				activate();
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				break;

			case fid_onActivate:
				onActivate();
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				break;

			case fid_unActivate:
				unActivate();
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				break;

			case fid_onUnActivate:
				onUnActivate();
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				break;

			case fid_switch:
				{
					std::vector<CStringType *> handles;
					if ( ( (NLAIAGENT::IBaseGroupType *) params)->size() )
					{
						IBaseGroupType *fw = (IBaseGroupType *) ( ((NLAIAGENT::IBaseGroupType *)params) )->getFront();
						//( ((NLAIAGENT::IBaseGroupType *)params))->popFront();
						while ( fw->size() )
						{
							handles.push_back( (CStringType *) fw->getFront() );
							fw->popFront();
						}
						std::vector<CComponentHandle *> switched;
						for ( int i = 0; i < (int) handles.size(); i++)
							switched.push_back( new CComponentHandle(  handles[ i ]->getStr() , (IAgent *) getParent() ) );

						switchActor( switched, false );
					}
					r.ResultState =  NLAIAGENT::processIdle;
					r.Result = NULL;
				}
				break;
				
			case fid_launch:
			
				if ( ( (NLAIAGENT::IBaseGroupType *) params)->size() )
				{
#ifdef NL_DEBUG
					const char *dbg_param_type = (const char *) params->getType();
					std::string dbg_param_string;
					params->getDebugString(dbg_param_string);
#endif
					const IObjectIA *child = ( ((NLAIAGENT::IBaseGroupType *)params) )->get();
#ifdef NL_DEBUG
					const char *dbg_param_front_type = (const char *) child->getType();
#endif
					addDynamicAgent( (NLAIAGENT::IBaseGroupType *) params);
					if ( child->isClassInheritedFrom( CStringVarName("Actor") ) != -1 )
					{
						if ( _TopLevel )
							((CActorScript *)child)->setTopLevel( _TopLevel );
						else
							((CActorScript *)child)->setTopLevel( this );
						
						((CActorScript *)child)->activate();
					}
/*

					if ( child->isClassInheritedFrom( CStringVarName("Actor") ) != -1 )
						((CActorScript *)child)->activate();
*/
					_Launched.push_back( (NLAIAGENT::IAgent *) child );

				}
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				return r;
				break;
			
			case fid_launched:
				{
					CVectorGroupType *result = new CVectorGroupType();
					std::list<IBasicAgent *>::iterator it_l = _Launched.begin();
					while ( it_l != _Launched.end() )
					{
						result->push( new CLocalMailBox( (const NLAIAGENT::IWordNumRef *) **it_l ) );
						it_l++;
					}
					r.ResultState = NLAIAGENT::processIdle;
					r.Result = result;
					return r;
				}
				break;

			case fid_pause:
				pause();
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				return r;
				break;

			case fid_restart:
				restart();
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				return r;
				break;

			case fid_success:
			case fid_msg_success:
				onSuccess(params);
				r.Result = new NLAIAGENT::CSuccessMsg();
				return r;
				break;

			case fid_failure:
			case fid_msg_failure:
				onFailure(params);
				r.Result = new NLAIAGENT::CFailureMsg();
				return r;
				break;

			case fid_toplevel:
				r.Result = new CLocalAgentMail( (IBasicAgent *) getTopLevel() );				
				return r;
				break;

			case fid_owner:
				r.Result = new CLocalAgentMail( (IBasicAgent *) getTopLevel()->getParent() );				
				return r;
				break;

			case fid_isactive:
				r.Result = new NLAILOGIC::CBoolType( _IsActivated );
				return r;
				break;
		}
		return r;
	}



	IObjectIA::CProcessResult CActorScript::runMethodBase(int index,IObjectIA *params)
	{	
		int i = index - CAgentScript::getMethodIndexSize();


		IObjectIA::CProcessResult r;
		std::vector<CStringType *> handles;

#ifndef NL_DEBUG
		/*
		char buf[1024];
		getDebugString(buf);
		*/
#endif

		switch( i )
		{
		
			case fid_activate:
				activate();
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				break;

			case fid_onActivate:
				onActivate();
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				break;

			case fid_unActivate:
				unActivate();
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				break;
			
			case fid_onUnActivate:
				onUnActivate();
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				return r;
				break;

			case fid_switch:
				if ( ( (NLAIAGENT::IBaseGroupType *) params)->size() )
				{
#ifdef NL_DEBUG
					const char *dbg_param_type = (const char *) params->getType();
					std::string dbg_param_string;
					params->getDebugString(dbg_param_string);
#endif
					const IObjectIA *fw = ( ((NLAIAGENT::IBaseGroupType *)params) )->get();
#ifdef NL_DEBUG
					const char *dbg_param_front_type = (const char *) fw->getType();
#endif

					//( ((NLAIAGENT::IBaseGroupType *)params))->popFront();
//					while ( fw->size() )
//					{
						handles.push_back( (CStringType *) fw);
//						fw->popFront();
//					}

					std::vector<CComponentHandle *> switched;
					int i;
					for ( i = 0; i < (int) handles.size(); i++)
						switched.push_back( new CComponentHandle( handles[ i ]->getStr(), (IAgent *) getParent() ) );
					switchActor( switched, false );
					for ( i = 0; i < (int) switched.size(); i++)
						delete switched[i];
				}
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				break;

			case fid_launch:
			
				if ( ( (NLAIAGENT::IBaseGroupType *) params)->size() )
				{
#ifdef NL_DEBUG
					const char *dbg_param_type = (const char *) params->getType();
					std::string dbg_param_string;
					params->getDebugString(dbg_param_string);
#endif
					const IObjectIA *child = ( ((NLAIAGENT::IBaseGroupType *)params) )->get();
#ifdef NL_DEBUG
					const char *dbg_param_front_type = (const char *) child->getType();
#endif
					addDynamicAgent( (NLAIAGENT::IBaseGroupType *) params);
					if ( child->isClassInheritedFrom( CStringVarName("Actor") ) != -1 )
					{
						if ( _TopLevel )
							((CActorScript *)child)->setTopLevel( _TopLevel );
						else
							((CActorScript *)child)->setTopLevel( this );
						
						((CActorScript *)child)->activate();
					}
/*

					if ( child->isClassInheritedFrom( CStringVarName("Actor") ) != -1 )
						((CActorScript *)child)->activate();
*/
					_Launched.push_back( (NLAIAGENT::IAgent *) child );

				}
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				return r;
				break;

			case fid_pause:
				pause();
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				return r;
				break;

			case fid_restart:
				restart();
				r.ResultState =  NLAIAGENT::processIdle;
				r.Result = NULL;
				return r;
				break;

			case fid_success:
			case fid_msg_success:
				onSuccess(params);
				r.Result = new NLAIAGENT::CSuccessMsg();
				return r;
				break;

			case fid_failure:
			case fid_msg_failure:
				onFailure(params);
				r.Result = new NLAIAGENT::CFailureMsg();
				return r;
				break;

			case fid_toplevel:
				r.Result = new CLocalAgentMail( (IBasicAgent *) getTopLevel() );				
				return r;
				break;

			case fid_owner:
				r.Result = new CLocalAgentMail( (IBasicAgent *) getTopLevel()->getParent() );				
				return r;
				break;

			case fid_isactive:
				r.Result = new NLAILOGIC::CBoolType( _IsActivated );
				return r;
				break;
		}
		return CAgentScript::runMethodBase(index, params);
	}

	int CActorScript::getBaseMethodCount() const
	{
		return CAgentScript::getBaseMethodCount() + fid_last;
	}

	tQueue CActorScript::getPrivateMember(const IVarName *className,const IVarName *name,const IObjectIA &param) const
	{		

#ifdef NL_DEBUG
		const char *dbg_func_name = name->getString();
		std::string buffer;
		param.getDebugString( buffer );
#endif

		tQueue result; 

		static NLAIAGENT::CStringVarName activate_name("activate");
		static NLAIAGENT::CStringVarName onactivate_name("onActivate");
		static NLAIAGENT::CStringVarName unactivate_name("unActivate");
		static NLAIAGENT::CStringVarName onunactivate_name("onUnActivate");
		static NLAIAGENT::CStringVarName switch_name("switch");
		static NLAIAGENT::CStringVarName launch_name("Launch");
		static NLAIAGENT::CStringVarName launched_name("Launched");
		static NLAIAGENT::CStringVarName tell_name("RunTell");
		static NLAIAGENT::CStringVarName toplevel_name("TopLevel");
		static NLAIAGENT::CStringVarName owner_name("Owner");
		static NLAIAGENT::CStringVarName success_name("Success");
		static NLAIAGENT::CStringVarName failure_name("Failure");
		static NLAIAGENT::CStringVarName pause_name("Pause");
		static NLAIAGENT::CStringVarName restart_name("Restart");
		static NLAIAGENT::CStringVarName isactive_name("IsActivated");

		if ( *name == activate_name )
		{
			NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod(  CAgentScript::getMethodIndexSize() + fid_activate, 0.0,NULL, r_type ) );
		}

		if ( *name == onactivate_name )
		{
			NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( CAgentScript::getMethodIndexSize() + fid_onActivate , 0.0,NULL, r_type ) );
		}

		if ( *name == unactivate_name )
		{
			CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( CAgentScript::getMethodIndexSize() + fid_onUnActivate, 0.0,NULL, r_type ) );
		}

		if ( *name == onunactivate_name )
		{
			CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( CAgentScript::getMethodIndexSize() + fid_unActivate, 0.0,NULL, r_type ) );
		}

		if ( *name == switch_name )
		{
			CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( CAgentScript::getMethodIndexSize() + fid_switch, 0.0, NULL, r_type ) );
		}

		if ( *name == launch_name )
		{
			CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( CAgentScript::getMethodIndexSize() + fid_launch, 0.0, NULL, r_type ) );
		}

		if ( *name == launched_name )
		{
			CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIAGENT::CVectorGroupType::IdVectorGroupType ) );
			result.push( NLAIAGENT::CIdMethod( CAgentScript::getMethodIndexSize() + fid_launched, 0.0, NULL, r_type ) );
		}

		// Processes succes and failure functions
		if ( *name == tell_name )
		{
			double d;
			d = ((NLAISCRIPT::CParam &)*ParamSuccessMsg).eval((NLAISCRIPT::CParam &)param);
			if ( d >= 0.0 )
			{
				NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
				result.push( NLAIAGENT::CIdMethod(  CAgentScript::getMethodIndexSize() + fid_msg_success, 0.0,NULL, r_type ) );
			}

			d = ((NLAISCRIPT::CParam &)*ParamFailureMsg).eval((NLAISCRIPT::CParam &)param);
			if ( d >= 0.0 )
			{
				NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
				result.push( NLAIAGENT::CIdMethod(  CAgentScript::getMethodIndexSize() + fid_msg_failure, 0.0,NULL, r_type ) );
			}
		}

		if ( *name == toplevel_name )
		{
			CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( CAgentScript::getMethodIndexSize() + fid_toplevel, 0.0, NULL, r_type ) );
		}

		if ( *name == owner_name )
		{
			CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( CAgentScript::getMethodIndexSize() + fid_owner, 0.0, NULL, r_type ) );
		}

		if ( *name == success_name )
		{
			CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( CAgentScript::getMethodIndexSize() + fid_success, 0.0, NULL, r_type ) );
		}

		if ( *name == failure_name )
		{
			CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( CAgentScript::getMethodIndexSize() + fid_failure, 0.0, NULL, r_type ) );
		}

		if ( *name == pause_name )
		{
			CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( CAgentScript::getMethodIndexSize() + fid_pause, 0.0, NULL, r_type ) );
		}

		if ( *name == restart_name )
		{
			CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( CAgentScript::getMethodIndexSize() + fid_restart, 0.0, NULL, r_type ) );
		}

		if ( *name == isactive_name )
		{
			CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( CAgentScript::getMethodIndexSize() + fid_isactive, 0.0, NULL, r_type ) );
		}

		if ( result.empty() )
			return CAgentScript::getPrivateMember(className, name, param);

		return result;
	}


	void CActorScript::cancel()
	{
#ifdef NL_DEBUG
		const char *dbg_this_type = (const char *) getType();
#endif
		unActivate();
	}

	float CActorScript::priority() const
	{
		// Look at predecessors priorities
		return 1.0;
	}

	void CActorScript::setTopLevel(CAgentScript *tl)
	{
		_TopLevel = tl;

		for (int i = 0; i < _NbComponents; i++ )
		{
			if ( _Components[i]->isClassInheritedFrom( NLAIAGENT::CStringVarName("Actor") ) != -1 )
			{
				if ( _TopLevel )
					( (CActorScript *)_Components[i] )->setTopLevel( _TopLevel );
				else
					( (CActorScript *)_Components[i] )->setTopLevel( this );
			}
		}

	}

	const CAgentScript *CActorScript::getTopLevel() const
	{
		return _TopLevel;
	}
}
