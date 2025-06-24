#include "LoginDialog.h"
#include "HttpMgr.h"
#include <QMessageBox>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QColor>
#include <QApplication>
#include <QScreen>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QFont>
#include "UserManager.h"

LoginDialog::LoginDialog(QWidget *parent)
	: QDialog(parent), _isMousePressed(false)
{
	ui.setupUi(this);
	
	// 设置窗口属性
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);
	setFixedSize(480, 560);  // 增大窗口尺寸
	
	// 清除UI文件中的布局
	if (ui.verticalLayout_2) {
		delete ui.verticalLayout_2;
	}
	
	// 创建内容背景控件
	QWidget* contentBackground = new QWidget(this);
	contentBackground->setObjectName("contentBackground");
	contentBackground->setStyleSheet(
		"QWidget#contentBackground {"
		"    background-color: white;"
		"    border-radius: 12px;"
		"}"
	);
	
	// 创建标题文本
	QLabel* titleLabel = new QLabel("Telegram", contentBackground);
	titleLabel->setStyleSheet(
		"QLabel {"
		"    color: #2AABEE;"
		"    font-size: 32px;"  // 增大字体
		"    font-weight: bold;"
		"}"
	);
	titleLabel->setAlignment(Qt::AlignCenter);
	
	QLabel* subtitleLabel = new QLabel("登录到 Telegram", contentBackground);
	subtitleLabel->setStyleSheet(
		"QLabel {"
		"    color: #222222;"
		"    font-size: 17px;"  // 增大字体
		"}"
	);
	subtitleLabel->setAlignment(Qt::AlignCenter);
	
	QLabel* descLabel = new QLabel("请输入您的账号信息", contentBackground);
	descLabel->setStyleSheet(
		"QLabel {"
		"    color: #999999;"
		"    font-size: 14px;"  // 增大字体
		"}"
	);
	descLabel->setAlignment(Qt::AlignCenter);
	
	// 初始化Material Design控件
	usernameField = new QtMaterialTextField(contentBackground);
	passwordField = new QtMaterialTextField(contentBackground);
	loginButton = new QtMaterialRaisedButton(contentBackground);
	registerButton = new QtMaterialFlatButton(contentBackground);
	statusLabel = new QLabel(contentBackground);
	
	// 设置控件属性
	usernameField->setPlaceholderText("请输入手机号或账号");
	usernameField->setTextColor(QColor(34, 34, 34));
	usernameField->setInkColor(QColor(42, 171, 238));
	usernameField->setFixedWidth(360);
	usernameField->setFixedHeight(45);
	usernameField->setShowLabel(false);
	usernameField->setUseThemeColors(false);
	usernameField->setFont(QFont("Microsoft YaHei", 11));
	
	// 设置样式，包括文本居中
	QString textFieldStyle = QString(
		"QtMaterialTextField {"
		"    qproperty-alignment: AlignCenter;"
		"}"
		"QLineEdit {"
		"    text-align: center;"
		"}"
		"QtMaterialTextField > QLineEdit {"
		"    text-align: center;"
		"}"
	);
	usernameField->setStyleSheet(textFieldStyle);
	
	passwordField->setPlaceholderText("请输入密码");
	passwordField->setEchoMode(QLineEdit::Password);
	passwordField->setTextColor(QColor(34, 34, 34));
	passwordField->setInkColor(QColor(42, 171, 238));
	passwordField->setFixedWidth(360);
	passwordField->setFixedHeight(45);
	passwordField->setShowLabel(false);
	passwordField->setUseThemeColors(false);
	passwordField->setFont(QFont("Microsoft YaHei", 11));
	passwordField->setStyleSheet(textFieldStyle);
	
	// 创建一个全局样式表
	QString globalStyle = QString(
		"QLineEdit {"
		"    text-align: center;"
		"}"
		"QtMaterialTextField {"
		"    qproperty-alignment: AlignCenter;"
		"}"
		"QtMaterialTextField QLineEdit {"
		"    text-align: center;"
		"}"
	);
	qApp->setStyleSheet(qApp->styleSheet() + globalStyle);
	
	loginButton->setText("登录");
	loginButton->setBackgroundColor(QColor(42, 171, 238));
	loginButton->setForegroundColor(Qt::white);
	loginButton->setFixedWidth(360);
	loginButton->setFixedHeight(45);
	loginButton->setHaloVisible(false);
	loginButton->setOverlayStyle(Material::TintedOverlay);
	loginButton->setRippleStyle(Material::CenteredRipple);
	loginButton->setCornerRadius(8);
	loginButton->setFont(QFont("Microsoft YaHei", 11));
	// 设置按钮文本居中
	loginButton->setTextAlignment(Qt::AlignCenter);
	loginButton->setStyleSheet(
		"QtMaterialRaisedButton {"
		"    text-align: center;"
		"    padding: 0px;"
		"}"
	);
	
	registerButton->setText("创建新账号");
	registerButton->setForegroundColor(QColor(42, 171, 238));
	registerButton->setFixedWidth(360);
	registerButton->setFixedHeight(45);
	registerButton->setHaloVisible(false);
	registerButton->setRippleStyle(Material::NoRipple);
	registerButton->setFont(QFont("Microsoft YaHei", 11));
	// 设置按钮文本居中
	registerButton->setTextAlignment(Qt::AlignCenter);
	registerButton->setStyleSheet(
		"QtMaterialFlatButton {"
		"    text-align: center;"
		"    padding: 0px;"
		"}"
	);
	
	statusLabel->setAlignment(Qt::AlignCenter);
	statusLabel->setStyleSheet(
		"QLabel {"
		"    color: #DE5347;"
		"    font-size: 13px;"
		"}"
	);
	statusLabel->setFixedHeight(20);
	
	// 内容布局
	QVBoxLayout* contentLayout = new QVBoxLayout(contentBackground);
	contentLayout->setContentsMargins(40, 40, 40, 40);  // 统一使用40的边距
	contentLayout->setSpacing(0);
	
	// 添加标题部分
	contentLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);
	contentLayout->addSpacing(8);
	contentLayout->addWidget(subtitleLabel, 0, Qt::AlignHCenter);
	contentLayout->addSpacing(6);
	contentLayout->addWidget(descLabel, 0, Qt::AlignHCenter);
	contentLayout->addSpacing(50);
	
	// Username Row - 确保完全对称
	QHBoxLayout* usernameLayout = new QHBoxLayout();
	usernameLayout->setContentsMargins(0, 0, 0, 0);  // 移除内边距
	usernameLayout->addStretch(1); // 左侧弹性空间
	usernameLayout->addWidget(usernameField);
	usernameLayout->addStretch(1); // 右侧弹性空间，确保比例相同
	contentLayout->addLayout(usernameLayout);
	contentLayout->addSpacing(25);
	
	// Password Row - 确保完全对称
	QHBoxLayout* passwordLayout = new QHBoxLayout();
	passwordLayout->setContentsMargins(0, 0, 0, 0);  // 移除内边距
	passwordLayout->addStretch(1);
	passwordLayout->addWidget(passwordField);
	passwordLayout->addStretch(1);
	contentLayout->addLayout(passwordLayout);
	contentLayout->addSpacing(10);
	
	// Status Label
	contentLayout->addWidget(statusLabel, 0, Qt::AlignHCenter);
	contentLayout->addSpacing(30);
	
	// Login Button Row - 确保完全对称
	QHBoxLayout* loginButtonLayout = new QHBoxLayout();
	loginButtonLayout->setContentsMargins(0, 0, 0, 0);  // 移除内边距
	loginButtonLayout->addStretch(1);
	loginButtonLayout->addWidget(loginButton);
	loginButtonLayout->addStretch(1);
	contentLayout->addLayout(loginButtonLayout);
	contentLayout->addSpacing(15);
	
	// Register Button Row - 确保完全对称
	QHBoxLayout* registerButtonLayout = new QHBoxLayout();
	registerButtonLayout->setContentsMargins(0, 0, 0, 0);  // 移除内边距
	registerButtonLayout->addStretch(1);
	registerButtonLayout->addWidget(registerButton);
	registerButtonLayout->addStretch(1);
	contentLayout->addLayout(registerButtonLayout);
	
	// 主窗口布局 - 简化边距
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(20, 20, 20, 20);  // 减小外边距
	mainLayout->addWidget(contentBackground);
	
	// 设置窗口阴影效果
	QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect(this);
	shadowEffect->setBlurRadius(20);
	shadowEffect->setColor(QColor(0, 0, 0, 40));
	shadowEffect->setOffset(0, 0);
	contentBackground->setGraphicsEffect(shadowEffect);
	
	// 设置窗口居中
	QScreen *screen = QGuiApplication::primaryScreen();
	if (screen) {
		QRect screenGeometry = screen->geometry();
		int x = (screenGeometry.width() - width()) / 2;
		int y = (screenGeometry.height() - height()) / 2;
		move(x, y);
	}
	
	// 连接信号和槽
	connect(loginButton, &QtMaterialRaisedButton::clicked, this, &LoginDialog::onLoginButtonClicked);
	connect(registerButton, &QtMaterialFlatButton::clicked, this, &LoginDialog::switchRegiser);
	
	// 添加输入框回车键响应
	connect(usernameField, &QtMaterialTextField::returnPressed, [this]() {
		passwordField->setFocus();
	});
	connect(passwordField, &QtMaterialTextField::returnPressed, this, &LoginDialog::onLoginButtonClicked);
	
	// 连接HttpMgr的信号
	connect(HttpMgr::GetInstance().get(), &HttpMgr::requestFinished,
			this, [this](ReqId reqId, const QJsonObject& response, ErrorCodes error, Modules module) {
				if (reqId == ReqId::Login) {
					if (error == ErrorCodes::Success) {
						if (response.contains("userInfo")) {
							QJsonObject userInfo = response["userInfo"].toObject();
							
							// 创建用户信息对象
							UserInfo user;
							user.userId = QString::number(userInfo["userId"].toInt());
							user.username = userInfo["username"].toString();
							user.nickname = userInfo["nickname"].toString();
							user.avatar = userInfo["avatar"].toString();
							user.isOnline = true;
							
							// 存储用户信息到UserManager
							UserManager::GetInstance()->setCurrentUser(user);
						}
						emit loginSuccess();
					} else {
						QString errorMsg;
						switch (error) {
							case ErrorCodes::UserNotExist:
							case static_cast<ErrorCodes>(3001):  // USER_NOT_FOUND
								errorMsg = "用户不存在";
								break;
							case ErrorCodes::PasswordError:
							case static_cast<ErrorCodes>(3003):  // USER_INVALID_PASSWORD
								errorMsg = "密码错误";
								break;
							case static_cast<ErrorCodes>(3004):  // USER_LOGIN_FAILED
								errorMsg = "登录失败";
								break;
							default:
								errorMsg = "登录失败: " + QString::number(static_cast<int>(error));
								break;
						}
						statusLabel->setText(errorMsg);
						loginButton->setEnabled(true);
						
						// 添加抖动动画
						QPropertyAnimation* animation = new QPropertyAnimation(statusLabel, "pos");
						animation->setDuration(100);
						QPoint origin = statusLabel->pos();
						animation->setKeyValueAt(0, origin);
						animation->setKeyValueAt(0.25, origin + QPoint(5, 0));
						animation->setKeyValueAt(0.75, origin - QPoint(5, 0));
						animation->setKeyValueAt(1, origin);
						animation->start(QAbstractAnimation::DeleteWhenStopped);
					}
				}
			});
	
	connect(HttpMgr::GetInstance().get(), &HttpMgr::networkError,
			this, [this](ReqId reqId, const QString& errorString, Modules module) {
				if (reqId == ReqId::Login) {
					statusLabel->setText("网络错误: " + errorString);
					loginButton->setEnabled(true);
				}
			});
			
	// 设置初始焦点
	usernameField->setFocus();
}

LoginDialog::~LoginDialog()
{}

void LoginDialog::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		_isMousePressed = true;
		_mousePos = event->globalPosition().toPoint() - pos();
	}
}

void LoginDialog::mouseReleaseEvent(QMouseEvent* event)
{
	_isMousePressed = false;
}

void LoginDialog::mouseMoveEvent(QMouseEvent* event)
{
	if (_isMousePressed && (event->buttons() & Qt::LeftButton)) {
		move(event->globalPosition().toPoint() - _mousePos);
	}
}

void LoginDialog::onLoginButtonClicked()
{
	// 清除之前的错误信息
	statusLabel->clear();
	
	// 获取输入内容
	QString username = usernameField->text().trimmed();
	QString password = passwordField->text();
	
	// 验证输入
	if (username.isEmpty()) {
		statusLabel->setText("请输入用户名");
		return;
	}
	
	if (password.isEmpty()) {
		statusLabel->setText("请输入密码");
		return;
	}
	
	// 禁用登录按钮，防止重复提交
	loginButton->setEnabled(false);
	statusLabel->setText("正在登录...");
	
	// 调用HttpMgr进行登录
	HttpMgr::GetInstance()->login(username, password);
}
