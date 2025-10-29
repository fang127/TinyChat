#include "UserData.h"

SearchInfo::SearchInfo(int uid, const QString &name, const QString &nick, const QString &desc, int sex, const QString &icon)
    : uid_(uid), name_(name), nick_(nick), desc_(desc), sex_(sex), icon_(icon)
{
}


AddFriendApply::AddFriendApply(int from_uid, QString name, QString desc, QString icon, QString nick, int sex)
    : _from_uid(from_uid), _name(name), _desc(desc), _icon(icon), _nick(nick), _sex(sex)
{
}
