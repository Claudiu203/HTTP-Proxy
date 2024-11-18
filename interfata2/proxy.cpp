#include "proxy.h"
#include <QTcpSocket>
#include <QTextStream>
#include <QHostAddress>
#include <QSslSocket>
#include <QDebug>
#include <QProcess>



Proxy::Proxy(QTextEdit *requestDisplay, QObject *parent)
    : QTcpServer(parent), requestDisplay(requestDisplay)
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


    displayRequest(currentRequest);

    qDebug() << "Cererea HTTP interceptată:";
    qDebug() << "Method: " << currentRequest.method;
    qDebug() << "URL: " << currentRequest.url;
    qDebug() << "HTTP Version: " << currentRequest.httpVersion;

    // Afișează anteturile și corpul cererii
    for (auto it = currentRequest.headers.begin(); it != currentRequest.headers.end(); ++it) {
        qDebug() << it.key() << ": " << it.value();
    }

    if (!currentRequest.body.isEmpty()) {
        qDebug() << "Corpul cererii: " << QString::fromUtf8(currentRequest.body);
    }

    qDebug() << "Așteaptă trimiterea manuală a cererii de către utilizator.";
}

void Proxy::onClientDisconnected()
{
    QTcpSocket *clientSocket = static_cast<QTcpSocket*>(sender());
    clientSocket->deleteLater();  // Eliberăm socket-ul clientului
}

void Proxy::sendRequest()
{

}


void Proxy::setQtext(QTextEdit *txt)
{
    this->requestDisplay=txt;
}

Request Proxy::parseHttpRequest(const QByteArray &requestData)
{
    Request request;
    QList<QByteArray> requestLines = requestData.split('\n');


    if (requestLines.isEmpty()) {
        qDebug() << "Cererea HTTP este goală.";
        return request;
    }

    // Prima linie: metoda, URL și versiunea HTTP
    QByteArray firstLine = requestLines.at(0).trimmed();
    QList<QByteArray> parts = firstLine.split(' ');

    // Verifică dacă linia inițială are exact trei părți (metodă, URL și versiune HTTP)
    if (parts.size() != 3) {
        qDebug() << "Linia inițială a cererii HTTP nu are formatul așteptat. Așteptat: [Metodă URL Versiune]";
        return request;
    }


    request.method = QString(parts.at(0));
    request.url = QString(parts.at(1));
    request.httpVersion = QString(parts.at(2));


    for (int i = 1; i < requestLines.size(); ++i) {
        QByteArray line = requestLines.at(i).trimmed();
        if (line.isEmpty()) {
            break; // Linie goală, sfârșitul antetelor
        }


        QList<QByteArray> headerParts = line.split(':');
        if (headerParts.size() < 2) { // Verifică dacă linia conține un antet și o valoare
            qDebug() << "Antetul nu are formatul așteptat: " << line;
            continue; // Sare peste antetul invalid
        }

        // Asignează antetul și valoarea acestuia în structura `headers`
        request.headers[QString(headerParts.at(0).trimmed())] = QString(headerParts.at(1).trimmed());
    }

    // Găsește corpul cererii (dacă există) după secțiunea de anteturi, după "\r\n\r\n"
    int bodyIndex = requestData.indexOf("\r\n\r\n");
    if (bodyIndex != -1 && requestData.size() > bodyIndex + 4) {
        request.body = requestData.mid(bodyIndex + 4);
    } else {
        qDebug() << "Nu s-a găsit un corp de date în cererea HTTP.";
    }

    return request;
}


