#include"CServer.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short& port)
	: _acceptor(ioc, tcp::endpoint(tcp::v4(), port)), _ioc(ioc), _socket(ioc)
{
}

void CServer::Start() {
	auto self = shared_from_this();
	//从contextpool中获取一个io_context
	auto& io_context = AsioIOServerPool::GetInstance()->GetIOService();
	std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(tcp::socket(io_context));
	_acceptor.async_accept(new_con->GetSocket(), [self,new_con](beast::error_code ec) {
		try {
			if (ec)
			{
				self->Start();
				return;
			}
		
			new_con->Start();

			self->Start();
		}
		catch (std::exception& exp) {
			std::cout << "Error is" << ec.what() << std::endl;
			self->Start();
		}
	});
}
