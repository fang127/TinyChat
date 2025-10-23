#include "ListItemBase.h"

#include <QStyleOption>
#include <QPainter>

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

void ListItemBase::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}


