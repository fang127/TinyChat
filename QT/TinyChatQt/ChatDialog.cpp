#include "ChatDialog.h"
#include "ChatUserWid.h"
#include "LoadDialog.h"
#include "TcpMgr.h"
#include "UserMgr.h"
#include "ConUserItem.h"
#include "ui_ChatDialog.h"
#include <QAction>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPixmap>
#include <QRandomGenerator>
ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ChatDialog), mode_(ChatUIMode::ChatMode),
      state_(ChatUIMode::ChatMode), loading_(false), lastWidget_(nullptr), curChatUid_(0)
{
    ui->setupUi(this);
    ui->chatAddBtn->setState("normal", "hover", "press");
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
    connect(
        ui->chatSearchEdit, &QLineEdit::textChanged,
        [clearAction](const QString &text)
        {
            if (!text.isEmpty())
            {
                clearAction->setIcon(QIcon(":/res/close_search.png"));
            }
            else
            {
                clearAction->setIcon(QIcon(
                    ":/res/close_transparent.png")); // 文本为空时，切换回透明图标
            }
        });

    // 连接清除动作的触发信号到槽函数，用于清除文本
    connect(
        clearAction, &QAction::triggered,
        [this, clearAction]()
        {
            ui->chatSearchEdit->clear();
            clearAction->setIcon(QIcon(
                ":/res/close_transparent.png")); // 清除文本后，切换回透明图标
            ui->chatSearchEdit->clearFocus();
            // 清除按钮被按下则不显示搜索框
            showSearch(false);
        });

    ui->chatSearchEdit->setMaxLength(15);
    showSearch(false);

    connect(ui->chatUserList, &ChatUserList::sigLoadingChatUser, this,
            &ChatDialog::slotLoadingChatUser);

    // 测试
    addChatUserList();

    QPixmap pixmap(":/res/head_1.jpg");
    ui->sideHeadLabel->setPixmap(pixmap); // 将图片设置到QLabel上
    QPixmap scaledPixmap =
        pixmap.scaled(ui->sideHeadLabel->size(),
                      Qt::KeepAspectRatio);     // 将图片缩放到label的大小
    ui->sideHeadLabel->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
    ui->sideHeadLabel->setScaledContents(
        true); // 设置QLabel自动缩放图片内容以适应大小

    ui->sideChatLabel->setProperty("state", "normal");
    ui->sideChatLabel->setState("normal", "hover", "pressed", "selected_normal",
                                "selected_hover", "selected_pressed");
    ui->sideContactLabel->setState("normal", "hover", "pressed",
                                   "selected_normal", "selected_hover",
                                   "selected_pressed");

    addLBGroup(ui->sideChatLabel);

    addLBGroup(ui->sideContactLabel);

    connect(ui->sideChatLabel, &StateWidget::clicked, this,
            &ChatDialog::slotSideChat);
    connect(ui->sideContactLabel, &StateWidget::clicked, this,
            &ChatDialog::slotSideContact);
    // 链接搜索框输入变化
    connect(ui->chatSearchEdit, &QLineEdit::textChanged, this,
            &ChatDialog::slotTextChanged);
    // 连接申请添加好友信号
    connect(TcpMgr::getInstance_().get(), &TcpMgr::sigFriendApply, this,
            &ChatDialog::slotApplyFriend);
    // 检测鼠标点击位置判断是否要情况搜索框
    this->installEventFilter(this); // 按照事件过滤器
    // 设置聊天label选中状态
    ui->sideChatLabel->setSelected(true);

    // 为searchList设置searchEdit
    ui->chatSearchList->setSearchEdit(ui->chatSearchEdit);

    //连接认证添加好友信号
    connect(TcpMgr::getInstance_().get(), &TcpMgr::sigAddAuthFriend, this, &ChatDialog::slotAddAuthFriend);

    //链接自己认证回复信号
    connect(TcpMgr::getInstance_().get(), &TcpMgr::sigAuthRsp, this,
            &ChatDialog::slotAuthRsp);

    // 连接跳转信号,搜索已有还有的跳转
    connect(ui->chatSearchList, &SearchList::sigJumpChatItem, this, &ChatDialog::slotJumpChatItem);
    // 连接加载联系人的信号
    connect(ui->chatConnList, &ContactUserList::sig_loading_contact_user, this, &ChatDialog::slotLoadingContact);
}

