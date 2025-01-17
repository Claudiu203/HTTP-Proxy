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
    proxy = new Proxy(ui->textEdit_1,ui->textEdit_2, this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButtonBrowser_clicked()
{
    QProcess *browserProcess = new QProcess(this);
    QString browserPath = "/snap/bin/chromium"; // Asigură-te că această cale este corectă pentru Chromium pe sistemul tău
    QStringList arguments;
    // Verificăm dacă checkbox-ul este bifat
    // Interceptare activă: Setăm proxy-ul
    arguments << "--user-data-dir=/tmp/custom_chromium_profile_intercept"; // Profil separat pentru interceptare
    arguments << "--proxy-server=127.0.0.1:8888"; // Adresa și portul proxy-ului tău
    arguments << "--proxy-bypass-list=localhost;127.0.0.1"; // Domenii excluse
    if(ui->checkBoxIntercept->isChecked())
        QMessageBox::information(this, "Intercept Mode", "Intercept mode is ON. Requests will be captured.");


    // Deschidem o fereastră nouă
    arguments << "--new-window";
    arguments << "http://example.com"; // URL-ul dorit (opțional)

    // Pornim Chromium cu argumentele configurate
    browserProcess->start(browserPath, arguments);
}


void MainWindow::on_pushButtonSend_clicked()
{
    //this->proxy->setQtext(ui->textEdit_1);
    this->proxy->sendRequest();
}

