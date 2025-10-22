#ifndef CHATUSERLIST_H
#define CHATUSERLIST_H

#include <QListWidget>
#include <QWheelEvent>
#include <QEvent>
#include <QScrollBar>
#include <QDebug>

class ChatUserList : public QListWidget
{
    Q_OBJECT
public:
    ChatUserList() = default;
    ChatUserList(QWidget *parent);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
signals:
    void sigLoadingChatUser();
};

#endif // CHATUSERLIST_H
