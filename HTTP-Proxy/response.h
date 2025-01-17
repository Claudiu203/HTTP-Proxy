#ifndef RESPONSE_H
#define RESPONSE_H

#include <QString>
#include <QMap>
#include <QByteArray>

class Response
{
public:
    Response();
    Response(const QString &protocol, int statusCode, const QString &statusMessage,
             const QMap<QString, QString> &headers = QMap<QString, QString>(),
             const QByteArray &body = QByteArray());

    QString toString() const;
    static Response parseFromRawData(const QByteArray &rawData);
     QByteArray toRawData() const;

    QString protocol;
    int statusCode;
    QString statusMessage;
    QMap<QString, QString> headers;
    QByteArray body;
};

#endif // RESPONSE_H
