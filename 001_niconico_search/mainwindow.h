#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "httpclient.h"
#include "searchresult.h"

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
    void onListRowsInserted(const QModelIndex &parent, int first, int last);

private:
    Ui::MainWindow *ui;
    HttpClient *m_HttpClient = nullptr;
    SearchResult m_SearchResult = {};
};

#endif // MAINWINDOW_H
