#include "eventpool.h"
#include "connect.h"
#include "win32helper.h"
namespace mynet{
	class Event:public EventBase{
    public:
        Event(Target *target):EventBase(target)
        {
			reset();
			valid = true;
		}
		
        bool checkValid()
        {
            return valid;
        }
           
		
		virtual void reset(){}
		virtual void deal(){};
		virtual void redo(){deal();}
		unsigned int getPeerHandle(){return 0;}
		int getHandle(){return target->getHandle();}
		void setInValid(){valid = false;}
		bool valid;
    };
	
   	class EventPoolSelect:public Pool{
   	public:
   		std::list<Event*> tasks;
   		fd_set rfds,wfds,errfds;
   		std::vector<Event*> events;
   		int index;
   		int maxCount;
   		EventPoolSelect()
   		{
   			index = maxCount = 0;
   		}
   		HANDLE poolHandle;
   		void bindEvent(Target *target,int eventType)
   		{
   			Event *event = new Event(target);
   			event->eventType = eventType;
   			tasks.push_back(event);
   		}
   		EventBase* pullEvent()
		{
			if (index == maxCount)
			{
				events.clear();
				index = maxCount = 0;
				FD_ZERO(&rfds);
				FD_ZERO(&wfds);
				FD_ZERO(&errfds);
				int FD_SET_SIZE = 0;
				for (std::list<Event*>::iterator iter = tasks.begin(); iter != tasks.end();++iter)
				{
					Event * task = (Event*)*iter;
					if (task->checkValid())
					{
						int fd = task->getHandle();
						if (-1 == fd){
							//printf("one task invalid %p\n",task);
							continue;
						}
						if (fd > FD_SET_SIZE) FD_SET_SIZE = fd;
						FD_SET(fd,&rfds);
						FD_SET(fd,&wfds);
						FD_SET(fd,&errfds);
						task->eventType = 0;
					}
				}
			
				struct timeval tv;
				tv.tv_usec = 50000;
				tv.tv_sec = 0;
				if (FD_SETSIZE)
				{
					int ret = select(FD_SET_SIZE + 1,&rfds,&wfds,&errfds,&tv);
					if (0 <= ret)
					{
						for (std::list<Event*>::iterator iter = tasks.begin(); iter != tasks.end();++iter)
						{
							Event * task =(Event*) *iter;
							task->eventType = 0;
							if (task->checkValid())
							{
								int fd = task->getHandle();
								if (FD_ISSET(fd,&errfds))
								{
									// ERROR
									task->eventType |= ERR_EVT;
									task->setInValid();
									FD_CLR(fd,&errfds);
									continue;
								}
								if (FD_ISSET(fd,&rfds))
								{
									task->eventType |= IN_EVT;
									// READ
									FD_CLR(fd, &rfds);
								}
								if (FD_ISSET(fd,&wfds))
								{
									task->eventType |= OUT_EVT;
									// WRITE
									FD_CLR(fd, &wfds);
								}
							}
							if (task->eventType != 0)
							{
								maxCount ++;
								events.push_back(task);	
							}
						}
					}
				}	
			}
			if (index < maxCount)
			{
				return events[index++];
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
			pool = new EventPoolSelect();
		return ((EventPoolSelect*)pool)->poolHandle != NULL;
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
        while(true)
        {
			char buffer[EventBase::MAX_BUFFER_LEN];
            memset(buffer,0,EventBase::MAX_BUFFER_LEN);
            int leftLen = ::recv(socket,buffer,EventBase::MAX_BUFFER_LEN,0);
			printf("---doReadData %d--\n",leftLen);
            if (leftLen == -1)
            {    
                return;
            }
            if (leftLen == 0)
            {
                //socket = -1;
                //evt->eventType = ERR_EVT;
                return;
            }
          
            
            Record *record = new Record(buffer,leftLen);
            
            recvs.write(record);
            
            if (leftLen <EventBase::MAX_BUFFER_LEN)
            {
                //              break;
            }
//			this->updateRecvCmds();
        }
	}
	/**
	 * pool 
	 **/
	void Connection::doSend(EventBase *evt,bool over)
	{
		bool tag = false;
        while (!sends.empty())
        {
            Record *record = NULL;
            if (sends.readOnly(record))
            {
                if (record->sendOver(this))
                {
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
        }
	}
	int Connection::send(void *cmd,unsigned int len)
    {
        if (socket == -1) return -1;
        return ::send(socket,(char*)cmd,len,0);
    }
};