#include "StateWidget.h"

#include <QBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>

StateWidget::StateWidget(QWidget *parent)
    : QWidget(parent), curstate_(ClickLbState::Normal)
{
    // 设置鼠标为手形状
    setCursor(Qt::PointingHandCursor);
    // 添加红点
    addRedPoint();
}

void StateWidget::setState(const QString &normal,
                           const QString &normalHover,
                           const QString &normalPress,
                           const QString &selected,
                           const QString &selectedHover,
                           const QString &selectedPress)
{
    normal_ = normal;
    normalHover_ = normalHover;
    normalPress_ = normalPress;

    selected_ = selected;
    selectedHover_ = selectedHover;
    selectedPress_ = selectedPress;

    setProperty("state", normal);
    repolish(this);
}

ClickLbState StateWidget::getCurState() { return curstate_; }

void StateWidget::clearState()
{
    curstate_ = ClickLbState::Normal;
    setProperty("state", normal_);
    repolish(this);
    update();
}

void StateWidget::setSelected(bool selected)
{
    if (selected)
    {
        curstate_ = ClickLbState::Selected;
        setProperty("state", selected_);
        repolish(this);
        update();
        return;
    }

    curstate_ = ClickLbState::Normal;
    setProperty("state", normal_);
    repolish(this);
    update();
    return;
}

// 设置红点
void StateWidget::addRedPoint()
{
    redPoint_ = new QLabel();
    redPoint_->setObjectName("redPoint");
    QVBoxLayout *layout2 = new QVBoxLayout;
    redPoint_->setAlignment(Qt::AlignCenter);
    layout2->addWidget(redPoint_);
    layout2->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout2);
    redPoint_->setVisible(false);
}

void StateWidget::showRedPoint(bool show) { redPoint_->setVisible(true); }

void StateWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    return;
}

void StateWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (curstate_ == ClickLbState::Selected)
        {
            qDebug() << "PressEvent , already to selected press: "
                     << selectedPress_;
            // emit clicked();
            //  调用基类的mousePressEvent以保证正常的事件处理
            QWidget::mousePressEvent(event);
            return;
        }

        if (curstate_ == ClickLbState::Normal)
        {
            qDebug() << "PressEvent , change to selected press: "
                     << selectedPress_;
            curstate_ = ClickLbState::Selected;
            setProperty("state", selectedPress_);
            repolish(this);
            update();
        }

        return;
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QWidget::mousePressEvent(event);
}

void StateWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (curstate_ == ClickLbState::Normal)
        {
            // qDebug()<<"ReleaseEvent , change to normal hover: "<<
            // _normal_hover;
            setProperty("state", normalHover_);
            repolish(this);
            update();
        }
        else
        {
            // qDebug()<<"ReleaseEvent , change to select hover: "<<
            // _selected_hover;
            setProperty("state", selectedHover_);
            repolish(this);
            update();
        }
        emit clicked();
        return;
    }
    // 调用基类的mousePressEvent以保证正常的事件处理
    QWidget::mouseReleaseEvent(event);
}

void StateWidget::enterEvent(QEnterEvent *event)
{
    // 在这里处理鼠标悬停进入的逻辑
    if (curstate_ == ClickLbState::Normal)
    {
        // qDebug()<<"enter , change to normal hover: "<< _normal_hover;
        setProperty("state", normalHover_);
        repolish(this);
        update();
    }
    else
    {
        // qDebug()<<"enter , change to selected hover: "<< _selected_hover;
        setProperty("state", selectedHover_);
        repolish(this);
        update();
    }

    QWidget::enterEvent(event);
}

void StateWidget::leaveEvent(QEvent *event)
{
    // 在这里处理鼠标悬停离开的逻辑
    if (curstate_ == ClickLbState::Normal)
    {
        // qDebug()<<"leave , change to normal : "<< _normal;
        setProperty("state", normal_);
        repolish(this);
        update();
    }
    else
    {
        // qDebug()<<"leave , change to select normal : "<< _selected;
        setProperty("state", selected_);
        repolish(this);
        update();
    }
    QWidget::leaveEvent(event);
}
