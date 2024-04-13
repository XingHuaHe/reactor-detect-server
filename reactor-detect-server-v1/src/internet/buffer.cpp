#include "buffer.h"


Buffer::Buffer(uint16_t sep): _sep(sep)
{

}

Buffer::~Buffer()
{

}

/*	������׷�ӵ�buf_��	*/
void Buffer::append(const char* data, size_t size)
{
	_buf.append(data, size);
}

/*	������׷�ӵ�buf_�У����ӱ���ͷ��	*/
void Buffer::appendwithseq(const char* data, size_t size)
{
	if (_sep == 0)	// û�зָ���
	{
		_buf.append(data, size);
	}
	else if (_sep == 1)
	{
		// ���ĳ���+��������
		_buf.append((char*)&size, 4);
		_buf.append(data, size);
	}
	else if (_sep == 2)
	{
		/*
		* 
		* ��������Э��
		* 
		*/
	}
}

/*	����buf_�Ĵ�С		*/
size_t Buffer::size()
{
	return _buf.size();
}

/*	����buf_���׵�ַ	*/
const char* Buffer::data()
{
	return _buf.data();
}

/*	���buf_			*/
void Buffer::clear()
{
	return _buf.clear();
}

/*	��buf_��pos��ʼ��ɾ��nn���ֽڣ�pos��0��ʼ	*/
void Buffer::erase(size_t pos, size_t n)
{
	_buf.erase(pos, n);
}

/*	��buf_�в�ֳ�һ�����ģ������s�У����buf_��û�б��ģ�����false	*/
bool Buffer::pickmessage(std::string& s)
{
	if (_buf.size() == 0)
	{
		return false;
	}

	if (_sep == 0)
	{
		s = _buf;
		_buf.clear();
	}
	else if (_sep == 1)
	{
		int len;
		memcpy(&len, _buf.data(), 4);	// ȡ 4 ���ֽڣ�Э��ǰ 4 �ֽڴ����ĳ��ȣ�
		if (_buf.size() < (len + 4))	// ���inputbuffer�е�������С�ڱ���ͷ����˵��inputbuffer�еı������ݲ�����
		{
			return false;
		}
		s = _buf.substr(4, len);		// ��inputbuffer�л�ȡһ������
		_buf.erase(0, 4 + len);			// ��inputbuffer��ɾ���ղ��ѻ�ȡ�ı���
	}

	return true;
}