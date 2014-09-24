#include "eventpool.h"
#include "connect.h"
#include "win32helper.h"
#include "memcheck.h"
namespace mynet{
	class Event:public EventBase{
    public:
        Event(Target *target):EventBase(target)
        {
			reset();
		}
		OVERLAPPED overlapped;
        bool checkValid()
        {
            return target;
        }
           
		WSABUF         m_wsaBuf;                                 
		char           buffer[MAX_BUFFER_LEN];             
		           
		DWORD msgLen;
		
		HANDLE poolHandle; 
		virtual void reset()
		{
			memset(buffer,0,MAX_BUFFER_LEN);
			memset(&overlapped,0,sizeof(overlapped));  
			m_wsaBuf.buf = buffer;
			m_wsaBuf.len = MAX_BUFFER_LEN;
			eventType     = 0;
			msgLen = 0;
		}
		virtual void deal(){};
		virtual void redo(){deal();}
		unsigned int getPeerHandle(){return 0;}
    };
	/**
	 *  
	 */
	template<typename TARGET>
	class InEvent:public Event{
	public:
		InEvent(TARGET *target):Event(target)
        {
			
		}
		void deal()
		{
			DWORD dwFlags = 0;
			DWORD dwBytes = 0;
			WSABUF *p_wbuf   = &m_wsaBuf;
			OVERLAPPED *p_ol = (OVERLAPPED*)&overlapped;
			reset();
			eventType = IN_EVT;
			int nBytesRecv = WSARecv((SOCKET)target->getHandle(), p_wbuf, 1, &msgLen, &dwFlags, p_ol, NULL );
			int result = WSAGetLastError();
			if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != result))
			{
				return;
			}
		}
	};
	/**
	 *  
	 */
	template<typename TARGET>
	class OutEvent:public Event{
	public:
		OutEvent(TARGET *target):Event(target)
		{
			
		}
		mynet::MyList<char*> buffers;
		void deal()
		{
			if (!dataLen) return;
			DWORD dwFlags = 0;
			msgLen = 0;
			WSABUF *p_wbuf   = &m_wsaBuf;
			m_wsaBuf.len = dataLen;
			OVERLAPPED *p_ol = (OVERLAPPED*)&overlapped;
			eventType = OUT_EVT;
			p_wbuf->buf = buffer;
			int nBytesRecv = WSASend((SOCKET)target->getHandle(), p_wbuf, 1, &msgLen, dwFlags, p_ol, NULL );
			int result = WSAGetLastError();
			if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != result))
			{
				printf("error happened int outevent deal\n");
				return;
			}
			return;
		}
	};
	/**
	 *   
	 **/
	template<typename TARGET>
	class AcceptEvent:public Event{
	public:
		AcceptEvent(TARGET *target):Event(target)
		{
			SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);  
			if( INVALID_SOCKET ==  socket)  
			{  
				return;  
			} 
			handle = socket;	
		}
		void deal()
		{
			DWORD dwBytes = 0;  
			eventType = ACCEPT_EVT;  
			WSABUF *p_wbuf   = &m_wsaBuf;
			OVERLAPPED *p_ol = (OVERLAPPED*)&overlapped;
			// 
			
			AcceptHelper::getMe().doAccept((SOCKET)target->getHandle(),handle,p_wbuf,p_ol);
		}
		SOCKET handle;
		virtual unsigned int getPeerHandle() {return (unsigned int)handle;}
	};
   	class EventPoolIOCP:public Pool{
   	public:
   		EventPoolIOCP()
   		{
   			poolHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0 );
   		}
   		HANDLE poolHandle;
   		void bindEvent(Target *target,int eventType)
   		{
   			 HANDLE tempHandle = CreateIoCompletionPort((HANDLE)target->getHandle(), poolHandle, (DWORD)target, 0);
			 if (eventType & ACCEPT_EVT)
			 {
			 	for (int i = 0; i < 10;i++)
			 	{
			 		target->inEvt = MYNEW AcceptEvent<Target>(target);
					target->inEvt->deal();
			 	}
			 }
			 if (eventType & IN_EVT)
			 {
				target->inEvt = MYNEW InEvent<Target>(target);
				target->inEvt->deal();
			 }
			 if (eventType & OUT_EVT)
			 {
				target->outEvt = MYNEW OutEvent<Target>(target);
				target->doSend(target->outEvt);
			 }
   		}
   		EventBase* pullEvent()
		{
			//  
			OVERLAPPED *pOverlapped = NULL;
			Target*target = NULL;
			DWORD dataLen = 0;
			BOOL bReturn = GetQueuedCompletionStatus(
				poolHandle,
				&dataLen,
				(PULONG_PTR)&target,
				&pOverlapped,
				100//INFINITE
			);
			//  
			if ( 0==(DWORD)target)
			{
				return NULL;
			}
			//  
			if( !bReturn )  
			{
				if (!target->inEvt) return NULL;
				target->inEvt->eventType = ERR_EVT;
				return target->inEvt;  
			}  
			else  if (target)
			{
				//  
				Event* evt = CONTAINING_RECORD(pOverlapped, Event, overlapped); 
				if (target != evt->target) return NULL;
				evt->dataLen = dataLen;
				if (evt && evt->isAccept() && evt->target)
				{
					bindEvent(evt->target,ACCEPT_EVT);
				}
				//  
				if (dataLen == 0 && (evt->isIn() || evt->isOut()))
				{
					evt->eventType = ERR_EVT; //  
				}
				return evt;
			}
			return NULL;
		}

   	};
	bool EventPool::init()
	{
		static bool initLoad = false;
		if (!initLoad)
		{
			WSADATA wsaData;
			int nResult;
			nResult = WSAStartup(MAKEWORD(2,2), &wsaData);
			initLoad = true;
		}
		if (!pool)
			pool = MYNEW EventPoolIOCP();
		return ((EventPoolIOCP*)pool)->poolHandle != NULL;
	}
    void EventPool::bindEvent(Target *target,int eventType)
	{
		if (target->getHandle() == -1 ) return;
		if (pool) pool->bindEvent(target,eventType);
	}

	EventBase* EventPool::pullEvent()
	{
		if (pool) return pool->pullEvent();
		return NULL;
	}

	/**
	 *  pool  
	 **/
	 void Connection::doRead(EventBase *evt)
	{
		InEvent<Connection>* event = static_cast<InEvent<Connection>*>( evt );
		
		Record *record = MYNEW Record(event->m_wsaBuf.buf,evt->dataLen);
		printf("Connection::doRead GetContent %u\n",evt->dataLen);
		recvs.write(record);
	//	updateRecvCmds();
		
		evt->redo();
	}
	/**
	 * pool 
	 **/
	void Connection::doSend(EventBase *evt,bool over)
	{
		bool tag = false;
		OutEvent<Connection>* event = static_cast<OutEvent<Connection>*>( evt );
		if (event->msgLen < event->dataLen && !over)
		{
			return;
		}
		event->dataLen = 0;
		event->reset();
		int leftLen = EventBase::MAX_BUFFER_LEN;
		while (!sends.empty() && leftLen > 0)
		{
			tag = true;
			Record *record = NULL;
			if (sends.readOnly(record))
			{
				unsigned int realCopySize = record->recv(event->buffer,leftLen);
				evt->dataLen += realCopySize;
				
				if (leftLen == realCopySize)
				{
					leftLen = 0;
					if (record->empty())
					{
						delete record;
						sends.pop();
					}
					break;
				}
				else
				{
					leftLen -= realCopySize;
					if(!record->empty())
					{
						// TODO ERROR
					}
					sends.pop();
				}
			}else break;
		}
		if (tag)
			evt->redo();
	}
	int Connection::send(void *cmd, unsigned int len)
	{
		if (socket == -1) return -1;
		return ::send(socket, (char*)cmd, len, 0);
	}
};