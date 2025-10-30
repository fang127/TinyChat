#ifndef CHATUSERWID_H
#define CHATUSERWID_H

#include <QWidget>
#include "ListItemBase.h"
#include "UserData.h"

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

    void setInfo(std::shared_ptr<UserInfo> userInfo);

    std::shared_ptr<UserInfo> getUserInfo();

private:
    Ui::ChatUserWid *ui;
    std::shared_ptr<UserInfo> userInfo_;
};

#endif // CHATUSERWID_H
