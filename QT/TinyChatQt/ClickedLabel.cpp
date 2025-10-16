#include "ClickedLabel.h"

#include <QMouseEvent>

ClickedLabel::ClickedLabel(QWidget *parent) : QLabel(parent),curstate_(ClickLbState::Normal)
{

}

void ClickedLabel::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(curstate_ == ClickLbState::Normal)
        {
            qDebug() << "clicked, change to selected hover: " << selectedHover_;
            curstate_ = ClickLbState::Selected;
            setProperty("state", selectedHover_);
            repolish(this);
            update();
        }
        else
        {
            qDebug() << "clicked, change to normal hover: " << normalHover_;
            // set CSS
            curstate_ = ClickLbState::Normal;
            setProperty("state", normalHover_);
            repolish(this);
            update();
        }
        emit clicked();
    }
    // base func to comfirm normal event was handle
    QLabel::mousePressEvent(event);
}

void ClickedLabel::enterEvent(QEnterEvent *event)
{
    if(curstate_ == ClickLbState::Normal)
    {
        qDebug() << "enter, change to normal hover: " << normalHover_;
        setProperty("state",normalHover_);
        repolish(this);
        update();
    }
    else
    {
        qDebug() << "enter, change to selected hover: " << selectedHover_;
        setProperty("state",selectedHover_);
        repolish(this);
        update();
    }

    QLabel::enterEvent(event);
}

void ClickedLabel::leaveEvent(QEvent *event)
{
    if(curstate_ == ClickLbState::Normal)
    {
        qDebug() << "leave, change to normal hover: " << normal_;
        setProperty("state",normal_);
        repolish(this);
        update();
    }
    else
    {
        qDebug() << "leave, change to selected hover: " << selected_;
        setProperty("state",selected_);
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
