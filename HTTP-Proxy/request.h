#ifndef REQUEST_H
#define REQUEST_H

#include <QString>
#include <QMap>
#include <QByteArray>
#include<QDebug>
#include <QUrl>

class Request
{
public:
    Request();
    Request(const QString &method, const QString &url, const QString &httpVersion,
            const QMap<QString, QString> &headers, const QByteArray &body);

    bool isValid() const;
    QString toString() const;
    static Request parseFromRawData(const QByteArray &rawData);
    QByteArray toRawData() const;

    QString method;
    QString url;
    QString httpVersion;
    QMap<QString, QString> headers;
    QByteArray body;
};

#endif // REQUEST_H
