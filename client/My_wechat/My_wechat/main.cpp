#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QFile>
#include "AppController.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    
    // 加载样式表
    QFile styleFile(":/style/material.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        a.setStyleSheet(styleSheet);
        styleFile.close();
    }
    a.setWindowIcon(QIcon(":/res/telegram.ico"));

    // 创建应用控制器并启动应用
    AppController appController;
    appController.start();

    return a.exec();
}