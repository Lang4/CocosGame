#include "MyClient.h"
#include "mythread.h"
#include "zlib.h"
#pragma pack(1)
struct PACK_HEAD
{
	unsigned char Header[2];
	unsigned short Len;
	PACK_HEAD()
	{
		Header[0] = 0;
		Header[1] = 0;
	}
};
struct t_NullCmd{
	unsigned char cmd;
	unsigned char para;
	t_NullCmd()
	{
		para = cmd = 0;
	}
};
#pragma pack()

MyClient * myclient = NULL;
MyClient & MyClient::getMe()
{
	if (!myclient) myclient = new MyClient();
	return *myclient;
}

MyClient::~MyClient()
{
	printf("MyClient 我卸载了\n");
}
void ReadBody::callback(mynet::Connection *con,void *cmd,int size)
{
	unsigned char bigPackageBuffer[65535] = {'\0'};
	if (client)
	{
		if (head & 0x80000000) // 解密
		{
			if (size % 8 == 0)
			{
				enc.encdec((char*)cmd,size,false);
				//printf("解密：\n");
			}
			//printf("获取解密报文\n");
		}
		if (head &0x40000000)
		{
			// 解压
			uLong nUnZipLen = 65535;
			Byte * in = (Byte*)cmd;
			uncompress(bigPackageBuffer, &nUnZipLen, in, size);
			size = nUnZipLen;
			//printf("获取压缩报文\n");
		}
		else
		{
			memcpy(bigPackageBuffer,(char*)cmd,size);
		}
		//printf("获取头大小%u\n",size);
		if (client->decodeType == 1)
		{
			int offset = 0;
			do{
				unsigned char smallCmd[65535] = {'\0'};
				// 解析小报文
				int localhead = *(int*)&bigPackageBuffer[offset];
				offset+= sizeof(int);
				int bodysize = localhead & 0xffff;
				printf("获取小报文大小%u 当前偏移:%u\n",bodysize,offset);
				int primSize = bodysize;
				if (bodysize < 65535)
				{
					if (localhead & 0x80000000)
					{
						if (bodysize % 8 == 0)
						{
							enc.encdec(&bigPackageBuffer[offset],bodysize,false);
							//printf("解密小报文：\n");
						}
					}
					if (localhead &0x40000000)
					{
						// 解压
						uLong nUnZipLen = 65535;
						Byte * in = (Byte*)(&bigPackageBuffer[offset]);
						uncompress(smallCmd, &nUnZipLen, in, bodysize);
						bodysize = nUnZipLen;
						//printf("获取压缩小报文大小:%u\n",bodysize);
					}
					else
					{
						memcpy(smallCmd,&bigPackageBuffer[offset],bodysize);
					}
					offset += primSize;
					client->msgParse((char*)smallCmd,bodysize);
				}
				else
				{
					break;
				}
			}while(offset < size);
		}
		else
		{
			client->msgParse((char*)bigPackageBuffer,size);
		}
		client->doReadHead();
	}
}
void MyClient::doReadBody(int size)
{
	readBody->head = size;
	printf("原报文 头 %u decodeType:%u\n",size,decodeType);
	if (decodeType == 0)
	{
		printf("获取头大小%u\n",size - 4 );
		this->read(size - 4,readBody);
	}
	if (decodeType == 1)
	{
		printf("获取头大小%u\n",size & 0xffff);
		this->read((size & 0xffff),readBody);
	}
}
void ReadHead::callback (mynet::Connection *con,void *cmd,int size)
{
	if (client)
	{
		int len = *(int*) cmd;
		client->doReadBody(len);
	}
}
MyClient::MyClient()
{
	readHead = new ReadHead();
	readBody = new ReadBody();
	readHead->client = this;
	readBody->client = this;
}
#pragma pack(1)
struct stSucLoginCmd{
	char cmd;
	char para;
	int time;
	int id;
	int tempid;
	char ip[64];
	unsigned short port;
	char key[8];
};
#pragma pack()
void MyClient::msgParse(void *cmd,int size)
{
	t_NullCmd * nullCmd = (t_NullCmd*) cmd;
	printf("处理消息 %u %u \n",nullCmd->cmd,nullCmd->para);
	if (nullCmd->cmd == nullCmd->para && nullCmd->para == 0)
	{
		sendMsg(cmd,size);
		return;
	}
	if (nullCmd->cmd < parser.size() && nullCmd->para < parser[nullCmd->cmd].size())
	{
		CmdParser *parse = parser[nullCmd->cmd ][nullCmd->para];
		if (parse)
		{
			parse->func(cmd,size);
			//parse->object->release();
		}
		if (nullCmd->cmd == 1 && nullCmd->para == 4)
		{
			stSucLoginCmd * suc = (stSucLoginCmd*) cmd;
			printf("%d",suc->port);
		}
	}
}

