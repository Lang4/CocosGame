#include "connect.h"
#include "win32helper.h"
namespace mynet{
    /**
     * Client
     **/
    int Client::init(const char *ip,unsigned short port)
    {
        SOCKET socket = (SOCKET)WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);//::socket(AF_INET,SOCK_STREAM,0);
        if(socket == -1)
        {
            // TODO error
        }
        struct sockaddr_in addrServer;

        memset(&addrServer,0,sizeof(sockaddr_in));
        addrServer.sin_family = AF_INET;
        addrServer.sin_addr.s_addr = inet_addr(ip);
        addrServer.sin_port = htons(port);
        
        if(connect(socket,(const struct sockaddr *)&addrServer,sizeof(sockaddr)) != 0)
        {
            // TODO error
            socket = -1;
			printf("连接失败 %s %u\n",ip,port);
		}
		else
			printf("连接成功\n");
		return socket;
    }
    void Client::close()
    {
        
        socket = -1;
    }
    /**
     * Server
     */
	Server::Server(const char *ip,WORD port)
	{
		init(ip,port);
	}
	void Server::init(const char *ip,WORD port)
	{
		struct sockaddr_in ServerAddress;

		//  

		socket = (unsigned int)WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

		//  
		ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress));
		ServerAddress.sin_family = AF_INET;
		//ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);                      
		ServerAddress.sin_addr.s_addr = inet_addr(ip);         
		ServerAddress.sin_port = htons(port);                          

		//  
		if (SOCKET_ERROR == ::bind((SOCKET)socket, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress))) 
		{
			return;
		}
		else
		{
		}

		//  
		if (SOCKET_ERROR == listen((SOCKET)socket,SOMAXCONN))
		{
			return;
		}
		else
		{
		}
	}
	unsigned long Server::getPeerHandle(){
		return 0;
	}
};