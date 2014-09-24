#include "eventpool.h"
#include "connect.h"
#include "sys/socket.h"
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
#include "sys/event.h"
#include <sys/types.h>

namespace mynet{

    class Event:public EventBase{
    public:
        Event(Target *target):EventBase(target){
            poolHandle = 0;
        }
        void startRead();
                
        void stopWrite();
        void startWrite();
        
        void delEevnt(int eventType);
        void addEvent(int eventType);
        void disableEvent(int eventType);
        void enableEvent(int eventType);

        void deal(){}
        void redo(){}
        int getPeerHandle()
        {
            return target->getPeerHandle();
        }
        int poolHandle;
    };
	 /**
     * Pool 处理器
     **/
    void Event::delEevnt(int eventType)
    {
        if ((eventType & IN_EVT) | (ACCEPT_EVT & eventType))
        {
            struct kevent kevts[1];
            EV_SET(&kevts[0], target->getHandle(), EVFILT_READ, EV_DELETE, 0, 0, this);
            kevent(poolHandle, kevts, 1, NULL, 0, NULL);
            this->eventType &= ~IN_EVT;
        }
        if (eventType & OUT_EVT)
        {
            struct kevent kevts[1];
            EV_SET(&kevts[0], target->getHandle(), EVFILT_WRITE,EV_DELETE, 0, 0, this);
            kevent(poolHandle, kevts, 1, NULL, 0, NULL);
            this->eventType &= ~OUT_EVT;
        }
    }
    void Event::addEvent(int eventType)
    {
        if ((eventType & IN_EVT) | (ACCEPT_EVT & eventType))
        {
            struct kevent kevts[1];
            EV_SET(&kevts[0], target->getHandle(), EVFILT_READ, EV_ADD, 0, 0, this);
            kevent(poolHandle, kevts, 1, NULL, 0, NULL);
        }
        if (eventType & OUT_EVT)
        {
            struct kevent kevts[1];
            EV_SET(&kevts[0], target->getHandle(), EVFILT_WRITE,EV_ADD, 0, 0, this);
            kevent(poolHandle, kevts, 1, NULL, 0, NULL);
        }
    }
    void Event::disableEvent(int eventType)
    {
        if ((eventType & IN_EVT) | (ACCEPT_EVT & eventType))
        {
            struct kevent kevts[1];
            EV_SET(&kevts[0], target->getHandle(), EVFILT_READ, EV_DISABLE, 0, 0, this);
            kevent(poolHandle, kevts, 1, NULL, 0, NULL);
        }
        if (eventType & OUT_EVT)
        {
            struct kevent kevts[1];
            EV_SET(&kevts[0], target->getHandle(), EVFILT_WRITE,EV_DISABLE, 0, 0, this);
            kevent(poolHandle, kevts, 1, NULL, 0, NULL);
        }
    }
    void Event::enableEvent(int eventType)
    {
        if ((eventType & IN_EVT) | (ACCEPT_EVT & eventType))
        {
            struct kevent kevts[1];
            EV_SET(&kevts[0], target->getHandle(), EVFILT_READ, EV_ENABLE, 0, 0, this);
            kevent(poolHandle, kevts, 1, NULL, 0, NULL);
        }
        if (eventType & OUT_EVT)
        {
            struct kevent kevts[1];
            EV_SET(&kevts[0], target->getHandle(), EVFILT_WRITE,EV_ENABLE, 0, 0, this);
            kevent(poolHandle, kevts, 1, NULL, 0, NULL);
        }
    }
    
    bool Event::isOut()
    {
        return eventType & OUT_EVT;
    }
    
    bool Event::isIn(){
        return eventType & IN_EVT;
    }
    
    bool Event::isErr()
    {
        return eventType & ERR_EVT;
    }
    int Event::getPeerHandle()
    {
        return target->getPeerHandle();
    }
    
    void Event::startRead()
    {
        enableEvent(IN_EVT);
    }
    
