#pragma once

#include "global.h"
#include "ui_LoginDialog.h"

class LoginDialog : public QDialog
{
	Q_OBJECT

public:
	LoginDialog(QWidget *parent = nullptr);
	~LoginDialog();

protected:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;

private:
	Ui::LoginDialogClass ui;
	QtMaterialTextField* usernameField;
	QtMaterialTextField* passwordField;
	QtMaterialRaisedButton* loginButton;
	QtMaterialFlatButton* registerButton;
	QLabel* statusLabel;
	bool _isMousePressed;
	QPoint _mousePos;

private slots:
	void onLoginButtonClicked();

signals:
	void switchRegiser();
	void loginSuccess();
};
