#include "proxy.h"
#include <QTcpSocket>
#include <QTextStream>
#include <QHostAddress>
#include <QSslSocket>
#include <QDebug>
#include <QProcess>



Proxy::Proxy(QTextEdit *requestDisplay,QTextEdit*sendDisplay, QObject *parent)
    : QTcpServer(parent), requestDisplay(requestDisplay),sendDisplay(sendDisplay)
{
    // Începe să asculte pe portul 8080
    if (!this->listen(QHostAddress::Any, 8080)) {
        qDebug() << "Error: Unable to start the proxy server!";
    }
}

Proxy::~Proxy()
{

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
    if (requests.size() >= 1) {
        writeRequestsToFile();  // Scrie cererile în fișier
        requests.clear();  // Golește lista
    }
}

void Proxy::onClientDisconnected()
{
    QTcpSocket *clientSocket = static_cast<QTcpSocket*>(sender());
    clientSocket->deleteLater();  // Eliberăm socket-ul clientului
}
void Proxy::sendRequest()
{
    workerThread = new QThread(this);  // Creează un thread nou
    QObject::connect(workerThread, &QThread::started, this, &Proxy::handleRequest);
    workerThread->start();  // Pornește thread-ul
}


void Proxy::setQtext(QTextEdit *txt)
{
    this->requestDisplay=txt;
}

void Proxy::writeRequestsToFile()
{
          QMutexLocker locker(&mutex);
    QFile file("/home/cris/proiect/cereri.txt");
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

void Proxy::handleRequest()
{
    QMutexLocker locker(&mutex);

    QString urlString = currentRequest.url;
    if (!urlString.startsWith("http://") && !urlString.startsWith("https://")) {
        urlString = "https://" + urlString;
    }

    QUrl url(urlString);
    if (!url.isValid()) {
        qDebug() << "URL invalid: " << urlString;
        return;
    }

    QString cacheKey = url.toString() + QString::number(currentRequest.headers.size());  // Example cache key based on URL + headers size
    if (isResponseInCache(cacheKey)) {
        currentResponse = cache.value(cacheKey);
        displaySend(currentResponse);
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
            return;
        }
    } else {
        serverSocket = new QTcpSocket();
        serverSocket->connectToHost(host, port);

        if (!serverSocket->waitForConnected(10000)) {
            qDebug() << "Eroare la conectarea TCP: " << serverSocket->errorString();
            return;
        }
    }

    serverSocket->write(requestData);
    if (!serverSocket->waitForBytesWritten(10000)) {
        qDebug() << "Eroare la trimiterea cererii: " << serverSocket->errorString();
        return;
    }

    if (!serverSocket->waitForReadyRead(10000)) {
        qDebug() << "Timeout la așteptarea răspunsului.";
        return;
    }

    while (serverSocket->bytesAvailable() || serverSocket->waitForReadyRead(100)) {
        responseData.append(serverSocket->readAll());
    }

    currentResponse = parseHttpResponse(responseData);
    displaySend(currentResponse);
    addResponseToCache(cacheKey, currentResponse);  // Cache the response

    emit requestCompleted();

    serverSocket->deleteLater();
    workerThread->quit();  // Termină thread-ull
}

Request Proxy::parseHttpRequest(const QByteArray &requestData)
{
   return Request::parseFromRawData(requestData);
}

Response Proxy::parseHttpResponse(const QByteArray &responseData)
{
   return Response::parseFromRawData(responseData);
}


