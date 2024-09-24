#include "log.h"

using namespace std;

Log::Log() {
    lineCount_ = 0;
    isAsync_ = false;
    writeThread_ = nullptr;
    deque_ = nullptr;
    toDay_ = 0;
    fp_ = nullptr;
}

Log::~Log() {
    if (writeThread_ && writeThread_->joinable()) { // 日志写入线程是否存在且可加入
        while (!deque->empty()) {
            deque_->flush(); // 队列不为空时，持续刷新，唤醒消费者
        };
        deque_->Close(); // 关闭日志队列
        writeThread_->join(); // 等待日志写入线程完成工作
    }

    if (fp_) { // 看文件指针是否有效
        lock_guard<mutex> locker(mtx_);
        flush(); // 刷新日志，把没有写入文件的日志刷新到文件
        fclose(fp_);
    }
}

int Log::GetLevel() {
    lock_guard<mutex> locker(mtx_);
    return level_;
}

void Log::SetLevel(int level) {
    lock_guard<mutex> locker(mtx_);
    level_ = level;
}

void Log::init(int level = 1, const char* path, const char* suffix,int maxQueueSize) {
    isOpen_ = true;
    level_ = level;
    if(maxQueueSize > 0) { // 异步方式
        isAsync_ = true;
        if(!deque_) { // 队列不空
            unique_ptr<BlockDeque<std::string>> newDeque(new BlockDeque<std::string>);
            deque_ = move(newDeque); // 用unique_ptr创建日志，同时利用move赋值给其它成员变量
            
            std::unique_ptr<std::thread> NewThread(new thread(FlushLogThread));
            writeThread_ = move(NewThread); //创建新线程同时赋给成员变量
        }
    } else {
        // 不是异步
        isAsync_ = false;
    }

    // 初始化日志
    lineCount_ = 0;

    time_t timer = time(nullptr);
    struct tm *sysTime = localtime(&timer);
    struct tm t = *sysTime;   
    path_ = path;
    suffix_ = suffix;
    char fileName[LOG_NAME_LEN] = {0}; // 生成合适的文件名
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s", 
            path_, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);
    toDay_ = t.tm_mday;

    {
        lock_guard<mutex> locker(mtx_); // 互斥锁保护临界区，确保多线程安全
        buff_.RetrieveAll(); // 清空缓冲区
        if(fp_) {  // 刷新关闭之前的日志文件
            flush();
            fclose(fp_); 
        }

        fp_ = fopen(fileName, "a"); // 以追加模式打开
        if(fp_ == nullptr) {
            mkdir(path_, 0777); // 再次创建打开
            fp_ = fopen(fileName, "a");
        } 
        assert(fp_ != nullptr); // 确保成功打开
    }
}

void Log::write(int level, const char *format, ...) {
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr); // 获取当前时间
    time_t tSec = now.tv_sec; //精确到秒
    struct tm *sysTime = localtime(&tSec); // 转换成本地时间
    struct tm t = *sysTime; // 复制时间结构体
    va_list vaList; // 定义可变参数列表

    /* 日志日期 日志行数 */
    /* 看日期是否变化，或者是否该日志条目超过了*/
    if (toDay_ != t.tm_mday || (lineCount_ && (lineCount_  %  MAX_LINES == 0)))
    {
        unique_lock<mutex> locker(mtx_); // 互斥锁
        locker.unlock(); // 释放，并发
        
        char newFile[LOG_NAME_LEN]; // 日志名
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        // 切换日志文件
        if (toDay_ != t.tm_mday) // 日志变化时生成新的日志名
        {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path_, tail, suffix_);
            toDay_ = t.tm_mday; // 更新当前日期，重置行生成器
            lineCount_ = 0;
        }
        else {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path_, tail, (lineCount_  / MAX_LINES), suffix_);
        }
        
        locker.lock(); // 重新获得锁
        flush(); // 刷新当前日志内容到文件
        fclose(fp_); // 关闭当前文件
        fp_ = fopen(newFile, "a");
        assert(fp_ != nullptr);
    }

    /* 写入日志内容 */
    {
        unique_lock<mutex> locker(mtx_);
        lineCount_++;
        // 写入缓冲区，同时写入日志时间戳
        int n = snprintf(buff_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                    t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                    t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
                    
        buff_.HasWritten(n); // 更改指针，增加评级
        AppendLogLevelTitle_(level);

        va_start(vaList, format); /*处理可变参数，格式化日志内容*/
        int m = vsnprintf(buff_.BeginWrite(), buff_.WritableBytes(), format, vaList);
        va_end(vaList);

        buff_.HasWritten(m); // 更新写入位置
        buff_.Append("\n\0", 2); // 添加换行符

        /*
        * 异步且队列未满，推入队列
        * 否则直接将日志内容写入文件
        */
        if(isAsync_ && deque_ && !deque_->full()) {
            deque_->push_back(buf f_.RetrieveAllToStr());
        } else {
            fputs(buff_.Peek(), fp_);
        }
        buff_.RetrieveAll(); // 清空日志缓冲区
    }
}

// 添加日志等级
void Log::AppendLogLevelTitle_(int level) {
    switch(level) {
    case 0:
        buff_.Append("[debug]: ", 9);
        break;
    case 1:
        buff_.Append("[info] : ", 9);
        break;
    case 2:
        buff_.Append("[warn] : ", 9);
        break;
    case 3:
        buff_.Append("[error]: ", 9);
        break;
    default:
        buff_.Append("[info] : ", 9);
        break;
    }
}

// 刷新日志，刷新队列（清空输入缓冲区）
void Log::flush() {
    if(isAsync_) { 
        deque_->flush(); 
    }
    fflush(fp_);
}

// 写线程真正的执行函数
void Log::AsyncWrite_() {
    string str = ""; // 建立一个字符串存储日志
    while(deque_->pop(str)) {
        lock_guard<mutex> locker(mtx_); // 加锁
        fputs(str.c_str(), fp_); // 将日志写入文件
    }
}

// 懒汉模式，局部静态变量法
Log* Log::Instance() {
    static Log inst;
    return &inst;
}

// 异步日志
void Log::FlushLogThread() {
    Log::Instance()->AsyncWrite_();
}