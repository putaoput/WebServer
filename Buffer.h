//@Author Lin Tao
//@Email putaopu@qq.com
#pragma once

#include <cstring>   //perror
#include <iostream>
#include <unistd.h>  // write
#include <sys/uio.h> //readv
#include <vector> //readv
#include <atomic>
#include <assert.h>

class Buffer
{
public:
	Buffer(int _size = 1024);//buffer的默认大小是1k
	~Buffer() = default;

	size_t writable_bytes()const;//可以写的大小
	size_t readable_bytes()const;
	size_t prependable_bytes() const;

	const char* peek()const;
	void ensure_writable(size_t _len);
	void has_written(size_t _len);

	void retrieve(size_t _len);
	void retrieve_until(const char* _end);

	void retrieve_all();
	std::string retrieve_all_to_str();

	const char* begin_write_const() const;
	char* begin_write();

	void append(const std::string& _str);
	void append(const char* _str, size_t _len);
	void append(const void* _data, size_t _len);
	void append(const Buffer& _buff);
	
	
	ssize_t read_fd(int _fd, int* _Errno);
	ssize_t write_fd(int _fd, int* _Errno);

	
private:
	const char* begin_ptr()const;
	void make_space(size_t _len);


	std::vector<char> buffer;
	std::atomic<std::size_t> readPos; //atomic ,c++11里面提供的原子操作
	std::atomic<std::size_t> writePos;
	char* beginPtr;
};

