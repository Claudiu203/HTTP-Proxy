#include "response.h"

Response::Response() : statusCode(0)
{}

Response::Response(const QString &protocol, int statusCode, const QString &statusMessage,
                   const QMap<QString, QString> &headers, const QByteArray &body)
    : protocol(protocol), statusCode(statusCode), statusMessage(statusMessage),
    headers(headers), body(body)
{
}

QString Response::toString() const
{
    QString responseString;

    // Reprezentarea protocolului, codului de stare și mesajului de stare
    responseString.append(QString("%1 %2 %3\r\n").arg(protocol).arg(statusCode).arg(statusMessage));

    // Reprezentarea antetelor
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        responseString.append(QString("%1: %2\r\n").arg(it.key()).arg(it.value()));
    }

    // Linie goală
    responseString.append("\r\n");

    // Adăugarea corpului răspunsului, dacă există
    if (!body.isEmpty()) {
        responseString.append(body);
    }

    return responseString;
}

Response Response::parseFromRawData(const QByteArray &rawData)
{
    Response response;

    // Convertim QByteArray-ul într-un QString pentru procesare ușoară
    QString rawString = QString::fromUtf8(rawData);

    // Împărțim răspunsul în linii
    QStringList lines = rawString.split("\r\n");

    // Prima linie conține protocolul, codul de stare și mesajul de stare
    if (!lines.isEmpty()) {
        QStringList statusLine = lines.at(0).split(" ");
        if (statusLine.size() >= 3) {
            response.protocol = statusLine.at(0);
            response.statusCode = statusLine.at(1).toInt();
            response.statusMessage = statusLine.mid(2).join(" ");  // Dacă mesajul are mai multe cuvinte
        }
    }

    // Procesăm antetele
    int i = 1; // Începem de la a doua linie
    while (i < lines.size() && !lines[i].isEmpty()) {
        QStringList header = lines.at(i).split(": ");
        if (header.size() == 2) {
            response.headers.insert(header.at(0), header.at(1));
        }
        ++i;
    }

    // Corpul răspunsului este după linia goală, deci îl preluăm de acolo
    if (i < lines.size()) {
        response.body = rawData.mid(rawString.indexOf("\r\n\r\n") + 4);
    }

    return response;
}

QByteArray Response::toRawData() const
{
    QByteArray rawData;

    // Construcția răspunsului sub formă de string
    QString responseString = toString();

    // Adăugăm datele la QByteArray
    rawData.append(responseString.toUtf8());

    return rawData;
}
