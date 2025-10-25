#ifndef FRIENDLABEL_H
#define FRIENDLABEL_H

#include <QFrame>

namespace Ui {
class FriendLabel;
}

class FriendLabel : public QFrame
{
    Q_OBJECT

public:
    explicit FriendLabel(QWidget *parent = nullptr);
    ~FriendLabel();
    int width();
    int height();
    QString text();
    void setText(QString text);

private:
    Ui::FriendLabel *ui;
    QString text_;
    int width_;
    int height_;
public slots:
    void slotClose();
signals:
    void sigClose(QString);
};

#endif // FRIENDLABEL_H
