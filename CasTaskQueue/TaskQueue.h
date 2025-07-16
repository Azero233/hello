#ifndef __MPMC_TASK_QUEUE__
#define __MPMC_TASK_QUEUE__

#include <atomic>
#include <functional>

constexpr int NODE_DELETE_THRESHOLD = 64;

typedef struct _NODE
{
	std::function<void(size_t)> fnTask;
	std::atomic<_NODE*> apNext;

	explicit _NODE(std::function<void(size_t)>&& t = {});	// explicit 只能放在类体内
} NODE, * LPNODE;

void ClearNodeArr(std::vector<LPNODE>& arrRetiredNodes);

// 基于 atomic 原子操作和 CAS 思想的无锁任务队列
class MpmcTaskQueue {
public:
	MpmcTaskQueue();
	~MpmcTaskQueue();

	void Enqueue(std::function<void(size_t)>&& task);
	bool Dequeue(std::function<void(size_t)>& task, std::vector<LPNODE>& arrRetiredNodes);
	bool Empty() const;

private:
	std::atomic<LPNODE> m_ptHead;
	std::atomic<LPNODE> m_ptTail;
};

#endif // __MPMC_TASK_QUEUE__
