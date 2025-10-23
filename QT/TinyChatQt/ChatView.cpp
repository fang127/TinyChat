#include "ChatView.h"

#include <QEvent>
#include <QPainter>
#include <QScrollBar>
#include <QStyleOption>

ChatView::ChatView(QWidget *parent) : QWidget(parent), isAppended(false)
{
    // 生成垂直布局
    QVBoxLayout *pMainLayout = new QVBoxLayout();
    this->setLayout(pMainLayout);
    // 设置外边距
    pMainLayout->setContentsMargins(0, 0, 0, 0);

    // 设置滚动区域
    mpScrollArea = new QScrollArea();
    mpScrollArea->setObjectName("chatArea");
    pMainLayout->addWidget(mpScrollArea);

    // 创建一个Widget
    QWidget *w = new QWidget(this);
    w->setObjectName("chatBG");
    w->setAutoFillBackground(true);

    // 创建子布局
    QVBoxLayout *pHLayout01 = new QVBoxLayout();
    pHLayout01->addWidget(new QWidget(), 100000);
    w->setLayout(pHLayout01);
    mpScrollArea->setWidget(w);

    // 默认滚动条为关闭
    mpScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScrollBar *pVScrollBar = mpScrollArea->verticalScrollBar();
    // 添加槽函数,滚动槽范围变化了会触发槽函数
    connect(pVScrollBar, &QScrollBar::rangeChanged, this,
            &ChatView::onVScrollBarMoved);

    // 把垂直ScrollBar放到上边 而不是原来的并排
    QHBoxLayout *pHLayout02 = new QHBoxLayout();
    pHLayout02->addWidget(pVScrollBar, 0, Qt::AlignRight);
    pHLayout02->setContentsMargins(0, 0, 0, 0);
    mpScrollArea->setLayout(pHLayout02);
    pVScrollBar->setHidden(true);

    mpScrollArea->setWidgetResizable(true);
    mpScrollArea->installEventFilter(this);
    initStyleSheet();
}

void ChatView::appendChatItem(QWidget *item)
{
    QVBoxLayout *vl =
        qobject_cast<QVBoxLayout *>(mpScrollArea->widget()->layout());
    vl->insertWidget(vl->count() - 1, item);
    isAppended = true;
}

void ChatView::prependChatItem(QWidget *item) {}

void ChatView::insertChatItem(QWidget *before, QWidget *item) {}

bool ChatView::eventFilter(QObject *o, QEvent *e)
{
    /*if(e->type() == QEvent::Resize && o == )
    {

    }
    else */
    if (e->type() == QEvent::Enter && o == mpScrollArea)
    {
        mpScrollArea->verticalScrollBar()->setHidden(
            mpScrollArea->verticalScrollBar()->maximum() == 0);
    }
    else if (e->type() == QEvent::Leave && o == mpScrollArea)
    {
        mpScrollArea->verticalScrollBar()->setHidden(true);
    }
    return QWidget::eventFilter(o, e);
}

void ChatView::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ChatView::initStyleSheet() {}

void ChatView::onVScrollBarMoved(int min, int max)
{
    // true则添加数据
    if (isAppended)
    {
        QScrollBar *pVScrollBar = mpScrollArea->verticalScrollBar();
        pVScrollBar->setSliderPosition(pVScrollBar->maximum());
        // 500毫秒可能调用多次
        QTimer::singleShot(500, [this]() { isAppended = true; });
    }
}
