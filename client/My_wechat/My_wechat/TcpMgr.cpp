// TCPMgr.cpp
#include "TCPMgr.h"
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include "UserManager.h"
// 定义常量
const uint16_t MAGIC_NUMBER = 0xCAFE;
const int HEARTBEAT_INTERVAL = 30000;  // 30秒一次心跳

// 命令码定义
enum CommandCode {
    CMD_HEARTBEAT = 0x0001,
    CMD_CHAT_MESSAGE = 0x0002,
    CMD_USER_STATUS = 0x0003,
    // 可扩展更多命令...
};

TCPMgr::TCPMgr() : m_socket(nullptr), m_heartbeatTimer(nullptr), m_connected(false)
{
    m_socket = new QTcpSocket(this);

    // 连接信号槽
    connect(m_socket, &QTcpSocket::connected, this, &TCPMgr::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TCPMgr::onDisconnected);
    connect(m_socket, &QAbstractSocket::errorOccurred,
        [this](QAbstractSocket::SocketError error) {
        this->onError(error);
    });
    connect(m_socket, &QTcpSocket::readyRead, this, &TCPMgr::onReadyRead);

    // 心跳定时器
    m_heartbeatTimer = new QTimer(this);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &TCPMgr::onHeartbeatTimer);
}

TCPMgr::~TCPMgr()
{
    if (m_socket) {
        m_socket->abort();
        delete m_socket;
    }

    if (m_heartbeatTimer) {
        m_heartbeatTimer->stop();
        delete m_heartbeatTimer;
    }
}

bool TCPMgr::connectToServer(const QString& host, quint16 port)
{
    if (m_connected) {
        disconnect();
    }

    m_socket->connectToHost(host, port);
    return m_socket->waitForConnected(5000);  // 等待最多5秒
}

void TCPMgr::disconnect()
{
    if (m_connected) {
        m_heartbeatTimer->stop();
        m_socket->disconnectFromHost();
    }
}

bool TCPMgr::sendMessage(uint16_t cmd, const QByteArray& data)
{
    if (!m_connected || !m_socket) {
        qDebug() << "Cannot send message: not connected";
        return false;
    }

    QMutexLocker locker(&m_sendMutex);

    PacketHeader header;
    header.magic = MAGIC_NUMBER;
    header.cmd = cmd;
    header.length = data.size();

    // 构建要发送的完整数据
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    // 写入包头
    stream << header.magic << header.cmd << header.length;

    // 写入数据
    if (!data.isEmpty()) {
        packet.append(data);
    }

    // 发送
    qint64 written = m_socket->write(packet);

    return written == packet.size();
}

bool TCPMgr::sendChatMessage(const QString& toUser, const QString& content)
{
    // 创建JSON数据
    QJsonObject json;
    json["from"] = UserManager::GetInstance()->getCurrentUser().userId;
    json["to"] = toUser;
    json["content"] = content;
    json["time"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    return sendMessage(CMD_CHAT_MESSAGE, data);
}

bool TCPMgr::isConnected() const
{
    return m_connected;
}

void TCPMgr::onConnected()
{
    m_connected = true;
    m_receiveBuffer.clear();
    m_heartbeatTimer->start(HEARTBEAT_INTERVAL);
    emit connected();

    qDebug() << "Connected to chat server";
}

void TCPMgr::onDisconnected()
{
    m_connected = false;
    m_heartbeatTimer->stop();
    emit disconnected();

    qDebug() << "Disconnected from chat server";
}

void TCPMgr::onError(QAbstractSocket::SocketError error)
{
    m_connected = false;
    emit connectionError(error);

    qDebug() << "Socket error:" << m_socket->errorString();
}

void TCPMgr::onReadyRead()
{
    m_receiveBuffer.append(m_socket->readAll());
    parseReceivedData();
}

void TCPMgr::onHeartbeatTimer()
{
    // 发送心跳包
    sendMessage(CMD_HEARTBEAT, QByteArray());
}

void TCPMgr::parseReceivedData()
{
    while (m_receiveBuffer.size() >= sizeof(PacketHeader)) {
        // 读取包头
        PacketHeader header;
        QDataStream stream(m_receiveBuffer);
        stream.setByteOrder(QDataStream::BigEndian);

        stream >> header.magic >> header.cmd >> header.length;

        // 检查魔数
        if (header.magic != MAGIC_NUMBER) {
            qDebug() << "Invalid packet magic number";
            m_receiveBuffer.clear();
            return;
        }

        // 检查是否接收到一个完整的包
        int packetSize = sizeof(PacketHeader) + header.length;
        if (m_receiveBuffer.size() < packetSize) {
            // 数据不足，等待更多数据
            return;
        }

        // 提取数据部分
        QByteArray packetData = m_receiveBuffer.mid(sizeof(PacketHeader), header.length);

        // 处理这个包
        Packet packet;
        packet.header = header;
        packet.data = packetData;
        processPacket(packet);

        // 移除已处理的数据
        m_receiveBuffer.remove(0, packetSize);
    }
}

void TCPMgr::processPacket(const Packet& packet)
{
    // 首先触发通用信号
    emit messageReceived(packet.header.cmd, packet.data);

    // 根据命令码处理特定业务逻辑
    switch (packet.header.cmd) {
    case CMD_HEARTBEAT:
        // 心跳包，无需处理
        break;

    case CMD_CHAT_MESSAGE:
    {
        // 解析聊天消息
        QJsonDocument doc = QJsonDocument::fromJson(packet.data);
        if (doc.isObject()) {
            QJsonObject json = doc.object();
            QString fromUser = json["from"].toString();
            QString content = json["content"].toString();
            QDateTime time = QDateTime::fromString(json["time"].toString(), Qt::ISODate);

            emit chatMessageReceived(fromUser, content, time);
        }
    }
    break;

    case CMD_USER_STATUS:
    {
        // 解析用户状态变更
        QJsonDocument doc = QJsonDocument::fromJson(packet.data);
        if (doc.isObject()) {
            QJsonObject json = doc.object();
            QString user = json["user"].toString();
            bool online = json["online"].toBool();

            emit userStatusChanged(user, online);
        }
    }
    break;

    default:
        qDebug() << "Unknown command code:" << packet.header.cmd;
        break;
    }
}