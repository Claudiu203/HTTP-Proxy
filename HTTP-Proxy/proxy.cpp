#include "proxy.h"
#include <QTcpSocket>
#include <QTextStream>
#include <QHostAddress>
#include <QSslSocket>
#include <QDebug>
#include <QProcess>
#include <zlib.h>



Proxy::Proxy(QTextEdit *requestDisplay,QTextEdit*sendDisplay, QObject *parent)
    : QTcpServer(parent), requestDisplay(requestDisplay),sendDisplay(sendDisplay)
{

    threadpool = new ThreadPool(4,this);
    // Începe să asculte pe portul 8080
    if (!this->listen(QHostAddress::Any, 8888)) {
        qDebug() << "Error: Unable to start the proxy server!";
    }
}

Proxy::~Proxy()
{
    delete threadpool;
}

void Proxy::displayRequest(const Request &currentRequest)
{
    if (requestDisplay) {
        QString displayText;
        //ELEMENTE CHEIE
        displayText.append(QString("%1 %2 %3\n").arg(currentRequest.method).arg(currentRequest.url).arg(currentRequest.httpVersion));
        // ANTETE
        for (auto it = currentRequest.headers.begin(); it != currentRequest.headers.end(); ++it) {
            displayText.append(QString("%1: %2\n").arg(it.key()).arg(it.value()));
        }
        // CORP CERERE
        if (!currentRequest.body.isEmpty()) {
            displayText.append("\n");
            displayText.append(QString::fromUtf8(currentRequest.body));
        }
        // Afișează cererea în QTextEdit
        requestDisplay->setPlainText(displayText);

    }
}

void Proxy::displaySend(const Response &currentResponse)
{

    if (sendDisplay) {
        QString displayText;
        // Verificăm dacă există un răspuns de tip "Bad Request" (400)
        if (currentResponse.statusCode == 400) {
            qDebug() << "E naspa cererea.......";
            displayText.append("Eroare: Bad Request (400)\n");
            displayText.append("Cererile au fost formate incorect sau conțin informații invalide.\n");
            qDebug() << "Bad Request: 400";
        } else {
            // Folosește toString() pentru a genera textul complet al răspunsului
            displayText.append(currentResponse.toString());

            // Dacă există un corp al răspunsului, îl adăugăm
            if (!currentResponse.body.isEmpty()) {
                displayText.append("\nCorpul răspunsului:\n");
                displayText.append(QString::fromUtf8(currentResponse.body));
            }
        }

        // Afișează răspunsul în QTextEdit
        QMetaObject::invokeMethod(sendDisplay, "setPlainText", Qt::QueuedConnection, Q_ARG(QString, displayText));
        qDebug() << displayText;
    } else {
        qDebug() << "sendDisplay nu este valid!";
    }

}

QByteArray Proxy::constructHttpRequest(const Request &request)
{
    return request.toRawData();
}

void Proxy::incomingConnection(qintptr socketDescriptor)
{
    // Crează un nou QTcpSocket pentru conexiunea clientului
    QTcpSocket *clientSocket = new QTcpSocket(this);

    // Asociază descriptorul de socket primit la noul clientSocket
    if (!clientSocket->setSocketDescriptor(socketDescriptor)) {
        qDebug() << "Nu s-a putut seta socketDescriptor-ul pentru client.";
        clientSocket->deleteLater();
        return;
    }
    // Conectează sloturile pentru citirea datelor și deconectare
    connect(clientSocket, &QTcpSocket::readyRead, this, &Proxy::onReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &Proxy::onClientDisconnected);
    pendingClientSocket = clientSocket;

    qDebug() << "Nouă conexiune acceptată de la" << clientSocket->peerAddress().toString();
}

void Proxy::onReadyRead()
{
    QTcpSocket *clientSocket = static_cast<QTcpSocket*>(sender());
    pendingClientSocket = clientSocket;
    pendingRequestData = clientSocket->readAll();
    currentRequest = parseHttpRequest(pendingRequestData);
    requests.append(currentRequest);
    displayRequest(currentRequest);
    // Afișează anteturile și corpul cererii
    for (auto it = currentRequest.headers.begin(); it != currentRequest.headers.end(); ++it) {
        qDebug() << it.key() << ": " << it.value();
    }
    if (!currentRequest.body.isEmpty()) {
        qDebug() << "Corpul cererii: " << QString::fromUtf8(currentRequest.body);
    }
    qDebug() << "Așteaptă trimiterea manuală a cererii de către utilizator.";
    if (requests.size() >= 2) {
        writeRequestsToFile();  // Scrie cererile în fișier
        requests.clear();  // Golește lista
    }
}

