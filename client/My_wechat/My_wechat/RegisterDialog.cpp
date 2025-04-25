#include "RegisterDialog.h"
#include "HttpMgr.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QTimer>
#include <QMouseEvent>
#include <QRegularExpression>
#include <QScreen>
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <qtmaterialtextfield.h>
#include <qtmaterialraisedbutton.h>
#include <qtmaterialflatbutton.h>

RegisterDialog::RegisterDialog(QWidget *parent)
	: QDialog(parent), _isMousePressed(false)
{
	ui.setupUi(this);
	
	// 清除UI文件中的布局
	if (ui.gridLayout) {
		delete ui.gridLayout;
	}
	
	setObjectName("RegisterDialog");
	
	// 1. 设置窗口属性
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);
	setFixedSize(480, 640);  // 设置固定大小，比登录界面稍大一些

	// 2. 内容卡片
	QWidget* card = new QWidget(this);
	card->setObjectName("cardWidget");
	card->setStyleSheet(
		"#cardWidget {"
		"    background-color: white;"
		"    border-radius: 12px;"
		"}"
	);

	// 3. 卡片内部布局
	QVBoxLayout* cardLayout = new QVBoxLayout(card);
	cardLayout->setContentsMargins(40, 40, 40, 40);
	cardLayout->setSpacing(20);

	// 4. 设置标题
	QLabel* titleLabel = new QLabel("Telegram", card);
	titleLabel->setStyleSheet(
		"QLabel {"
		"    color: #2AABEE;"
		"    font-size: 32px;"
		"    font-weight: bold;"
		"}"
	);
	titleLabel->setAlignment(Qt::AlignCenter);
	
	QLabel* subtitleLabel = new QLabel("创建新账号", card);
	subtitleLabel->setStyleSheet(
		"QLabel {"
		"    color: #222222;"
		"    font-size: 17px;"
		"}"
	);
	subtitleLabel->setAlignment(Qt::AlignCenter);

	// 5. 设置输入框属性
	QString textFieldStyle = QString(
		"QtMaterialTextField {"
		"    qproperty-alignment: AlignCenter;"
		"    font-size: 14px;"
		"}"
		"QLineEdit {"
		"    text-align: center;"
		"}"
		"QtMaterialTextField > QLineEdit {"
		"    text-align: center;"
		"}"
	);

	usernameField = new QtMaterialTextField(card);
	usernameField->setLabel("用户名");
	usernameField->setPlaceholderText("请输入用户名");
	usernameField->setTextColor(QColor(34, 34, 34));
	usernameField->setInkColor(QColor(42, 171, 238));
	usernameField->setLabelColor(QColor(42, 171, 238));
	usernameField->setFixedWidth(360);
	usernameField->setFixedHeight(55);  // 增加高度以适应浮动标签
	usernameField->setShowLabel(true);  // 显示浮动标签
	usernameField->setUseThemeColors(false);
	usernameField->setFont(QFont("Microsoft YaHei", 11));
	usernameField->setStyleSheet(textFieldStyle);

	passwordField = new QtMaterialTextField(card);
	passwordField->setLabel("密码");
	passwordField->setPlaceholderText("请输入密码");
	passwordField->setEchoMode(QLineEdit::Password);
	passwordField->setTextColor(QColor(34, 34, 34));
	passwordField->setInkColor(QColor(42, 171, 238));
	passwordField->setLabelColor(QColor(42, 171, 238));
	passwordField->setFixedWidth(360);
	passwordField->setFixedHeight(55);
	passwordField->setShowLabel(true);
	passwordField->setUseThemeColors(false);
	passwordField->setFont(QFont("Microsoft YaHei", 11));
	passwordField->setStyleSheet(textFieldStyle);

	confirmPasswordField = new QtMaterialTextField(card);
	confirmPasswordField->setLabel("确认密码");
	confirmPasswordField->setPlaceholderText("请再次输入密码");
	confirmPasswordField->setEchoMode(QLineEdit::Password);
	confirmPasswordField->setTextColor(QColor(34, 34, 34));
	confirmPasswordField->setInkColor(QColor(42, 171, 238));
	confirmPasswordField->setLabelColor(QColor(42, 171, 238));
	confirmPasswordField->setFixedWidth(360);
	confirmPasswordField->setFixedHeight(55);
	confirmPasswordField->setShowLabel(true);
	confirmPasswordField->setUseThemeColors(false);
	confirmPasswordField->setFont(QFont("Microsoft YaHei", 11));
	confirmPasswordField->setStyleSheet(textFieldStyle);

	emailField = new QtMaterialTextField(card);
	emailField->setLabel("邮箱地址");
	emailField->setPlaceholderText("请输入邮箱地址");
	emailField->setTextColor(QColor(34, 34, 34));
	emailField->setInkColor(QColor(42, 171, 238));
	emailField->setLabelColor(QColor(42, 171, 238));
	emailField->setFixedWidth(360);
	emailField->setFixedHeight(55);
	emailField->setShowLabel(true);
	emailField->setUseThemeColors(false);
	emailField->setFont(QFont("Microsoft YaHei", 11));
	emailField->setStyleSheet(textFieldStyle);

	// 添加验证码输入框及按钮
	QHBoxLayout* verifyCodeLayout = new QHBoxLayout();
	
	// 验证码输入框
	QtMaterialTextField* verifyCodeField = new QtMaterialTextField(card);
	verifyCodeField->setLabel("验证码");
	verifyCodeField->setPlaceholderText("请输入验证码");
	verifyCodeField->setTextColor(QColor(34, 34, 34));
	verifyCodeField->setInkColor(QColor(42, 171, 238));
	verifyCodeField->setLabelColor(QColor(42, 171, 238));
	verifyCodeField->setFixedWidth(220); // 宽度减小以适应按钮
	verifyCodeField->setFixedHeight(55);
	verifyCodeField->setShowLabel(true);
	verifyCodeField->setUseThemeColors(false);
	verifyCodeField->setFont(QFont("Microsoft YaHei", 11));
	verifyCodeField->setStyleSheet(textFieldStyle);
	
	// 获取验证码按钮
	QtMaterialRaisedButton* getVerifyCodeBtn = new QtMaterialRaisedButton("获取验证码", card);
	getVerifyCodeBtn->setBackgroundColor(QColor(42, 171, 238));
	getVerifyCodeBtn->setForegroundColor(Qt::white);
	getVerifyCodeBtn->setFixedWidth(130);
	getVerifyCodeBtn->setFixedHeight(40);
	getVerifyCodeBtn->setHaloVisible(false);
	getVerifyCodeBtn->setOverlayStyle(Material::TintedOverlay);
	getVerifyCodeBtn->setRippleStyle(Material::CenteredRipple);
	getVerifyCodeBtn->setCornerRadius(4);
	getVerifyCodeBtn->setFont(QFont("Microsoft YaHei", 9));
	
	verifyCodeLayout->addWidget(verifyCodeField);
	verifyCodeLayout->addSpacing(10);
	verifyCodeLayout->addWidget(getVerifyCodeBtn);
	
	// 6. 设置状态label
	statusLabel = new QLabel(card);
	statusLabel->setAlignment(Qt::AlignCenter);
	statusLabel->setStyleSheet(
		"QLabel {"
		"    color: #DE5347;"
		"    font-size: 13px;"
		"}"
	);
	statusLabel->setFixedHeight(20);

	// 7. 设置按钮
	registerButton = new QtMaterialRaisedButton("注册", card);
	registerButton->setBackgroundColor(QColor(42, 171, 238));
	registerButton->setForegroundColor(Qt::white);
	registerButton->setFixedWidth(360);
	registerButton->setFixedHeight(45);
	registerButton->setHaloVisible(false);
	registerButton->setOverlayStyle(Material::TintedOverlay);
	registerButton->setRippleStyle(Material::CenteredRipple);
	registerButton->setCornerRadius(8);
	registerButton->setFont(QFont("Microsoft YaHei", 11));

	backButton = new QtMaterialFlatButton("返回登录", card);
	backButton->setForegroundColor(QColor(42, 171, 238));
	backButton->setFixedWidth(360);
	backButton->setFixedHeight(45);
	backButton->setHaloVisible(false);
	backButton->setRippleStyle(Material::NoRipple);
	backButton->setFont(QFont("Microsoft YaHei", 11));

	// 8. 把控件加进卡片布局
	cardLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
	cardLayout->addSpacing(8);
	cardLayout->addWidget(subtitleLabel, 0, Qt::AlignCenter);
	cardLayout->addSpacing(30);
	
	// 添加输入框，确保居中对齐
	QHBoxLayout* usernameLayout = new QHBoxLayout();
	usernameLayout->addStretch(1);
	usernameLayout->addWidget(usernameField);
	usernameLayout->addStretch(1);
	cardLayout->addLayout(usernameLayout);
	cardLayout->addSpacing(15);
	
	QHBoxLayout* passwordLayout = new QHBoxLayout();
	passwordLayout->addStretch(1);
	passwordLayout->addWidget(passwordField);
	passwordLayout->addStretch(1);
	cardLayout->addLayout(passwordLayout);
	cardLayout->addSpacing(15);
	
	QHBoxLayout* confirmPasswordLayout = new QHBoxLayout();
	confirmPasswordLayout->addStretch(1);
	confirmPasswordLayout->addWidget(confirmPasswordField);
	confirmPasswordLayout->addStretch(1);
	cardLayout->addLayout(confirmPasswordLayout);
	cardLayout->addSpacing(15);
	
	QHBoxLayout* emailLayout = new QHBoxLayout();
	emailLayout->addStretch(1);
	emailLayout->addWidget(emailField);
	emailLayout->addStretch(1);
	cardLayout->addLayout(emailLayout);
	cardLayout->addSpacing(20);
	
	// 在emailLayout之后添加验证码布局
	cardLayout->addLayout(verifyCodeLayout);
	cardLayout->addSpacing(20);
	
	cardLayout->addWidget(statusLabel, 0, Qt::AlignCenter);
	cardLayout->addSpacing(15);
	
	// 添加注册按钮
	QHBoxLayout* registerButtonLayout = new QHBoxLayout();
	registerButtonLayout->addStretch(1);
	registerButtonLayout->addWidget(registerButton);
	registerButtonLayout->addStretch(1);
	cardLayout->addLayout(registerButtonLayout);
	cardLayout->addSpacing(10);
	
	// 添加返回按钮
	QHBoxLayout* backButtonLayout = new QHBoxLayout();
	backButtonLayout->addStretch(1);
	backButtonLayout->addWidget(backButton);
	backButtonLayout->addStretch(1);
	cardLayout->addLayout(backButtonLayout);

	// 9. 外层总布局
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(20, 20, 20, 20);
	mainLayout->addWidget(card);

	// 10. 添加阴影效果
	QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect(this);
	shadowEffect->setBlurRadius(20);
	shadowEffect->setColor(QColor(0, 0, 0, 40));
	shadowEffect->setOffset(0, 0);
	card->setGraphicsEffect(shadowEffect);

	// 11. 设置窗口居中
	QScreen* screen = QGuiApplication::primaryScreen();
	if (screen) {
		QRect screenGeometry = screen->geometry();
		int x = (screenGeometry.width() - width()) / 2;
		int y = (screenGeometry.height() - height()) / 2;
		move(x, y);
	}
	
	// 连接信号和槽
	connect(backButton, &QtMaterialFlatButton::clicked, this, &RegisterDialog::switchToLogin);
	connect(registerButton, &QtMaterialRaisedButton::clicked, this, &RegisterDialog::registerAccount);
	
	// 添加输入框回车键响应
	connect(usernameField, &QtMaterialTextField::returnPressed, [this]() {
		passwordField->setFocus();
	});
	connect(passwordField, &QtMaterialTextField::returnPressed, [this]() {
		confirmPasswordField->setFocus();
	});
	connect(confirmPasswordField, &QtMaterialTextField::returnPressed, [this]() {
		emailField->setFocus();
	});
	connect(emailField, &QtMaterialTextField::returnPressed, this, &RegisterDialog::registerAccount);
	
	// 连接HttpMgr的信号
	connect(HttpMgr::GetInstance().get(), &HttpMgr::requestFinished,
			this, [this](ReqId reqId, const QJsonObject& response, ErrorCodes error, Modules module) {
				if (reqId == ReqId::Register) {
					registerButton->setEnabled(true);  // 重新启用注册按钮
					if (error == ErrorCodes::Success) {
						QMessageBox::information(this, "注册成功", "账号注册成功，请返回登录！");
						emit returnLogin();
					} else {
						QString errorMsg;
						switch (error) {
							case ErrorCodes::UserAlreadyExist:
								errorMsg = "用户名已存在";
								break;
							case ErrorCodes::InvalidParams:
								errorMsg = "输入参数无效";
								break;
							default:
								errorMsg = "注册失败: " + QString::number(static_cast<int>(error));
								break;
						}
						statusLabel->setText(errorMsg);
						qDebug() << errorMsg;
					}
				}
			});
	
	connect(HttpMgr::GetInstance().get(), &HttpMgr::networkError,
			this, [this](ReqId reqId, const QString& errorString, Modules module) {
				if (reqId == ReqId::Register) {
					registerButton->setEnabled(true);  // 重新启用注册按钮
					statusLabel->setText("网络错误: " + errorString);
				}
			});
			
	// 连接获取验证码按钮
	connect(getVerifyCodeBtn, &QtMaterialRaisedButton::clicked, this, [this, getVerifyCodeBtn, verifyCodeField]() {
		// 验证邮箱
		QString email = emailField->text();
		if (email.isEmpty()) {
			statusLabel->setText("请输入邮箱地址");
			return;
		}
		
		QRegularExpression emailRegex("\\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}\\b");
		if (!emailRegex.match(email).hasMatch()) {
			statusLabel->setText("请输入有效的邮箱地址");
			return;
		}
		
		// 禁用按钮并修改文本
		getVerifyCodeBtn->setDisabled(true);
		getVerifyCodeBtn->setText("发送中...");
		
		// 发送获取验证码请求
		HttpMgr::GetInstance()->getVerifyCode(email);
	});
	
	// 添加验证码响应处理
	connect(HttpMgr::GetInstance().get(), &HttpMgr::requestFinished, this,
		[this, getVerifyCodeBtn](ReqId reqId, const QJsonObject& response, ErrorCodes error, Modules module) {
			if (reqId == ReqId::GetVerifyCode && module == Modules::User) {
				// 处理验证码响应
				if (error == ErrorCodes::Success) {
					statusLabel->setText("验证码已发送，请查看邮箱");
					statusLabel->setStyleSheet("QLabel { color: #4CAF50; font-size: 13px; }");
					
					// 开始倒计时
					startVerifyCodeCountdown(getVerifyCodeBtn);
				} else {
					statusLabel->setText(QString("获取验证码失败：错误码 %1").arg(static_cast<int>(error)));
					statusLabel->setStyleSheet("QLabel { color: #DE5347; font-size: 13px; }");
					getVerifyCodeBtn->setDisabled(false);
					getVerifyCodeBtn->setText("获取验证码");
				}
			}
		});
	
	// 设置初始焦点
	usernameField->setFocus();
}

