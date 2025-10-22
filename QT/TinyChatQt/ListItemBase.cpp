#include "ListItemBase.h"

ListItemBase::ListItemBase(QWidget *parent) : QWidget(parent)
{

}

void ListItemBase::setItemType(ListItemType itemType)
{
    itemType_ = itemType;
}

ListItemType ListItemBase::getItemType()
{
    return itemType_;
}


