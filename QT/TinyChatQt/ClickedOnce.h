#ifndef CLICKEDONCE_H
#define CLICKEDONCE_H

#include <QLabel>
#include <QMouseEvent>

class ClickedOnce : public QLabel
{
    Q_OBJECT
public:
    ClickedOnce(QWidget *parent = nullptr);
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
signals:
    void clicked(QString);
};

#endif // CLICKEDONCE_H
