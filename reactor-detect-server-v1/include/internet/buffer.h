#pragma once
#include <string>
#include <cstring>


class Buffer
{
public:
	Buffer(uint16_t sep = 1);
	~Buffer();

	/**/

	/*	把数据追加到buf_中	*/
	void append(const char* data, size_t size);
	/*	把数据追加到buf_中，附加报文头部	*/
	void appendwithseq(const char* data, size_t size);
	/*	从buf_的pos开始，删除nn个字节，pos从0开始	*/
	void erase(size_t pos, size_t n);
	/*	返回buf_的大小		*/
	size_t size();
	/*	返回buf_的首地址	*/
	const char* data();
	/*	清空buf_			*/
	void clear();
	/*	从buf_中拆分出一个报文，存放在s中，如果buf_中没有报文，返回false	*/
	bool pickmessage(std::string& s);

private:
	const uint16_t _sep;	// 报文的分隔符：0-无分隔符(固定长度、视频会议)；1-四字节的报头；2-"\r\n\r\n"分隔符（http协议）
	std::string _buf;		// 用于存放数据
};

