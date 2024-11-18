#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include<QUrl>
#include<QDesktopServices>
#include <QMessageBox>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>        // dacă folosești QLabel
#include <QPushButton>   // pentru butoane
#include <QWidget>
#include <QProcess>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    proxy = new Proxy(ui->textEdit_1, this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButtonBrowser_clicked()
{

    if (!ui->checkBoxIntercept->isChecked()) {
        QDesktopServices::openUrl(QUrl("http://google.com"));  // Deschide pagina de pornire Google
    } else {
        // Intercept activat
        QMessageBox::information(this, "Intercept Mode", "Intercept mode is ON. Requests will be captured.");

        // Deschide Firefox cu profilul specific și URL-ul de pornire
        QString url = "http://google.com"; // Poți seta URL-ul pe care vrei să-l deschizi
        openFirefoxWithProxyProfile(url);
    }
}


void MainWindow::on_pushButtonSend_clicked()
{
    //this->proxy->setQtext(ui->textEdit_1);
    this->proxy->sendRequest();
}

void MainWindow::openFirefoxWithProxyProfile(const QString &url)
{
    QString firefoxPath = "/usr/bin/firefox"; // Calea către Firefox
    QStringList args;
    args << "-p" << "http_proxy" // Profilul specific pe care îl vei folosi
         << "--no-remote"
         << "-new-instance"     // Lansează o instanță nouă a Firefox
         << url;                // URL-ul care va fi deschis

    QProcess *process = new QProcess();
    process->start(firefoxPath, args);
}

