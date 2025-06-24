#pragma once
#include "Singleton.h"
#include "global.h"
#include <QString>
#include <QUrl>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

/**
 * HTTP�����������
 * ʹ�õ���ģʽ��CRTP����ʵ��
 */
class HttpMgr : public QObject, public Singleton<HttpMgr>
{
	Q_OBJECT

public:
	~HttpMgr();

	// �û��������
	void registerUser(const QString& username, const QString& password, const QString& email, const QString& verifyCode);
	void login(const QString& username, const QString& password);
	void getVerifyCode(const QString& email);
	void updateUserInfo(const QString& userId, const QJsonObject& userInfo);
	void getUserInfo(const QString& userId);

	// ��ȡ��������ַ
	QString getServerUrl() const { return SERVER_URL; }

private:
	//����ģʽ��Ϊ���ܹ���Singleton��ʹ�ã�������Ҫ������Ԫ��
	friend class Singleton<HttpMgr>;
	HttpMgr();

	QNetworkAccessManager _manager;
	
	// ����POST����
	void postRequest(const QString& path, const QJsonObject& data, ReqId reqId, Modules module);
	// ����GET����
	void getRequest(const QString& path, const QMap<QString, QString>& params, ReqId reqId, Modules module);
	// ����������Ӧ
	void handleResponse(QNetworkReply* reply, ReqId reqId, Modules module);

private slots:
	void onNetworkError(QNetworkReply::NetworkError error);

signals:
	// ��������ź�
	void requestFinished(ReqId reqId, const QJsonObject& response, ErrorCodes error, Modules module);
	// ��������ź�
	void networkError(ReqId reqId, const QString& errorString, Modules module);
};

