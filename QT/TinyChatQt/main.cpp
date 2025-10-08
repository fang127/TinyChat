#include "mainwindow.h"
#include "global.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置背景颜色和字体
    QFile qss(":/style/stylesheet.qss");
    if (qss.open(QFile::ReadOnly))
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

    // set config path
    QString fileName = "config.ini";
    QString appPath = QCoreApplication::applicationDirPath();
    QString configPath = QDir::toNativeSeparators(appPath + QDir::separator() + fileName);
    QSettings settings(configPath,QSettings::IniFormat); // read config file
    // read host ip and port
    QString gateHost = settings.value("GateServer/host").toString();
    QString gatePort = settings.value("GateServer/port").toString();
    gateUrlPrefix = "http://" + gateHost + ":" + gatePort;

    MainWindow w;

    // 设置logo
    QIcon icon(":/res/logo.png");
    if (!icon.isNull())
    {
        w.setWindowIcon(icon);
    }
    else
    {
        qDebug("图标加载失败");
    }
    w.setWindowIcon(icon); // 使用图标
    w.show();
    return a.exec();
}
