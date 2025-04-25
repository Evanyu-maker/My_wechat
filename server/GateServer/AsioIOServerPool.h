#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include"const.h"
#include<vector>
#include"Singleton.h"
class AsioIOServerPool:public Singleton<AsioIOServerPool>{
public:
	friend class Singleton<AsioIOServerPool>;
	
public:
	using IOService = boost::asio::io_context;
	using WorkGuard = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
	using WorkPtr = std::unique_ptr<WorkGuard>;
	
	~AsioIOServerPool();
	
	AsioIOServerPool(const AsioIOServerPool&) = delete;
	AsioIOServerPool& operator=(AsioIOServerPool&) = delete;
	
	// 使用 round-robin 的方式返回一个 io_service
	boost::asio::io_context& GetIOService();

	void stop();
private:
	AsioIOServerPool(std::size_t = 2);
	std::vector<IOService> _ioServices;
	std::vector<WorkPtr> _works;
	std::vector<std::thread> _threads;
	std::size_t  _nextIOService;
};