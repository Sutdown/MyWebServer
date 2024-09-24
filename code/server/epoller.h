#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h> //epoll_ctl()
#include <fcntl.h>  // fcntl()
#include <unistd.h> // close()
#include <assert.h> // close()
#include <vector>
#include <errno.h>

/*基于epoll的事件处理类*/
class Epoller {
public:
    explicit Epoller(int maxEvent = 1024);

    ~Epoller();

    /*添加事件，修改事件，删除事件epoll_ctl*/
    bool AddFd(int fd, uint32_t events);
    bool ModFd(int fd, uint32_t events);
    bool DelFd(int fd);

    /*等待事件发生，epoll_wait*/
    int Wait(int timeoutMs = -1);

    /*第i个事件的类型or文件描述符*/
    int GetEventFd(size_t i) const;
    uint32_t GetEvents(size_t i) const;
        
private:
    int epollFd_;

    std::vector<struct epoll_event> events_;    
};

#endif //EPOLLER_H