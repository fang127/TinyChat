#ifndef APPLYFRIEND_H
#define APPLYFRIEND_H

#include <QDialog>
#include "ClickedLabel.h"
#include "UserData.h"
#include "FriendLabel.h"

namespace Ui {
class ApplyFriend;
}

class ApplyFriend : public QDialog
{
    Q_OBJECT

public:
    explicit ApplyFriend(QWidget *parent = nullptr);
    ~ApplyFriend();

    void initTipLBs();
    void addTipLBs(ClickedLabel *label,QPoint curPoint, QPoint &nextPoint, int textWidth, int textHeight);
    bool eventFilter(QObject *obj, QEvent *event);
    void setSearchInfo(std::shared_ptr<SearchInfo> si);

private:
    Ui::ApplyFriend *ui;
    void resetLabels();

    //已经创建好的标签
    QMap<QString, ClickedLabel*> addLabels_;
    std::vector<QString> addLabelKeys_;
    QPoint labelPoint_;
    //用来在输入框显示添加新好友的标签
    QMap<QString, FriendLabel*> friendLabels_;
    std::vector<QString> friendLabelKeys_;
    void addLabel(QString name);
    std::vector<QString> tipData_;
    QPoint tipCurPoint_;
    std::shared_ptr<SearchInfo> si_;
public slots:
    // 显示更多label标签
    void showMoreLabel();
    // 输入label按下回车触发将标签加入展示栏
    void slotLabelEnter();
    // 点击关闭，移除展示栏好友便签
    void slotRemoveFriendLabel(QString);
    // 通过点击tip实现增加和减少好友便签
    void slotChangeFriendLabelByTip(QString, ClickLbState);
    // 输入框文本变化显示不同提示
    void slotLabelTextChange(const QString& text);
    // 输入框输入完成
    void slotLabelEditFinished();
    // 输入标签显示提示框，点击提示框内容后添加好友便签
    void slotAddFirendLabelByClickTip(QString text);
    // 处理确认回调
    void slotApplySure();
    // 处理取消回调
    void slotApplyCancel();
};

#endif // APPLYFRIEND_H
