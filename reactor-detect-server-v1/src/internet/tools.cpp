#include "tools.h"


// ���÷������� IO
void setnonblocking(int fd)
{
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}