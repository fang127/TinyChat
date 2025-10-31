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
    void setInfo(std::shared_ptr<FriendInfo> friendInfo);
    std::shared_ptr<UserInfo> getUserInfo();
    void updateLastMsg(std::vector<std::shared_ptr<TextChatData>> msgs);
private:
    Ui::ChatUserWid *ui;
    std::shared_ptr<UserInfo> userInfo_;
};

#endif // CHATUSERWID_H