void RegisterDialog::switchToLogin()
{
	// 发射信号通知My_wechat切换到登录界面，不直接操作父窗口
	emit returnLogin();
}

void RegisterDialog::registerAccount()
{
	// 清除之前的错误信息
	statusLabel->clear();
	
	// 获取输入内容
	QString username = usernameField->text().trimmed();
	QString password = passwordField->text();
	QString confirmPassword = confirmPasswordField->text();
	QString email = emailField->text().trimmed();
	
	// 验证输入
	if (username.isEmpty()) {
		statusLabel->setText("请输入用户名");
		return;
	}
	
	if (password.isEmpty()) {
		statusLabel->setText("请输入密码");
		return;
	}
	
	if (confirmPassword.isEmpty()) {
		statusLabel->setText("请确认密码");
		return;
	}
	
	if (password != confirmPassword) {
		statusLabel->setText("两次输入的密码不一致");
		return;
	}
	
	if (email.isEmpty()) {
		statusLabel->setText("请输入邮箱地址");
		return;
	}
	
	// 简单的邮箱格式验证
	QRegularExpression  emailRegex("\\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,4}\\b");
	
	QRegularExpressionMatch match = emailRegex.match(email);
	if (!match.hasMatch()) {
		statusLabel->setText("邮箱格式不正确");
		return;
	}
	
	// 禁用注册按钮，防止重复提交
	registerButton->setEnabled(false);
	statusLabel->setText("正在注册...");
	
	// 调用HttpMgr进行注册
	HttpMgr::GetInstance()->registerUser(username, password, email);
}

void RegisterDialog::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		_isMousePressed = true;
		_mousePos = event->globalPosition().toPoint() - pos();
	}
}

void RegisterDialog::mouseReleaseEvent(QMouseEvent *event)
{
	_isMousePressed = false;
}

void RegisterDialog::mouseMoveEvent(QMouseEvent *event)
{
	if (_isMousePressed && (event->buttons() & Qt::LeftButton)) {
		move(event->globalPosition().toPoint() - _mousePos);
	}
}

// 在类的适当位置添加倒计时方法
void RegisterDialog::startVerifyCodeCountdown(QtMaterialRaisedButton* btn) {
	static int remainingSeconds = 60;
	remainingSeconds = 60;
	
	// 创建并启动定时器
	QTimer* countdownTimer = new QTimer(this);
	connect(countdownTimer, &QTimer::timeout, this, [countdownTimer, btn]() {
		remainingSeconds--;
		if (remainingSeconds > 0) {
			btn->setText(QString("%1秒").arg(remainingSeconds));
		} else {
			countdownTimer->stop();
			countdownTimer->deleteLater();
			btn->setDisabled(false);
			btn->setText("获取验证码");
		}
	});
	
	countdownTimer->start(1000);
}

RegisterDialog::~RegisterDialog()
{}
