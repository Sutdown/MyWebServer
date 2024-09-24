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
    /*���������*/
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    /*buffer�п�д���ɶ�����Ԥ���ռ�*/
    size_t WritableBytes() const;       
    size_t ReadableBytes() const ;
    size_t PrependableBytes() const;

    /*�ɶ��ĵ�һ����ַ��ȷ���ɶ����ȣ��ƶ���д����*/
    const char* Peek() const;
    void EnsureWriteable(size_t len);
    void HasWritten(size_t len);

    /*�ƶ��ɶ����ȣ�����ĩβ*/
    void Retrieve(size_t len);
    void RetrieveUntil(const char* end);

    /*�������У���д�±�ȫ������*/
    void RetrieveAll() ;
    std::string RetrieveAllToStr();

    /*дָ��λ��*/
    const char* BeginWriteConst() const;
    char* BeginWrite();

    /*�Զ����stack���в��������str��������*/
    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

    /*��д�Ľ����ӿ�*/
    ssize_t ReadFd(int fd, int* Errno);
    ssize_t WriteFd(int fd, int* Errno);

private:
    char* BeginPtr_();
    const char* BeginPtr_() const;
    void MakeSpace_(size_t len); // ��չ�ռ�

    /*atomic��һ��ԭ�����ͣ���֤���̵߳�����£���ȫ�����ܵ�ִ�г�����±���*/
    std::vector<char> buffer_; // �洢ʵ��
    std::atomic<std::size_t> readPos_; // ��������
    std::atomic<std::size_t> writePos_; // д������
};

#endif //BUFFER_H