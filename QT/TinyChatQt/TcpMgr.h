#ifndef TCPMGR_H
#define TCPMGR_H

#include "singleton.h"

#include <QString>
#include <QtNetwork/QTcpSocket>

// 客户端Tcp管理类
class TcpMgr : public Singleton<TcpMgr>,
               public QObject,
               public std::enable_shared_from_this<TcpMgr>
{
    friend class Singleton<TcpMgr>;

    Q_OBJECT
public:
    ~TcpMgr() = default;

    void closeConnection();

public slots:
    // 连接成功
    void slotTcpConnect(ServerInfo serverInfo);
    // 发生数据
    void slotSendData(ReqId reqId, QString data);

signals:
    // 连接成功
    void sigConSuccess(bool bsuccess);
    // 发生数据
    void sigSendData(ReqId reqid, QString data);
    // 连接关闭
    void sigConnectionClosed();
    // 登录错误信号
    void sigLoginFailed(int err);

private:
    TcpMgr();
    // 注册回包函数
    void initHandlers();
    // 处理消息
    void handleMsg(ReqId id, int len, QByteArray data);

    QString host_;       // 服务器地址
    uint16_t port_;      // 服务器端口
    QTcpSocket socket_;  // 套接字
    QByteArray buffer_;  // 接收发送缓冲区
    bool bRecvPending_;  // 判断是否粘包
    quint16 messageId_;  // 接收消息的id
    quint16 messageLen_; // 消息内容的长度
    // 回包处理函数
    QMap<ReqId,
         std::function<void(const ReqId &, const int &, const QByteArray &)>>
        handlers_;
};

#endif // TCPMGR_H