/*void Proxy::onReadyRead()
{
    QTcpSocket *clientSocket = static_cast<QTcpSocket *>(sender());
    QByteArray requestData = clientSocket->readAll();
    qDebug() << "Received request:\n" << QString::fromUtf8(requestData);

    QList<QByteArray> lines = requestData.split('\n');
    if (lines.isEmpty()) {
        qDebug() << "Error: Invalid request format.";
        clientSocket->write("HTTP/1.1 400 Bad Request\r\n\r\n");
        clientSocket->disconnectFromHost();
        return;
    }

    QByteArray requestLine = lines.first().trimmed();
    QList<QByteArray> requestParts = requestLine.split(' ');

    if (requestParts.size() < 3) {
        qDebug() << "Error: Malformed request line.";
        clientSocket->write("HTTP/1.1 400 Bad Request\r\n\r\n");
        clientSocket->disconnectFromHost();
        return;
    }

    QString method = QString::fromUtf8(requestParts.at(0));
    QString target = QString::fromUtf8(requestParts.at(1));

    if (method.toUpper() == "CONNECT") {
        // Handle CONNECT requests for HTTPS
        QStringList targetParts = target.split(':');
        if (targetParts.size() != 2) {
            qDebug() << "Error: Invalid target format for CONNECT.";
            clientSocket->write("HTTP/1.1 400 Bad Request\r\n\r\n");
            clientSocket->disconnectFromHost();
            return;
        }

        QString host = targetParts.at(0);
        quint16 port = targetParts.at(1).toUShort();

        QTcpSocket *serverSocket = new QTcpSocket(this);
        serverSocket->connectToHost(host, port);

        if (!serverSocket->waitForConnected(5000)) {
            qDebug() << "Error: Could not connect to target server " << host << ":" << port;
            clientSocket->write("HTTP/1.1 502 Bad Gateway\r\n\r\n");
            clientSocket->disconnectFromHost();
            serverSocket->deleteLater();
            return;
        }

        // Respond with 200 Connection Established
        clientSocket->write("HTTP/1.1 200 Connection Established\r\n\r\n");
        clientSocket->waitForBytesWritten();

        // Tunnel encrypted data transparently
        connect(clientSocket, &QTcpSocket::readyRead, [=]() {
            serverSocket->write(clientSocket->readAll());
        });

        connect(serverSocket, &QTcpSocket::readyRead, [=]() {
            clientSocket->write(serverSocket->readAll());
        });

        connect(clientSocket, &QTcpSocket::disconnected, serverSocket, &QTcpSocket::disconnectFromHost);
        connect(serverSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::disconnectFromHost);

        qDebug() << "Established CONNECT tunnel for " << host << ":" << port;

    } else {
        // Handle standard HTTP requests (GET, POST, etc.)
        QUrl targetUrl(target);
        QString host = targetUrl.host();
        quint16 port = targetUrl.port() == -1 ? 80 : targetUrl.port();

        QTcpSocket *serverSocket = new QTcpSocket(this);
        serverSocket->connectToHost(host, port);

        if (!serverSocket->waitForConnected(5000)) {
            qDebug() << "Error: Could not connect to target server " << host << ":" << port;
            clientSocket->write("HTTP/1.1 502 Bad Gateway\r\n\r\n");
            clientSocket->disconnectFromHost();
            serverSocket->deleteLater();
            return;
        }

        // Forward the client's HTTP request to the target server
        serverSocket->write(requestData);
        if (!serverSocket->waitForBytesWritten(5000)) {
            qDebug() << "Error: Could not send request to target server.";
            clientSocket->write("HTTP/1.1 502 Bad Gateway\r\n\r\n");
            clientSocket->disconnectFromHost();
            serverSocket->deleteLater();
            return;
        }

        // Wait for the server's response
        if (!serverSocket->waitForReadyRead(5000)) {
            qDebug() << "Error: Timeout waiting for server response.";
            clientSocket->write("HTTP/1.1 504 Gateway Timeout\r\n\r\n");
            clientSocket->disconnectFromHost();
            serverSocket->deleteLater();
            return;
        }

        // Send the server's response back to the client
        QByteArray responseData = serverSocket->readAll();
        clientSocket->write(responseData);
        clientSocket->waitForBytesWritten();

        serverSocket->close();
        clientSocket->disconnectFromHost();

        qDebug() << "Forwarded HTTP request for " << target;
    }
}*/


