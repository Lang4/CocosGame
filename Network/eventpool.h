#pragma once
namespace mynet{
	class Target;
	enum EVENT_TYPE{
		ACCEPT_EVT = 1 << 0,  
		OUT_EVT = 1 << 1, 
		IN_EVT =1 << 2,  
		ERR_EVT = 1 << 3, 
	};

	
	/**
	 * EevntBase	 */
	class EventBase{
	public:
		Target * target; // Target
		int eventType;     //  
		unsigned dataLen; // 
		virtual void deal() = 0;
		
		EventBase(Target * target):target(target)
		{
			dataLen = 0;
			eventType = 0;
		}
		virtual unsigned int getPeerHandle() = 0;
		
		bool isIn()
		{
			return eventType &IN_EVT;
		}
		bool isOut()
		{
			return eventType & OUT_EVT;
		}
		bool isErr()
		{
			return eventType & ERR_EVT;
		}
		bool isAccept()
		{
			return eventType & ACCEPT_EVT;
		}
		virtual void redo() = 0;
		static const unsigned int MAX_BUFFER_LEN = 9000;
	};
    class Pool{
    public:
    	virtual	void bindEvent(Target *target,int eventType) = 0;
    	virtual EventBase* pullEvent() = 0;
    	virtual ~Pool(){}
    };
	/**
	 * EventPool 
	 **/
    class EventPool {
    public:
        EventPool()
        {
           pool = 0;
        }
		bool init();
		Pool* pool;
        void bindEvent(Target *target,int eventType);
		EventBase* pullEvent();
		~EventPool()
		{
			if (pool)
				delete pool;
		}
    };
};