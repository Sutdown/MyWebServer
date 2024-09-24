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
    /*初始化日志，日志保存路径，文件后缀，最大容量*/
    void init(int level, const char* path = "./log", 
                const char* suffix =".log",
                int maxQueueCapacity = 1024);

    /*懒汉模式，局部静态变量法*/
    static Log* Instance();
    /*异步日志的写线程函数*/
    static void FlushLogThread();

    /*写日志*/
    void write(int level, const char *format,...);
    void flush();

    int GetLevel();
    void SetLevel(int level);
    bool IsOpen() { return isOpen_; }
    
private:
    Log();
    /*添加日志等级*/
    void AppendLogLevelTitle_(int level);
    virtual ~Log();
    /*异步写日志*/
    void AsyncWrite_();

private:
    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char* path_; // 路径
    const char* suffix_; // 后缀

    int MAX_LINES_; // 最大日志行数

    int lineCount_; // 日志行数
    int toDay_; // 按照当天日期区分文件
    bool isOpen_;
 
    Buffer buff_; // 输出缓冲区
    int level_; // 日志等级
    bool isAsync_; // 是否异步

    FILE* fp_; // 打开log的文件指针
    std::unique_ptr<BlockDeque<std::string>> deque_; // 阻塞队列
    std::unique_ptr<std::thread> writeThread_; // 写线程的指针
    std::mutex mtx_; // 同步日志的锁
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
* 四个宏定义，用于不同类型日志输出，也就是外部使用日志接口
* ...表示可变参数，_VA_ARGS_就是将...的值复制到这里
* ##的作用在于可变参数为0时，去掉前面的逗号
*/
#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif //LOG_H