#ifndef PROXY_H
#define PROXY_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QTextEdit>
#include<QPushButton>

struct Request {
    QString method;
    QString url;
    QString httpVersion;
    QMap<QString, QString> headers;
    QByteArray body;
};
class Proxy : public QTcpServer
{
    Q_OBJECT

public:
    Proxy(QTextEdit *requestDisplay, QObject *parent = nullptr);
    ~Proxy();
    void displayRequest(const Request& currentRequest);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

public slots:
    void onReadyRead();
    void onClientDisconnected();
    void sendRequest();
    void setQtext(QTextEdit *txt);

private:


    // Funcție pentru a analiza cererea HTTP din QByteArray în structura Request
    Request parseHttpRequest(const QByteArray &requestData);


    QTextEdit *requestDisplay;  // Referință la QTextEdit pentru a vizualiza cererile HTTP
    QTcpSocket *pendingClientSocket; // Socketul clientului care așteaptă răspunsul serverului
    QByteArray pendingRequestData;  // Cererea HTTP temporară pentru modificare și trimitere
    Request currentRequest;
};

#endif // PROXY_H
