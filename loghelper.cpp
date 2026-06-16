#include "loghelper.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QAbstractSocket>

void LogHelper::record(int userId, const QString &type, const QString &detail)
{
    // 获取本机 IP 地址（简单获取第一个非回环 IPv4 地址）
    QString ip = "127.0.0.1";
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    for (const QHostAddress &addr : list) {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol && addr != QHostAddress::LocalHost) {
            ip = addr.toString();
            break;
        }
    }

    QSqlQuery query;
    query.prepare("INSERT INTO operation_logs (user_id, operation_type, operation_detail, ip_address, created_at) "
                  "VALUES (?, ?, ?, ?, NOW())");
    query.addBindValue(userId);
    query.addBindValue(type);
    query.addBindValue(detail);
    query.addBindValue(ip);
    if (!query.exec()) {
        qDebug() << "写入日志失败:" << query.lastError().text();
    }
}