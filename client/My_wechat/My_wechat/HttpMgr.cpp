#include "HttpMgr.h"
#include <QNetworkRequest>
#include <QUrlQuery>

HttpMgr::HttpMgr() {
	// 注册元类型，使其可以在信号槽中使用
	qRegisterMetaType<ReqId>("ReqId");
	qRegisterMetaType<Modules>("Modules");
	qRegisterMetaType<ErrorCodes>("ErrorCodes");
	
	connect(&_manager, &QNetworkAccessManager::finished,
			this, [this](QNetworkReply* reply) {
				ReqId reqId = reply->property("reqId").value<ReqId>();
				Modules module = reply->property("module").value<Modules>();
				handleResponse(reply, reqId, module);
			});
}

HttpMgr::~HttpMgr() {
	
}

void HttpMgr::registerUser(const QString& username, const QString& password, const QString& email) {
	QJsonObject data;
	data["username"] = username;
	data["password"] = password;
	data["email"] = email;
	
	postRequest("/register", data, ReqId::Register, Modules::User);
}

void HttpMgr::login(const QString& username, const QString& password) {
	QJsonObject data;
	data["username"] = username;
	data["password"] = password;
	
	postRequest("/login", data, ReqId::Login, Modules::User);
}

void HttpMgr::getVerifyCode(const QString& email) {
	QJsonObject data;
	data["email"] = email;
	
	postRequest("/get_varifycode", data, ReqId::GetVerifyCode, Modules::User);
}

void HttpMgr::updateUserInfo(const QString& userId, const QJsonObject& userInfo) {
	QJsonObject data = userInfo;
	data["userId"] = userId;
	
	postRequest("/update_user_info", data, ReqId::UpdateUserInfo, Modules::User);
}

void HttpMgr::getUserInfo(const QString& userId) {
	QMap<QString, QString> params;
	params["userId"] = userId;
	
	getRequest("/get_user_info", params, ReqId::GetUserInfo, Modules::User);
}

void HttpMgr::postRequest(const QString& path, const QJsonObject& data, ReqId reqId, Modules module) {
	QUrl url(SERVER_URL + path);
	QNetworkRequest request(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	
	// 将数据转换为JSON文档
	QJsonDocument doc(data);
	QByteArray jsonData = doc.toJson();
	
	// 创建请求
	QNetworkReply* reply = _manager.post(request, jsonData);
	reply->setProperty("reqId", QVariant::fromValue(reqId));
	reply->setProperty("module", QVariant::fromValue(module));
	
	// 连接错误信号
	connect(reply, &QNetworkReply::errorOccurred,
			this, &HttpMgr::onNetworkError);
}

void HttpMgr::getRequest(const QString& path, const QMap<QString, QString>& params, ReqId reqId, Modules module) {
	QUrl url(SERVER_URL + path);
	QUrlQuery query;
	
	// 添加查询参数
	for (auto it = params.begin(); it != params.end(); ++it) {
		query.addQueryItem(it.key(), it.value());
	}
	url.setQuery(query);
	
	QNetworkRequest request(url);
	QNetworkReply* reply = _manager.get(request);
	reply->setProperty("reqId", QVariant::fromValue(reqId));
	reply->setProperty("module", QVariant::fromValue(module));
	
	// 连接错误信号
	connect(reply, &QNetworkReply::errorOccurred,
			this, &HttpMgr::onNetworkError);
}

void HttpMgr::handleResponse(QNetworkReply* reply, ReqId reqId, Modules module) {
	reply->deleteLater();
	
	if (reply->error() != QNetworkReply::NoError) {
		emit networkError(reqId, reply->errorString(), module);
		return;
	}
	
	// 读取响应数据
	QByteArray responseData = reply->readAll();
	QJsonDocument doc = QJsonDocument::fromJson(responseData);
	
	if (doc.isNull()) {
		emit networkError(reqId, "Invalid JSON response", module);
		return;
	}
	
	QJsonObject response = doc.object();
	
	// 检查错误码
	int errorCode = response["error"].toInt(static_cast<int>(ErrorCodes::JsonError));
	emit requestFinished(reqId, response, static_cast<ErrorCodes>(errorCode), module);
}

void HttpMgr::onNetworkError(QNetworkReply::NetworkError error) {
	QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
	if (!reply) return;
	
	ReqId reqId = reply->property("reqId").value<ReqId>();
	Modules module = reply->property("module").value<Modules>();
	
	emit networkError(reqId, reply->errorString(), module);
}
