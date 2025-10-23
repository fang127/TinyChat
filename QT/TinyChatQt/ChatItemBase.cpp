#include "ChatItemBase.h"
#include <QSpacerItem>

ChatItemBase::ChatItemBase(ChatRole role, QWidget *parent) : role_(role), QWidget(parent)
{
    mpNameLabel_ = new QLabel();
    mpNameLabel_->setObjectName("chatUserName");
    QFont font("Microsoft YaHei");
    font.setPointSize(9);
    mpNameLabel_->setFont(font);
    mpNameLabel_->setFixedHeight(20);

    mpIconLabel_ = new QLabel();
    mpIconLabel_->setScaledContents(true);
    mpIconLabel_->setFixedSize(42,42);

    mpBubble_ = new QWidget();

    QGridLayout *pGLayout = new QGridLayout();
    pGLayout->setVerticalSpacing(3);
    pGLayout->setHorizontalSpacing(3);
    pGLayout->setContentsMargins(3,3,3,3);

    QSpacerItem *pSpacer = new QSpacerItem(40,20,QSizePolicy::Expanding,QSizePolicy::Minimum);

    if(role_ == ChatRole::Self)
    {
        mpNameLabel_->setContentsMargins(0,0,8,0);
        mpNameLabel_->setAlignment(Qt::AlignRight);
        pGLayout->addWidget(mpNameLabel_, 0,1, 1,1);
        pGLayout->addWidget(mpIconLabel_, 0, 2, 2,1, Qt::AlignTop);
        pGLayout->addItem(pSpacer, 1, 0, 1, 1);
        pGLayout->addWidget(mpBubble_, 1,1, 1,1);
        pGLayout->setColumnStretch(0, 2);
        pGLayout->setColumnStretch(1, 3);
    }
    else
    {
        mpNameLabel_->setContentsMargins(8,0,0,0);
        mpNameLabel_->setAlignment(Qt::AlignLeft);
        pGLayout->addWidget(mpIconLabel_, 0, 0, 2,1, Qt::AlignTop);
        pGLayout->addWidget(mpNameLabel_, 0,1, 1,1);
        pGLayout->addWidget(mpBubble_, 1,1, 1,1);
        pGLayout->addItem(pSpacer, 2, 2, 1, 1);
        pGLayout->setColumnStretch(1, 3);
        pGLayout->setColumnStretch(2, 2);
    }
    this->setLayout(pGLayout);
}

void ChatItemBase::setUserName(const QString &name)
{
    mpNameLabel_->setText(name);
}

void ChatItemBase::setUserIcon(const QPixmap &icon)
{
    mpIconLabel_->setPixmap(icon);
}

void ChatItemBase::setWidget(QWidget *w)
{
    QGridLayout *pGLayout = (qobject_cast<QGridLayout *>)(this->layout());
    pGLayout->replaceWidget(mpBubble_, w);
    delete mpBubble_;
    mpBubble_ = w;
}
