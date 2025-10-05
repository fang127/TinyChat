#ifndef HTTPMGR_H
#define HTTPMGR_H

#include "singleton.h"
#include <QString>
#include <QUrl>
#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonDocument>

class HttpMgr : public QObject, public Singleton<HttpMgr>,public std::enable_shared_from_this<HttpMgr>
{
    Q_OBJECT
public:
    ~HttpMgr();
private:
    friend class Singleton<HttpMgr>;

    HttpMgr();

    QNetworkAccessManager manager_; // http管理者

    void postHttpReq(QUrl url, QJsonObject json, ReqId reqId, Modules mod);

private slots:
    void slotHttpFinish(ReqId id, QString res, ErrorCodes err, Modules mod);

signals:
    void sigHttpFinish(ReqId id, QString res, ErrorCodes err, Modules mod);
    void sigRegModFinish(ReqId id, QString res, ErrorCodes err);
};

#endif // HTTPMGR_H
