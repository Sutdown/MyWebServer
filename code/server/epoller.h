#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h> //epoll_ctl()
#include <fcntl.h>  // fcntl()
#include <unistd.h> // close()
#include <assert.h> // close()
#include <vector>
#include <errno.h>

/*����epoll���¼�������*/
class Epoller {
public:
    explicit Epoller(int maxEvent = 1024);

    ~Epoller();

    /*����¼����޸��¼���ɾ���¼�epoll_ctl*/
    bool AddFd(int fd, uint32_t events);
    bool ModFd(int fd, uint32_t events);
    bool DelFd(int fd);

    /*�ȴ��¼�������epoll_wait*/
    int Wait(int timeoutMs = -1);

    /*��i���¼�������or�ļ�������*/
    int GetEventFd(size_t i) const;
    uint32_t GetEvents(size_t i) const;
        
private:
    int epollFd_;

    std::vector<struct epoll_event> events_;    
};

#endif //EPOLLER_H