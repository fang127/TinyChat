#include "SearchList.h"
#include "AddUserItem.h"
#include "CustomizeEdit.h"
#include "FindSuccessDialog.h"
#include "TcpMgr.h"
#include "findfaildialog.h"

#include <QJsonDocument>

SearchList::SearchList(QWidget *parent)
    : QListWidget(parent), findDlg_(nullptr), searchEdit_(nullptr),
      sendPending_(false)
{
    Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);
    // 连接点击的信号和槽
    connect(this, &QListWidget::itemClicked, this,
            &SearchList::slotItemClicked);
    // 添加条目
    addTipItem();
    // 连接搜索条目
    connect(TcpMgr::getInstance_().get(), &TcpMgr::sigUserSearch, this,
            &SearchList::slotUserSearch);
}

void SearchList::closeFindDlg()
{
    if (findDlg_)
    {
        findDlg_->hide();
        findDlg_ = nullptr;
    }
}

void SearchList::setSearchEdit(QWidget *edit)
{
    searchEdit_ = edit;
}

void SearchList::waitPending(bool pending)
{
    // 正在发送状态
    if (pending == true)
    {
        loadingDialog_ = new LoadDialog(this);
        loadingDialog_->setModal(true);
        loadingDialog_->show();
        sendPending_ = pending;
    }
    else
    {
        loadingDialog_->hide();
        loadingDialog_->deleteLater();
        sendPending_ = false;
    }
}

void SearchList::addTipItem()
{
    auto *invalid_item = new QWidget();
    QListWidgetItem *itemTmp = new QListWidgetItem;
    // qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    itemTmp->setSizeHint(QSize(250, 10));
    this->addItem(itemTmp);
    invalid_item->setObjectName("invalid_item");
    this->setItemWidget(itemTmp, invalid_item);
    itemTmp->setFlags(itemTmp->flags() & ~Qt::ItemIsSelectable);

    auto *addUserItem = new AddUserItem();
    QListWidgetItem *item = new QListWidgetItem;
    // qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(addUserItem->sizeHint());
    this->addItem(item);
    this->setItemWidget(item, addUserItem);
}

void SearchList::slotItemClicked(QListWidgetItem *item)
{
    QWidget *widget = this->itemWidget(item); // 获取自定义widget对象
    if (!widget)
    {
        qDebug() << "slot item clicked widget is nullptr";
        return;
    }

    // 对自定义widget进行操作， 将item 转化为基类ListItemBase
    ListItemBase *customItem = qobject_cast<ListItemBase *>(widget);
    if (!customItem)
    {
        qDebug() << "slot item clicked widget is nullptr";
        return;
    }

    auto itemType = customItem->getItemType();
    if (itemType == ListItemType::INVALID_ITEM)
    {
        qDebug() << "slot invalid item clicked ";
        return;
    }

    if (itemType == ListItemType::ADD_USER_TIP_ITEM)
    {
        if (sendPending_)
        {
            return;
        }

        if(!searchEdit_)
        {
            return;
        }

        waitPending(true);
        // 获取search的信息
        auto searchInfo = dynamic_cast<CustomizeEdit *>(searchEdit_);
        auto uidStr = searchInfo->text();
        QJsonObject jsonObj;
        jsonObj["uid"] = uidStr;
        QJsonDocument doc(jsonObj);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
        // 发送消息
        emit TcpMgr::getInstance_()
            -> sigSendData(ReqId::ID_SEARCH_USER_REQ, jsonData);
        return;
    }

    // 清楚弹出框
    closeFindDlg();
}

void SearchList::slotUserSearch(std::shared_ptr<SearchInfo> si)
{
    waitPending(false);
    if(si == nullptr)
    {
        findDlg_ = std::make_shared<FindFailDialog>(this);
    }
    else
    {
        // 1.已经是自己好友 2.未添加
        findDlg_ = std::make_shared<FindSuccessDialog>(this);

        std::dynamic_pointer_cast<FindSuccessDialog>(findDlg_)->setSearchInfo(si);
    }

    findDlg_->show();

}
