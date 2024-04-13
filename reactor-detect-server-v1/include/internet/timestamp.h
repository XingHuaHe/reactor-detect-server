#pragma once
#include <time.h>
#include <iostream>


class Timestamp
{
public:
	/*  �õ�ǰʱ���ʼ������				*/
	Timestamp();
	/*  ��һ��������ʾ��ʱ���ʼ������		*/
	Timestamp(int64_t secsinceepoch);
	~Timestamp();

	/*	���ص�ǰʱ���Timestamp����			*/
	static Timestamp now();
	/*	����������ʾ��ʱ��					*/
	time_t toint() const;
	/*	�����ַ�����ʾ��ʱ�䣬��ʽ��yyyy-mm-dd hh24:mi:ss	*/
	std::string tostring() const;

private:
	time_t _secsinceepoch;  // ������ʾ��ʱ�䣨��1970����������ȥ��������
};
