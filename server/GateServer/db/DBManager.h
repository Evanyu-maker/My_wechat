#pragma once
#include "DBConnectionPool.h"
#include "../const.h"
#include <iostream>

/**
 * 数据库管理器类
 * 提供数据库相关功能的初始化和管理
 */
class DBManager {
public:
    /**
     * 初始化数据库连接池
     * 从配置文件读取数据库配置并初始化连接池
     * 
     * @return 是否初始化成功
     */
    static bool initDBConnectionPool() {
        try {
            // 构建数据库配置
            DBPoolConfig config;
            
            // 从配置文件读取数据库连接信息
            config.host = gCfgMgr.getCfgVal(MYSQL_CONFIG_SECTION, MYSQL_HOST_KEY, "localhost");
            config.user = gCfgMgr.getCfgVal(MYSQL_CONFIG_SECTION, MYSQL_USER_KEY, "root");
            config.password = gCfgMgr.getCfgVal(MYSQL_CONFIG_SECTION, MYSQL_PASSWD_KEY, "");
            config.database = gCfgMgr.getCfgVal(MYSQL_CONFIG_SECTION, MYSQL_SCHEMA_KEY, "my_wechat");
            
            // 端口转换为整数
            std::string portStr = gCfgMgr.getCfgVal(MYSQL_CONFIG_SECTION, MYSQL_PORT_KEY, "3306");
            try {
                int port = std::stoi(portStr);
                // 将端口号添加到主机地址中
                config.host += ":" + portStr;
            } catch (const std::exception& e) {
                std::cerr << "数据库端口配置错误: " << e.what() << std::endl;
                return false;
            }
            
            // 使用默认值初始化其他配置
            config.initialSize = DB_DEFAULT_INITIAL_SIZE;
            config.maxSize = DB_DEFAULT_MAX_SIZE;
            config.minSize = DB_DEFAULT_MIN_SIZE;
            config.maxIdleTime = DB_DEFAULT_MAX_IDLE_TIME;
            config.connectionTimeout = DB_DEFAULT_TIMEOUT;
            config.validationInterval = DB_DEFAULT_VALIDATION_INTERVAL;
            config.timeBetweenEvictionRuns = DB_DEFAULT_EVICTION_INTERVAL;
            config.maxWaitQueueSize = DB_DEFAULT_MAX_WAIT_QUEUE_SIZE;
            
            // 初始化连接池
            if (!DBConnectionPool::getInstance().init(config)) {
                std::cerr << "初始化数据库连接池失败" << std::endl;
                return false;
            }
            
            std::cout << "数据库连接池初始化成功" << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "初始化数据库连接池异常: " << e.what() << std::endl;
            return false;
        }
    }
    
    /**
     * 关闭数据库连接池
     */
    static void shutdownDBConnectionPool() {
        try {
            DBConnectionPool::getInstance().shutdown();
            std::cout << "数据库连接池已关闭" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "关闭数据库连接池异常: " << e.what() << std::endl;
        }
    }
}; 