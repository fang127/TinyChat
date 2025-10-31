#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include "StateWidget.h"
#include "global.h"
#include "UserData.h"

#include <QDialog>
#include <QListWidgetItem>
namespace Ui
{
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();
    // 测试
    void addChatUserList(); // 初始化聊天
    void clearLabelState(StateWidget *lb);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void slotLoadingChatUser();
    void slotLoadingContact();
    void slotSideChat();
    void slotSideContact();
    void slotTextChanged(const QString &str);
    void slotApplyFriend(std::shared_ptr<AddFriendApply> apply);
    void slotAddAuthFriend(std::shared_ptr<AuthInfo> authInfo);
    void slotAuthRsp(std::shared_ptr<AuthRsp> authRsp);
    void slotJumpChatItem(std::shared_ptr<SearchInfo> si);
    void slotFriendInfoPage(std::shared_ptr<UserInfo> userInfo);
    void slotSwitchApplyFriendPage();
    void slotJumpChatFromInfoPage(std::shared_ptr<UserInfo> userInfo);
    void slotItemClicked(QListWidgetItem *item);
    void slotAppendSendChatMsg(std::shared_ptr<TextChatData> msgData);
    void slotTextChatMsg(std::shared_ptr<TextChatMsg> msg);
private:
    void showSearch(bool status = false);
    void addLBGroup(StateWidget *lb);
    void handleGlobalMousePress(QMouseEvent *event);
    void setSelectChatItem(int uid = 0);
    void setSelectChatPage(int uid = 0);
    void loadMoreChatUser(); // 加载更多聊天
    void loadMoreConUser(); // 加载更多联系人
    void updateChatMsg(std::vector<std::shared_ptr<TextChatData> > msgdata);
    Ui::ChatDialog *ui;
    ChatUIMode mode_;
    ChatUIMode state_;
    bool loading_;
    QList<StateWidget *> lbList_;
    QWidget *lastWidget_;
    QMap<int, QListWidgetItem*> chatItemsAddeds_; // 正在聊天的item
    int curChatUid_; // 当前正在聊天的uid
};

#endif // CHATDIALOG_H