ChatDialog::~ChatDialog() { delete ui; }

// 加载好友
void ChatDialog::addChatUserList()
{
    auto friendList = UserMgr::getInstance_()->getChatListPerPage();
    if(!friendList.empty())
    {
        for(auto &friendEle : friendList)
        {
            auto it = chatItemsAddeds_.find(friendEle->_uid);
            {
                if(it != chatItemsAddeds_.end())
                {
                    continue;
                }
                auto *chatUserWid = new ChatUserWid();
                auto userInfo = std::make_shared<UserInfo>(friendEle);
                chatUserWid->setInfo(userInfo);
                QListWidgetItem *item = new QListWidgetItem;
                item->setSizeHint(chatUserWid->sizeHint());
                ui->chatUserList->addItem(item);
                ui->chatUserList->setItemWidget(item, chatUserWid);
                chatItemsAddeds_.insert(friendEle->_uid, item);
            }
        }

        UserMgr::getInstance_()->updateChatLoadedCount();
    }

    //模拟测试条目
        // 创建QListWidgetItem，并设置自定义的widget
        for(int i = 0; i < 13; i++)
        {
            int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
            int str_i = randomValue%strs.size();
            int head_i = randomValue%heads.size();
            int name_i = randomValue%names.size();

            auto *chatUserWid = new ChatUserWid();
            auto userInfo = std::make_shared<UserInfo>(0,names[name_i],
                                                        names[name_i],heads[head_i],0,strs[str_i]);
            chatUserWid->setInfo(userInfo);
            QListWidgetItem *item = new QListWidgetItem;
            //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
            item->setSizeHint(chatUserWid->sizeHint());
            ui->chatUserList->addItem(item);
            ui->chatUserList->setItemWidget(item, chatUserWid);
        }
}

void ChatDialog::clearLabelState(StateWidget *lb)
{
    for (auto &ele : lbList_)
    {
        if (ele == lb)
        {
            continue;
        }

        ele->clearState();
    }
}

bool ChatDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        handleGlobalMousePress(mouseEvent);
    }

    return QDialog::eventFilter(watched, event);
}

void ChatDialog::slotLoadingChatUser()
{
    if (loading_)
    {
        return;
    }

    loading_ = true;
    LoadDialog *loadingDialog = new LoadDialog(this);
    loadingDialog->setModal(true);
    loadingDialog->show();
    qDebug() << "add new data to list......";
    loadMoreChatUser();
    loadingDialog->deleteLater();
    loading_ = false;
}

void ChatDialog::slotLoadingContact()
{
    if (loading_)
    {
        return;
    }

    loading_ = true;
    LoadDialog *loadingDialog = new LoadDialog(this);
    loadingDialog->setModal(true);
    loadingDialog->show();
    qDebug() << "add new data to list......";
    loadMoreConUser();
    loadingDialog->deleteLater();
    loading_ = false;
}

void ChatDialog::slotSideChat()
{
    qDebug() << "receive side chat clicked";
    clearLabelState(ui->sideChatLabel);
    ui->stackedWidget->setCurrentWidget(ui->chatPage);
    state_ = ChatUIMode::ChatMode;
    showSearch(false);
}

void ChatDialog::slotSideContact()
{
    qDebug() << "receive side contact clicked";
    clearLabelState(ui->sideContactLabel);
    // 设置
    if (lastWidget_ == nullptr)
    {
        ui->stackedWidget->setCurrentWidget(ui->friendPage);
        lastWidget_ = ui->friendPage;
    }
    else
    {
        ui->stackedWidget->setCurrentWidget(lastWidget_);
    }

    state_ = ChatUIMode::ContactMode;
    showSearch(false);
}

void ChatDialog::slotTextChanged(const QString &str)
{
    // qDebug()<< "receive slot text changed str is " << str;
    if (!str.isEmpty())
    {
        showSearch(true);
    }
}

