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
    void addChatUserList();
    void clearLabelState(StateWidget *lb);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void slotLoadingChatUser();
    void slotSideChat();
    void slotSideContact();
    void slotTextChanged(const QString &str);
    void slotApplyFriend(std::shared_ptr<AddFriendApply> apply);
    void slotAddAuthFriend(std::shared_ptr<AuthInfo> authInfo);
    void slotAuthRsp(std::shared_ptr<AuthRsp> authRsp);
private:
    void showSearch(bool status = false);
    void addLBGroup(StateWidget *lb);
    void handleGlobalMousePress(QMouseEvent *event);

    Ui::ChatDialog *ui;
    ChatUIMode mode_;
    ChatUIMode state_;
    bool loading_;
    QList<StateWidget *> lbList_;
    QWidget *lastWidget_;
    QMap<int, QListWidgetItem*> chatItemsAddeds_;
};

#endif // CHATDIALOG_H
