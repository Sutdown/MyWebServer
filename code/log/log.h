#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>           // vastart va_end
#include <assert.h>
#include <sys/stat.h>         //mkdir
#include "blockqueue.h"
#include "../buffer/buffer.h"

class Log {
public:
    /*��ʼ����־����־����·�����ļ���׺���������*/
    void init(int level, const char* path = "./log", 
                const char* suffix =".log",
                int maxQueueCapacity = 1024);

    /*����ģʽ���ֲ���̬������*/
    static Log* Instance();
    /*�첽��־��д�̺߳���*/
    static void FlushLogThread();

    /*д��־*/
    void write(int level, const char *format,...);
    void flush();

    int GetLevel();
    void SetLevel(int level);
    bool IsOpen() { return isOpen_; }
    
private:
    Log();
    /*�����־�ȼ�*/
    void AppendLogLevelTitle_(int level);
    virtual ~Log();
    /*�첽д��־*/
    void AsyncWrite_();

private:
    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char* path_; // ·��
    const char* suffix_; // ��׺

    int MAX_LINES_; // �����־����

    int lineCount_; // ��־����
    int toDay_; // ���յ������������ļ�
    bool isOpen_;
 
    Buffer buff_; // ���������
    int level_; // ��־�ȼ�
    bool isAsync_; // �Ƿ��첽

    FILE* fp_; // ��log���ļ�ָ��
    std::unique_ptr<BlockDeque<std::string>> deque_; // ��������
    std::unique_ptr<std::thread> writeThread_; // д�̵߳�ָ��
    std::mutex mtx_; // ͬ����־����
};

#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::Instance();\
        if (log->IsOpen() && log->GetLevel() <= level) {\
            log->write(level, format, ##__VA_ARGS__); \
            log->flush();\
        }\
    } while(0);

/*
* �ĸ��궨�壬���ڲ�ͬ������־�����Ҳ�����ⲿʹ����־�ӿ�
* ...��ʾ�ɱ������_VA_ARGS_���ǽ�...��ֵ���Ƶ�����
* ##���������ڿɱ����Ϊ0ʱ��ȥ��ǰ��Ķ���
*/
#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif //LOG_H