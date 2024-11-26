#include "request.h"

Request::Request() {}

Request::Request(const QString &method, const QString &url, const QString &httpVersion,
                 const QMap<QString, QString> &headers, const QByteArray &body)
    : method(method), url(url), httpVersion(httpVersion), headers(headers), body(body) {}

bool Request::isValid() const
{
    return !method.isEmpty() && !url.isEmpty() && httpVersion.startsWith("HTTP/");
}

QString Request::toString() const
{

    QString requestString;
    requestString.append(QString("%1 %2 %3\n").arg(method).arg(url).arg(httpVersion));
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        requestString.append(QString("%1: %2\n").arg(it.key()).arg(it.value()));
    }
    if (!body.isEmpty()) {
        requestString.append("\n");
        requestString.append(QString::fromUtf8(body));
    }
    return requestString;
}

Request Request::parseFromRawData(const QByteArray &rawData)
{
    Request request;
    QList<QByteArray> lines = rawData.split('\n');
    if (lines.isEmpty()) {
        qDebug() << "Cererea HTTP este goală.";
        return request;
    }
    QByteArray firstLine = lines.at(0).trimmed();
    QList<QByteArray> parts = firstLine.split(' ');
    if (parts.size() != 3) {
        qDebug() << "Linia inițială a cererii HTTP nu este corectă.";
        return request;
    }

    request.method = QString(parts.at(0));
    request.url = QString(parts.at(1));
    request.httpVersion = QString(parts.at(2));

    // Antete
    for (int i = 1; i < lines.size(); ++i) {
        QByteArray line = lines.at(i).trimmed();
        if (line.isEmpty()) break;
        int colonIndex = line.indexOf(':');
        if (colonIndex == -1) continue;
        QString headerName = QString(line.left(colonIndex)).trimmed();
        QString headerValue = QString(line.mid(colonIndex + 1)).trimmed();
        request.headers[headerName] = headerValue;
    }

    // Corpul cererii
    int bodyIndex = rawData.indexOf("\r\n\r\n");
    if (bodyIndex != -1 && rawData.size() > bodyIndex + 4) {
        request.body = rawData.mid(bodyIndex + 4);
    }

    return request;
}

QByteArray Request::toRawData() const
{
    QByteArray requestData;

    //extrage calea, interogarea și protocolul
    QUrl parsedUrl(url);
    if (!parsedUrl.isValid()) {


        qDebug() << "URL invalid în Request::toRawData: " << url;
        return QByteArray();

    }

    // Preluăm calea și interogarea
    QString pathAndQuery = parsedUrl.path();
    if (parsedUrl.hasQuery()) {
        pathAndQuery += "?" + parsedUrl.query();
    }

    if (pathAndQuery.isEmpty()) {
        pathAndQuery = "/";
    }

    // Preluăm protocolul din URL (http sau https)
    QString protocol = parsedUrl.scheme();  // Va returna "http" sau "https"

    // Linie de început
    requestData.append(QString("%1 %2 %3\r\n")
                           .arg(method.isEmpty() ? "GET" : method)  // Folosim "GET" implicit dacă metoda nu este setată
                           .arg(pathAndQuery)                       // Calea și interogarea
                           .arg(httpVersion.isEmpty() ? "HTTP/1.1" : httpVersion)  // Folosim "HTTP/1.1" implicit dacă nu este specificat
                           .toUtf8());

    // Adăugăm antetele cererii
    bool hasHostHeader = false;
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        if (it.key().compare("Host", Qt::CaseInsensitive) == 0) {
            hasHostHeader = true;  // Dacă avem deja antetul "Host"
        }
        requestData.append(it.key().toUtf8() + ": " + it.value().toUtf8() + "\r\n");
    }

    // Dacă nu există antetul "Host", îl adăugăm
    if (!hasHostHeader) {
        QString hostHeader = parsedUrl.host();  // Gazda din URL

        // Dacă portul nu este implicit (80 pentru http sau 443 pentru https), îl adăugăm în antet
        if ((protocol == "http" && parsedUrl.port() != 80) || (protocol == "https" && parsedUrl.port() != 443)) {
            hostHeader += ":" + QString::number(parsedUrl.port());
        }

        requestData.append(QString("Host: %1\r\n").arg(hostHeader).toUtf8());
    }

    // Dacă există un corp, adăugăm Content-Length
    if (!body.isEmpty()) {
        requestData.append(QString("Content-Length: %1\r\n").arg(body.size()).toUtf8());
    }

    // Linie goală pentru a separa antetele de corp
    requestData.append("\r\n");

    // Adăugăm corpul cererii, dacă există
    if (!body.isEmpty()) {
        requestData.append(body);
    }

    return requestData;
}
