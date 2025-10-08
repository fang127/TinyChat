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
    setCentralWidget(loginDialog_);
    // loginDialog_->show();
    // 创建和注册消息链接
    connect(loginDialog_, &LoginDialog::switchRegister, this,
            &MainWindow::slotSwitchReg);

    registerDialog_ = new RegisterDialog(this);

    registerDialog_->setWindowFlags(Qt::CustomizeWindowHint |
                                    Qt::FramelessWindowHint);
    loginDialog_->setWindowFlags(Qt::CustomizeWindowHint |
                                 Qt::FramelessWindowHint);

    registerDialog_->hide();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::slotSwitchReg()
{
    setCentralWidget(registerDialog_);
    loginDialog_->hide();
    registerDialog_->show();
}
