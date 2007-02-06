#include "stdmisc.h"
#include "nel/misc/inter_window_msg_queue.h"
//
#include "nel/misc/mem_stream.h"
#include "nel/misc/shared_memory.h"

namespace NLMISC
{


CSynchronized<CInterWindowMsgQueue::TMessageQueueMap> CInterWindowMsgQueue::_MessageQueueMap("CInterWindowMsgQueue::_MessageQueueMap");
const uint CInterWindowMsgQueue::_CurrentVersion = 0;
CInterWindowMsgQueue::TOldWinProcMap CInterWindowMsgQueue::_OldWinProcMap;




//**************************************************************************************************

////////////////////////////////////////
// CInterWindowMsgQueue::CProtagonist //
////////////////////////////////////////

//**************************************************************************************************
CInterWindowMsgQueue::CProtagonist::CProtagonist() : _Id(0),
													 _Wnd(0),
													 _SharedMemMutex(0),													 
													 _SharedWndHandle(NULL)
{
}


//**************************************************************************************************
CInterWindowMsgQueue::CProtagonist::~CProtagonist()
{
	release();	
}

//**************************************************************************************************
void CInterWindowMsgQueue::CProtagonist::release()
{		
	CloseHandle(_SharedMemMutex);
	_SharedMemMutex = 0;				
	if (_SharedWndHandle)
	{
		CSharedMemory::closeSharedMemory(_SharedWndHandle);
		_SharedWndHandle = NULL;
	}
	_Wnd = 0;
	_Id = 0;
	// unhook window	
}

//**************************************************************************************************
void CInterWindowMsgQueue::CProtagonist::acquireSMMutex()
{
	nlassert(_SharedMemMutex);
	DWORD result = WaitForSingleObject(_SharedMemMutex, INFINITE);
	nlassert(result != WAIT_FAILED);
}

//**************************************************************************************************
void CInterWindowMsgQueue::CProtagonist::releaseSMMutex()
{
	nlassert(_SharedMemMutex);
	ReleaseMutex(_SharedMemMutex);
}

//**************************************************************************************************
bool CInterWindowMsgQueue::CProtagonist::init(uint32 id)
{
	nlassert(id != 0);
	nlassert(id != 0x3a732235); // cf doc of NLMISC::CSharedMemory : this id is reserved
	nlassert(_Id == 0); // init done twice
	release();		
	// create a system wide mutex
	_SharedMemMutex = CreateMutex(NULL, FALSE, toString("NL_MUTEX_%d", (int) id).c_str());
	if (!_SharedMemMutex) return false;	
	_Id = id;
	return true;
}

//**************************************************************************************************
void CInterWindowMsgQueue::CProtagonist::setWnd(HWND wnd)
{
	nlassert(wnd != 0);
	nlassert(_SharedWndHandle == NULL); // setWnd was called before ?
										// setWnd should be called once for the 'local' window at beginning
	acquireSMMutex();
	_SharedWndHandle = CSharedMemory::createSharedMemory(toSharedMemId((int) _Id), sizeof(HWND));
	if (_SharedWndHandle)
	{
		*(HWND *) _SharedWndHandle = wnd;
	}
	releaseSMMutex();
	_Wnd = wnd;
}

//**************************************************************************************************
HWND CInterWindowMsgQueue::CProtagonist::getWnd()
{
	if (!_SharedMemMutex)
	{
		nlassert(_Wnd == 0);
		nlassert(!_SharedWndHandle);
		return 0;
	}
	if (_Wnd != 0)
	{
		// local window case		
		return _Wnd;
	}
	// this is the foreign window
	// access shared memory just for the time of the query 	(this allow to see if foreign window is still alive)
	nlassert(!_SharedWndHandle);
	HWND result = 0;
	acquireSMMutex();
	void *sharedMem = CSharedMemory::accessSharedMemory(toSharedMemId((int) _Id));
	if (sharedMem)
	{
		result = *(HWND *) sharedMem;
		CSharedMemory::closeSharedMemory(sharedMem);		
	}
	releaseSMMutex();
	return result;

}


//**************************************************************************************************

/////////////////////////////////////
// CInterWindowMsgQueue::CSendTask //
/////////////////////////////////////

//**************************************************************************************************
CInterWindowMsgQueue::CSendTask::CSendTask(CInterWindowMsgQueue *parent) : _StopAsked(false)
{
	nlassert(parent);
	_Parent = parent;
}

//**************************************************************************************************
void CInterWindowMsgQueue::CSendTask::run()
{
	while(!_StopAsked)
	{		
		nlassert(_Parent);
		HWND targetWindow = _Parent->_ForeignWindow.getWnd();
		if (targetWindow == 0)
		{
			// there is no receiver, so message queue has become irrelevant, so just clear it
			_Parent->clearOutQueue();
		}
		else
		{			
			TMsgList nestedMsgs;	
			CMemStream outMsg;
			{					
				CSynchronized<TMsgList>::CAccessor outMessageQueue(&_Parent->_OutMessageQueue);					
				nestedMsgs.swap(outMessageQueue.value());					
			}
			if (!nestedMsgs.empty())
			{
				CMemStream msgOut(false);
				msgOut.serialVersion(CInterWindowMsgQueue::_CurrentVersion);
				uint32 fromId(_Parent->_LocalWindow.getId());
				uint32 toId(_Parent->_ForeignWindow.getId());
				msgOut.serial(fromId);
				msgOut.serial(toId);
				msgOut.serialCont(nestedMsgs);				
				COPYDATASTRUCT cds;
				cds.dwData = 0;
				cds.cbData = msgOut.length();
				cds.lpData = (PVOID) msgOut.buffer();
				for(;;)
				{
					HRESULT result = ::SendMessage(targetWindow, WM_COPYDATA, (WPARAM) _Parent->_LocalWindow.getWnd(), (LPARAM) &cds);
					if (result) break;
					// retry ...
					Sleep(30);
					if (!_Parent->_ForeignWindow.getWnd())
					{
						nlwarning("CInterWindowMsgQueue : tried to send message, but destination window has been closed");
						break;
					}								
				}
			}
		}
		Sleep(30);
	}
}

//**************************************************************************************************
void CInterWindowMsgQueue::CSendTask::stop()
{
	_StopAsked = true;
}


//**************************************************************************************************

//////////////////////////
// CInterWindowMsgQueue //
//////////////////////////

//**************************************************************************************************
CInterWindowMsgQueue::CInterWindowMsgQueue() : _SendTask(NULL),
											   _SendThread(NULL),
											   _OutMessageQueue("CInterWindowMsgQueue::_OutMessageQueue")
{	
}

//**************************************************************************************************
bool CInterWindowMsgQueue::init(HWND ownerWindow, uint32 localId, uint32 foreignId)
{			
	nlassert(ownerWindow != NULL);
	nlassert(localId != 0);
	nlassert(foreignId != 0);
	nlassert(localId != foreignId);
	{				
		typedef CSynchronized<TMessageQueueMap>::CAccessor TAccessor;
		// NB : use a 'new' instead of an automatic object here, because I got an 'INTERNAL COMPILER ERROR' compiler file 'msc1.cpp', line 1794
		// else, this is one of the way recommended by microsoft to solve the problem.
		std::auto_ptr<TAccessor> messageQueueMap(new TAccessor(&_MessageQueueMap));		
		CMsgQueueIdent msgQueueIdent(ownerWindow, localId, foreignId);
		if (messageQueueMap->value().count(msgQueueIdent))
		{
			// message queue already exists
			return false;
		}				
		WNDPROC oldWinProc = (WNDPROC) GetWindowLong(ownerWindow, GWL_WNDPROC);
		uint &refCount = _OldWinProcMap[ownerWindow].RefCount;	
		++ refCount;
		if (refCount == 1)
		{
			nlassert(oldWinProc != ListenerProc); // first registration so the winproc must be different
			SetWindowLong(ownerWindow, GWL_WNDPROC, (LONG) ListenerProc);	
			_OldWinProcMap[ownerWindow].OldWinProc = oldWinProc;
		}
		else
		{
			nlassert(oldWinProc == ListenerProc);
		}				
		//
		messageQueueMap->value()[msgQueueIdent] = this;
		_SendTask = new CSendTask(this);
		_SendThread = IThread::create(_SendTask);
		_SendThread->start();
		// init the window handle in shared memory last,
		// this way we are sure that the ne win proc has been installed, and can start received messages
		bool ok = _LocalWindow.init(localId) && _ForeignWindow.init(foreignId);
		if (!ok)
		{
			release();
		}
		_LocalWindow.setWnd(ownerWindow);
	}
	return true;
}

//**************************************************************************************************
void CInterWindowMsgQueue::release()
{	
	if (_LocalWindow.getWnd() != 0)
	{
		if (IsWindow(_LocalWindow.getWnd())) // handle gracefully case where the window has been destroyed before
											 // this manager
		{
			WNDPROC currWinProc = (WNDPROC) GetWindowLong(_LocalWindow.getWnd(), GWL_WNDPROC);
			if (currWinProc != ListenerProc)
			{
				nlassert(0); // IF THIS ASSERT FIRES : 
							 // either : 
							 // - The window handle has been removed, and recreated
							 // - The window has been hooked by someone else
							 // in the application, but not unhooked properly.
							 // Hook (performed at init) and unhook (performed here at release) should be
							 // done in reverse order ! We got no way to unhook correclty, else.
			}
		}
		TOldWinProcMap::iterator it = _OldWinProcMap.find(_LocalWindow.getWnd());
		nlassert(it != _OldWinProcMap.end());
		nlassert(it->second.RefCount > 0);
		-- it->second.RefCount;
		if (it->second.RefCount == 0)
		{
			if (IsWindow(_LocalWindow.getWnd()))
			{
				SetWindowLong(_LocalWindow.getWnd(), GWL_WNDPROC, (LONG) it->second.OldWinProc);
			}
			_OldWinProcMap.erase(it);
		}		
	}
	if (_SendThread)
	{
		if (_SendTask)
		{
			_SendTask->stop();
			_SendThread->wait();
		}
		delete _SendTask;
		delete _SendThread;
		_SendTask = NULL;
		_SendThread = NULL;
	}
	if (_LocalWindow.getId() != 0)
	{		
		typedef CSynchronized<TMessageQueueMap>::CAccessor TAccessor;
		// NB : use a 'new' instead of an automatic object here, because I got an 'INTERNAL COMPILER ERROR' compiler file 'msc1.cpp', line 1794
		// else, this is one of the way recommended by microsoft to solve the problem.
		std::auto_ptr<TAccessor> messageQueueMap(new TAccessor(&_MessageQueueMap));				
		TMessageQueueMap::iterator it = messageQueueMap->value().find(CMsgQueueIdent(_LocalWindow.getWnd(), _LocalWindow.getId(), _ForeignWindow.getId()));
		nlassert(it != messageQueueMap->value().end());
		messageQueueMap->value().erase(it);		
	}	
	clearOutQueue();
	_LocalWindow.release();
	_ForeignWindow.release();
}


//=====================================================
LRESULT CALLBACK CInterWindowMsgQueue::ListenerProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	if (uMsg == WM_COPYDATA) // WM_COPYDATA messages are sent by the other window to communicate with the client
	{
		// ctruct a write stream
		CMemStream msgIn;
		COPYDATASTRUCT *cds = (COPYDATASTRUCT *) lParam;
		if (cds->lpData)
		{
			uint32 fromId;
			uint32 toId;
			TMsgList nestedMsgs;			
			try
			{
				
				msgIn.serialBuffer((uint8 *) cds->lpData, cds->cbData);
				// make it a read stream
				msgIn.resetPtrTable();
				msgIn.invert();				
				nlassert(msgIn.isReading());
				msgIn.serialVersion(_CurrentVersion);
				msgIn.serial(fromId);
				msgIn.serial(toId);
				msgIn.serialCont(nestedMsgs);

				CInterWindowMsgQueue messageQueue;
				{
					typedef CSynchronized<TMessageQueueMap>::CAccessor TAccessor;
					// NB : use a 'new' instead of an automatic object here, because I got an 'INTERNAL COMPILER ERROR' compiler file 'msc1.cpp', line 1794
					// else, this is one of the way recommended by microsoft to solve the problem.
					std::auto_ptr<TAccessor> messageQueueMap(new TAccessor(&_MessageQueueMap));									
					TMessageQueueMap::iterator it = messageQueueMap->value().find(CMsgQueueIdent(hwnd, toId, fromId));
					if (it != messageQueueMap->value().end())
					{
						// no mutex stuff here, we're in the main thread
						TMsgList &targetList = it->second->_InMessageQueue;
						targetList.splice(targetList.end(), nestedMsgs); // append
						return TRUE;
					}
					else
					{
						nlwarning("CInterWindowMsgQueue : Received inter window message from '%x' to '%x', but there's no associated message queue", (int) fromId, (int) toId);						
					}					
				}								
			}
			catch(EStream &)
			{
				nlwarning("CInterWindowMsgQueue : Bad message format in inter window communication");
			}

		}
		else
		{			
			// msg received with NULL content
			nlwarning("CInterWindowMsgQueue : NULL message received");
		}
		return FALSE;
	}
	else
	{
		TOldWinProcMap::iterator it = _OldWinProcMap.find(hwnd);
		nlassert(it != _OldWinProcMap.end());			
		return CallWindowProc(it->second.OldWinProc, hwnd, uMsg, wParam, lParam);
	}
}

