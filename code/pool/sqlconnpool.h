#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include "../log/log.h"

class SqlConnPool {
public:
    /*单例模式*/
    static SqlConnPool *Instance();

    /*得到连接和释放连接*/
    MYSQL *GetConn();
    void FreeConn(MYSQL * conn);
    int GetFreeConnCount();

    void Init(const char* host, int port,
              const char* user,const char* pwd, 
              const char* dbName, int connSize);
    void ClosePool();

private:
    SqlConnPool();
    ~SqlConnPool();

    int MAX_CONN_; // 最大连接数
    int useCount_;
    int freeCount_;

    std::queue<MYSQL *> connQue_; // 连接队列
    std::mutex mtx_; // 互斥锁
    sem_t semId_; // 信号量
};


#endif // SQLCONNPOOL_H