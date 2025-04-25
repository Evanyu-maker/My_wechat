#pragma once
#include"const.h"
#include"Httpconnection.h"
#include"AsioIOServerPool.h"
//CTRLºº ı LOCP
class CServer :public std::enable_shared_from_this<CServer>
{
public:
    CServer(boost::asio::io_context& ioc, unsigned short& port);
    void Start();
private:
    tcp::acceptor  _acceptor;
    net::io_context& _ioc;
    boost::asio::ip::tcp::socket   _socket;
};