//**************************************************************************************************
CInterWindowMsgQueue::~CInterWindowMsgQueue()
{		
	release();
}

//**************************************************************************************************
void CInterWindowMsgQueue::clearOutQueue()
{
	CSynchronized<TMsgList>::CAccessor  outMessageQueue(&_OutMessageQueue);	
	if (!outMessageQueue.value().empty())
	{
		nlwarning("CInterWindowMsgQueue : clearing out queue");
		outMessageQueue.value().clear();
	}
}

//**************************************************************************************************
void CInterWindowMsgQueue::sendMessage(CMemStream &msg)
{
	if (!msg.isReading())
	{
		msg.invert();
	}
	msg.resetPtrTable();
	{
		CSynchronized<TMsgList>::CAccessor outMessageQueue(&_OutMessageQueue);		
		std::vector<uint8> sentMsg(msg.buffer(), msg.buffer() + msg.length());		
		outMessageQueue.value().push_back(CMsg());
		outMessageQueue.value().back().Msg.swap(sentMsg);		
	}
}


//**************************************************************************************************
bool CInterWindowMsgQueue::pumpMessage(CMemStream &dest)
{
	if (_InMessageQueue.empty()) return false;
	if (dest.isReading())
	{
		dest.invert();
		dest.clear();		
	}
	std::vector<uint8> &msgIn = _InMessageQueue.front().Msg;
	dest.serialBuffer(&(msgIn[0]), msgIn.size());
	_InMessageQueue.pop_front();
	// make dest a read stream
	dest.invert();
	dest.resetPtrTable();
	return true;
}

//**************************************************************************************************
bool CInterWindowMsgQueue::connected() const
{
	return const_cast<CProtagonist &>(_ForeignWindow).getWnd() != NULL;
}

//**************************************************************************************************
uint CInterWindowMsgQueue::getSendQueueSize() const
{
	CSynchronized<TMsgList>::CAccessor outMessageQueue(&const_cast<CSynchronized<TMsgList> &>(_OutMessageQueue));
	return outMessageQueue.value().size();	
}

//**************************************************************************************************
uint CInterWindowMsgQueue::getReceiveQueueSize() const
{
	return _InMessageQueue.size();
}



} // NLMISC

