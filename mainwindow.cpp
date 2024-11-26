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

    if (!ui->checkBoxIntercept->isChecked()) {
        QDesktopServices::openUrl(QUrl("http://google.com"));  // Deschide pagina de pornire Google
    } else {
        // Intercept activat
        QMessageBox::information(this, "Intercept Mode", "Intercept mode is ON. Requests will be captured.");

        // Pornim un proces de browser în sistem
        // Exemplu: deschide browser-ul Chrome cu un URL gol sau pagina de pornire
        QProcess *browserProcess = new QProcess(this);
        QString browserPath = "/usr/bin/firefox";  // Înlocuiește cu calea completă către executabilul browser-ului tău (ex: Chrome, Firefox)
        browserProcess->start(browserPath);

    }
}


void MainWindow::on_pushButtonSend_clicked()
{
    //this->proxy->setQtext(ui->textEdit_1);
    this->proxy->sendRequest();
}

