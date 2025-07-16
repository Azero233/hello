#include "TaskQueue.h"
#include <thread>
#include <iostream>
#include <chrono>
#include <atomic>
#include <vector>
#include <cassert>
#include <string>
#include <mutex>
#include <queue>
#include <crtdbg.h>

void MpmcTaskQueueTest() {
    MpmcTaskQueue queue;

    const int totalTasksPerProducer = 10000;
    const int produceCount = 2;
    const int consumerCount = 2;

    std::atomic<int> producedCount{ 0 };
    std::atomic<int> consumedCount{ 0 };

    // ---- 生产者线程 ----
    std::vector<std::thread> producers;
    for (int i = 0; i < produceCount; ++i) {
        producers.emplace_back([i, &queue, &producedCount, totalTasksPerProducer]() {
            for (int j = 0; j < totalTasksPerProducer; ++j) {
                queue.Enqueue([i, j](size_t) {
                    // 模拟任务处理
                    std::string strMsg = "[Producer " + std::to_string(i) + "] Task " + std::to_string(j) + "\n";
                    std::cout << strMsg;
                    });
                ++producedCount;
            }
            });
    }

    // ---- 消费者线程 ----
    std::vector<std::thread> consumers;
    for (int i = 0; i < consumerCount; ++i) {
        consumers.emplace_back([i, &queue, &consumedCount, &producedCount, totalTasksPerProducer, produceCount]() {
            std::function<void(size_t)> task;
            std::vector<LPNODE> arrRetiredNodes;
            while (true) {
                if (queue.Dequeue(task, arrRetiredNodes)) {
                    task(i); // 执行任务
                    ++consumedCount;
                }
                else {
                    // 队列空了，判断是否所有任务已完成
                    if (consumedCount.load() >= totalTasksPerProducer * produceCount) {
                        break;
                    }
                    // 小睡防止自旋过热
                    std::this_thread::sleep_for(std::chrono::microseconds(50));
                }
            }
            ClearNodeArr(arrRetiredNodes);
            });
    }

    // 等待所有线程完成
    for (auto& p : producers) p.join();
    for (auto& c : consumers) c.join();

    std::cout << "Produced: " << producedCount.load() << std::endl;
    std::cout << "Consumed: " << consumedCount.load() << std::endl;

    assert(producedCount == consumedCount);
    std::cout << "Test Passed: All tasks processed correctly." << std::endl;
}

void ThreadTest()
{
    std::mutex mtx;
    std::condition_variable cond;

    std::queue<int> q;
    std::thread pro([&mtx, &cond, &q]()->void {
        for (int i = 0; i < 10000; ++i) {
            std::unique_lock<std::mutex> lock(mtx);
            q.emplace(i);
            lock.unlock();
            cond.notify_one();
            //std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }
        });

    std::thread com([&mtx, &cond, &q]()->void {
        int count = 0;
        while (count < 10000)
        {
            std::queue<int> local;
            {
                std::unique_lock<std::mutex> lock(mtx);
                cond.wait(lock, [&q] {
                    return !q.empty();
                    });
                std::swap(local, q);
            }
            while (!local.empty()) {
                int i = local.front();
                local.pop();
                std::cout << std::to_string(i) << "\n";
                ++count;
            }
        }
        });

    pro.join();
    com.join();
}

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);   // 内存泄露检测

    MpmcTaskQueueTest();
    //ThreadTest();
    return 0;
}
