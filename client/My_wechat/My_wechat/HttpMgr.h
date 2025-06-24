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
 * HTTP请求管理器类
 * 使用单例模式和CRTP技术实现
 */
class HttpMgr : public QObject, public Singleton<HttpMgr>
{
	Q_OBJECT

public:
	~HttpMgr();

	// 用户相关请求
	void registerUser(const QString& username, const QString& password, const QString& email, const QString& verifyCode);
	void login(const QString& username, const QString& password);
	void getVerifyCode(const QString& email);
	void updateUserInfo(const QString& userId, const QJsonObject& userInfo);
	void getUserInfo(const QString& userId);

	// 获取服务器地址
	QString getServerUrl() const { return SERVER_URL; }

private:
	//单例模式，为了能够在Singleton中使用，所以需要加上友元。
	friend class Singleton<HttpMgr>;
	HttpMgr();

	QNetworkAccessManager _manager;
	
	// 发送POST请求
	void postRequest(const QString& path, const QJsonObject& data, ReqId reqId, Modules module);
	// 发送GET请求
	void getRequest(const QString& path, const QMap<QString, QString>& params, ReqId reqId, Modules module);
	// 处理请求响应
	void handleResponse(QNetworkReply* reply, ReqId reqId, Modules module);

private slots:
	void onNetworkError(QNetworkReply::NetworkError error);

signals:
	// 请求完成信号
	void requestFinished(ReqId reqId, const QJsonObject& response, ErrorCodes error, Modules module);
	// 网络错误信号
	void networkError(ReqId reqId, const QString& errorString, Modules module);
};

