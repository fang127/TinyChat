#include "ApplyFriend.h"
#include "ClickedOnce.h"
#include "FriendLabel.h"
#include "TcpMgr.h"
#include "UserMgr.h"
#include "ui_ApplyFriend.h"
#include <QScrollBar>

ApplyFriend::ApplyFriend(QWidget *parent)
    : QDialog(parent), ui(new Ui::ApplyFriend)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    this->setObjectName("ApplyFriend");
    this->setModal(true);
    ui->nameEdit->setPlaceholderText(tr("happy"));
    ui->lbEdit->setPlaceholderText("搜索、添加标签");
    ui->backEdit->setPlaceholderText("添加备注");

    ui->lbEdit->setMaxLength(21);
    ui->lbEdit->move(2, 2);
    ui->lbEdit->setFixedHeight(20);
    ui->lbEdit->setMaxLength(10);
    ui->inputWid->hide();

    tipCurPoint_ = QPoint(5, 5);
    tipData_ = {
        "同学",          "家人",           "菜鸟教程",       "C++ Primer",
        "Rust 程序设计", "父与子学Python", "nodejs开发指南", "go 语言开发指南",
        "游戏伙伴",      "金融投资",       "微信读书",       "拼多多拼友"};

    connect(ui->moreLB, &ClickedOnce::clicked, this,
            &ApplyFriend::showMoreLabel);
    initTipLBs();
    // 链接输入标签回车事件
    connect(ui->lbEdit, &CustomizeEdit::returnPressed, this,
            &ApplyFriend::slotLabelEnter);
    connect(ui->lbEdit, &CustomizeEdit::textChanged, this,
            &ApplyFriend::slotLabelTextChange);
    connect(ui->lbEdit, &CustomizeEdit::editingFinished, this,
            &ApplyFriend::slotLabelEditFinished);
    connect(ui->tipLB, &ClickedOnce::clicked, this,
            &ApplyFriend::slotAddFriendLabelByClickTip);

    ui->scrollArea->horizontalScrollBar()->setHidden(true);
    ui->scrollArea->verticalScrollBar()->setHidden(true);
    ui->scrollArea->installEventFilter(this);
    ui->sureBtn->setState("normal", "hover", "press");
    ui->cancelBtn->setState("normal", "hover", "press");
    // 连接确认和取消按钮的槽函数
    connect(ui->cancelBtn, &ClickedBtn::clicked, this,
            &ApplyFriend::slotApplyCancel);
    connect(ui->sureBtn, &ClickedBtn::clicked, this,
            &ApplyFriend::slotApplySure);
}

ApplyFriend::~ApplyFriend()
{
    qDebug() << "ApplyFriend destruct";
    delete ui;
}

void ApplyFriend::initTipLBs()
{
    int lines = 1;
    for (int i = 0; i < tipData_.size(); i++)
    {

        auto *lb = new ClickedLabel(ui->LBListWid);
        lb->setState("normal", "hover", "pressed", "selected_normal",
                     "selected_hover", "selected_pressed");
        lb->setObjectName("tipslb");
        lb->setText(tipData_[i]);
        connect(lb, &ClickedLabel::clicked, this,
                &ApplyFriend::slotChangeFriendLabelByTip);

        QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
        int textWidth =
            fontMetrics.horizontalAdvance(lb->text()); // 获取文本的宽度
        int textHeight = fontMetrics.height();         // 获取文本的高度

        if (tipCurPoint_.x() + textWidth + tip_offset > ui->LBListWid->width())
        {
            lines++;
            if (lines > 2)
            {
                delete lb;
                return;
            }
            // 换行
            tipCurPoint_.setX(tip_offset);
            tipCurPoint_.setY(tipCurPoint_.y() + textHeight + 15);
        }

        auto next_point = tipCurPoint_;

        addTipLBs(lb, tipCurPoint_, next_point, textWidth, textHeight);

        tipCurPoint_ = next_point;
    }
}

void ApplyFriend::addTipLBs(ClickedLabel *label,
                            QPoint curPoint,
                            QPoint &nextPoint,
                            int textWidth,
                            int textHeight)
{
    label->move(curPoint);
    label->show();
    addLabels_.insert(label->text(), label);
    addLabelKeys_.push_back(label->text());
    nextPoint.setX(label->pos().x() + textWidth + 15);
    nextPoint.setY(label->pos().y());
}

