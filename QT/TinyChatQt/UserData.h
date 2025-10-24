#ifndef USERDATA_H
#define USERDATA_H

#include <QString>

class SearchInfo
{
public:
    SearchInfo(int uid, const QString &name,const QString &nick, const QString &desc, int sex);
    int uid_;
    QString name_;
    QString nick_;
    QString desc_;
    int sex_;
};

#endif // USERDATA_H
