#include "PictureBubble.h"
#include <QLabel>

#define PIC_MAX_WIDTH 160
#define PIC_MAX_HEIGHT 90

PictureBubble::PictureBubble(const QPixmap &picture, ChatRole role, QWidget *parent)
    :BubbleFrame(role, parent)
{
    QLabel *lb = new QLabel();
    lb->setScaledContents(true);
    QPixmap pix = picture.scaled(QSize(PIC_MAX_WIDTH, PIC_MAX_HEIGHT), Qt::KeepAspectRatio);
    lb->setPixmap(pix);
    this->setWidget(lb);

    int leftMargin = this->layout()->contentsMargins().left();
    int rightMargin = this->layout()->contentsMargins().right();
    int vMargin = this->layout()->contentsMargins().bottom();
    setFixedSize(pix.width()+leftMargin + rightMargin, pix.height() + vMargin *2);
}
