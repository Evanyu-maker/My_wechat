// TCPMgr.h
#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QByteArray>
#include <QMutex>
#include <queue>
#include "Singleton.h"
#include "global.h"

// 数据包头结构
struct PacketHeader {
    uint16_t magic;    // 魔数，固定为0xCAFE
    uint16_t cmd;      // 命令码
    uint32_t length;   // 数据长度
};

// 消息包完整结构
struct Packet {
    PacketHeader header;
    QByteArray data;
};

// TCP管理类
class TCPMgr : public QObject, public Singleton<TCPMgr>
{
    Q_OBJECT
        friend class Singleton<TCPMgr>;

public:
    // 连接到服务器
    bool connectToServer(const QString& host, quint16 port);

    // 断开连接
    void disconnect();

    // 发送消息
    bool sendMessage(uint16_t cmd, const QByteArray& data);

    // 发送聊天消息
    bool sendChatMessage(const QString& toUser, const QString& content);

    // 获取连接状态
    bool isConnected() const;

signals:
    // 连接状态信号
    void connected();
    void disconnected();
    void connectionError(QAbstractSocket::SocketError error);

    // 消息接收信号
    void messageReceived(uint16_t cmd, const QByteArray& data);

    // 特定业务信号
    void chatMessageReceived(const QString& fromUser, const QString& content, QDateTime time);
    void userStatusChanged(const QString& user, bool online);

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onReadyRead();
    void onHeartbeatTimer();

private:
    TCPMgr();
    ~TCPMgr();

    // 处理接收到的完整数据包
    void processPacket(const Packet& packet);

    // 解析接收缓冲区中的数据包
    void parseReceivedData();

private:
    QTcpSocket* m_socket;
    QTimer* m_heartbeatTimer;
    QByteArray m_receiveBuffer;
    bool m_connected;
    QMutex m_sendMutex;
};

#define g_tcpMgr TCPMgr::instance()