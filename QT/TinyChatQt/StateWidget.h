#ifndef STATEWIDGET_H
#define STATEWIDGET_H

#include "global.h"

#include <QLabel>
#include <QWidget>

class StateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StateWidget(QWidget *parent = nullptr);

    void setState(const QString &normal = "",
                  const QString &normalHover = "",
                  const QString &normalPress = "",
                  const QString &selected = "",
                  const QString &selectedHover = "",
                  const QString &selectedPress = "");
    ClickLbState getCurState();
    void clearState();

    void setSelected(bool selected);
    void addRedPoint();
    void showRedPoint(bool show = true);

protected:
    void paintEvent(QPaintEvent *event) override;

    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void enterEvent(QEnterEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;

private:
    QString normal_;
    QString normalHover_;
    QString normalPress_;
    QString selected_;
    QString selectedHover_;
    QString selectedPress_;

    ClickLbState curstate_;
    QLabel *redPoint_;
signals:
    void clicked(void);
};

#endif // STATEWIDGET_H