void Proxy::onClientDisconnected()
{
    QTcpSocket *clientSocket = static_cast<QTcpSocket*>(sender());
    clientSocket->deleteLater();  // Eliberăm socket-ul clientului
}
void Proxy::sendRequest()
{
    qDebug() << "Apel sendRequest.";
    threadpool->enqueueTask([this]() {
        qDebug() << "Task creat în sendRequest.";
        handleRequest();
    });
    qDebug() << "Task trimis spre execuție.";
}


void Proxy::setQtext(QTextEdit *txt)
{
    this->requestDisplay=txt;
}

void Proxy::writeRequestsToFile()
{
    QMutexLocker locker(&mutex);
    QFile file("/home/claudiu/Documents/cereri.txt");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        // Scrie fiecare cerere din lista requests
        for (const Request& req : requests) {
            QString dateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
            // Scrie data și ora înainte de cerere
            out << QString("Data: %1\n").arg(dateTime);
            // Scrie linia de început a cererii (metoda, URL, versiune)
            out << QString("%1 %2 %3\n").arg(req.method).arg(req.url).arg(req.httpVersion);
            // Scrie anteturile
            for (auto it = req.headers.begin(); it != req.headers.end(); ++it) {
                out << QString("%1: %2\n").arg(it.key()).arg(it.value());
            }
            // Scrie corpul cererii
            if (!req.body.isEmpty()) {
                out << "\n" << QString::fromUtf8(req.body) << "\n";
            }
            // Adaugă un separator între cereri
            out << "\n\n";
        }
        file.close();
    } else {
        qDebug() << "Eroare la deschiderea fișierului pentru scriere!";
    }
}

bool Proxy::isResponseInCache(const QString &cacheKey)
{
    return cache.contains(cacheKey);
}

void Proxy::addResponseToCache(const QString &cacheKey, const Response &response)
{
    cache.insert(cacheKey, response);
}

void Proxy::handleRequest() {
    threadpool->enqueueTask([this]() {
        QMutexLocker locker(&mutex); // Protejează resursele partajate

        QString urlString = currentRequest.url;
        if (!urlString.startsWith("http://") && !urlString.startsWith("https://")) {
            urlString = "https://" + urlString;
        }

        QUrl url(urlString);
        if (!url.isValid()) {
            qDebug() << "URL invalid: " << urlString;
            return;
        }

        QString cacheKey = url.toString() + QString::number(currentRequest.headers.size());
        if (isResponseInCache(cacheKey)) {
            currentResponse = cache.value(cacheKey);

            QMetaObject::invokeMethod(this, [this]() {
                displaySend(currentResponse);
            }, Qt::QueuedConnection);

            emit requestCompleted();
            return;
        }

        QString host = url.host();
        quint16 port = url.port() == -1 ? (url.scheme() == "https" ? 443 : 80) : url.port();
        QByteArray requestData = constructHttpRequest(currentRequest);
        QByteArray responseData;
        QAbstractSocket *serverSocket = nullptr;

        if (url.scheme() == "https") {
            serverSocket = new QSslSocket();
            auto *sslSocket = qobject_cast<QSslSocket *>(serverSocket);
            sslSocket->connectToHostEncrypted(host, port);

            if (!sslSocket->waitForEncrypted(10000)) {
                qDebug() << "Eroare la conectarea SSL: " << sslSocket->errorString();
                serverSocket->deleteLater();
                return;
            }
        } else {
            serverSocket = new QTcpSocket();
            serverSocket->connectToHost(host, port);

            if (!serverSocket->waitForConnected(10000)) {
                qDebug() << "Eroare la conectarea TCP: " << serverSocket->errorString();
                serverSocket->deleteLater();
                return;
            }
        }

        serverSocket->write(requestData);
        if (!serverSocket->waitForBytesWritten(10000)) {
            qDebug() << "Eroare la trimiterea cererii: " << serverSocket->errorString();
            serverSocket->deleteLater();
            return;
        }

        if (!serverSocket->waitForReadyRead(10000)) {
            qDebug() << "Timeout la așteptarea răspunsului.";
            serverSocket->deleteLater();
            return;
        }

        while (serverSocket->bytesAvailable() || serverSocket->waitForReadyRead(100)) {
            responseData.append(serverSocket->readAll());
        }

        currentResponse = parseHttpResponse(responseData);

        if (currentResponse.headers.value("Transfer-Encoding") == "chunked") {
            currentResponse.body = decodeChunked(currentResponse.body);
        }

        if (currentResponse.headers.value("Content-Encoding") == "gzip") {
            qDebug() << "Răspuns comprimat cu gzip. Decomprimăm...";

            QByteArray decompressedData = decompressGzip(currentResponse.body);
            if (!decompressedData.isEmpty()) {
                currentResponse.body = decompressedData; // Înlocuiește corpul răspunsului cu cel decomprimat
            } else {
                qDebug() << "Eroare la decomprimarea răspunsului.";
            }
        } else {
            qDebug() << "Răspunsul nu este comprimat.";
        }

        QMetaObject::invokeMethod(this, [this]() {
            displaySend(currentResponse);
        }, Qt::QueuedConnection);

        addResponseToCache(cacheKey, currentResponse); // Cache the response

        emit requestCompleted();

        serverSocket->deleteLater();
    });
}




