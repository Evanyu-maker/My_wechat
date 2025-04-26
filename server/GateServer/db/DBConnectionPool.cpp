#include "DBConnectionPool.h"
#include <sstream>
#include <iostream>

bool DBConnectionPool::init(const DBPoolConfig& config) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_running) {
        return false; // 已初始化
    }
    
    try {
        m_config = config;
        
        // 获取驱动实例
        m_driver = get_driver_instance();
        
        // 初始化连接池
        for (int i = 0; i < m_config.initialSize; ++i) {
            auto conn = createConnection();
            if (conn) {
                m_idleConnections.push(conn);
                m_allConnections.push_back(conn);
            } else {
                // 无法创建初始连接，清理并返回失败
                for (auto c : m_allConnections) {
                    delete c;
                }
                m_allConnections.clear();
                while (!m_idleConnections.empty()) {
                    m_idleConnections.pop();
                }
                return false;
            }
        }
        
        // 启动维护线程
        m_running = true;
        m_heartbeatThread = std::thread(&DBConnectionPool::heartbeatChecker, this);
        m_managerThread = std::thread(&DBConnectionPool::connectionManager, this);
        
        return true;
    } catch (const sql::SQLException& e) {
        std::cerr << "SQL Error: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}

ConnectionWrapper* DBConnectionPool::createConnection() {
    try {
        auto* conn = m_driver->connect(m_config.host, m_config.user, m_config.password);
        conn->setSchema(m_config.database);
        
        // 设置连接属性
        // 例如：conn->setClientOption("OPT_CONNECT_TIMEOUT", "10");
        
        auto wrapper = new ConnectionWrapper(conn, m_nextConnectionId++);
        
        // 执行回调
        if (m_onConnectionCreate) {
            m_onConnectionCreate(wrapper);
        }
        
        return wrapper;
    } catch (const sql::SQLException& e) {
        std::cerr << "Failed to create connection: " << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<ConnectionWrapper> DBConnectionPool::getConnection(int timeoutSeconds) {
    // 增加等待线程计数
    ++m_waitingThreads;
    
    // 确保在函数退出时减少等待线程计数
    struct ScopeGuard {
        std::atomic<int>& counter;
        ScopeGuard(std::atomic<int>& c) : counter(c) {}
        ~ScopeGuard() { --counter; }
    } guard(m_waitingThreads);
    
    ConnectionWrapper* conn = nullptr;
    
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        
        auto waitTime = timeoutSeconds > 0 
            ? std::chrono::seconds(timeoutSeconds) 
            : std::chrono::seconds(m_config.connectionTimeout);
        
        // 等待可用连接或超时
        bool timeout = false;
        while (!m_running || (m_idleConnections.empty() && m_allConnections.size() >= m_config.maxSize)) {
            if (!m_running) {
                throw std::runtime_error("连接池已关闭");
            }
            
            if (waitTime.count() > 0) {
                // 等待条件变量通知或超时
                if (m_condition.wait_for(lock, waitTime) == std::cv_status::timeout) {
                    timeout = true;
                    break;
                }
            } else {
                // 没有超时限制，一直等待
                m_condition.wait(lock);
            }
        }
        
        if (timeout) {
            throw std::runtime_error("获取连接超时");
        }
        
        if (!m_idleConnections.empty()) {
            // 获取并移除队首连接
            conn = m_idleConnections.front();
            m_idleConnections.pop();
            
            // 验证连接是否有效
            if (!validateConnection(conn)) {
                closeConnection(conn);
                
                // 创建新连接
                conn = createConnection();
                if (!conn) {
                    throw std::runtime_error("无法创建数据库连接");
                }
            }
            
            // 标记为使用中
            conn->setState(ConnectionState::IN_USE);
            m_activeConnections.insert(conn);
        } else if (m_allConnections.size() < m_config.maxSize) {
            // 创建新连接
            conn = createConnection();
            if (conn) {
                conn->setState(ConnectionState::IN_USE);
                m_allConnections.push_back(conn);
                m_activeConnections.insert(conn);
            }
        }
    }
    
    if (!conn) {
        throw std::runtime_error("无法获取数据库连接");
    }
    
    // 执行获取连接回调
    if (m_onConnectionAcquire) {
        m_onConnectionAcquire(conn);
    }
    
    // 使用智能指针管理连接，并设置自定义删除器
    return std::shared_ptr<ConnectionWrapper>(conn, [this](ConnectionWrapper* c) {
        releaseConnection(c);
    });
}

void DBConnectionPool::releaseConnection(ConnectionWrapper* conn) {
    if (!conn) return;
    
    // 执行释放回调
    if (m_onConnectionRelease) {
        m_onConnectionRelease(conn);
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_running) {
        closeConnection(conn);
        return;
    }
    
    // 从活动连接集合中移除
    auto it = m_activeConnections.find(conn);
    if (it != m_activeConnections.end()) {
        m_activeConnections.erase(it);
    }
    
    // 检查连接状态
    if (conn->getState() == ConnectionState::BROKEN) {
        // 销毁并创建新连接
        auto pos = std::find(m_allConnections.begin(), m_allConnections.end(), conn);
        if (pos != m_allConnections.end()) {
            *pos = createConnection();
            if (*pos) {
                m_idleConnections.push(*pos);
            }
        }
        delete conn;
    } else {
        // 标记为空闲并归还连接池
        conn->setState(ConnectionState::IDLE);
        m_idleConnections.push(conn);
    }
    
    // 通知等待的线程
    m_condition.notify_one();
}

bool DBConnectionPool::validateConnection(ConnectionWrapper* conn) {
    if (!conn) return false;
    
    try {
        return conn->ping();
    } catch (...) {
        conn->setState(ConnectionState::BROKEN);
        return false;
    }
}

void DBConnectionPool::shutdown() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_running) {
            return;
        }
        m_running = false;
    }
    
    // 通知所有等待线程
    m_condition.notify_all();
    
    // 等待管理线程结束
    if (m_heartbeatThread.joinable()) {
        m_heartbeatThread.join();
    }
    
    if (m_managerThread.joinable()) {
        m_managerThread.join();
    }
    
    // 清理所有连接
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto conn : m_allConnections) {
        delete conn;
    }
    
    m_allConnections.clear();
    m_activeConnections.clear();
    while (!m_idleConnections.empty()) {
        m_idleConnections.pop();
    }
}

