#pragma once
#include "global.h"
#include "ui_RegisterDialog.h"
#include <qtmaterialtextfield.h>
#include <qtmaterialraisedbutton.h>
#include <qtmaterialflatbutton.h>
#include <QLabel>

class RegisterDialog : public QDialog
{
	Q_OBJECT

public:
	RegisterDialog(QWidget *parent = nullptr);
	~RegisterDialog();

protected:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;

private:
	Ui::RegisterDialogClass ui;
	bool _isMousePressed;
	QPoint _mousePos;
	
	// 自定义控件
	QtMaterialTextField* usernameField;
	QtMaterialTextField* passwordField;
	QtMaterialTextField* confirmPasswordField;
	QtMaterialTextField* emailField;
	QtMaterialTextField* verifyCodeField;
	QtMaterialRaisedButton* registerButton;
	QtMaterialFlatButton* backButton;
	QLabel* statusLabel;

	// 验证码相关
	void startVerifyCodeCountdown(QtMaterialRaisedButton* btn);

private slots:
	void switchToLogin();
	void registerAccount();

signals:
	void returnLogin();
};
