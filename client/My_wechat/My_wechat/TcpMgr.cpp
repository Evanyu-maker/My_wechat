// TCPMgr.cpp
#include "TCPMgr.h"
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include "UserManager.h"
// ���峣��
const uint16_t MAGIC_NUMBER = 0xCAFE;
const int HEARTBEAT_INTERVAL = 30000;  // 30��һ������

// �����붨��
enum CommandCode {
    CMD_HEARTBEAT = 0x0001,
    CMD_CHAT_MESSAGE = 0x0002,
    CMD_USER_STATUS = 0x0003,
    // ����չ��������...
};

TCPMgr::TCPMgr() : m_socket(nullptr), m_heartbeatTimer(nullptr), m_connected(false)
{
    m_socket = new QTcpSocket(this);

    // �����źŲ�
    connect(m_socket, &QTcpSocket::connected, this, &TCPMgr::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TCPMgr::onDisconnected);
    connect(m_socket, &QAbstractSocket::errorOccurred,
        [this](QAbstractSocket::SocketError error) {
        this->onError(error);
    });
    connect(m_socket, &QTcpSocket::readyRead, this, &TCPMgr::onReadyRead);

    // ������ʱ��
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
    return m_socket->waitForConnected(5000);  // �ȴ����5��
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

    // ����Ҫ���͵���������
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    // д���ͷ
    stream << header.magic << header.cmd << header.length;

    // д������
    if (!data.isEmpty()) {
        packet.append(data);
    }

    // ����
    qint64 written = m_socket->write(packet);

    return written == packet.size();
}

bool TCPMgr::sendChatMessage(const QString& toUser, const QString& content)
{
    // ����JSON����
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
    // ����������
    sendMessage(CMD_HEARTBEAT, QByteArray());
}

void TCPMgr::parseReceivedData()
{
    while (m_receiveBuffer.size() >= sizeof(PacketHeader)) {
        // ��ȡ��ͷ
        PacketHeader header;
        QDataStream stream(m_receiveBuffer);
        stream.setByteOrder(QDataStream::BigEndian);

        stream >> header.magic >> header.cmd >> header.length;

        // ���ħ��
        if (header.magic != MAGIC_NUMBER) {
            qDebug() << "Invalid packet magic number";
            m_receiveBuffer.clear();
            return;
        }

        // ����Ƿ���յ�һ�������İ�
        int packetSize = sizeof(PacketHeader) + header.length;
        if (m_receiveBuffer.size() < packetSize) {
            // ���ݲ��㣬�ȴ���������
            return;
        }

        // ��ȡ���ݲ���
        QByteArray packetData = m_receiveBuffer.mid(sizeof(PacketHeader), header.length);

        // ���������
        Packet packet;
        packet.header = header;
        packet.data = packetData;
        processPacket(packet);

        // �Ƴ��Ѵ��������
        m_receiveBuffer.remove(0, packetSize);
    }
}

void TCPMgr::processPacket(const Packet& packet)
{
    // ���ȴ���ͨ���ź�
    emit messageReceived(packet.header.cmd, packet.data);

    // ���������봦���ض�ҵ���߼�
    switch (packet.header.cmd) {
    case CMD_HEARTBEAT:
        // �����������账��
        break;

    case CMD_CHAT_MESSAGE:
    {
        // ����������Ϣ
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
        // �����û�״̬���
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