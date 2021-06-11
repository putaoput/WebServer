//@Author Lin Tao
//@Email putaopu@qq.com

#include "Buffer.h"

//-------------------------Buffer的实现-------------------------
Buffer::Buffer(int _size)
	:buffer(_size),
	readPos(0),
	writePos(0),
	beginPtr(&*buffer.begin())
{}

size_t Buffer::readable_bytes() const {
	return writePos - readPos;
}
size_t Buffer::writable_bytes() const {
	return buffer.size() - writePos;
}

size_t Buffer::prependable_bytes() const {
	return readPos;
}

const char* Buffer::peek()const {
	return beginPtr + readPos;//返回当前读取字符的指针
}

void Buffer::retrieve(size_t _len)//重新得到
{
	assert(_len <= readable_bytes());
	readPos += _len;
}

void Buffer::retrieve_until(const char* _end) {
	assert(peek() <= _end);
	retrieve(_end - peek());//直接移到_end 所在位置
}

void Buffer::retrieve_all() {//直接重置全部
	bzero(&buffer[0], buffer.size());
	readPos = 0;
	writePos = 0;

}

std::string Buffer::retrieve_all_to_str() {
	std::string str(peek(), readable_bytes());//返回所有已经写入但是还没有读入的字符，然后清空
	retrieve_all();
	return str;
}

const char* Buffer::begin_write_const()const {
	return beginPtr + writePos;
}

char* Buffer::begin_write() {
	return beginPtr + writePos;
}

void Buffer::append(const std::string& _str) {
	append(_str.data(), _str.size());
		//string的data()方法返回的是char数组，而c_str()会在数据的末尾加上"\0结束符"
	//size()是兼容stl，length和size没有区别
}

void Buffer::append(const void* _data, size_t _len) {
	assert(_data);//保证不为空
	append(static_cast<const char*>(_data), _len);

}

void Buffer::append(const char* _str, size_t _len) {
	assert(_str);
	ensure_writable(_len);
	std::copy(_str, _str + _len, begin_write());//这个是标准库函数
	has_written(_len);
}


void Buffer::append(const Buffer& _buff) {
	append(_buff.peek(), _buff.readable_bytes());//把所有已经写入_buffer但是没有被读出的数据写到新的buff里
}

void Buffer::ensure_writable(size_t _len) {
	if (writable_bytes() < _len) {
		make_space(_len);
	}
	assert(writable_bytes() >= _len);
}

void Buffer::has_written(size_t _len){
	writePos += _len;
}
//ssize_t:有符号整型
ssize_t Buffer::read_fd(int _fd, int* _saveErrno) {
	char buff[65535];
	struct iovec iov[2];
	const size_t writable = writable_bytes();
	iov[0].iov_base = beginPtr + writePos;
	iov[0].iov_len = writable;
	iov[1].iov_base = buff;
	iov[1].iov_len = sizeof(buff);

	const ssize_t len = readv(_fd, iov, 2);
	if (len < 0) {
		*_saveErrno = errno;
	}
	else if (static_cast<size_t>(len) <= writable) {
		writePos += len;
	}
	else {
		writePos += len;
		append(buff, len - writable);
	}

	return len;
}

ssize_t Buffer::write_fd(int _fd, int* _saveErrno) {
	size_t readSize = readable_bytes();
	ssize_t len = write(_fd, peek(), readSize);
	if (len < 0) {
		*_saveErrno = errno;
		return len;
	}

	readPos += len;
	return len;
}

void Buffer::make_space(size_t _len) {
	if (writable_bytes() + prependable_bytes() < _len) {
		//除去已经写入但是尚未读取的字节，求出剩余空间
		buffer.resize(writePos + _len + 1);
		//resize()分配了内存，reserve()设置了容量，没有真正分配内存
	}
	else {
		size_t readable = readable_bytes();
		std::copy(beginPtr + readPos, beginPtr + writePos, beginPtr);
		//把已经写入但是尚未读取的直接拷贝到buffer开头处，然后将重置readPos和writePos
		readPos = 0;
		writePos = readPos + readable;
	}
}


/*
#include <sys/uio.h>

struct iovec {
	ptr_t iov_base; // Starting address
size_t iov_len;  //Length in bytes 
};
struct iovec定义了一个向量元素。通常，这个结构用作一个多元素的数组。
	对于每一个传输的元素，指针成员iov_base指向一个缓冲区，
	这个缓冲区是存放的是readv所接收的数据或是writev将要发送的数据。
	成员iov_len在各种情况下分别确定了接收的最大长度以及实际写入的长度。
*/

/*
为什么引出readv()和writev()
因为使用read()将数据读到不连续的内存、使用write()将不连续的内存发送出去，要经过多次的调用read、write
如果要从文件中读一片连续的数据至进程的不同区域，有两种方案：①使用read()一次将它们读至一个较大的缓冲区中，然后将它们分成若干部分复制到不同的区域； ②调用read()若干次分批将它们读至不同区域。
同样，如果想将程序中不同区域的数据块连续地写至文件，也必须进行类似的处理。

怎么解决多次系统调用+拷贝带来的开销呢？
UNIX提供了另外两个函数—readv()和writev()，它们只需一次系统调用就可以实现在文件和进程的多个缓冲区之间传送数据，免除了多次系统调用或复制数据的开销。
readv/writev
在一次函数调用中：
① writev以顺序iov[0]、iov[1]至iov[iovcnt-1]从各缓冲区中聚集输出数据到fd
② readv则将从fd读入的数据按同样的顺序散布到各缓冲区中，readv总是先填满一个缓冲区，然后再填下一个

#include <sys/uio.h>
ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
————————————————
*/