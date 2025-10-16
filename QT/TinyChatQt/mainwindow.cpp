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
    connect(loginDialog_, &LoginDialog::switchRegister, this,
            &MainWindow::slotSwitchReg);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::slotSwitchReg()
{
    registerDialog_ = new RegisterDialog(this);
    registerDialog_->hide();
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

    setCentralWidget(loginDialog_);
    registerDialog_->hide();
    loginDialog_->show();
    //连接登录界面注册信号
    connect(loginDialog_, &LoginDialog::switchRegister, this, &MainWindow::slotSwitchReg);
}