void DBConnectionPool::heartbeatChecker() {
    while (m_running) {
        std::this_thread::sleep_for(std::chrono::seconds(m_config.validationInterval));
        
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_running) break;
        
        // 检查空闲连接
        std::queue<ConnectionWrapper*> tempQueue;
        while (!m_idleConnections.empty()) {
            auto conn = m_idleConnections.front();
            m_idleConnections.pop();
            
            if (validateConnection(conn)) {
                tempQueue.push(conn);
            } else {
                closeConnection(conn);
            }
        }
        
        // 恢复可用连接
        m_idleConnections = std::move(tempQueue);
    }
}

void DBConnectionPool::connectionManager() {
    while (m_running) {
        std::this_thread::sleep_for(std::chrono::seconds(m_config.timeBetweenEvictionRuns));
        
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_running) break;
        
        auto now = std::chrono::steady_clock::now();
        
        // 检查空闲连接是否过期
        std::queue<ConnectionWrapper*> tempQueue;
        size_t idleCount = m_idleConnections.size();
        
        while (!m_idleConnections.empty()) {
            auto conn = m_idleConnections.front();
            m_idleConnections.pop();
            
            auto idleTime = std::chrono::duration_cast<std::chrono::seconds>(
                now - conn->getLastAccessTime()).count();
            
            // 如果空闲时间超过最大值且空闲连接数大于最小值，则关闭
            if (idleTime > m_config.maxIdleTime && idleCount > m_config.minSize) {
                closeConnection(conn);
                idleCount--;
            } else {
                tempQueue.push(conn);
            }
        }
        
        // 恢复剩余连接
        m_idleConnections = std::move(tempQueue);
        
        // 如果空闲连接少于最小值，创建新连接
        while (m_idleConnections.size() < m_config.minSize && 
               m_allConnections.size() < m_config.maxSize) {
            auto conn = createConnection();
            if (conn) {
                m_idleConnections.push(conn);
                m_allConnections.push_back(conn);
            } else {
                break; // 创建失败，停止尝试
            }
        }
    }
}

void DBConnectionPool::closeConnection(ConnectionWrapper* conn) {
    if (!conn) return;
    
    // 从所有连接中移除
    auto it = std::find(m_allConnections.begin(), m_allConnections.end(), conn);
    if (it != m_allConnections.end()) {
        m_allConnections.erase(it);
    }
    
    // 从活动连接中移除
    m_activeConnections.erase(conn);
    
    delete conn;
}

DBConnectionPool::PoolStats DBConnectionPool::getStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    PoolStats stats;
    stats.totalConnections = m_allConnections.size();
    stats.activeConnections = m_activeConnections.size();
    stats.idleConnections = m_idleConnections.size();
    stats.waitingThreads = m_waitingThreads.load();
    
    return stats;
}

DBConnectionPool::~DBConnectionPool() {
    shutdown();
} 