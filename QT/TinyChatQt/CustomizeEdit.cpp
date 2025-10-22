#include "CustomizeEdit.h"

CustomizeEdit::CustomizeEdit(QWidget *parent) : QLineEdit(parent),maxLen_(0)
{
    connect(this, &QLineEdit::textChanged, this, &CustomizeEdit::limitTextLength);
}

void CustomizeEdit::setMaxLength(int maxLen)
{
    maxLen_ = maxLen;
}

void CustomizeEdit::focusOutEvent(QFocusEvent *event)
{
    // 执行失去焦点时的处理逻辑
    //qDebug() << "CustomizeEdit focusout";
    // 调用基类的focusOutEvent()方法，保证基类的行为得到执行
    QLineEdit::focusOutEvent(event);
    //发送失去焦点得信号
    emit sigFoucusOut();
}

void CustomizeEdit::limitTextLength(const QString &text)
{
    if(maxLen_ <= 0)
    {
        return;
    }

    QByteArray byteArray = text.toUtf8();

    if(byteArray.size() > maxLen_)
    {
        byteArray = byteArray.left(maxLen_);
        this->setText(QString::fromUtf8(byteArray));
    }
}
