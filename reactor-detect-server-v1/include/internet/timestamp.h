#pragma once
#include <time.h>
#include <iostream>


class Timestamp
{
public:
	/*  用当前时间初始化对象				*/
	Timestamp();
	/*  用一个整数表示的时间初始化对象		*/
	Timestamp(int64_t secsinceepoch);
	~Timestamp();

	/*	返回当前时间的Timestamp对象			*/
	static Timestamp now();
	/*	返回整数表示的时间					*/
	time_t toint() const;
	/*	返回字符串表示的时间，格式：yyyy-mm-dd hh24:mi:ss	*/
	std::string tostring() const;

private:
	time_t _secsinceepoch;  // 整数表示的时间（从1970到现在已逝去的秒数）
};
