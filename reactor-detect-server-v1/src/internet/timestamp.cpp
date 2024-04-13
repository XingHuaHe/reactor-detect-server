#include "timestamp.h"


Timestamp::Timestamp()
{
	_secsinceepoch = time(0);   // ȡϵͳ��ǰʱ��
}

/*  ��һ��������ʾ��ʱ���ʼ������		*/
Timestamp::Timestamp(int64_t secsinceepoch): _secsinceepoch(secsinceepoch)
{

}

Timestamp::~Timestamp()
{
}

/*	���ص�ǰʱ���Timestamp����			*/
Timestamp Timestamp::now()
{
	return Timestamp();   // ���ص�ǰʱ��
}

/*	����������ʾ��ʱ��					*/
time_t Timestamp::toint() const
{
	return _secsinceepoch;
}

/*	�����ַ�����ʾ��ʱ�䣬��ʽ��yyyy-mm-dd hh24:mi:ss	*/
std::string Timestamp::tostring() const
{
	char buf[32] = { 0 };
	tm* tm_time = localtime(&_secsinceepoch);
	snprintf(buf, 20, "%4d-%02d-%02d %02d:%02d:%02d",
		tm_time->tm_year + 1900,
		tm_time->tm_mon + 1,
		tm_time->tm_mday,
		tm_time->tm_hour,
		tm_time->tm_min,
		tm_time->tm_sec);
	return buf;
}