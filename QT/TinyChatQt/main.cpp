#include "mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置背景颜色和字体
    QFile qss(":/style/stylesheet.qss");
    if(qss.open(QFile::ReadOnly))
    {
        qDebug("open success");
        QString style = QLatin1String(qss.readAll());
        a.setStyleSheet(style);
        qss.close();
    }
    else
    {
        qDebug("open failed");
    }

    MainWindow w;

    // 设置logo
    QIcon icon(":/res/logo.png");
    if (!icon.isNull())
    {
        w.setWindowIcon(icon);
    } else
    {
        qDebug("图标加载失败");
    }
    w.setWindowIcon(icon);  // 使用图标
    w.show();
    return a.exec();
}
