/*
*
* �ȷ��ͣ������
* ��һ�������Ͷ����������Ϣ
* �ڶ��������ն�����ش���Ϣ
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



	// ��1������
	for (int i = 0; i < 10; i++)
	{
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "This is the %dth message.", i);

		char tmpbuf[1024];					// ��ʱ��buffer������ͷ��+��������
		memset(tmpbuf, 0, sizeof(tmpbuf));
		int len = strlen(buf);				// ���㱨�ĵĴ�С
		memcpy(tmpbuf, &len, 4);			// ƴ�ӱ���ͷ��
		memcpy(tmpbuf + 4, buf, len);		// ƴ�ӱ�������

		send(servsock, tmpbuf, len + 4, 0);	// �������ķ��͸������
	}

	//  ��2������
	for (int i = 0; i < 10; i++)
	{
		int len;
		recv(servsock, &len, 4, 0);			// �ȶ�ȡ4�ֽڵı���ͷ��

		memset(buf, 0, sizeof(buf));
		recv(servsock, buf, len, 0);			// ��ȡ��������

		printf("recv:%s\n", buf);
	}

	return 0;
}