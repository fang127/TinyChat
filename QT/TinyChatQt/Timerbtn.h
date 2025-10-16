#ifndef TIMERBTN_H
#define TIMERBTN_H

#include <QPushButton>
#include <QTimer>

class TimerBtn : public QPushButton
{
public:
    TimerBtn(QWidget *parent = nullptr);
    ~TimerBtn();

    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    QTimer *timer_;
    int counter_;
};

#endif // TIMERBTN_H
