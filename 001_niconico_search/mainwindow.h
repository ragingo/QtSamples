#pragma once

#include <QMainWindow>
#include "searchresult.h"

namespace Ui {
    class MainWindow;
}

class HttpClient;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSearchButtonClicked();
    void onVideoListRowInserted(const QModelIndex &parent, int first, int last);
    void onVideoListCurrentRowChanged(int rowIndex);
    void onVideoListItemIconDownloaded(int rowIndex, QByteArray data);

private: signals:
    void videoListItemIconDownloaded(int rowIndex, QByteArray src);

private:
    Ui::MainWindow *ui = nullptr;
    HttpClient *m_HttpClient = nullptr;
    SearchResult m_SearchResult = {};
};
