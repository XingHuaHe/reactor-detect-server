/*
* 
* 控制台输入信息，并将信息发送到服务器。退出输入‘q’。
* 
*/

#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>



int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		printf("usage:. / client ip port\n");
		printf("example:./client www.hexinghuacloud.com 5001\n\n");
		return -1;
	}
	
	char buf[1024];

	// socket.
	int servsock;
	if ((servsock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket() failed.\n");
		return -1;
	}

	// socket address.
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));
	struct hostent* h;
	if ((h = gethostbyname(argv[1])) == nullptr)
	{
		printf("gethostbyname failed.\n");
		close(servsock);
		return -1;
	}
	memcpy(&servaddr.sin_addr, h->h_addr, h->h_length);
	
	// connect.
	if (connect(servsock, (sockaddr*)&servaddr, sizeof(servaddr)) != 0)
	{
		printf("connect(%s:%s) failed.\n", argv[1], argv[2]);
		close(servsock);
		return -1;
	}
	printf("connect ok.\n");

	// send.
	while (true)
	{
		memset(buf, 0, sizeof(buf));
		printf("please input:"); scanf("%s", buf);

		if (strcmp(buf, "q") == 0)
		{
			break;
		}

		if (send(servsock, buf, strlen(buf), 0) <= 0)
		{
			printf("write() failed.\n");
			close(servsock);
			return -1;
		}

		memset(buf, 0, sizeof(buf));
		if (recv(servsock, buf, sizeof(buf), 0) <= 0)
		{
			printf("read() failed.\n");
			close(servsock);
			return -1;
		}
		printf("recv:%s\n", buf);
	}
	close(servsock);

	return 0;
}