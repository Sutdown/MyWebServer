#ifndef BUFFER_H
#define BUFFER_H
#include <cstring>   //perror
#include <iostream>
#include <unistd.h>  // write
#include <sys/uio.h> //readv
#include <vector> //readv
#include <atomic>
#include <assert.h>
class Buffer {
public:
    /*构造和析构*/
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    /*buffer中可写，可读，可预留空间*/
    size_t WritableBytes() const;       
    size_t ReadableBytes() const ;
    size_t PrependableBytes() const;

    /*可读的第一个地址，确保可读长度，移动可写长度*/
    const char* Peek() const;
    void EnsureWriteable(size_t len);
    void HasWritten(size_t len);

    /*移动可读长度，读到末尾*/
    void Retrieve(size_t len);
    void RetrieveUntil(const char* end);

    /*读完所有，读写下表全部归零*/
    void RetrieveAll() ;
    std::string RetrieveAllToStr();

    /*写指针位置*/
    const char* BeginWriteConst() const;
    char* BeginWrite();

    /*对额外的stack进行操作，添加str到缓冲区*/
    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

    /*读写的交互接口*/
    ssize_t ReadFd(int fd, int* Errno);
    ssize_t WriteFd(int fd, int* Errno);

private:
    char* BeginPtr_();
    const char* BeginPtr_() const;
    void MakeSpace_(size_t len); // 扩展空间

    /*atomic是一种原子类型，保证多线程的情况下，安全高性能的执行程序更新变量*/
    std::vector<char> buffer_; // 存储实体
    std::atomic<std::size_t> readPos_; // 读缓冲区
    std::atomic<std::size_t> writePos_; // 写缓冲区
};

#endif //BUFFER_H