void ChatDialog::slotApplyFriend(std::shared_ptr<AddFriendApply> apply)
{
    qDebug() << "receive apply friend slot, applyuid is " << apply->_from_uid
             << " name is " << apply->_name << " desc is " << apply->_desc;

    bool b_already =
    UserMgr::getInstance_()->alreadyApply(apply->_from_uid);
    // 已经存在直接返回
    if(b_already)
    {
         return;
    }

    UserMgr::getInstance_()->addApplyList(std::make_shared<ApplyInfo>(apply));
    ui->sideContactLabel->showRedPoint(true);
    ui->chatConnList->showRedPoint(true);
    ui->friendPage->addNewApply(apply);
}

void ChatDialog::slotAddAuthFriend(std::shared_ptr<AuthInfo> authInfo)
{
    qDebug() << "receive slot_add_auth_friend uid is " << authInfo->_uid
            << " name is " << authInfo->_name << " nick is " << authInfo->_nick;
    bool isFriend = UserMgr::getInstance_()->checkFriendByUid(authInfo->_uid);
    if(isFriend)
    {
        return;
    }

    UserMgr::getInstance_()->addFriend(authInfo);

    int randomValue = QRandomGenerator::global()->bounded(100);
    int str = randomValue % strs.size();
    int head = randomValue % heads.size();
    int name = randomValue % names.size();

    auto *chatUserWid = new ChatUserWid();
    auto userInfo = std::make_shared<UserInfo>(authInfo);
    chatUserWid->setInfo(userInfo);
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(chatUserWid->sizeHint());

    ui->chatUserList->insertItem(0, item);
    ui->chatUserList->setItemWidget(item, chatUserWid);
    chatItemsAddeds_.insert(authInfo->_uid, item);
}

void ChatDialog::slotAuthRsp(std::shared_ptr<AuthRsp> authRsp)
{
    qDebug() << "receive slot_auth_rsp uid is " << authRsp->_uid
            << " name is " << authRsp->_name << " nick is " << authRsp->_nick;

        //判断如果已经是好友则跳过
        auto isfriend = UserMgr::getInstance_()->checkFriendByUid(authRsp->_uid);
        if(isfriend)
        {
            return;
        }

        UserMgr::getInstance_()->addFriend(authRsp);
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        int str_i = randomValue % strs.size();
        int head_i = randomValue % heads.size();
        int name_i = randomValue % names.size();

        auto* chat_user_wid = new ChatUserWid();
        auto user_info = std::make_shared<UserInfo>(authRsp);
        chat_user_wid->setInfo(user_info);
        QListWidgetItem* item = new QListWidgetItem;
        //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
        item->setSizeHint(chat_user_wid->sizeHint());
        ui->chatUserList->insertItem(0, item);
        ui->chatUserList->setItemWidget(item, chat_user_wid);
        chatItemsAddeds_.insert(authRsp->_uid, item);
}

void ChatDialog::slotJumpChatItem(std::shared_ptr<SearchInfo> si)
{
    qDebug() << "slot jump chat item ";
        auto find_iter = chatItemsAddeds_.find(si->uid_);
        if(find_iter != chatItemsAddeds_.end())
        {
            qDebug() << "jump to chat item , uid is " << si->uid_;
            ui->chatUserList->scrollToItem(find_iter.value());
            ui->sideChatLabel->setSelected(true);
            setSelectChatItem(si->uid_);
            //更新聊天界面信息
            setSelectChatPage(si->uid_);
            slotSideChat();
            return;
        }

        //如果没找到，则创建新的插入listwidget

        auto* chatUserWid = new ChatUserWid();
        auto user_info = std::make_shared<UserInfo>(si);
        chatUserWid->setInfo(user_info);
        QListWidgetItem* item = new QListWidgetItem;
        //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
        item->setSizeHint(chatUserWid->sizeHint());
        ui->chatUserList->insertItem(0, item);
        ui->chatUserList->setItemWidget(item, chatUserWid);

        chatItemsAddeds_.insert(si->uid_, item);

        ui->sideChatLabel->setSelected(true);
        setSelectChatItem(si->uid_);
        //更新聊天界面信息
        setSelectChatPage(si->uid_);
        slotSideChat();

}

