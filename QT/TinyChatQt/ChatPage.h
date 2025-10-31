#ifndef CHATPAGE_H
#define CHATPAGE_H

#include <QWidget>
#include "UserData.h"

namespace Ui
{
class ChatPage;
}

class ChatPage : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPage(QWidget *parent = nullptr);
    ~ChatPage();

    void setUserInfo(std::shared_ptr<UserInfo> userInfo);
    void appendChatMsg(std::shared_ptr<TextChatData> msg);
protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void on_sendBtn_clicked();

signals:
    void sigAppendSendChatMsg(std::shared_ptr<TextChatData> msg);
private:
    Ui::ChatPage *ui;
    std::shared_ptr<UserInfo> userInfo_;
};

#endif // CHATPAGE_H
