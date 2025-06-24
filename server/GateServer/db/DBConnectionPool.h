#pragma once
#include "../const.h"
#include <mysql/jdbc.h>
#include <memory>
#include <mutex>
#include <iostream>
#include <atomic>
#include <vector>
#include <queue>
#include <unordered_set>
#include <thread>
#include <chrono>
#include <functional>
#include <algorithm>

// 连接包装类
class ConnectionWrapper {
public:
    ConnectionWrapper(sql::Connection* conn, int id)
        : m_connection(conn), m_id(id), m_state(ConnectionState::IDLE),
          m_lastAccessTime(std::chrono::steady_clock::now()) {}
    
    ~ConnectionWrapper() {
        if (m_connection) {
            try {
                m_connection->close();
            } catch (...) {}
            delete m_connection;
        }
    }
    
    sql::Connection* getConnection() { return m_connection; }
    int getId() const { return m_id; }
    
    ConnectionState getState() const { 
        return m_state; 
    }
    
    void setState(ConnectionState state) { 
        m_state = state; 
        m_lastAccessTime = std::chrono::steady_clock::now();
    }
    
    std::chrono::steady_clock::time_point getLastAccessTime() const {
        return m_lastAccessTime;
    }
    
    void updateAccessTime() {
        m_lastAccessTime = std::chrono::steady_clock::now();
    }
    
    // 心跳检测
    bool ping() {
        try {
            if (!m_connection || m_connection->isClosed()) {
                return false;
            }
            
            // 执行一个简单的查询来测试连接
            std::unique_ptr<sql::Statement> stmt(m_connection->createStatement());
            std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery("SELECT 1"));
            return rs->next() && rs->getInt(1) == 1;
        } catch (...) {
            return false;
        }
    }
    
private:
    sql::Connection* m_connection;
    int m_id;
    ConnectionState m_state;
    std::chrono::steady_clock::time_point m_lastAccessTime;
};

// 连接池配置
struct DBPoolConfig {
    std::string host;
    std::string user;
    std::string password;
    std::string database;
    int initialSize;            // 初始连接数
    int maxSize;                // 最大连接数
    int minSize;                // 最小空闲连接数
    int maxIdleTime;            // 连接最大空闲时间(秒)
    int connectionTimeout;      // 获取连接超时时间(秒)
    int validationInterval;     // 连接有效性检查间隔(秒)
    int timeBetweenEvictionRuns;// 空闲连接清理间隔(秒)
    int maxWaitQueueSize;       // 最大等待队列大小
    
    // 默认构造函数 - 使用默认值初始化
    DBPoolConfig()
        : initialSize(DB_DEFAULT_INITIAL_SIZE),
          maxSize(DB_DEFAULT_MAX_SIZE),
          minSize(DB_DEFAULT_MIN_SIZE),
          maxIdleTime(DB_DEFAULT_MAX_IDLE_TIME),
          connectionTimeout(DB_DEFAULT_TIMEOUT),
          validationInterval(DB_DEFAULT_VALIDATION_INTERVAL),
          timeBetweenEvictionRuns(DB_DEFAULT_EVICTION_INTERVAL),
          maxWaitQueueSize(DB_DEFAULT_MAX_WAIT_QUEUE_SIZE) {}
};

// 数据库连接池类
class DBConnectionPool {
public:
    // 构造函数
    DBConnectionPool() = default;
    ~DBConnectionPool();
    
    // 初始化连接池
    bool init(const DBPoolConfig& config);
    
    // 获取连接
    std::shared_ptr<ConnectionWrapper> getConnection(int timeoutSeconds = 0);
    
    // 关闭连接池
    void shutdown();
    
    // 获取连接池状态信息
    struct PoolStats {
        int totalConnections;
        int activeConnections;
        int idleConnections;
        int waitingThreads;
    };
    
    PoolStats getStats() const;
    
    // 回调函数类型定义
    using ConnectionEventCallback = std::function<void(ConnectionWrapper*)>;
    
    // 设置连接创建回调
    void setOnConnectionCreate(ConnectionEventCallback callback) {
        m_onConnectionCreate = callback;
    }
    
    // 设置连接获取回调
    void setOnConnectionAcquire(ConnectionEventCallback callback) {
        m_onConnectionAcquire = callback;
    }
    
    // 设置连接释放回调
    void setOnConnectionRelease(ConnectionEventCallback callback) {
        m_onConnectionRelease = callback;
    }
    
private:
    // 创建新连接
    ConnectionWrapper* createConnection();
    
    // 检查连接有效性
    bool validateConnection(ConnectionWrapper* conn);
    
    // 心跳检测线程函数
    void heartbeatChecker();
    
    // 连接池维护线程函数
    void connectionManager();
    
    // 标记连接为可用
    void releaseConnection(ConnectionWrapper* conn);
    
    // 关闭并移除连接
    void closeConnection(ConnectionWrapper* conn);
    
    // 配置
    DBPoolConfig m_config;
    
    // 连接管理
    std::vector<ConnectionWrapper*> m_allConnections;
    std::queue<ConnectionWrapper*> m_idleConnections;
    std::unordered_set<ConnectionWrapper*> m_activeConnections;
    
    // 线程同步
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
    
    // 维护线程
    std::thread m_heartbeatThread;
    std::thread m_managerThread;
    
    // 状态标志
    std::atomic<bool> m_running{false};
    std::atomic<int> m_waitingThreads{0};
    
    // 连接计数
    int m_nextConnectionId{0};
    
    // 回调函数
    ConnectionEventCallback m_onConnectionCreate;
    ConnectionEventCallback m_onConnectionAcquire;
    ConnectionEventCallback m_onConnectionRelease;
    
    // MySQL驱动
    sql::Driver* m_driver{nullptr};
}; 