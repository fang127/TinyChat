#include "TextBubble.h"

#include <QEvent>
#include <QTextBlock>

TextBubble::TextBubble(ChatRole role, const QString &text, QWidget *parent) : BubbleFrame(role,parent)
{
    textEdit_ = new QTextEdit();
    textEdit_->setReadOnly(true);
    textEdit_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEdit_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEdit_->installEventFilter(this);
    QFont font("Microsoft YaHei");
    font.setPointSize(12);
    textEdit_->setFont(font);
    setPlainText(text);
    setWidget(textEdit_);
    initStyleSheet();
}

bool TextBubble::eventFilter(QObject *o, QEvent *e)
{
    if(textEdit_ == o && e->type() == QEvent::Paint)
    {
        adjustTextHeight(); //PaintEvent中设置
    }
    return BubbleFrame::eventFilter(o, e);
}

void TextBubble::adjustTextHeight()
{
    qreal docMargin = textEdit_->document()->documentMargin();    //字体到边框的距离默认为4
    QTextDocument *doc = textEdit_->document();
    qreal textHeight = 0;
    // 把每一段的高度相加=文本高
    for (QTextBlock it = doc->begin(); it != doc->end(); it = it.next())
    {
        QTextLayout *pLayout = it.layout();
        QRectF textRect = pLayout->boundingRect();                             //这段的rect
        textHeight += textRect.height();
    }
    int vMargin = this->layout()->contentsMargins().top();
    // 设置这个气泡需要的高度 文本高+文本边距+TextEdit边框到气泡边框的距离
    setFixedHeight(textHeight + docMargin *2 + vMargin*2 );
}

void TextBubble::setPlainText(const QString &text)
{
    textEdit_->setPlainText(text);
    //m_pTextEdit->setHtml(text);
    //找到段落中最大宽度
    qreal doc_margin = textEdit_->document()->documentMargin();
    int margin_left = this->layout()->contentsMargins().left();
    int margin_right = this->layout()->contentsMargins().right();
    QFontMetricsF fm(textEdit_->font());
    QTextDocument *doc = textEdit_->document();
    int max_width = 0;
    //遍历每一段找到 最宽的那一段
    for (QTextBlock it = doc->begin(); it != doc->end(); it = it.next())    //字体总长
    {
        int txtW = int(fm.horizontalAdvance(it.text()));
        max_width = max_width < txtW ? txtW : max_width;                 //找到最长的那段
    }
    //设置这个气泡的最大宽度 只需要设置一次
    setMaximumWidth(max_width + doc_margin * 2 + (margin_left + margin_right) + 1);        //设置最大宽度
}

void TextBubble::initStyleSheet()
{
    textEdit_->setStyleSheet("QTextEdit{background:transparent;border:none}");
}
