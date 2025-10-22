#ifndef CHATUSERWID_H
#define CHATUSERWID_H

#include <QWidget>
#include "ListItemBase.h"

namespace Ui {
class ChatUserWid;
}

class ChatUserWid : public ListItemBase
{
    Q_OBJECT

public:
    explicit ChatUserWid(QWidget *parent = nullptr);
    ~ChatUserWid();

    QSize sizeHint()const override;

    void setInfo(const QString &name, const QString &head, const QString &msg);
private:
    Ui::ChatUserWid *ui;

    QString name_;
    QString head_;
    QString msg_;
};

#endif // CHATUSERWID_H
