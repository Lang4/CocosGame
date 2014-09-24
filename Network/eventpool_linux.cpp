#include "eventpool.h"
#include "connect.h"
#include "sys/socket.h"
#include "sys/epoll.h"
#include <sys/time.h>
#include "sys/poll.h"
#include "sys/select.h"
#include "sys/types.h"
#include "netinet/in.h"
#include "fcntl.h"
#include "netdb.h"
#include "signal.h"
#include "memory.h"
#include <arpa/inet.h>
#include <pthread.h>
#include "strings.h"
#include "iconv.h"
#include <strings.h>
#include "unistd.h"
#include "poll.h"
#include "vector"
#include "list"
#include "stdio.h"
#include "sstream"
namespace mynet{
	/**
     * Pool 处理器
     **/

	class Event:public EventBase{
	public:
		struct epoll_event ev;
		void deal(int eventType)
		{
			target->evt = this;
			ev.data.ptr = this;
			set(eventType);
			epoll_ctl(poolHandle,EPOLL_CTL_ADD,target->getHandle(),&ev);
		}
		void set(int eventType)
		{
			EventBase::eventType = eventType;
			ev.events = 0;
			if (eventType & IN_EVT)
			{
				ev.events |= EPOLLIN;
			}
			if (eventType & OUT_EVT)
			{
				ev.events |= EPOLLOUT;
			}
			if (eventType & ACCEPT_EVT)
			{
				ev.events |= EPOLLIN;
			}
		}
		void stopWrite()
		{
			set(IN_EVT);
			epoll_ctl(poolHandle,EPOLL_CTL_MOD,target->getHandle(),&ev);
		}
		void startRead()
		{
			
		}
		void stopRead()
		{
			set(OUT_EVT);
			epoll_ctl(poolHandle,EPOLL_CTL_MOD,target->getHandle(),&ev);
		}
		void startWrite()
		{
			set(OUT_EVT|IN_EVT);
			epoll_ctl(poolHandle,EPOLL_CTL_MOD,target->getHandle(),&ev);	
		}
		int getPeerHandle()
		{
			return target->getPeerHandle();
		}
		bool isOut()
		{
			return (ev.events & EPOLLOUT);
		}
		bool isIn()
		{
			return (ev.events & EPOLLIN);
		}
		bool isErr()
		{
			return (eventType & ERROR_EVT);
		}
		bool isAccept()
		{
			return (eventType & ACCEPT_EVT) && (ev.events & EPOLLIN);
		} 
	};

	class EpollEventPool:public Pool{
	public:
		EpollEventPool()
		{
		}
		void init();
		void bindEvent(Target *target,int eventType);
		std::vector<struct epoll_event> eventBuffer;
		int poolHandle;
		EventBase *pullEvent();
	};

	void EpollEventPool::init()
	{
		eventBuffer.resize(1024);
		poolHandle = epoll_create(256);
	}	
	void EpollEventPool::bindEvent(Target *target,int eventType)
	{
		Event<Target> *evt = new Event<Target>(target);
		evt->poolHandle = poolHandle;
		evt->deal(eventType); // 构建事件 
	}
	EventBase *EpollEventPool::pullEvent()
	{
		static std::list<EventBase*> events;
		int retcode = epoll_wait(poolHandle,&eventBuffer[0],eventBuffer.size(),800);
		if (events.empty())
		{
			for (int i = 0; i < retcode;i++)
			{
				EventBase *target = (EventBase*) eventBuffer[i].data.ptr;
				if (target)
				{
					events.push_back(target);
				}
			}
		}
		else
		{
			events.pop_front();
		}
		if (events.empty()) return NULL;
		return events.front();
	}	
	void EventPool::init()
	{
		pool = new EpollEventPool();
		pool->init();
	}
	void EventPool:: bindEvent(Target *target,int eventType)
	{
		if (pool) pool->bindEvent(target,eventType);
	}
	EventBase * EventPool::pullEvent()
	{
		if (pool) return pool->pullEvent();
		return NULL;
	}	

	/**
	 **/
	 void Connection::doRead(EventBase *evt)
	{
		Event* event = static_cast<Event*>( evt );
		while(true)
		{
			memset(buffer,0,EventBase::MAX_BUFFER_LEN);
			int leftLen = ::recv(socket,buffer,EventBase::MAX_BUFFER_LEN,0);
			logToFile(buffer,leftLen);
			if (leftLen == -1)
			{
				//printf("---接受数据到头了--\n");
				 return;
			}
			if (leftLen == 0)
			{
				event->eventType = ERROR_EVT;
				return;
			}
			
			Record *record = new Record(buffer,leftLen);
			recvs.write(record);
			doReadBack();
			
			if (leftLen < EventBase::MAX_BUFFER_LEN)
			{
		//		break;
			}
		}
		event->startRead();
	}
	/**
	 * 
	 **/
	void Connection::doSend(EventBase *evt,bool over)
	{
		bool tag = false;
		Event* event = static_cast<Event*>( evt );
		event->stopWrite();
		while (!sends.empty())
		{
			Record *record = NULL;
			if (sends.readOnly(record))
			{
				if (record->sendOver(this))
				{
					//printf("%s\n","doSend");
					sends.pop();
					delete record;
				}
				else
				{
					 tag = true;
					break;
				}
			}else break;
		}
		if (tag)
		{
			event->startWrite();
		}
	}

};