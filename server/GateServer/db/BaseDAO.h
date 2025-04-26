#pragma once
#include "DBConnectionPool.h"
#include <memory>
#include <string>
#include <stdexcept>
#include <sstream>
#include <vector>

// DAO操作结果
template <typename T>
class DAOResult {
public:
    DAOResult() : m_success(false), m_data(nullptr) {}
    
    explicit DAOResult(bool success) : m_success(success), m_data(nullptr) {}
    
    DAOResult(bool success, const std::string& message) 
        : m_success(success), m_message(message), m_data(nullptr) {}
    
    DAOResult(bool success, const std::string& message, std::shared_ptr<T> data) 
        : m_success(success), m_message(message), m_data(data) {}
    
    bool isSuccess() const { return m_success; }
    
    const std::string& getMessage() const { return m_message; }
    
    std::shared_ptr<T> getData() const { return m_data; }
    
    void setSuccess(bool success) { m_success = success; }
    
    void setMessage(const std::string& message) { m_message = message; }
    
    void setData(std::shared_ptr<T> data) { m_data = data; }
    
private:
    bool m_success;
    std::string m_message;
    std::shared_ptr<T> m_data;
};

// 特化用于无返回数据的结果
template <>
class DAOResult<void> {
public:
    DAOResult() : m_success(false) {}
    
    explicit DAOResult(bool success) : m_success(success) {}
    
    DAOResult(bool success, const std::string& message) 
        : m_success(success), m_message(message) {}
    
    bool isSuccess() const { return m_success; }
    
    const std::string& getMessage() const { return m_message; }
    
    void setSuccess(bool success) { m_success = success; }
    
    void setMessage(const std::string& message) { m_message = message; }
    
private:
    bool m_success;
    std::string m_message;
};

// SQL异常处理宏
#define DAO_TRY try {
#define DAO_CATCH(result_type) \
    } catch (const sql::SQLException& e) { \
        std::stringstream ss; \
        ss << "SQL错误 [" << e.getErrorCode() << "]: " << e.what(); \
        return DAOResult<result_type>(false, ss.str()); \
    } catch (const std::exception& e) { \
        return DAOResult<result_type>(false, e.what()); \
    } catch (...) { \
        return DAOResult<result_type>(false, "未知错误"); \
    }

class BaseDAO {
protected:
    // 获取数据库连接
    std::shared_ptr<ConnectionWrapper> getConnection() {
        return DBConnectionPool::GetInstance()->getConnection();
    }
    
    // 调用存储过程 - 无参数版本
    std::shared_ptr<sql::ResultSet> callProcedure(
        const std::string& procName, sql::Connection* conn) {
        std::string sql = "CALL " + procName + "()";
        std::unique_ptr<sql::Statement> stmt(conn->createStatement());
        return std::shared_ptr<sql::ResultSet>(stmt->executeQuery(sql));
    }
    
    // 调用存储过程 - 带参数版本（返回结果集）
    std::shared_ptr<sql::ResultSet> callProcedureWithParams(
        const std::string& procName, 
        const std::vector<std::string>& params,
        sql::Connection* conn) {
        
        std::string sql = "CALL " + procName + "(";
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) {
                sql += ", ";
            }
            sql += params[i];
        }
        sql += ")";
        
        std::unique_ptr<sql::Statement> stmt(conn->createStatement());
        return std::shared_ptr<sql::ResultSet>(stmt->executeQuery(sql));
    }
    
    // 调用存储过程 - 使用预处理语句（更安全，防注入）
    std::shared_ptr<sql::PreparedStatement> prepareProcedureCall(
        const std::string& procName, int paramCount,
        sql::Connection* conn) {
        
        std::string sql = "CALL " + procName + "(";
        for (int i = 0; i < paramCount; ++i) {
            if (i > 0) {
                sql += ", ";
            }
            sql += "?";
        }
        sql += ")";
        
        return std::shared_ptr<sql::PreparedStatement>(
            conn->prepareStatement(sql));
    }
    
    // 调用存储过程 - 执行更新操作（无结果集返回）
    int callProcedureForUpdate(
        const std::string& procName,
        const std::vector<std::string>& params,
        sql::Connection* conn) {
        
        std::string sql = "CALL " + procName + "(";
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) {
                sql += ", ";
            }
            sql += params[i];
        }
        sql += ")";
        
        std::unique_ptr<sql::Statement> stmt(conn->createStatement());
        return stmt->executeUpdate(sql);
    }
    
    // 执行多结果集存储过程调用
    std::vector<std::shared_ptr<sql::ResultSet>> callProcedureMultiResults(
        const std::string& procName,
        const std::vector<std::string>& params,
        sql::Connection* conn) {
        
        std::vector<std::shared_ptr<sql::ResultSet>> results;
        
        std::string sql = "CALL " + procName + "(";
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) {
                sql += ", ";
            }
            sql += params[i];
        }
        sql += ")";
        
        std::unique_ptr<sql::Statement> stmt(conn->createStatement());
        
        if (stmt->execute(sql)) {
            do {
                std::shared_ptr<sql::ResultSet> rs(stmt->getResultSet());
                if (rs) {
                    results.push_back(rs);
                }
            } while (stmt->getMoreResults());
        }
        
        return results;
    }
    
    // 执行事务
    template<typename Func>
    auto executeTransaction(Func func) -> decltype(func(std::declval<sql::Connection*>())) {
        auto connWrapper = getConnection();
        auto conn = connWrapper->getConnection();
        
        // 存储原始自动提交状态
        bool autoCommit = conn->getAutoCommit();
        
        try {
            // 开始事务
            conn->setAutoCommit(false);
            
            // 执行事务函数
            auto result = func(conn);
            
            // 提交事务
            conn->commit();
            
            // 恢复原始自动提交状态
            conn->setAutoCommit(autoCommit);
            
            return result;
        } catch (...) {
            // 回滚事务
            try {
                conn->rollback();
            } catch (...) {
                // 忽略回滚过程中的异常
            }
            
            // 恢复原始自动提交状态
            try {
                conn->setAutoCommit(autoCommit);
            } catch (...) {
                // 忽略状态恢复过程中的异常
            }
            
            // 重新抛出异常
            throw;
        }
    }
}; 