#include "httpmgr.h"

HttpMgr::~HttpMgr() {}

HttpMgr::HttpMgr()
{
    connect(this, &HttpMgr::sigHttpFinish, this, &HttpMgr::slotHttpFinish);
}

void HttpMgr::postHttpReq(QUrl url, QJsonObject json, ReqId reqId, Modules mod)
{
    // 发送数据
    // 创建一个HTTP POST请求，并设置请求头和请求体
    QByteArray data = QJsonDocument(json).toJson();
    // 通过url构造请求
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader,
                      QByteArray::number(data.length()));
    // 发送请求，并处理响应,
    // 获取自己的智能指针，构造伪闭包并增加智能指针引用计数
    // self对象确保在下面post发送数据过程中HttpMgr对象不会被析构，从而确保在发送完数据，reply受到信号可以调用回调函数，该回调函数会使用HttpMgr对象内的资源
    auto self = shared_from_this();
    QNetworkReply *reply = manager_.post(request, data);
    // 设置信号和槽等待发送完成
    QObject::connect(reply, &QNetworkReply::finished,
                     [self, reply, reqId, mod]()
                     {
                         // 处理错误
                         if (reply->error() != QNetworkReply::NoError)
                         {
                             qDebug() << reply->errorString();
                             // 发送信号通知界面出现错误
                             emit self->sigHttpFinish(
                                 reqId, "", ErrorCodes::ERR_NETWORK, mod);
                             reply->deleteLater();
                             return;
                         }
                         // 没有错误则读回请求
                         QString res = reply->readAll();
                         // 发送信号通知完成
                         emit self->sigHttpFinish(reqId, res,
                                                  ErrorCodes::SUCCESS, mod);
                         reply->deleteLater();
                         return;
                     });
}

void HttpMgr::slotHttpFinish(ReqId id, QString res, ErrorCodes err, Modules mod)
{
    // 注册模块
    if (mod == Modules::REGISTERMOD)
    {
        // 发送信号通知指定注册模块http的响应结束了
        emit sigRegModFinish(id, res, err);
    }

    if (mod == Modules::RESETPASSMOD)
    {
        emit sigResetModFinish(id,res,err);
    }
}