    void Event::stopWrite()
    {
        disableEvent(OUT_EVT);
    }
    void Event::startWrite()
    {
        enableEvent(OUT_EVT);
    }
    class KqueueEventPool{
    public:
        void init();
        void bindEvent(Target *target,int eventType);
        static const unsigned int MAX_EVENT_COUNT = 100;
        EventBase * pullEvent();
        int poolHandle;
        struct kevent events[MAX_EVENT_COUNT];
        int index;
        int maxCount;
    };
    /**
     * KqueueEventPool
     */
    void KqueueEventPool::init()
    {
        poolHandle = kqueue();
        index = 0;
        maxCount = 0;
    }
    void KqueueEventPool::bindEvent(Target *target,int eventType)
    {
        Event*evt = new Event(target);
        target->evt = evt;
        evt->poolHandle = poolHandle;
        evt->addEvent(eventType);
    }
    EventBase * KqueueEventPool::pullEvent()
    {
        if (index == maxCount)
        {
            maxCount = kevent(poolHandle, NULL, 0, (struct kevent *)events, MAX_EVENT_COUNT, NULL);
            index = 0;
        }
        if (index < maxCount)
        {
            EventBase* base = (EventBase*) events[index].udata;
            if (!base) return NULL;
            base->dataLen = events[index].data;
            base->eventType = 0;
            if (events[index].flags & EV_ERROR)
            {
                base->eventType |= ERR_EVT;
            }
            switch (events[index].filter)
            {
                case EVFILT_READ:
                {
                    base->eventType |= IN_EVT;
                }break;
                case EVFILT_WRITE:
                {
                    base->eventType |= OUT_EVT;
                }break;
            }
            index++;
            return base;
        }
        return NULL;
    }
    void EventPool::init()
    {
        pool = new KqueueEventPool();
        pool->init();
    }
    void EventPool::bindEvent(Target *target,int eventType)
    {
        if (pool) pool->bindEvent(target,eventType);
    }
    EventBase *EventPool::pullEvent()
    {
        if (pool) return pool->pullEvent();
        return NULL;
    }
    Connection::Connection()
    {
        directDealCmd = true;
        allReadSize = 0;
    }

    /**
     * Connection 处理
     **/
    void Connection::destroy()
    {
        Target::destroy();
        ::close(socket);
        socket = -1;
    }

    /**
     * 发送消息
     */
    void Connection::sendCmd(void *cmd,unsigned int len)
    {
        Decoder  decoder;
        decoder.encode(cmd,len);
        sends.write(decoder.getRecord());
        if (evt)
            evt->startWrite();
    }
    /**
     * 从socket 读数据
     */
    int Connection::read(void *cmd,unsigned int len)
    {
        return ::recv(socket,cmd,len,0);
    }
    /**
     * 向socket 写数据
     */
    int Connection::send(void *cmd,unsigned int len)
    {
        if (socket == -1) return -1;
        return ::send(socket,cmd,len,0);
    }
    /**
     * 将消息接受到缓存
     **/
    unsigned int Connection::recv(void *cmd,unsigned int size)
    {
        unsigned int realcopy = 0;
        while (!recvs.empty())
        {
            Record *record = NULL;
            if (recvs.readOnly(record))
            {
                realcopy = record->recv(cmd,size);
                if (record->empty())
                {
                    delete record;
                    recvs.pop();
                }
                if (realcopy == size)
                {
                    return size;
                }
            }
        }
        return realcopy;
    }
    /**
     * 在pool 中处理接受
     **/
    void Connection::doRead(EventBase *evt)
    {
        Event<Connection>* event = static_cast<Event<Connection>*>( evt );
        while(true)
        {
            memset(buffer,0,EventBase::MAX_BUFFER_LEN);
            int leftLen = ::recv(socket,buffer,EventBase::MAX_BUFFER_LEN,0);
            allReadSize += leftLen;
            if (leftLen == -1)
            {
                // printf("---接受数据到头了 %d--\n",allReadSize);
                return;
            }
            if (leftLen == 0)
            {
                socket = -1;
                evt->eventType = ERR_EVT;
                return;
            }
          
            
            Record *record = new Record(buffer,leftLen);
            //recvs.push_back(record);
            recvs.write(record);
            
            if (leftLen <EventBase::MAX_BUFFER_LEN)
            {
                //              break;
            }
        }
    }
    
    /**
     * 在pool 中处理发送
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