bool ApplyFriend::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->scrollArea && event->type() == QEvent::Enter)
    {
        ui->scrollArea->verticalScrollBar()->setHidden(false);
    }
    else if (obj == ui->scrollArea && event->type() == QEvent::Leave)
    {
        ui->scrollArea->verticalScrollBar()->setHidden(true);
    }
    return QObject::eventFilter(obj, event);
}

void ApplyFriend::setSearchInfo(std::shared_ptr<SearchInfo> si)
{
    si_ = si;
    auto applyname = UserMgr::getInstance_()->getName();
    auto bakname = si->name_;
    ui->nameEdit->setText(applyname);
    ui->backEdit->setText(bakname);
}

void ApplyFriend::resetLabels()
{
    auto max_width = ui->groupWid->width();
    auto label_height = 0;
    for (auto iter = friendLabels_.begin(); iter != friendLabels_.end(); iter++)
    {
        // todo... 添加宽度统计
        if (labelPoint_.x() + iter.value()->width() > max_width)
        {
            labelPoint_.setY(labelPoint_.y() + iter.value()->height() + 6);
            labelPoint_.setX(2);
        }

        iter.value()->move(labelPoint_);
        iter.value()->show();

        labelPoint_.setX(labelPoint_.x() + iter.value()->width() + 2);
        labelPoint_.setY(labelPoint_.y());
        label_height = iter.value()->height();
    }

    if (friendLabels_.isEmpty())
    {
        ui->lbEdit->move(labelPoint_);
        return;
    }

    if (labelPoint_.x() + MIN_APPLY_LABEL_ED_LEN > ui->groupWid->width())
    {
        ui->lbEdit->move(2, labelPoint_.y() + label_height + 6);
    }
    else
    {
        ui->lbEdit->move(labelPoint_);
    }
}

void ApplyFriend::addLabel(QString name)
{
    if (friendLabels_.find(name) != friendLabels_.end())
    {
        return;
    }

    auto tmplabel = new FriendLabel(ui->groupWid);
    tmplabel->setText(name);
    tmplabel->setObjectName("FriendLabel");

    auto max_width = ui->groupWid->width();
    // todo... 添加宽度统计
    if (labelPoint_.x() + tmplabel->width() > max_width)
    {
        labelPoint_.setY(labelPoint_.y() + tmplabel->height() + 6);
        labelPoint_.setX(2);
    }
    else
    {
    }

    tmplabel->move(labelPoint_);
    tmplabel->show();
    friendLabels_[tmplabel->text()] = tmplabel;
    friendLabelKeys_.push_back(tmplabel->text());

    connect(tmplabel, &FriendLabel::sigClose, this,
            &ApplyFriend::slotRemoveFriendLabel);

    labelPoint_.setX(labelPoint_.x() + tmplabel->width() + 2);

    if (labelPoint_.x() + MIN_APPLY_LABEL_ED_LEN > ui->groupWid->width())
    {
        ui->lbEdit->move(2, labelPoint_.y() + tmplabel->height() + 2);
    }
    else
    {
        ui->lbEdit->move(labelPoint_);
    }

    ui->lbEdit->clear();

    if (ui->groupWid->height() < labelPoint_.y() + tmplabel->height() + 2)
    {
        ui->groupWid->setFixedHeight(labelPoint_.y() + tmplabel->height() * 2 +
                                     2);
    }
}

