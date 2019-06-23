#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "httpclient.h"

namespace Ui {
class MainWindow;
}

class QNetworkReply;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnSearch_clicked();

private:
    Ui::MainWindow *ui;
    HttpClient *m_HttpClient = nullptr;
};

#endif // MAINWINDOW_H
