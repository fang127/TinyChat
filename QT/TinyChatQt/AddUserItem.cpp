#include "AddUserItem.h"
#include "ui_AddUserItem.h"

AddUserItem::AddUserItem(QWidget *parent) :
    ListItemBase(parent),
    ui(new Ui::AddUserItem)
{
    ui->setupUi(this);
    setItemType(ListItemType::ADD_USER_TIP_ITEM);
}

AddUserItem::~AddUserItem()
{
    delete ui;
}
