#include "ChatDialog.h"
#include "ui_ChatDialog.h"
#include "ChatUserWid.h"
#include "LoadDialog.h"

#include <QAction>
#include <QRandomGenerator>

ChatDialog::ChatDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChatDialog),
    mode_(ChatUIMode::ChatMode),
    state_(ChatUIMode::ChatMode),
    loading_(false)
{
    ui->setupUi(this);
    ui->chatAddBtn->setState("normal","hover","press");
    ui->chatSearchEdit->setMaxLength(15);
    // 设置输入的长度限制以及关闭等图标的配置
    QAction *searchAction = new QAction(ui->chatSearchEdit);
    searchAction->setIcon(QIcon(":/res/search.png"));
    ui->chatSearchEdit->addAction(searchAction, QLineEdit::LeadingPosition);
    ui->chatSearchEdit->setPlaceholderText(QStringLiteral("搜索"));
    ui->chatSearchEdit->setMaxLength(15);
    // 创建一个清除动作并设置图标
    // 将清除动作添加到LineEdit的末尾位置
    QAction *clearAction = new QAction(ui->chatSearchEdit);
    clearAction->setIcon(QIcon(":/res/close_transparent.png"));
    ui->chatSearchEdit->addAction(clearAction, QLineEdit::TrailingPosition);

    // 当需要显示清除图标时，更改为实际的清除图标
    connect(ui->chatSearchEdit, &QLineEdit::textChanged, [clearAction](const QString &text) {
        if (!text.isEmpty()) {
            clearAction->setIcon(QIcon(":/res/close_search.png"));
        } else {
            clearAction->setIcon(QIcon(":/res/close_transparent.png")); // 文本为空时，切换回透明图标
        }

    });

    // 连接清除动作的触发信号到槽函数，用于清除文本
    connect(clearAction, &QAction::triggered, [this, clearAction]() {
        ui->chatSearchEdit->clear();
        clearAction->setIcon(QIcon(":/res/close_transparent.png")); // 清除文本后，切换回透明图标
        ui->chatSearchEdit->clearFocus();
        //清除按钮被按下则不显示搜索框
        showSearch(false);
    });
    showSearch(false);

    connect(ui->chatUserList, &ChatUserList::sigLoadingChatUser, this, &ChatDialog::slotLoadingChatUser);

    // 测试
    addChatUserList();
}

ChatDialog::~ChatDialog()
{
    delete ui;
}

// 测试
void ChatDialog::addChatUserList()
{
    // 创建QListWidgetItem，并设置自定义的widget
        for(int i = 0; i < 13; i++)
        {
            int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
            int str_i = randomValue%strs.size();
            int head_i = randomValue%heads.size();
            int name_i = randomValue%names.size();

            auto *chatUserWid = new ChatUserWid();
            chatUserWid->setInfo(names[name_i], heads[head_i], strs[str_i]);
            QListWidgetItem *item = new QListWidgetItem;
            //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
            item->setSizeHint(chatUserWid->sizeHint());
            ui->chatUserList->addItem(item);
            ui->chatUserList->setItemWidget(item, chatUserWid);
        }
}

void ChatDialog::slotLoadingChatUser()
{
    if(loading_)
    {
        return;
    }

    loading_ = true;
    LoadDialog *loadingDialog = new LoadDialog(this);
    loadingDialog->setModal(true);
    loadingDialog->show();
    qDebug() << "add new data to list......";
    addChatUserList();
    loadingDialog->deleteLater();
    loading_ = false;
}

void ChatDialog::showSearch(bool status)
{
    if(status)
    {
        ui->chatUserList->hide();
        ui->chatConnList->hide();
        ui->chatSearchList->show();
        mode_ = ChatUIMode::SearchMode;
    }
    else if(state_ == ChatUIMode::ChatMode)
    {
        ui->chatUserList->show();
        ui->chatConnList->hide();
        ui->chatSearchList->hide();
        mode_ = ChatUIMode::ChatMode;
    }
    else if(state_ == ChatUIMode::ContactMode)
    {
        ui->chatUserList->hide();
        ui->chatSearchList->hide();
        ui->chatConnList->show();
        mode_ = ChatUIMode::ContactMode;
    }
}
