#include "ClickedOnce.h"

ClickedOnce::ClickedOnce(QWidget *parent)
{
    setCursor(Qt::PointingHandCursor);
}

void ClickedOnce::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        emit clicked(this->text());
        return;
    }
    QLabel::mouseReleaseEvent(event);
}

