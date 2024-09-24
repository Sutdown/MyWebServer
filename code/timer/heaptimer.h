 #ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h> 
#include <functional> 
#include <assert.h> 
#include <chrono>
#include "../log/log.h"

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode {
    int id;
    TimeStamp expires; // ��ʱʱ���
    TimeoutCallBack cb; // ��ʱ�ص�����
    bool operator<(const TimerNode& t) {
        return expires < t.expires;
    }
};
/*���ڶ���С�ѵĶ�ʱ����*/
class HeapTimer {
public:
    HeapTimer() { heap_.reserve(64); }
    ~HeapTimer() { clear(); }
    
    void adjust(int id, int newExpires)
    void add(int id, int timeOut, const TimeoutCallBack& cb); // ��Ӷ�ʱ����㣬ID��������ֱ�Ӳ���Ȼ�������id��������³�ʱʱ��
    void doWork(int id);// �����ص�������ɾ���ö�ʱ��
    void clear(); // ���

    void tick(); // ���Ѷ�Ԫ���Ƿ�ʱ����ʱ��ص�ɾ��
    void pop(); // ɾ�����糬ʱ�Ķ�ʱ��
    int GetNextTick(); // ��ȡ��һ����ʱ��ʱ����ʣ��ʱ��

private:
    void del_(size_t i);  // ɾ����ʱ��
    void siftup_(size_t i); // ���ϵ���
    bool siftdown_(size_t index, size_t n); // ���µ���
    void SwapNode_(size_t i, size_t j); // �����������λ��

    std::vector<TimerNode> heap_;
    std::unordered_map<int, size_t> ref_; // ��ʱ��ID�����н������������ڿ��ٲ��Ҷ�ʱ��
};

#endif //HEAP_TIMER_H