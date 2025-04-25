#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <iostream>
#include<hiredis.h>

class RedisConPool {
public:
    RedisConPool(size_t poolSize, const char* host, int port, const char* pwd);
    ~RedisConPool();
    redisContext* getConnection();
    void returnConnection(redisContext* context);
    void Close();

private:
    std::atomic<bool> b_stop_;
    size_t poolSize_;
    const char* host_;
    int port_;
    std::queue<redisContext*> connections_;
    std::mutex mutex_;
    std::condition_variable cond_;
}; 