Request Proxy::parseHttpRequest(const QByteArray &requestData)
{
    return Request::parseFromRawData(requestData);
}

Response Proxy::parseHttpResponse(const QByteArray &responseData)
{
    return Response::parseFromRawData(responseData);
}


QByteArray Proxy::decompressGzip(const QByteArray &compressedData) {
    if (compressedData.isEmpty()) {
        qDebug() << "Datele comprimate sunt goale.";
        return QByteArray();
    }

    z_stream stream;
    memset(&stream, 0, sizeof(stream)); // Inițializează structura stream

    // Inițializare decomprimare cu gzip (16 + MAX_WBITS este specific pentru gzip)
    if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
        qDebug() << "Eroare la inițializarea decomprimării gzip.";
        return QByteArray();
    }

    QByteArray decompressedData; // Datele decomprimate vor fi salvate aici
    char buffer[4096];           // Buffer temporar pentru decomprimare

    stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(compressedData.data()));
    stream.avail_in = compressedData.size();

    // Procesul de decomprimare
    do {
        stream.next_out = reinterpret_cast<Bytef *>(buffer);
        stream.avail_out = sizeof(buffer);

        int ret = inflate(&stream, Z_NO_FLUSH);
        if (ret == Z_STREAM_END) {
            // Sfârșitul stream-ului de date
            decompressedData.append(buffer, sizeof(buffer) - stream.avail_out);
            break;
        } else if (ret != Z_OK) {
            // Eroare la decomprimare
            qDebug() << "Eroare la decomprimarea gzip:" << ret;
            inflateEnd(&stream);
            return QByteArray();
        }

        // Adaugă date decomprimate în buffer
        decompressedData.append(buffer, sizeof(buffer) - stream.avail_out);
    } while (stream.avail_out == 0); // Continuă până când toate datele sunt decomprimate

    inflateEnd(&stream); // Curăță resursele alocate de zlib
    return decompressedData;
}

QByteArray Proxy::decodeChunked(const QByteArray &chunkedData) {
    QByteArray decodedData;
    int index = 0;

    while (index < chunkedData.size()) {
        // Găsește lungimea chunk-ului în hexadecimal
        int endOfSize = chunkedData.indexOf("\r\n", index);
        if (endOfSize == -1) break;

        QString chunkSizeHex = chunkedData.mid(index, endOfSize - index);
        bool ok;
        int chunkSize = chunkSizeHex.toInt(&ok, 16); // Convertim din hexadecimal în decimal
        if (!ok || chunkSize == 0) break; // Sfârșitul datelor sau eroare

        // Obține datele chunk-ului
        index = endOfSize + 2; // Sari peste "\r\n"
        decodedData.append(chunkedData.mid(index, chunkSize));
        index += chunkSize + 2; // Sari peste chunk și "\r\n"
    }

    return decodedData;
}


