#include "SearchList.h"
#include "TcpMgr.h"
#include "CustomizeEdit.h"
#include "AddUserItem.h"
#include "FindSuccessDialog.h"

SearchList::SearchList(QWidget *parent)
    :QListWidget(parent),findDlg_(nullptr), searchEdit_(nullptr), sendPending_(false)
{
    Q_UNUSED(parent);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);
    //连接点击的信号和槽
    connect(this, &QListWidget::itemClicked, this, &SearchList::slotItemClicked);
    //添加条目
    addTipItem();
    //连接搜索条目
    connect(TcpMgr::getInstance_().get(), &TcpMgr::sigUserSearch, this, &SearchList::slotUserSearch);
}

void SearchList::closeFindDlg()
{
    if(findDlg_)
    {
        findDlg_->hide();
        findDlg_ = nullptr;
    }
}

void SearchList::setSearchEdit(QWidget *edit)
{

}

void SearchList::waitPending(bool pending)
{

}

void SearchList::addTipItem()
{
    auto *invalid_item = new QWidget();
    QListWidgetItem *itemTmp = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    itemTmp->setSizeHint(QSize(250,10));
    this->addItem(itemTmp);
    invalid_item->setObjectName("invalid_item");
    this->setItemWidget(itemTmp, invalid_item);
    itemTmp->setFlags(itemTmp->flags() & ~Qt::ItemIsSelectable);


    auto *addUserItem = new AddUserItem();
    QListWidgetItem *item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(addUserItem->sizeHint());
    this->addItem(item);
    this->setItemWidget(item, addUserItem);
}

void SearchList::slotItemClicked(QListWidgetItem *item)
{
    QWidget *widget = this->itemWidget(item); //获取自定义widget对象
    if(!widget)
    {
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }

    // 对自定义widget进行操作， 将item 转化为基类ListItemBase
    ListItemBase *customItem = qobject_cast<ListItemBase*>(widget);
    if(!customItem)
    {
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }

    auto itemType = customItem->getItemType();
    if(itemType == ListItemType::INVALID_ITEM)
    {
        qDebug()<< "slot invalid item clicked ";
        return;
    }

    if(itemType == ListItemType::ADD_USER_TIP_ITEM)
    {

        //add code ...
        findDlg_ = std::make_shared<FindSuccessDialog>(this);
        auto si = std::make_shared<SearchInfo>(0,"Donk","Donk","hello , my friend!",0);
        std::dynamic_pointer_cast<FindSuccessDialog>(findDlg_)->setSearchInfo(si);
        findDlg_->show();
        return;
    }

    //清楚弹出框
    closeFindDlg();
}

void SearchList::slotUserSearch(std::shared_ptr<SearchInfo> si)
{

}
