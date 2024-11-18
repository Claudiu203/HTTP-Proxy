#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "proxy.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:


    void on_pushButtonBrowser_clicked();

    void on_pushButtonSend_clicked();
    void openFirefoxWithProxyProfile(const QString &url);
private:
    Ui::MainWindow *ui;
    Proxy *proxy ;
};
#endif // MAINWINDOW_H
