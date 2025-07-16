#include "TaskQueue.h"
#include <iostream>

_NODE::_NODE(std::function<void(size_t)>&& t) 
	: fnTask(std::move(t)), apNext(nullptr)
{
}


void ClearNodeArr(std::vector<LPNODE>& arrRetiredNodes)
{
	for (LPNODE node : arrRetiredNodes)
	{
		delete node;
	}
	arrRetiredNodes.clear();
}

MpmcTaskQueue::MpmcTaskQueue()
{
	// 初始 dummy 节点
	LPNODE dummy = new NODE();
	m_ptHead.store(dummy, std::memory_order_relaxed);
	m_ptTail.store(dummy, std::memory_order_relaxed);
}

MpmcTaskQueue::~MpmcTaskQueue()
{
	// 清理所有节点
	LPNODE node = m_ptHead.load();
	while (node != nullptr)
	{
		LPNODE next = node->apNext.load();
		delete node;
		node = next;
	}
}

// 多生产者安全
void MpmcTaskQueue::Enqueue(std::function<void(size_t)>&& task)
{
	LPNODE newNode = new NODE(std::move(task));		// 用于存放插入的节点
	LPNODE prevNode = nullptr;						// 用于暂存 m_ptTail
	while (true)
	{
		prevNode = m_ptTail.load(std::memory_order_acquire);
		LPNODE next = prevNode->apNext.load(std::memory_order_acquire);

		if (next == nullptr)
		{
			if (prevNode->apNext.compare_exchange_weak(next, newNode))
			{
				m_ptTail.compare_exchange_weak(prevNode, newNode);
				return;
			}
		}
		else
		{
			m_ptTail.compare_exchange_weak(prevNode, next);
		}
	}
}

// 多消费者安全
// std::vector<LPNODE>& arrRetiredNodes 为过期节点暂存，当内部存放的节点数超过阈值，便释放它们
bool MpmcTaskQueue::Dequeue(std::function<void(size_t)>& task, std::vector<LPNODE>& arrRetiredNodes)
{
	// thread_local 是 C++11 引入的关键字，用来声明 线程局部存储（Thread-Local Storage, TLS）
	// 作用是为每个线程提供一个变量的独立实例
	//static thread_local std::vector<LPNODE> s_arrRetiredNodes;

	LPNODE prevHead = nullptr;
	while (true)
	{
		prevHead = m_ptHead.load(std::memory_order_acquire);
		LPNODE next = prevHead->apNext.load(std::memory_order_acquire);

		if (next == nullptr)
		{
			return false;
		}

		std::function<void(size_t)> tmp = next->fnTask;
		if (m_ptHead.compare_exchange_weak(prevHead, next))
		{
			task = std::move(tmp);

			// 解决思路：延迟释放节点（Safe Memory Reclamation）
			arrRetiredNodes.push_back(prevHead);
			if (arrRetiredNodes.size() >= NODE_DELETE_THRESHOLD)
			{
				//for (LPNODE node : arrRetiredNodes)
				//{
				//	delete node;
				//}
				//arrRetiredNodes.clear();
				ClearNodeArr(arrRetiredNodes);
			}
			return true;
		}
	}
}

bool MpmcTaskQueue::Empty() const
{
	LPNODE head = m_ptHead.load(std::memory_order_acquire);
	LPNODE next = head->apNext.load(std::memory_order_acquire);
	return next == nullptr;
}
