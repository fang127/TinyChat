#ifndef HTTPMGR_H
#define HTTPMGR_H

#include "singleton.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QtNetwork/QNetworkAccessManager>

class HttpMgr : public QObject,
                public Singleton<HttpMgr>,
                public std::enable_shared_from_this<HttpMgr>
{
    Q_OBJECT
public:
    ~HttpMgr();

    void postHttpReq(QUrl url, QJsonObject json, ReqId reqId, Modules mod);

private:
    friend class Singleton<HttpMgr>;

    HttpMgr();

    QNetworkAccessManager manager_; // http管理者

private slots:
    void slotHttpFinish(ReqId id, QString res, ErrorCodes err, Modules mod);

signals:
    void sigHttpFinish(ReqId id, QString res, ErrorCodes err, Modules mod);
    void sigRegModFinish(ReqId id, QString res, ErrorCodes err);
    void sigResetModFinish(ReqId id, QString res, ErrorCodes err);
};

#endif // HTTPMGR_H
