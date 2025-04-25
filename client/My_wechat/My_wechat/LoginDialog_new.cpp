#include "LoginDialog.h"

LoginDialog::LoginDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setObjectName("loginDialog");
	
	// 美化用户名输入框
	ui.lineEdit->hide();
	usernameField = new QtMaterialTextField(this);
	usernameField->setLabel("用户名");
	usernameField->setTextColor(QColor("#333333"));
	usernameField->setLabelColor(QColor("#07C160"));
	usernameField->setInkColor(QColor("#07C160"));
	usernameField->setFixedWidth(300);
	
	// 美化密码输入框
	ui.lineEdit_2->hide();
	passwordField = new QtMaterialTextField(this);
	passwordField->setLabel("密码");
	passwordField->setTextColor(QColor("#333333"));
	passwordField->setLabelColor(QColor("#07C160"));
	passwordField->setInkColor(QColor("#07C160"));
	passwordField->setFixedWidth(300);
	passwordField->setEchoMode(QLineEdit::Password);
	
	// 替换登录按钮
	ui.pushButton->hide();
	loginButton = new QtMaterialRaisedButton("登录", this);
	loginButton->setBackgroundColor(QColor("#07C160"));
	loginButton->setFixedWidth(300);
	
	// 替换注册按钮
	ui.pushButton_2->hide();
	registerButton = new QtMaterialFlatButton("注册新账号", this);
	registerButton->setTextColor(QColor("#07C160"));
	registerButton->setHaloVisible(false);
	registerButton->setFixedWidth(300);
	
	// 添加到布局中
	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->setAlignment(Qt::AlignCenter);
	mainLayout->setSpacing(20);
	
	QLabel* logoLabel = new QLabel(this);
	logoLabel->setPixmap(QPixmap(":/res/wechat_logo.png").scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	logoLabel->setAlignment(Qt::AlignCenter);
	
	QLabel* titleLabel = new QLabel("微信登录", this);
	titleLabel->setAlignment(Qt::AlignCenter);
	titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #07C160;");
	
	mainLayout->addStretch();
	mainLayout->addWidget(logoLabel);
	mainLayout->addWidget(titleLabel);
	mainLayout->addSpacing(30);
	mainLayout->addWidget(usernameField, 0, Qt::AlignCenter);
	mainLayout->addWidget(passwordField, 0, Qt::AlignCenter);
	mainLayout->addSpacing(20);
	mainLayout->addWidget(loginButton, 0, Qt::AlignCenter);
	mainLayout->addWidget(registerButton, 0, Qt::AlignCenter);
	mainLayout->addStretch();
	
	// 设置新的布局
	delete layout();
	setLayout(mainLayout);
	
	// 绑定信号和槽
	connect(registerButton, &QtMaterialFlatButton::clicked, this, &LoginDialog::switchRegiser);
	connect(loginButton, &QtMaterialRaisedButton::clicked, this, &LoginDialog::onLoginButtonClicked);
}

void LoginDialog::onLoginButtonClicked()
{
	QString username = usernameField->text();
	QString password = passwordField->text();
	
	if (username.isEmpty() || password.isEmpty()) {
		QMessageBox::warning(this, "登录失败", "用户名和密码不能为空");
		return;
	}
	
	// 这里应该有真实的登录验证逻辑
	// 为了演示，我们简单地检查用户名是否是"admin"，密码是否是"123456"
	if (username == "admin" && password == "123456") {
		// 登录成功，发送信号
		emit loginSuccess();
	} else {
		QMessageBox::warning(this, "登录失败", "用户名或密码错误");
	}
}

LoginDialog::~LoginDialog()
{}
