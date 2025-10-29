#include "ClickedLabel.h"
#include "global.h"

#include <QMouseEvent>

ClickedLabel::ClickedLabel(QWidget *parent) : QLabel(parent),curstate_(ClickLbState::Normal)
{
    this->setCursor(Qt::PointingHandCursor);
}

// 鼠标点击事件
void ClickedLabel::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) // 仅处理左键
    {
        if(curstate_ == ClickLbState::Normal)
        {
            // qDebug() << "clicked, change to selected hover: " << selectedPress_;
            curstate_ = ClickLbState::Selected;// 切换到Selected状态：更新状态→设置选中状态的悬停样式→刷新
            setProperty("state", selectedPress_);
            repolish(this);
            update();
        }
        else
        {
            // qDebug() << "clicked, change to normal hover: " << normalPress_;
            // set CSS
            curstate_ = ClickLbState::Normal;
            setProperty("state", normalPress_);
            repolish(this);// 重新应用样式表
            update();// 触发重绘
        }
        return;
    }
    // base func to comfirm normal event was handle
    QLabel::mousePressEvent(event);
}

void ClickedLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) // 仅处理左键
    {
        if(curstate_ == ClickLbState::Normal)
        {
            // qDebug() << "clicked, change to selected hover: " << normalHover_;
            // curstate_ = ClickLbState::Selected;// 切换到Selected状态：更新状态→设置选中状态的悬停样式→刷新
            setProperty("state", normalHover_);
            repolish(this);
            update();
        }
        else
        {
            // qDebug() << "clicked, change to normal hover: " << selectedHover_;
            // set CSS
            // curstate_ = ClickLbState::Normal;
            setProperty("state", selectedHover_);
            repolish(this);// 重新应用样式表
            update();// 触发重绘
        }
        emit clicked(this->text(), curstate_);
        return;
    }
    // base func to comfirm normal event was handle
    QLabel::mouseReleaseEvent(event);
}

// 鼠标进入事件
void ClickedLabel::enterEvent(QEnterEvent *event)
{
    if(curstate_ == ClickLbState::Normal)
    {
        // qDebug() << "enter, change to normal hover: " << normalHover_;
        setProperty("state",normalHover_);// 默认状态→显示默认悬停样式
        repolish(this);
        update();
    }
    else
    {
        // qDebug() << "enter, change to selected hover: " << selectedHover_;
        setProperty("state",selectedHover_);// 选中状态→显示选中悬停样式
        repolish(this);
        update();
    }

    QLabel::enterEvent(event);
}

// 鼠标离开事件
void ClickedLabel::leaveEvent(QEvent *event)
{
    if(curstate_ == ClickLbState::Normal)
    {
        // qDebug() << "leave, change to normal hover: " << normal_;
        setProperty("state",normal_);// 默认状态→回到默认样式
        repolish(this);
        update();
    }
    else
    {
        // qDebug() << "leave, change to selected hover: " << selected_;
        setProperty("state",selected_);// 选中状态→回到选中默认样式
        repolish(this);
        update();
    }

    QLabel::leaveEvent(event);
}

void ClickedLabel::setState(QString normal, QString hover, QString press, QString select, QString selectHover, QString selectPress)
{
    normal_ = normal;
    normalHover_ = hover;
    normalPress_ = press;

    selected_ = select;
    selectedHover_ = selectHover;
    selectedPress_ = selectPress;

    setProperty("state",normal);
    repolish(this);
}

ClickLbState ClickedLabel::getCurState()
{
    return curstate_;
}

bool ClickedLabel::setCurState(ClickLbState state)
{
    curstate_ = state;
    if(curstate_ == ClickLbState::Normal)
    {
        setProperty("state",normal_);
        repolish(this);
    }
    else if(curstate_ == ClickLbState::Selected)
    {
        setProperty("state", selected_);
        repolish(this);
    }

    return true;
}

void ClickedLabel::resetNormalState()
{
    curstate_ = ClickLbState::Normal;
    setProperty("state", normal_);
    repolish(this);
}