void ApplyFriend::showMoreLabel()
{
    qDebug() << "receive more label clicked";
    ui->moreLBWid->hide();

    ui->LBListWid->setFixedWidth(325);
    tipCurPoint_ = QPoint(5, 5);
    auto next_point = tipCurPoint_;
    int textWidth;
    int textHeight;
    // 重拍现有的label
    for (auto &added_key : addLabelKeys_)
    {
        auto added_lb = addLabels_[added_key];

        QFontMetrics fontMetrics(added_lb->font()); // 获取QLabel控件的字体信息
        textWidth =
            fontMetrics.horizontalAdvance(added_lb->text()); // 获取文本的宽度
        textHeight = fontMetrics.height();                   // 获取文本的高度

        if (tipCurPoint_.x() + textWidth + tip_offset > ui->LBListWid->width())
        {
            tipCurPoint_.setX(tip_offset);
            tipCurPoint_.setY(tipCurPoint_.y() + textHeight + 15);
        }
        added_lb->move(tipCurPoint_);

        next_point.setX(added_lb->pos().x() + textWidth + 15);
        next_point.setY(tipCurPoint_.y());

        tipCurPoint_ = next_point;
    }

    // 添加未添加的
    for (int i = 0; i < tipData_.size(); i++)
    {
        auto iter = addLabels_.find(tipData_[i]);
        if (iter != addLabels_.end())
        {
            continue;
        }

        auto *lb = new ClickedLabel(ui->LBListWid);
        lb->setState("normal", "hover", "pressed", "selected_normal",
                     "selected_hover", "selected_pressed");
        lb->setObjectName("tipslb");
        lb->setText(tipData_[i]);
        connect(lb, &ClickedLabel::clicked, this,
                &ApplyFriend::slotChangeFriendLabelByTip);

        QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
        int textWidth =
            fontMetrics.horizontalAdvance(lb->text()); // 获取文本的宽度
        int textHeight = fontMetrics.height();         // 获取文本的高度

        if (tipCurPoint_.x() + textWidth + tip_offset > ui->LBListWid->width())
        {

            tipCurPoint_.setX(tip_offset);
            tipCurPoint_.setY(tipCurPoint_.y() + textHeight + 15);
        }

        next_point = tipCurPoint_;

        addTipLBs(lb, tipCurPoint_, next_point, textWidth, textHeight);

        tipCurPoint_ = next_point;
    }

    int diff_height =
        next_point.y() + textHeight + tip_offset - ui->LBListWid->height();
    ui->LBListWid->setFixedHeight(next_point.y() + textHeight + tip_offset);

    // qDebug()<<"after resize ui->LBListWid size is " << ui->LBListWid->size();
    ui->scrollContent->setFixedHeight(ui->scrollContent->height() +
                                      diff_height);
}

void ApplyFriend::slotLabelEnter()
{
    if (ui->lbEdit->text().isEmpty())
    {
        return;
    }
    auto text = ui->lbEdit->text();
    addLabel(ui->lbEdit->text());
    ui->inputWid->hide();

    auto find_it = std::find(tipData_.begin(), tipData_.end(), text);
    // 找到了就只需设置状态为选中即可
    if (find_it == tipData_.end())
    {
        tipData_.push_back(text);
    }

    // 判断标签展示栏是否有该标签
    auto find_add = addLabels_.find(text);
    if (find_add != addLabels_.end())
    {
        find_add.value()->setCurState(ClickLbState::Selected);
        return;
    }

    // 标签展示栏也增加一个标签, 并设置绿色选中
    auto *lb = new ClickedLabel(ui->LBListWid);
    lb->setState("normal", "hover", "pressed", "selected_normal",
                 "selected_hover", "selected_pressed");
    lb->setObjectName("tipslb");
    lb->setText(text);
    connect(lb, &ClickedLabel::clicked, this,
            &ApplyFriend::slotChangeFriendLabelByTip);
    qDebug() << "ui->LBListWid->width() is " << ui->LBListWid->width();
    qDebug() << "tipCurPoint_.x() is " << tipCurPoint_.x();

    QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
    int textWidth = fontMetrics.horizontalAdvance(lb->text()); // 获取文本的宽度
    int textHeight = fontMetrics.height();                     // 获取文本的高度
    qDebug() << "textWidth is " << textWidth;

    if (tipCurPoint_.x() + textWidth + tip_offset + 3 > ui->LBListWid->width())
    {

        tipCurPoint_.setX(5);
        tipCurPoint_.setY(tipCurPoint_.y() + textHeight + 15);
    }

    auto next_point = tipCurPoint_;

    addTipLBs(lb, tipCurPoint_, next_point, textWidth, textHeight);
    tipCurPoint_ = next_point;

    int diff_height =
        next_point.y() + textHeight + tip_offset - ui->LBListWid->height();
    ui->LBListWid->setFixedHeight(next_point.y() + textHeight + tip_offset);

    lb->setCurState(ClickLbState::Selected);

    ui->scrollContent->setFixedHeight(ui->scrollContent->height() +
                                      diff_height);
}

