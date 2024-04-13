#pragma once
#include <string>
#include <cstring>


class Buffer
{
public:
	Buffer(uint16_t sep = 1);
	~Buffer();

	/**/

	/*	������׷�ӵ�buf_��	*/
	void append(const char* data, size_t size);
	/*	������׷�ӵ�buf_�У����ӱ���ͷ��	*/
	void appendwithseq(const char* data, size_t size);
	/*	��buf_��pos��ʼ��ɾ��nn���ֽڣ�pos��0��ʼ	*/
	void erase(size_t pos, size_t n);
	/*	����buf_�Ĵ�С		*/
	size_t size();
	/*	����buf_���׵�ַ	*/
	const char* data();
	/*	���buf_			*/
	void clear();
	/*	��buf_�в�ֳ�һ�����ģ������s�У����buf_��û�б��ģ�����false	*/
	bool pickmessage(std::string& s);

private:
	const uint16_t _sep;	// ���ĵķָ�����0-�޷ָ���(�̶����ȡ���Ƶ����)��1-���ֽڵı�ͷ��2-"\r\n\r\n"�ָ�����httpЭ�飩
	std::string _buf;		// ���ڴ������
};

