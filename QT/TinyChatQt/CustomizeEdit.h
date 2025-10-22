#ifndef CUSTOMIZEEDIT_H
#define CUSTOMIZEEDIT_H

#include <QLineEdit>
#include <QtDebug>

class CustomizeEdit : public QLineEdit
{
    Q_OBJECT
public:
    CustomizeEdit() = default;
    CustomizeEdit(QWidget *parent = nullptr);
    void setMaxLength(int maxLen);
protected:
    void focusOutEvent(QFocusEvent *event) override;
private:
    void limitTextLength(const QString &text);

    int maxLen_;
signals:
    void sigFoucusOut();
};

#endif // CUSTOMIZEEDIT_H
