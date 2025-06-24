#pragma once
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include<memory>
#include<iostream>
#include<map>
#include<functional>
#include<unordered_map>
#include<json/json.h>
#include<json/value.h>
#include<json/reader.h>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include"ConfigMgr.h" 
#include<queue>
#include<hiredis.h>
#include <atomic>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include<unordered_set>


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

enum ErrorCodes {
	Success = 0,
	Error_Json = 1001,//Json解析错误
	RPCFailed = 1002,//RPC调用失败
	InvalidParams = 1003, // 无效参数
	TokenInvalid = 1004, // 验证码或令牌无效
	
	// 数据库错误码 (2000-2999)
	DB_ERROR_BASE = 2000,
	DB_CONNECTION_FAILED = 2001,   // 数据库连接失败
	DB_QUERY_FAILED = 2002,        // 查询执行失败
	DB_TRANSACTION_FAILED = 2003,  // 事务执行失败
	DB_PROCEDURE_FAILED = 2004,    // 存储过程调用失败
	DB_CONNECTION_TIMEOUT = 2005,  // 连接获取超时
	DB_POOL_INIT_FAILED = 2006,    // 连接池初始化失败
	
	// 用户相关错误码 (3000-3999)
	USER_ERROR_BASE = 3000,
	USER_NOT_FOUND = 3001,         // 用户不存在
	USER_ALREADY_EXISTS = 3002,    // 用户已存在
	USER_INVALID_PASSWORD = 3003,  // 密码错误
	USER_LOGIN_FAILED = 3004,      // 登录失败
	USER_REGISTER_FAILED = 3005,   // 注册失败
	USER_UPDATE_FAILED = 3006,     // 用户信息更新失败
	
	// 好友相关错误码 (4000-4999)
	FRIEND_ERROR_BASE = 4000,
	FRIEND_NOT_FOUND = 4001,       // 好友不存在
	FRIEND_ALREADY_EXISTS = 4002,  // 已经是好友
	FRIEND_ADD_FAILED = 4003,      // 添加好友失败
	FRIEND_REMOVE_FAILED = 4004,   // 删除好友失败
	FRIEND_LIST_FAILED = 4005,     // 获取好友列表失败
};

// 数据库连接状态
enum class ConnectionState {
    IDLE,       // 空闲
    IN_USE,     // 使用中
    BROKEN,     // 连接断开
    EXPIRED     // 过期
};

// 数据库配置项名称常量
const char* const MYSQL_CONFIG_SECTION = "Mysql";
const char* const MYSQL_HOST_KEY = "Host";
const char* const MYSQL_PORT_KEY = "Port";
const char* const MYSQL_USER_KEY = "User";
const char* const MYSQL_PASSWD_KEY = "Passwd";
const char* const MYSQL_SCHEMA_KEY = "Schema";

// 数据库连接池默认配置
const int DB_DEFAULT_INITIAL_SIZE = 5;
const int DB_DEFAULT_MAX_SIZE = 20;
const int DB_DEFAULT_MIN_SIZE = 5;
const int DB_DEFAULT_MAX_IDLE_TIME = 60;
const int DB_DEFAULT_TIMEOUT = 30;
const int DB_DEFAULT_VALIDATION_INTERVAL = 30;
const int DB_DEFAULT_EVICTION_INTERVAL = 30;
const int DB_DEFAULT_MAX_WAIT_QUEUE_SIZE = 1000;

class ConfigMgr;
