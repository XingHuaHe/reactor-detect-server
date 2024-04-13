#include <stdlib.h>
#include "detectserver.h"
#include <signal.h>


class DetectServer;
DetectServer* detectserver;

void Stop(int sig)
{
	//printf("sig=%d\n", sig);
	detectserver->Stop(); // 调用 DetectServer::Stop()

	delete detectserver;
	exit(0);
}


int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("usage: ./reactor port\n");
		printf("example: ./reactor 5001\n\n");
		return 0;
	}
	printf("status: [OK] reactor server running.\n\n");

	signal(SIGTERM, Stop);	// signal 2
	signal(SIGINT, Stop);	// signal 15

	detectserver = new DetectServer(atoi(argv[1]), "/root/xinghua.he/yolov7-deploy/weights/yolov7-tiny.torchscript.pt", 3, 0);
	detectserver->Start();

	return 0;
}