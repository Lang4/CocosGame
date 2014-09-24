#pragma once
#include "mynet.h"
#include "encdec.h"
#include "cocos2d.h"

USING_NS_CC;
class MyClient;

class ReadBody:public mynet::IRead{
public:
	int head;
	CEncrypt enc;
	MyClient *client;
	void callback (mynet::Connection *con,void *cmd,int size);
};
class ReadHead:public mynet::IRead{
public:
	MyClient *client;
	void callback(mynet::Connection *con,void *cmd,int size);
};
class CmdParser{
public:
	unsigned char cmd;
	unsigned char para;
	Ref *object;
	std::function<void (void*,int) > func;
	CmdParser()
	{
		cmd = para = 0;
		object = NULL;
	}
};
class MyClient:public mynet::Client{
public:
	MyClient();
	void init(const char *ip,unsigned short port)
	{
		decodeType = 0;
		socket = mynet::Client::init(ip,port);
	}
	void doReadHead()
	{
		this->read(4,readHead);
	}
	int decodeType ;
	void doReadBody(int size);
	ReadBody *readBody;
	ReadHead *readHead;

	void msgParse(void *cmd,int size);

	void get(unsigned char cmd, unsigned char para, Ref *object, std::function<void(void*, int) >);

	std::vector<std::vector<CmdParser *> > parser;

	static MyClient & getMe();
	void sendMsg(void *cmd,int size);

	~MyClient();

	void setDesKey(const_DES_cblock *des_key);
};

class MyNetNode:public Node{
public:
	MyNetNode(){ ithread = NULL; }
	static MyNetNode * getInstance();
	void addTo(Node *node);
	CREATE_FUNC(MyNetNode);
	bool init();
	mynet::EventPool pool;
	void update(float dt);
	void addClient(int index,MyClient *client);
	MyClient * getClient(int index);
	std::vector<MyClient *> clients;
	virtual ~MyNetNode();
	void doIO();
	bool doIOOver;
	std::thread *ithread;
};

#define theClient MyClient::getMe()

#define BIND_MSG(cmd,para,target,function) \
do{theClient.get(cmd, para, this, std::bind(&target::function, (target*) this,std::placeholders::_1,std::placeholders::_2));\
}while(0)