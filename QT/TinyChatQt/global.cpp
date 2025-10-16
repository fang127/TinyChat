#include "global.h"

QString gateUrlPrefix="";

std::function<void(QWidget *)> repolish = [](QWidget *w){
    w->style()->unpolish(w);
    w->style()->polish(w);
};

std::function<QString(QString)> xorString = [](QString input)
{
    QString result = input;
    int length = input.length();
    for(int i = 0;i < length;++i)
    {
        // 对每个字符进行异或，可以优化，这里假设字符都是ASCII
        result[i] = QChar(static_cast<ushort>(input[i].unicode()) ^ static_cast<ushort>(length));
    }

    return result;
};
