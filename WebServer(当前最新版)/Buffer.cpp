//@Author Lin Tao
//@Email putaopu@qq.com

#include "Buffer.h"

//-------------------------Buffer��ʵ��-------------------------
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
	return beginPtr + readPos;//���ص�ǰ��ȡ�ַ���ָ��
}

void Buffer::retrieve(size_t _len)//���µõ�
{
	assert(_len <= readable_bytes());
	readPos += _len;
}

void Buffer::retrieve_until(const char* _end) {
	assert(peek() <= _end);
	retrieve(_end - peek());//ֱ���Ƶ�_end ����λ��
}

void Buffer::retrieve_all() {//ֱ������ȫ��
	bzero(&buffer[0], buffer.size());
	readPos = 0;
	writePos = 0;

}

std::string Buffer::retrieve_all_to_str() {
	std::string str(peek(), readable_bytes());//���������Ѿ�д�뵫�ǻ�û�ж�����ַ���Ȼ�����
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
		//string��data()�������ص���char���飬��c_str()�������ݵ�ĩβ����"\0������"
	//size()�Ǽ���stl��length��sizeû������
}

void Buffer::append(const void* _data, size_t _len) {
	assert(_data);//��֤��Ϊ��
	append(static_cast<const char*>(_data), _len);

}

void Buffer::append(const char* _str, size_t _len) {
	assert(_str);
	ensure_writable(_len);
	std::copy(_str, _str + _len, begin_write());//����Ǳ�׼�⺯��
	has_written(_len);
}


void Buffer::append(const Buffer& _buff) {
	append(_buff.peek(), _buff.readable_bytes());//�������Ѿ�д��_buffer����û�б�����������д���µ�buff��
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
//ssize_t:�з�������
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
		//��ȥ�Ѿ�д�뵫����δ��ȡ���ֽڣ����ʣ��ռ�
		buffer.resize(writePos + _len + 1);
		//resize()�������ڴ棬reserve()������������û�����������ڴ�
	}
	else {
		size_t readable = readable_bytes();
		std::copy(beginPtr + readPos, beginPtr + writePos, beginPtr);
		//���Ѿ�д�뵫����δ��ȡ��ֱ�ӿ�����buffer��ͷ����Ȼ������readPos��writePos
		readPos = 0;
		writePos = readPos + readable;
	}
}


