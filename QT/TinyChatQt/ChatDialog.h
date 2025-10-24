#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include "StateWidget.h"
#include "global.h"

#include <QDialog>

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
    // 测试
    std::vector<QString> strs = {
        "hello world !", "nice to meet u", "New year，new life",
        "You have to love yourself",
        "My love is written in the wind ever since the whole world is you"};

    std::vector<QString> heads = {":/res/head_1.jpg", ":/res/head_2.jpg",
                                  ":/res/head_3.jpg", ":/res/head_4.jpg",
                                  ":/res/head_5.jpg"};

    std::vector<QString> names = {"llfc", "zack",   "golang", "cpp",
                                  "java", "nodejs", "python", "rust"};
};

#endif // CHATDIALOG_H
