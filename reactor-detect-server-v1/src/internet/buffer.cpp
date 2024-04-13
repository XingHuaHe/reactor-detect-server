#include "buffer.h"


Buffer::Buffer(uint16_t sep): _sep(sep)
{

}

Buffer::~Buffer()
{

}

/*	把数据追加到buf_中	*/
void Buffer::append(const char* data, size_t size)
{
	_buf.append(data, size);
}

/*	把数据追加到buf_中，附加报文头部	*/
void Buffer::appendwithseq(const char* data, size_t size)
{
	if (_sep == 0)	// 没有分隔符
	{
		_buf.append(data, size);
	}
	else if (_sep == 1)
	{
		// 报文长度+报文内容
		_buf.append((char*)&size, 4);
		_buf.append(data, size);
	}
	else if (_sep == 2)
	{
		/*
		* 
		* 自行增加协议
		* 
		*/
	}
}

/*	返回buf_的大小		*/
size_t Buffer::size()
{
	return _buf.size();
}

/*	返回buf_的首地址	*/
const char* Buffer::data()
{
	return _buf.data();
}

/*	清空buf_			*/
void Buffer::clear()
{
	return _buf.clear();
}

/*	从buf_的pos开始，删除nn个字节，pos从0开始	*/
void Buffer::erase(size_t pos, size_t n)
{
	_buf.erase(pos, n);
}

/*	从buf_中拆分出一个报文，存放在s中，如果buf_中没有报文，返回false	*/
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
		memcpy(&len, _buf.data(), 4);	// 取 4 个字节（协议前 4 字节代表报文长度）
		if (_buf.size() < (len + 4))	// 如果inputbuffer中的数据量小于报文头部，说明inputbuffer中的报文内容不完整
		{
			return false;
		}
		s = _buf.substr(4, len);		// 从inputbuffer中获取一个报文
		_buf.erase(0, 4 + len);			// 从inputbuffer中删除刚才已获取的报文
	}

	return true;
}