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
    TimeStamp expires; // 超时时间点
    TimeoutCallBack cb; // 超时回调函数
    bool operator<(const TimerNode& t) {
        return expires < t.expires;
    }
};
/*基于对最小堆的定时器类*/
class HeapTimer {
public:
    HeapTimer() { heap_.reserve(64); }
    ~HeapTimer() { clear(); }
    
    void adjust(int id, int newExpires)
    void add(int id, int timeOut, const TimeoutCallBack& cb); // 添加定时器结点，ID不存在则直接插入然后调整；id存在则更新超时时间
    void doWork(int id);// 触发回调函数，删除该定时器
    void clear(); // 清空

    void tick(); // 检查堆顶元素是否超时，超时则回调删除
    void pop(); // 删除最早超时的定时器
    int GetNextTick(); // 获取下一个超时定时器的剩余时间

private:
    void del_(size_t i);  // 删除定时器
    void siftup_(size_t i); // 向上调整
    bool siftdown_(size_t index, size_t n); // 向下调整
    void SwapNode_(size_t i, size_t j); // 交换两个结点位置

    std::vector<TimerNode> heap_;
    std::unordered_map<int, size_t> ref_; // 定时器ID到堆中结点的索引，便于快速查找定时器
};

#endif //HEAP_TIMER_H