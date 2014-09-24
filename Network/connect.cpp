#include "connect.h"
#include "win32helper.h"
#include "memcheck.h"
namespace mynet{
	void Connection::destroy()
	{
		Target::destroy();
		closesocket(socket);
	}
	Connection:: Connection()
	{
		directDealCmd = true;
		memset(nowCmd,0,65535);
	}
	 unsigned long Connection::getHandle(){
		 return (unsigned long)socket;
	 }
	 void Connection::setHandle(unsigned long socket){
		 this->socket = socket;
	 }
	 
	 /**
	  *  
	  */
	 void Connection::sendCmd(void *cmd,unsigned int len)
	 {
		Record *record = new Record(cmd,len);
		sends.write(record);
		if (outEvt)
			doSend(outEvt,false);
	 }
	
	
	void Connection::updateRecvCmds()
	{
		ReadCmd *cmd = NULL;
		if (!cmds.empty())
		{
			if (cmds.readOnly(cmd))
			{
				Record *record = NULL;
				while (!recvs.empty())
				{
					if (recvs.readOnly(record))
					{
						cmd->offset += record->recv(nowCmd+ cmd->offset,cmd->size - cmd->offset);
						if (record->empty())
						{
							delete record;
							recvs.pop();
						}
						if (cmd->offset == cmd->size)
						{
							cmds.pop();
							// 处理回调
							cmd->func->callback(this,nowCmd,cmd->size);
							memset(nowCmd,0,65535);
							delete cmd;
							break;
						}
					}else break;
				}
			}
		}
	}
};