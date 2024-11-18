#include "mainwindow.h"
#include<QDebug>
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    qDebug()<<"***************************\n";
    w.show();
    return a.exec();
}
