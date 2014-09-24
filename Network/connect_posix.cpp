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
	void __setnonblock(int socket)
	{
		int opts;
		opts=fcntl(socket,F_GETFL);
		if(opts<0)
		{
			return;
		}
		opts = opts|O_NONBLOCK;
		if(fcntl(socket,F_SETFL,opts)<0)
		{
			return;
		} 
	}
   void Client::init(const char *ip,unsigned short port)
	{
		socket = ::socket(AF_INET,SOCK_STREAM,0);
		if(socket == -1)
		{
			// TODO error
		}
		struct sockaddr_in addrServer;
		memset(&addrServer,0,sizeof(sockaddr_in));
		addrServer.sin_family = AF_INET;
		addrServer.sin_addr.s_addr = inet_addr(ip);
		addrServer.sin_port = htons(port);
		this->peerIp = ip;
		__setnonblock(socket);
		if(connect(socket,(const struct sockaddr *)&addrServer,sizeof(sockaddr)) != 0)
		{
			//printf("connect error! -->%s:%u\n",ip,port);	
		}

	}
	
	bool Client::destroy()
	{
		if (socket != -1)
			::close(socket);
		socket = -1;
		return false;	 
	}
	void Server::init(const char *ip,unsigned short port)
	{
		struct sockaddr_in ServerAddress;

		socket = ::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		__setnonblock(socket);
		int reuse = 1;
		setsockopt(socket,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
	
		memset((char *)&ServerAddress,0, sizeof(ServerAddress));
		ServerAddress.sin_family = AF_INET;
		ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr(ip);         
		ServerAddress.sin_port = htons(port);                          

		bind(socket, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)); 
		
		listen(socket,4026);
	}

	int Server::getPeerHandle(){
		socklen_t clilen = 1024;
		struct sockaddr_in addr;
		int con = ::accept(socket,(struct sockaddr*)(&addr),&clilen);	
		printf("accept:ip:%sport:%u",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
		__setnonblock(con);
		return con;
	}	
	#if (0)
	int Server::getPeerHandle(Connection *conn)
	{
		socklen_t clilen = 1024;
		struct sockaddr_in addr;
		int con = ::accept(socket,(struct sockaddr*)(&addr),&clilen);	
		printf("accept:ip:%sport:%u",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
		setnonblock(con);
		conn->peerIp = inet_ntoa(addr.sin_addr);
		conn->peerPort = ntohs(addr.sin_port);
		conn->setHandle(con);
	}
	#endif
};