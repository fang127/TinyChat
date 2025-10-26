#include "FriendLabel.h"
#include "ui_FriendLabel.h"

#include <QFontMetrics>

FriendLabel::FriendLabel(QWidget *parent)
    : QFrame(parent), ui(new Ui::FriendLabel)
{
    ui->setupUi(this);
    ui->friendCloseLabel->setState("normal", "hover", "pressed",
                                   "selected_normal", "selected_hover",
                                   "selected_pressed");
    connect(ui->friendCloseLabel, &ClickedLabel::clicked, this,
            &FriendLabel::slotClose);
}

FriendLabel::~FriendLabel() { delete ui; }

void FriendLabel::setText(QString text)
{
    text_ = text;
    ui->tipLabel->setText(text_);
    ui->tipLabel->adjustSize();

    QFontMetrics fontMetrics(ui->tipLabel->font()); // 获取QLabel控件的字体信息
    auto textWidth =
        fontMetrics.horizontalAdvance(ui->tipLabel->text()); // 获取文本的宽度
    auto textHeight = fontMetrics.height();                  // 获取文本的高度

    qDebug() << " ui->tipLabel.width() is " << ui->tipLabel->width();
    qDebug() << " ui->closeLabel->width() is " << ui->friendCloseLabel->width();
    qDebug() << " textWidth is " << textWidth;
    this->setFixedWidth(ui->tipLabel->width() + ui->friendCloseLabel->width() +
                        5);
    this->setFixedHeight(textHeight + 2);
    qDebug() << "  this->setFixedHeight " << this->height();
    width_ = this->width();
    height_ = this->height();
}

int FriendLabel::width() { return width_; }

int FriendLabel::height() { return height_; }

QString FriendLabel::text() { return text_; }

void FriendLabel::slotClose() { emit sigClose(text_); }
