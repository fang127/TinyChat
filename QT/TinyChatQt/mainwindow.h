#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "logindialog.h"
#include "registerdialog.h"
#include "Resetdialog.h"
#include "ChatDialog.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void slotSwitchReg();
    void slotSwitchLogin();
    void slotSwitchReset();
    void slotSwitchLogin2();
    void slotSwitchChat();
private:
    Ui::MainWindow *ui;
    LoginDialog *loginDialog_;
    RegisterDialog *registerDialog_;
    Resetdialog *resetDialog_;
    ChatDialog *chatDialog_;
};
#endif // MAINWINDOW_H
