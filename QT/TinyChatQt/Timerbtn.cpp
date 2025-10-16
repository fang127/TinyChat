#include "Timerbtn.h"

#include <QMouseEvent>
#include <QDebug>

TimerBtn::TimerBtn(QWidget *parent) : QPushButton(parent),counter_(10)
{
    timer_ = new QTimer(this); // 定时器

    connect(timer_,&QTimer::timeout,[this](){
        counter_--;
        if(counter_ <= 0)
        {
            timer_->stop();
            counter_ = 10;
            this->setText("获取");
            this->setEnabled(true);
            return;
        }
        this->setText(QString::number(counter_));
    });
}

TimerBtn::~TimerBtn()
{
    timer_->stop();
}

void TimerBtn::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        qDebug() << "MyButton was released:";
        this->setEnabled(false);
        this->setText(QString::number(counter_));
        timer_->start(1000); // 1000毫秒触发一次timeout，从而调用connect中的对象
        emit clicked();
    }

    // 调用基类的mouseReleaseEvent确保正常的事件处理
    QPushButton::mouseReleaseEvent(e);
}
