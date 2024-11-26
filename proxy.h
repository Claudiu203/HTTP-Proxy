#ifndef PROXY_H
#define PROXY_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QTextEdit>
#include<QPushButton>
#include<QList>
#include<QFile>
#include<QThread>
#include<QSslSocket>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>  // Pentru QMetaObject::invokeMethod
#include <QTextEdit>
#include"request.h"
#include"response.h"
#include <QMap>

class Proxy: public QTcpServer
{
    Q_OBJECT
public:
    Proxy(QTextEdit *requestDisplay,QTextEdit *sendDisplay, QObject *parent = nullptr);
    ~Proxy();
    void displayRequest(const Request& currentRequest);
    void displaySend(const Response& currentResponse);
    QByteArray constructHttpRequest(const Request &request);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

public slots:
    void onReadyRead();
    void onClientDisconnected();
    void sendRequest();
    void setQtext(QTextEdit *txt);
    void writeRequestsToFile();

    bool isResponseInCache(const QString &cacheKey);
    void addResponseToCache(const QString &cacheKey, const Response &response);
private slots:
    void handleRequest();

signals:
    void requestCompleted();

private:


    // Funcție pentru a analiza cererea HTTP din QByteArray în structura Request
    Request parseHttpRequest(const QByteArray &requestData);
    Response parseHttpResponse(const QByteArray &responseData);


    QTextEdit *requestDisplay;  // Referință la QTextEdit pentru a vizualiza cererile HTTP
    QTextEdit *sendDisplay;
    QTcpSocket *pendingClientSocket;// Socketul clientului care așteaptă răspunsul serverului
    QByteArray pendingRequestData;  // Cererea HTTP temporară pentru modificare și trimitere
    QList<Request>requests;
    QMap<QString, Response> cache;
    Request currentRequest;
    Response currentResponse;

    QThread *workerThread;
    QMutex mutex;
};


#endif // PROXY_H