void ApplyFriend::slotRemoveFriendLabel(QString name)
{
    qDebug() << "receive close signal";

    labelPoint_.setX(2);
    labelPoint_.setY(6);

    auto find_iter = friendLabels_.find(name);

    if (find_iter == friendLabels_.end())
    {
        return;
    }

    auto find_key = friendLabelKeys_.end();
    for (auto iter = friendLabelKeys_.begin(); iter != friendLabelKeys_.end();
         iter++)
    {
        if (*iter == name)
        {
            find_key = iter;
            break;
        }
    }

    if (find_key != friendLabelKeys_.end())
    {
        friendLabelKeys_.erase(find_key);
    }

    delete find_iter.value();

    friendLabels_.erase(find_iter);

    resetLabels();

    auto find_add = addLabels_.find(name);
    if (find_add == addLabels_.end())
    {
        return;
    }

    find_add.value()->resetNormalState();
}

void ApplyFriend::slotChangeFriendLabelByTip(QString lbtext, ClickLbState state)
{
    auto it = addLabels_.find(lbtext);
    if (it == addLabels_.end())
    {
        return;
    }

    if (state == ClickLbState::Selected)
    {
        // 编写添加逻辑
        addLabel(lbtext);
        return;
    }

    if (state == ClickLbState::Normal)
    {
        // 编写删除逻辑
        slotRemoveFriendLabel(lbtext);
        return;
    }
}

void ApplyFriend::slotLabelTextChange(const QString &text)
{
    if (text.isEmpty())
    {
        ui->tipLB->setText("");
        ui->inputWid->hide();
        return;
    }

    auto iter = std::find(tipData_.begin(), tipData_.end(), text);
    if (iter == tipData_.end())
    {
        auto new_text = add_prefix + text;
        ui->tipLB->setText(new_text);
        ui->inputWid->show();
        return;
    }
    ui->tipLB->setText(text);
    ui->inputWid->show();
}

void ApplyFriend::slotLabelEditFinished() { ui->inputWid->hide(); }

void ApplyFriend::slotAddFriendLabelByClickTip(QString text)
{
    int index = text.indexOf(add_prefix);
    if (index != -1)
    {
        text = text.mid(index + add_prefix.length());
    }
    addLabel(text);

    auto find_it = std::find(tipData_.begin(), tipData_.end(), text);
    // 找到了就只需设置状态为选中即可
    if (find_it == tipData_.end())
    {
        tipData_.push_back(text);
    }

    // 判断标签展示栏是否有该标签
    auto find_add = addLabels_.find(text);
    if (find_add != addLabels_.end())
    {
        find_add.value()->setCurState(ClickLbState::Selected);
        return;
    }

    // 标签展示栏也增加一个标签, 并设置绿色选中
    auto *lb = new ClickedLabel(ui->LBListWid);
    lb->setState("normal", "hover", "pressed", "selected_normal",
                 "selected_hover", "selected_pressed");
    lb->setObjectName("tipslb");
    lb->setText(text);
    connect(lb, &ClickedLabel::clicked, this,
            &ApplyFriend::slotChangeFriendLabelByTip);
    qDebug() << "ui->LBListWid->width() is " << ui->LBListWid->width();
    qDebug() << "tipCurPoint_.x() is " << tipCurPoint_.x();

    QFontMetrics fontMetrics(lb->font()); // 获取QLabel控件的字体信息
    int textWidth = fontMetrics.horizontalAdvance(lb->text()); // 获取文本的宽度
    int textHeight = fontMetrics.height();                     // 获取文本的高度
    qDebug() << "textWidth is " << textWidth;

    if (tipCurPoint_.x() + textWidth + tip_offset + 3 > ui->LBListWid->width())
    {

        tipCurPoint_.setX(5);
        tipCurPoint_.setY(tipCurPoint_.y() + textHeight + 15);
    }

    auto next_point = tipCurPoint_;

    addTipLBs(lb, tipCurPoint_, next_point, textWidth, textHeight);
    tipCurPoint_ = next_point;

    int diff_height =
        next_point.y() + textHeight + tip_offset - ui->LBListWid->height();
    ui->LBListWid->setFixedHeight(next_point.y() + textHeight + tip_offset);

    lb->setCurState(ClickLbState::Selected);

    ui->scrollContent->setFixedHeight(ui->scrollContent->height() +
                                      diff_height);
}

void ApplyFriend::slotApplySure()
{
    qDebug() << "slot apply sure called";
    this->hide();
    deleteLater();
}

void ApplyFriend::slotApplyCancel()
{
    qDebug() << "slot apply cancel";
    this->hide();
    deleteLater();
}
