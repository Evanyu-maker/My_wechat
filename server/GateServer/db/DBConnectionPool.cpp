#include "DBConnectionPool.h"
#include <sstream>
#include <iostream>

bool DBConnectionPool::init(const DBPoolConfig& config) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_running) {
        return false; // Already initialized
    }
    
    try {
        m_config = config;
        
        // Get driver instance
        m_driver = get_driver_instance();
        
        // Initialize connection pool
        for (int i = 0; i < m_config.initialSize; ++i) {
            auto conn = createConnection();
            if (conn) {
                m_idleConnections.push(conn);
                m_allConnections.push_back(conn);
            } else {
                // Unable to create initial connections, clean up and return failure
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
        
        // Start maintenance threads
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
        
        // Set connection properties
        // For example: conn->setClientOption("OPT_CONNECT_TIMEOUT", "10");
        
        auto wrapper = new ConnectionWrapper(conn, m_nextConnectionId++);
        
        // Execute callback
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
    // Increase waiting thread count
    ++m_waitingThreads;
    
    // Ensure waiting thread count is decreased on function exit
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
        
        // Wait for available connection or timeout
        bool timeout = false;
        while (!m_running || (m_idleConnections.empty() && m_allConnections.size() >= m_config.maxSize)) {
            if (!m_running) {
                throw std::runtime_error("Connection pool is closed");
            }
            
            if (waitTime.count() > 0) {
                // Wait for condition variable notification or timeout
                if (m_condition.wait_for(lock, waitTime) == std::cv_status::timeout) {
                    timeout = true;
                    break;
                }
            } else {
                // No timeout limit, wait indefinitely
                m_condition.wait(lock);
            }
        }
        
        if (timeout) {
            throw std::runtime_error("Connection acquisition timeout");
        }
        
        if (!m_idleConnections.empty()) {
            // Get and remove the front connection
            conn = m_idleConnections.front();
            m_idleConnections.pop();
            
            // Validate connection
            if (!validateConnection(conn)) {
                closeConnection(conn);
                
                // Create new connection
                conn = createConnection();
                if (!conn) {
                    throw std::runtime_error("Unable to create database connection");
                }
            }
            
            // Mark as in use
            conn->setState(ConnectionState::IN_USE);
            m_activeConnections.insert(conn);
        } else if (m_allConnections.size() < m_config.maxSize) {
            // Create new connection
            conn = createConnection();
            if (conn) {
                conn->setState(ConnectionState::IN_USE);
                m_allConnections.push_back(conn);
                m_activeConnections.insert(conn);
            }
        }
    }
    
    if (!conn) {
        throw std::runtime_error("Unable to acquire database connection");
    }
    
    // Execute connection acquisition callback
    if (m_onConnectionAcquire) {
        m_onConnectionAcquire(conn);
    }
    
    // Use smart pointer to manage connection with custom deleter
    return std::shared_ptr<ConnectionWrapper>(conn, [this](ConnectionWrapper* c) {
        releaseConnection(c);
    });
}

void DBConnectionPool::releaseConnection(ConnectionWrapper* conn) {
    if (!conn) return;
    
    // Execute release callback
    if (m_onConnectionRelease) {
        m_onConnectionRelease(conn);
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_running) {
        closeConnection(conn);
        return;
    }
    
    // Remove from active connections set
    auto it = m_activeConnections.find(conn);
    if (it != m_activeConnections.end()) {
        m_activeConnections.erase(it);
    }
    
    // Check connection state
    if (conn->getState() == ConnectionState::BROKEN) {
        // Destroy and create new connection
        auto pos = std::find(m_allConnections.begin(), m_allConnections.end(), conn);
        if (pos != m_allConnections.end()) {
            *pos = createConnection();
            if (*pos) {
                m_idleConnections.push(*pos);
            }
        }
        delete conn;
    } else {
        // Mark as idle and return to pool
        conn->setState(ConnectionState::IDLE);
        m_idleConnections.push(conn);
    }
    
    // Notify waiting threads
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
    
    // Notify all waiting threads
    m_condition.notify_all();
    
    // Wait for management threads to end
    if (m_heartbeatThread.joinable()) {
        m_heartbeatThread.join();
    }
    
    if (m_managerThread.joinable()) {
        m_managerThread.join();
    }
    
    // Clean up all connections
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
        
        // Check idle connections
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
        
        // Restore available connections
        m_idleConnections = std::move(tempQueue);
    }
}

void DBConnectionPool::connectionManager() {
    while (m_running) {
        std::this_thread::sleep_for(std::chrono::seconds(m_config.timeBetweenEvictionRuns));
        
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_running) break;
        
        auto now = std::chrono::steady_clock::now();
        
        // Check idle connections
        std::queue<ConnectionWrapper*> tempQueue;
        size_t idleCount = m_idleConnections.size();
        
        while (!m_idleConnections.empty()) {
            auto conn = m_idleConnections.front();
            m_idleConnections.pop();
            
            auto idleTime = std::chrono::duration_cast<std::chrono::seconds>(
                now - conn->getLastAccessTime()).count();
            
            // If idle time exceeds maximum value and idle connection count is greater than minimum value, close
            if (idleTime > m_config.maxIdleTime && idleCount > m_config.minSize) {
                closeConnection(conn);
                idleCount--;
            } else {
                tempQueue.push(conn);
            }
        }
        
        // Restore remaining connections
        m_idleConnections = std::move(tempQueue);
        
        // If idle connections are less than minimum value, create new connection
        while (m_idleConnections.size() < m_config.minSize && 
               m_allConnections.size() < m_config.maxSize) {
            auto conn = createConnection();
            if (conn) {
                m_idleConnections.push(conn);
                m_allConnections.push_back(conn);
            } else {
                break; // Create failed, stop trying
            }
        }
    }
}

void DBConnectionPool::closeConnection(ConnectionWrapper* conn) {
    if (!conn) return;
    
    // Remove from all connections
    auto it = std::find(m_allConnections.begin(), m_allConnections.end(), conn);
    if (it != m_allConnections.end()) {
        m_allConnections.erase(it);
    }
    
    // Remove from active connections
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