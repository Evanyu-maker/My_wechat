#include "AsioIOServerPool.h"

AsioIOServerPool::AsioIOServerPool(std::size_t size):_ioServices(size),_works(size),_nextIOService(0)
{
	for (std::size_t i = 0; i < size; i++)
	{
		auto guard = boost::asio::make_work_guard(_ioServices[i]);
		_works[i] = std::make_unique<WorkGuard>(std::move(guard));
	}

	for (std::size_t i = 0; i < _ioServices.size(); i++)
	{
		_threads.emplace_back([this,i]() {
			_ioServices[i].run();
		});
	}
}
AsioIOServerPool::~AsioIOServerPool()
{
	stop();
	for (auto& thread : _threads)
	{
		if (thread.joinable())
		{
			thread.join();
		}
	}
}
boost::asio::io_context& AsioIOServerPool::GetIOService() {
	// round-robin
	auto& service = _ioServices[_nextIOService++];
	if (_nextIOService == _ioServices.size()) {
		_nextIOService = 0;
	}
	return service;
}

void AsioIOServerPool::stop() {
	_works.clear(); // 销毁所有 work 对象

	// 等待线程退出
	for (auto& t : _threads) {
		if (t.joinable()) t.join();
	}
}