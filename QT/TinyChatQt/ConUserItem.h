#ifndef CONUSERITEM_H
#define CONUSERITEM_H

#include <QWidget>
#include "ListItemBase.h"
#include "UserData.h"

namespace Ui {
class ConUserItem;
}

class ConUserItem : public ListItemBase
{
    Q_OBJECT
public:
    explicit ConUserItem(QWidget *parent = nullptr);
    ~ConUserItem();
    QSize sizeHint() const override;
    void setInfo(std::shared_ptr<AuthInfo> auth_info);
    void setInfo(std::shared_ptr<AuthRsp> auth_rsp);
    void setInfo(int uid, QString name, QString icon);
    void showRedPoint(bool show = false);
private:
    Ui::ConUserItem *ui;
    std::shared_ptr<UserInfo> _info;

};

#endif // CONUSERITEM_H
