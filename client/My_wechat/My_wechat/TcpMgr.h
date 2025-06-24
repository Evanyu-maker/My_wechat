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

// ���ݰ�ͷ�ṹ
struct PacketHeader {
    uint16_t magic;    // ħ�����̶�Ϊ0xCAFE
    uint16_t cmd;      // ������
    uint32_t length;   // ���ݳ���
};

// ��Ϣ�������ṹ
struct Packet {
    PacketHeader header;
    QByteArray data;
};

// TCP������
class TCPMgr : public QObject, public Singleton<TCPMgr>
{
    Q_OBJECT
        friend class Singleton<TCPMgr>;

public:
    // ���ӵ�������
    bool connectToServer(const QString& host, quint16 port);

    // �Ͽ�����
    void disconnect();

    // ������Ϣ
    bool sendMessage(uint16_t cmd, const QByteArray& data);

    // ����������Ϣ
    bool sendChatMessage(const QString& toUser, const QString& content);

    // ��ȡ����״̬
    bool isConnected() const;

signals:
    // ����״̬�ź�
    void connected();
    void disconnected();
    void connectionError(QAbstractSocket::SocketError error);

    // ��Ϣ�����ź�
    void messageReceived(uint16_t cmd, const QByteArray& data);

    // �ض�ҵ���ź�
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

    // ������յ����������ݰ�
    void processPacket(const Packet& packet);

    // �������ջ������е����ݰ�
    void parseReceivedData();

private:
    QTcpSocket* m_socket;
    QTimer* m_heartbeatTimer;
    QByteArray m_receiveBuffer;
    bool m_connected;
    QMutex m_sendMutex;
};

#define g_tcpMgr TCPMgr::instance()