#include "sqlconnpool.h"
using namespace std;

SqlConnPool::SqlConnPool() {
    useCount_ = 0;
    freeCount_ = 0;
}

// ����ģʽ������ģʽ
SqlConnPool* SqlConnPool::Instance() {
    static SqlConnPool connPool;
    return &connPool;
}

/*��ʼ�����ӳ�*/
void SqlConnPool::Init(const char* host, int port,
            const char* user,const char* pwd, const char* dbName,
            int connSize = 10) {
    assert(connSize > 0);
    for (int i = 0; i < connSize; i++) {
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);
        if (!sql) {
            LOG_ERROR("MySql init error!");
            assert(sql);
        }
        sql = mysql_real_connect(sql, host,
                                 user, pwd,
                                 dbName, port, nullptr, 0);
        if (!sql) {
            LOG_ERROR("MySql Connect error!");
        }
        connQue_.push(sql);
    }
    MAX_CONN_ = connSize;
    /*�ź�����ʼ��*/
    /*���0��ʾ�ź�����������ͬһ�����̵Ķ���߳�֮���ͬ��*/
    sem_init(&semId_, 0, MAX_CONN_);
}

MYSQL* SqlConnPool::GetConn() {
    MYSQL *sql = nullptr;
    if(connQue_.empty()){
        LOG_WARN("SqlConnPool busy!");
        return nullptr;
    }
    /*
    * �ź������������ӳ��еĿ���������������ֹ�������Ƶ��߳�ͬʱ��������
    * �������������ٽ����Ĺ�����Դ����ֹ���ݾ������������еĲ�������
    */
    sem_wait(&semId_);
    {
        lock_guard<mutex> locker(mtx_);
        sql = connQue_.front();
        connQue_.pop();
    }
    return sql;
}

/*���ݿ��ͷ����ӣ��ٶ����м���sql*/
void SqlConnPool::FreeConn(MYSQL* sql) {
    assert(sql);
    lock_guard<mutex> locker(mtx_);
    connQue_.push(sql);
    sem_post(&semId_);
}

void SqlConnPool::ClosePool() {
    /*��������Ȼ���ͷŶ��У��ر���Ŀ*/
    lock_guard<mutex> locker(mtx_);
    while(!connQue_.empty()) {
        auto item = connQue_.front();
        connQue_.pop();
        mysql_close(item);
    }
    /*��������ֹ��mysql��Ĺ���
    * 1.�ͷź�mysql���ʼ��ʱ�����ȫ����Դ�������ڴ棬�̣߳���
    * 2.��֤Ӧ�ó���ɾ��Ĺرպ�mysql��صĿ⣬������Դй©
    */
    mysql_library_end();        
}

int SqlConnPool::GetFreeConnCount() {
    lock_guard<mutex> locker(mtx_);
    return connQue_.size();
}

SqlConnPool::~SqlConnPool() {
    ClosePool();
}
