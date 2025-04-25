#pragma once
#include"const.h"
#include"LogicSystem.h"
class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
	friend class LogicSystem;
public:
	using Socket = tcp::socket;
	HttpConnection(tcp::socket socket);
	void Start();
	Socket& GetSocket();
private:
	void CheckDeadline();
	void WriteResponse();
	void HandleReq();
	void PreParseGetParam();
	
	Socket _socket;

	std::unordered_map<std::string, std::string> _get_params;
	beast::flat_buffer _buffer{ 8192 };
	//http::dynamic_body 是boost.beast提供的一种动态请求体类型。
	//用来解析请求。
	http::request<http::dynamic_body> _request;

	http::response<http::dynamic_body> _response;
	
	net::steady_timer deadline_{
		_socket.get_executor(),
		std::chrono::seconds(60)
	};

	std::string _get_url;
};

