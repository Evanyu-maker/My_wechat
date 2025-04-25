#include "RegisterDialog.h"

RegisterDialog::RegisterDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setObjectName("RegisterDialog");
	
	// 美化用户名输入框
	ui.lineEdit->hide();
	usernameField = new QtMaterialTextField(this);
	usernameField->setLabel("用户名");
	usernameField->setTextColor(QColor("#333333"));
	usernameField->setLabelColor(QColor("#07C160"));
	usernameField->setInkColor(QColor("#07C160"));
	usernameField->setFixedWidth(300);
	
	// 美化密码输入框
	passwordField = new QtMaterialTextField(this);
	passwordField->setLabel("密码");
	passwordField->setTextColor(QColor("#333333"));
	passwordField->setLabelColor(QColor("#07C160"));
	passwordField->setInkColor(QColor("#07C160"));
	passwordField->setFixedWidth(300);
	passwordField->setEchoMode(QLineEdit::Password);
	
	// 确认密码输入框
	confirmPasswordField = new QtMaterialTextField(this);
	confirmPasswordField->setLabel("确认密码");
	confirmPasswordField->setTextColor(QColor("#333333"));
	confirmPasswordField->setLabelColor(QColor("#07C160"));
	confirmPasswordField->setInkColor(QColor("#07C160"));
	confirmPasswordField->setFixedWidth(300);
	confirmPasswordField->setEchoMode(QLineEdit::Password);
	
	// 邮箱输入框
	emailField = new QtMaterialTextField(this);
	emailField->setLabel("邮箱");
	emailField->setTextColor(QColor("#333333"));
	emailField->setLabelColor(QColor("#07C160"));
	emailField->setInkColor(QColor("#07C160"));
	emailField->setFixedWidth(300);
	
	// 替换注册按钮
	registerButton = new QtMaterialRaisedButton("注册", this);
	registerButton->setBackgroundColor(QColor("#07C160"));
	registerButton->setFixedWidth(300);
	
	// 返回按钮
	backButton = new QtMaterialFlatButton("返回登录", this);
	backButton->setStyleSheet("color: #07C160;");
	backButton->setHaloVisible(false);
	backButton->setFixedWidth(300);
	
	// 状态标签
	statusLabel = new QLabel(this);
	statusLabel->setAlignment(Qt::AlignCenter);
	statusLabel->setStyleSheet("color: #666;");
	statusLabel->setFixedHeight(30);
	
	// 添加到布局中
	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->setAlignment(Qt::AlignCenter);
	mainLayout->setSpacing(20);
	
	QLabel* titleLabel = new QLabel("注册新账号", this);
	titleLabel->setAlignment(Qt::AlignCenter);
	titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #07C160;");
	
	mainLayout->addStretch();
	mainLayout->addWidget(titleLabel);
	mainLayout->addSpacing(30);
	mainLayout->addWidget(usernameField, 0, Qt::AlignCenter);
	mainLayout->addWidget(passwordField, 0, Qt::AlignCenter);
	mainLayout->addWidget(confirmPasswordField, 0, Qt::AlignCenter);
	mainLayout->addWidget(emailField, 0, Qt::AlignCenter);
	mainLayout->addWidget(statusLabel, 0, Qt::AlignCenter);
	mainLayout->addSpacing(20);
	mainLayout->addWidget(registerButton, 0, Qt::AlignCenter);
	mainLayout->addWidget(backButton, 0, Qt::AlignCenter);
	mainLayout->addStretch();
	
	// 设置新的布局
	delete layout();
	setLayout(mainLayout);
	
	// 绑定信号和槽
	connect(registerButton, &QtMaterialRaisedButton::clicked, this, &RegisterDialog::onRegisterButtonClicked);
	connect(backButton, &QtMaterialFlatButton::clicked, this, &RegisterDialog::onBackButtonClicked);
}

void RegisterDialog::onRegisterButtonClicked()
{
	QString username = usernameField->text();
	QString password = passwordField->text();
	QString confirmPassword = confirmPasswordField->text();
	QString email = emailField->text();
	
	// 验证输入
	if (username.isEmpty() || password.isEmpty() || confirmPassword.isEmpty() || email.isEmpty()) {
		statusLabel->setText("所有字段都不能为空");
		statusLabel->setStyleSheet("color: red;");
		return;
	}
	
	if (password != confirmPassword) {
		statusLabel->setText("两次密码输入不一致");
		statusLabel->setStyleSheet("color: red;");
		return;
	}
	
	if (!email.contains("@") || !email.contains(".")) {
		statusLabel->setText("请输入有效的邮箱地址");
		statusLabel->setStyleSheet("color: red;");
		return;
	}
	
	// 注册成功
	statusLabel->setText("注册成功！");
	statusLabel->setStyleSheet("color: green;");
	
	// 清空输入框
	usernameField->clear();
	passwordField->clear();
	confirmPasswordField->clear();
	emailField->clear();
	
	// 延迟返回登录界面
	QTimer::singleShot(1500, this, &RegisterDialog::onBackButtonClicked);
}

void RegisterDialog::onBackButtonClicked()
{
	hide();
	parentWidget()->findChild<QDialog*>("loginDialog")->show();
}

RegisterDialog::~RegisterDialog()
{}
