#pragma once
#include "DBConnectionPool.h"
#include "../const.h"
#include <iostream>
#include <memory>

/**
 * Database Manager Class
 * Provides initialization and management of database-related functionality
 */
class DBManager {
public:
    /**
     * Get the singleton instance
     */
    static DBManager& GetInstance() {
        // Meyer's singleton - guaranteed to be thread-safe in C++11 and later
        static DBManager instance;
        return instance;
    }
    
    /**
     * Initialize the database connection pool
     * Returns whether the pool was initialized successfully
     * 
     * @return Whether initialization was successful
     */
    bool initDBConnectionPool() {
        if (m_initialized) {
            return true; // Already initialized
        }
        
        try {
            // Build database configuration
            DBPoolConfig config;
            
            // Read database connection information from config file
            config.host = ConfigMgr::Inst()[MYSQL_CONFIG_SECTION][MYSQL_HOST_KEY];
            config.user = ConfigMgr::Inst()[MYSQL_CONFIG_SECTION][MYSQL_USER_KEY];
            config.password = ConfigMgr::Inst()[MYSQL_CONFIG_SECTION][MYSQL_PASSWD_KEY];
            config.database = ConfigMgr::Inst()[MYSQL_CONFIG_SECTION][MYSQL_SCHEMA_KEY];
            
            // Convert port to integer
            std::string portStr = ConfigMgr::Inst()[MYSQL_CONFIG_SECTION][MYSQL_PORT_KEY];
            if (portStr.empty()) {
                portStr = "3306"; // Default port
            }
            
            try {
                int port = std::stoi(portStr);
                // Add port number to host address
                config.host += ":" + portStr;
            } catch (const std::exception& e) {
                std::cerr << "Database port configuration error: " << e.what() << std::endl;
                return false;
            }
            
            // Initialize other configs with default values
            config.initialSize = DB_DEFAULT_INITIAL_SIZE;
            config.maxSize = DB_DEFAULT_MAX_SIZE;
            config.minSize = DB_DEFAULT_MIN_SIZE;
            config.maxIdleTime = DB_DEFAULT_MAX_IDLE_TIME;
            config.connectionTimeout = DB_DEFAULT_TIMEOUT;
            config.validationInterval = DB_DEFAULT_VALIDATION_INTERVAL;
            config.timeBetweenEvictionRuns = DB_DEFAULT_EVICTION_INTERVAL;
            config.maxWaitQueueSize = DB_DEFAULT_MAX_WAIT_QUEUE_SIZE;
            
            // Create and initialize connection pool
            m_connectionPool = std::make_shared<DBConnectionPool>();
            if (m_connectionPool && m_connectionPool->init(config)) {
                std::cout << "Database connection pool initialized successfully" << std::endl;
                m_initialized = true;
                return true;
            } else {
                std::cerr << "Failed to initialize database connection pool" << std::endl;
                return false;
            }
        } catch (const std::exception& e) {
            std::cerr << "Database connection pool initialization exception: " << e.what() << std::endl;
            return false;
        }
    }
    
    /**
     * Get a database connection
     * @param timeoutSeconds Timeout in seconds when obtaining a connection, 0 means use default timeout
     * @return Smart pointer to a connection wrapper
     */
    std::shared_ptr<ConnectionWrapper> getConnection(int timeoutSeconds = 0) {
        if (!m_connectionPool || !m_initialized) {
            throw std::runtime_error("Database connection pool not initialized");
        }
        return m_connectionPool->getConnection(timeoutSeconds);
    }
    
    /**
     * Shutdown the database connection pool
     */
    void shutdownDBConnectionPool() {
        try {
            if (m_connectionPool) {
                m_connectionPool->shutdown();
                std::cout << "Database connection pool has been closed" << std::endl;
                m_initialized = false;
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception while closing database connection pool: " << e.what() << std::endl;
        }
    }
    
    /**
     * Get connection pool statistics
     */
    DBConnectionPool::PoolStats getPoolStats() const {
        if (!m_connectionPool || !m_initialized) {
            throw std::runtime_error("Database connection pool not initialized");
        }
        return m_connectionPool->getStats();
    }
    
    /**
     * Check if the database connection pool is initialized
     */
    bool isInitialized() const {
        return m_initialized;
    }
    
private:
    // Private constructor - initialization now moved to initDBConnectionPool()
    DBManager() : m_initialized(false) {}
    
    ~DBManager() {
        shutdownDBConnectionPool();
    }
    
    // Non-copyable
    DBManager(const DBManager&) = delete;
    DBManager& operator=(const DBManager&) = delete;
    
    // Database connection pool instance
    std::shared_ptr<DBConnectionPool> m_connectionPool;
    // Flag to track initialization status
    bool m_initialized = false;
};

// Global DBManager instance access macro
#define gDBManager DBManager::GetInstance() 