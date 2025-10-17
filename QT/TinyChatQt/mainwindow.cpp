#include "mainwindow.h"
#include "./ui_mainwindow.h"

/******************************************************************************
 *
 * @file       mainwindow.cpp
 * @brief      主窗口
 *
 * @author     FangHaonan
 * @date       2025/10/04
 * @history
 *****************************************************************************/

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 加载登录界面
    loginDialog_ = new LoginDialog(this);

    loginDialog_->setWindowFlags(Qt::CustomizeWindowHint |
                                 Qt::FramelessWindowHint);

    setCentralWidget(loginDialog_);
    // loginDialog_->show();
    // 创建和注册消息链接
    connect(loginDialog_, &LoginDialog::sigSwitchRegister, this,
            &MainWindow::slotSwitchReg);
    // 连接了重置密码的信号和槽
    connect(loginDialog_, &LoginDialog::sigSwitchReset,this, &MainWindow::slotSwitchReset);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::slotSwitchReg()
{
    registerDialog_ = new RegisterDialog(this);

    registerDialog_->setWindowFlags(Qt::CustomizeWindowHint |
                                    Qt::FramelessWindowHint);

    //连接注册界面返回登录信号
    connect(registerDialog_, &RegisterDialog::sigSwitchLogin, this, &MainWindow::slotSwitchLogin);

    setCentralWidget(registerDialog_);
    loginDialog_->hide();
    registerDialog_->show();
}

void MainWindow::slotSwitchLogin()
{
    // 加载登录界面
    loginDialog_ = new LoginDialog(this);

    loginDialog_->setWindowFlags(Qt::CustomizeWindowHint |
                                 Qt::FramelessWindowHint);
    //连接登录界面注册信号
    connect(loginDialog_, &LoginDialog::sigSwitchRegister, this, &MainWindow::slotSwitchReg);
    connect(loginDialog_, &LoginDialog::sigSwitchReset, this, &MainWindow::slotSwitchReset);
    setCentralWidget(loginDialog_);
    registerDialog_->hide();
    loginDialog_->show();
}

//从重置界面返回登录界面
void MainWindow::slotSwitchLogin2()
{
    //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    loginDialog_ = new LoginDialog(this);
    loginDialog_->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint);
    setCentralWidget(loginDialog_);

    resetDialog_->hide();
    loginDialog_->show();
    //连接登录界面忘记密码信号
    connect(loginDialog_, &LoginDialog::sigSwitchReset, this, &MainWindow::slotSwitchReset);
    //连接登录界面注册信号
    connect(loginDialog_, &LoginDialog::sigSwitchRegister, this, &MainWindow::slotSwitchReg);
}

void MainWindow::slotSwitchReset()
{
    // 创建一个CentralWidget, 并将其设置为MainWindow的中心部件
    resetDialog_ = new Resetdialog(this);

    resetDialog_->setWindowFlags(Qt::CustomizeWindowHint |
                                 Qt::FramelessWindowHint);
    setCentralWidget(resetDialog_);
    connect(resetDialog_, &Resetdialog::sigSwitchLogin, this, &MainWindow::slotSwitchLogin2);

    loginDialog_->hide();
    resetDialog_->show();
}
