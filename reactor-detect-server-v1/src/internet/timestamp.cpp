#include "timestamp.h"


Timestamp::Timestamp()
{
	_secsinceepoch = time(0);   // 取系统当前时间
}

/*  用一个整数表示的时间初始化对象		*/
Timestamp::Timestamp(int64_t secsinceepoch): _secsinceepoch(secsinceepoch)
{

}

Timestamp::~Timestamp()
{
}

/*	返回当前时间的Timestamp对象			*/
Timestamp Timestamp::now()
{
	return Timestamp();   // 返回当前时间
}

/*	返回整数表示的时间					*/
time_t Timestamp::toint() const
{
	return _secsinceepoch;
}

/*	返回字符串表示的时间，格式：yyyy-mm-dd hh24:mi:ss	*/
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