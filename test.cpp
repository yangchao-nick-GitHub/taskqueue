#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include "taskqueue.hpp"

class TaskQueuePerformanceTest {
public:
    static void testThroughput(int numTasks, int threadNum) {
        TaskQueue tq(threadNum);
        std::atomic<int> completedTasks(0);

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < numTasks; ++i) {
            tq.submit([&completedTasks]() {
                // 模拟任务执行时间
                std::this_thread::sleep_for(std::chrono::microseconds(10));
                completedTasks.fetch_add(1);
            });
        }

        while (completedTasks.load() < numTasks) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        std::cout << "Total time taken: " << duration << " seconds" << std::endl;
    }

    static void testResponseTime(int numTasks, int threadNum) {
        TaskQueue tq(threadNum);
        std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> startTimes(numTasks);
        std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> endTimes(numTasks);
        std::atomic<int> completedTasks(0);

        for (int i = 0; i < numTasks; ++i) {
            startTimes[i] = std::chrono::high_resolution_clock::now();
            tq.submit([i, &endTimes, &completedTasks]() {
                // 模拟任务执行时间
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                endTimes[i] = std::chrono::high_resolution_clock::now();
                completedTasks.fetch_add(1);
            });
        }

        while (completedTasks.load() < numTasks) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        double totalResponseTime = 0.0;
        for (int i = 0; i < numTasks; ++i) {
            auto responseTime = std::chrono::duration_cast<std::chrono::microseconds>(endTimes[i] - startTimes[i]).count();
            totalResponseTime += responseTime;
        }

        std::cout << "Average response time: " << totalResponseTime / numTasks << " microseconds" << std::endl;
    }

    static void testConcurrency(int numTasks, int maxThreads) {
        // for (int threadNum = 1; threadNum <= maxThreads; ++threadNum) {
        //     std::cout << "Testing with " << threadNum << " threads:" << std::endl;
            
        //     testResponseTime(numTasks, threadNum);
        // }
        testThroughput(numTasks, maxThreads);
    }
};

int main() {
    const int numTasks = 1000;
    const int maxThreads = 4;

    TaskQueuePerformanceTest::testConcurrency(numTasks, maxThreads);

    return 0;
}