void ChatDialog::showSearch(bool status)
{
    if (status)
    {
        ui->chatUserList->hide();
        ui->chatConnList->hide();
        ui->chatSearchList->show();
        mode_ = ChatUIMode::SearchMode;
    }
    else if (state_ == ChatUIMode::ChatMode)
    {
        ui->chatUserList->show();
        ui->chatConnList->hide();
        ui->chatSearchList->hide();
        mode_ = ChatUIMode::ChatMode;
    }
    else if (state_ == ChatUIMode::ContactMode)
    {
        ui->chatUserList->hide();
        ui->chatSearchList->hide();
        ui->chatConnList->show();
        mode_ = ChatUIMode::ContactMode;
    }
}

void ChatDialog::addLBGroup(StateWidget *lb) { lbList_.push_back(lb); }

void ChatDialog::handleGlobalMousePress(QMouseEvent *event)
{
    // 实现点击位置的判断和处理逻辑
    // 先判断是否处于搜索模式，如果不处于搜索模式则直接返回
    if (mode_ != ChatUIMode::SearchMode)
    {
        return;
    }

    // 将鼠标点击位置转换为搜索列表坐标系中的位置
    QPoint posInSearchList =
        ui->chatSearchList->mapFromGlobal(event->globalPos());
    // 判断点击位置是否在聊天列表的范围内
    if (!ui->chatSearchList->rect().contains(posInSearchList))
    {
        // 如果不在聊天列表内，清空输入框
        ui->chatSearchEdit->clear();
        showSearch(false);
    }
}

void ChatDialog::setSelectChatItem(int uid)
{
    if(ui->chatUserList->count() <= 0)
    {
            return;
    }

    if(uid == 0)
    {
        ui->chatUserList->setCurrentRow(0);
        QListWidgetItem *firstItem = ui->chatUserList->item(0);
        if(!firstItem)
        {
            return;
        }

        //转为widget
        QWidget *widget = ui->chatUserList->itemWidget(firstItem);
        if(!widget)
        {
            return;
        }

        auto conItem = qobject_cast<ChatUserWid*>(widget);
        if(!conItem)
        {
            return;
        }

        curChatUid_ = conItem->getUserInfo()->_uid;

        return;
    }

    auto find_iter = chatItemsAddeds_.find(uid);
    if(find_iter == chatItemsAddeds_.end())
    {
        qDebug() << "uid " <<uid<< " not found, set curent row 0";
        ui->chatUserList->setCurrentRow(0);
        return;
    }

    ui->chatUserList->setCurrentItem(find_iter.value());

    curChatUid_ = uid;
}

void ChatDialog::setSelectChatPage(int uid)
{
}

void ChatDialog::loadMoreChatUser()
{
    auto friendList = UserMgr::getInstance_()->getChatListPerPage();
    if(!friendList.empty())
    {
        for(auto &friendEle : friendList)
        {
            auto it = chatItemsAddeds_.find(friendEle->_uid);
            {
                if(it != chatItemsAddeds_.end())
                {
                    continue;
                }
                auto *chatUserWid = new ChatUserWid();
                auto userInfo = std::make_shared<UserInfo>(friendEle);
                chatUserWid->setInfo(userInfo);
                QListWidgetItem *item = new QListWidgetItem;
                item->setSizeHint(chatUserWid->sizeHint());
                ui->chatUserList->addItem(item);
                ui->chatUserList->setItemWidget(item, chatUserWid);
                chatItemsAddeds_.insert(friendEle->_uid, item);
            }
        }
    }
    UserMgr::getInstance_()->updateChatLoadedCount();
}

void ChatDialog::loadMoreConUser()
{
    auto friend_list = UserMgr::getInstance_()->getConListPerPage();
    if (friend_list.empty() == false)
    {
        for(auto & friend_ele : friend_list)
        {
            auto *chatUserWid = new ConUserItem();
            chatUserWid->setInfo(friend_ele->_uid,friend_ele->_name,
                                   friend_ele->_icon);
            QListWidgetItem *item = new QListWidgetItem;
            //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
            item->setSizeHint(chatUserWid->sizeHint());
            ui->chatUserList->addItem(item);
            ui->chatUserList->setItemWidget(item, chatUserWid);
        }

        //更新已加载条目
        UserMgr::getInstance_()->updateContactLoadedCount();
    }
}
