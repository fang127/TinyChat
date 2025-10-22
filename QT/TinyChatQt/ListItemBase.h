#ifndef LISTITEMBASE_H
#define LISTITEMBASE_H

#include <QWidget>
#include "global.h"

class ListItemBase : public QWidget
{
    Q_OBJECT
public:
    explicit ListItemBase(QWidget *parent = nullptr);

    void setItemType(ListItemType itemType);

    ListItemType getItemType();
private:
    ListItemType itemType_;

public slots:

signals:
};

#endif // LISTITEMBASE_H
