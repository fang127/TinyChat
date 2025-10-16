#ifndef CLICKEDLABEL_H
#define CLICKEDLABEL_H

#include "global.h"

#include <QLabel>


class ClickedLabel : public QLabel
{
    Q_OBJECT
public:
    ClickedLabel(QWidget *parent=nullptr);

    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void enterEvent(QEnterEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;

    void setState(QString normal = "", QString hover = "", QString press = "", QString select = "", QString selectHover = "", QString selectPress = "");

    ClickLbState getCurState();
private:
    QString normal_;
    QString normalHover_;
    QString normalPress_;

    QString selected_;
    QString selectedHover_;
    QString selectedPress_;

    ClickLbState curstate_;
signals:
    void clicked(void);
};

#endif // CLICKEDLABEL_H