void MyClient::get(unsigned char cmd,unsigned char para,Ref *object,std::function<void (void*,int) > func)
{
	if (cmd >= parser.size()) parser.resize(cmd+1);
	if (para >= parser[cmd].size()) parser[cmd].resize(para + 1);
	if (!parser[cmd][para])
	{
		CmdParser *parse = new CmdParser();
		parse->cmd = cmd;
		parse->para = para;
		parse->func = func;
		parse->object = object;
		object->retain();
		parser[cmd][para] = parse;
		printf("add handler :%u %u \n",cmd,para);
	}
}
               
void MyClient::sendMsg(void *cmd,int size)
{
	std::vector<char > buffer;
	
	buffer.resize(size + sizeof(int));
	memcpy(&buffer[sizeof(int)],cmd,size); // 写内容
	(*(unsigned int *)&buffer[0]) = size;
	sendCmd(&buffer[0],buffer.size());
}

void MyClient::setDesKey(const_DES_cblock *des_key)
{
	readBody->enc.setEncMethod(CEncrypt::ENCDEC_DES);
	readBody->enc.set_key_des(des_key);
}
MyNetNode * me = NULL;
MyNetNode * MyNetNode::getInstance()
{
	if (!me){
		me = MyNetNode::create();
		me->retain();
	}
	return me;
}
void MyNetNode::addTo(Node *node)
{
	if (!this->getParent())
	{
		node->addChild(this);
	}
}
bool MyNetNode::init()
{
	pool.init();
	doIOOver = true;
	if (!ithread) ithread = new std::thread(std::bind(&MyNetNode::doIO, this));
	this->scheduleUpdate();
	printf("MyNetNode::init\n");
	return true;
}
void MyNetNode::doIO()
{
	while (doIOOver)
	{
		mynet::EventBase *evt = pool.pullEvent();
		if (evt)
		{
			if (evt->isErr())
			{
				MyClient *conn = (MyClient*)evt->target;
				if (conn)
				{
					printf("删除网络连接%p\n", evt->target);
					conn->destroy();
				}
				continue;
			}
			if (evt->isOut())
			{
				mynet::Connection *conn = (mynet::Connection*) evt->target;
				if (conn)
				{
					conn->doSend(evt);
				}
				printf("out\n");
			}
			if (evt->isIn())
			{
				mynet::Connection *conn = (mynet::Connection*) evt->target;
				if (conn)
				{
					conn->doRead(evt);
				}
				else
				{
					printf("error!!!!!\n");
				}
				printf("in\n");
			}
		}
	}
	printf("over\n");
}
void MyNetNode::update(float dt)
{
	for (int index = 0; index < clients.size();index++)
	{
		if (clients[index])
		{
			clients[index]->updateRecvCmds();
		}
	}
}
void MyNetNode::addClient(int index,MyClient *client)
{
	if (index == -1)
	{
		clients.push_back(client);
		pool.bindEvent(client,mynet::IN_EVT | mynet::OUT_EVT);
		client->doReadHead(); // 开始读报文头
		return;
	}
	if (index >= clients.size()) clients.resize(index+1);
	if (NULL != clients[index])
	{
		delete client;
		return ;
	}
	clients[index] = client;
	pool.bindEvent(client,mynet::IN_EVT | mynet::OUT_EVT);
	client->doReadHead(); // 开始读报文头
	printf("增加一个连接%p\n",client);
}
MyClient * MyNetNode::getClient(int index)
{
	if (index < clients.size())
	{
		return clients[index];
	}
	return NULL;
}

MyNetNode::~MyNetNode()
{
	doIOOver = false;
	if (ithread)
	{
		ithread->join();
	}
	for (int index = 0; index < clients.size();index++)
	{
		if (clients[index])
		{
			delete clients[index];
		}
	}
	clients.clear();
	printf("清除网络连接\n");
}