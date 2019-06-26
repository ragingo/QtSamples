#include <functional>
#include <QString>
#include <QtNetwork>
#include <QJsonDocument>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "httpclient.h"

namespace {

    // API仕様: https://site.nicovideo.jp/search-api-docs/search.html
    constexpr auto SearchApiUrl = QStringViewLiteral("https://api.search.nicovideo.jp/api/v2/video/contents/search");

    QUrlQuery buildSearchParams(QString keyword) {
        QUrlQuery params;
        params.addQueryItem("q", keyword);
        params.addQueryItem("targets", "title");
        params.addQueryItem("fields", "contentId,title,viewCounter,thumbnailUrl");
        params.addQueryItem("_sort", "-viewCounter");
        params.addQueryItem("_offset", "0");
        params.addQueryItem("_limit", "30");
        params.addQueryItem("_context", "apiguide");
        return params;
    }

    SearchResult parseSearchResponse(QByteArray response) {
        SearchResult result = {};

        auto json = QJsonDocument::fromJson(response);
        auto root = json.object();
        auto meta = root["meta"];
        auto metaObj = meta.toObject();

        result.meta.status = metaObj["status"].toInt();
        result.meta.totalCount = metaObj["totalCount"].toInt();

        if (result.meta.totalCount == 0) {
            return result;
        }

        auto data = root["data"];

        foreach(const auto& item, data.toArray()) {
            result.data.items.emplace_back(
                item["contentId"].toString(),
                item["title"].toString(),
                item["viewCounter"].toInt(),
                item["thumbnailUrl"].toString()
            );
        }

        return result;
    }

    QIcon convertToQIcon(QByteArray data) {
        QImage img;
        img.loadFromData(data);
        return QIcon(QPixmap::fromImage(img));
    }

    QMutex g_Mutex;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // https://blog.qt.io/jp/2011/03/23/go-to-slot-2/
    // 命名規則に従わない。スロットへの接続は自分でやる。
    connect(ui->btnSearch, &QPushButton::clicked, this, &MainWindow::onSearchButtonClicked);
    connect(ui->listVideos->model(), &QAbstractItemModel::rowsInserted, this, &MainWindow::onVideoListRowInserted);
    connect(ui->listVideos, &QListWidget::currentRowChanged, this, &MainWindow::onVideoListCurrentRowChanged);

    connect(this, &MainWindow::videoListItemIconDownloaded, this, &MainWindow::onVideoListItemIconDownloaded);

    m_HttpClient = new HttpClient(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 検索ボタンクリック
void MainWindow::onSearchButtonClicked()
{
    auto keyword = ui->txtKeyword->text();

    QUrl url(SearchApiUrl.toString());
    url.setQuery(buildSearchParams(keyword));

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");

    m_HttpClient->setReplyFinishedCallback([&](QNetworkReply *reply) {
        Q_ASSERT(reply);

        if(reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            return;
        }

        ui->listVideos->clear();
        ui->listVideos->setIconSize(QSize(130, 100));

        auto body = reply->readAll();
        m_SearchResult = parseSearchResponse(std::move(body));

        auto counterText =
            QString("%1/%2").arg(
                QLocale().toString(m_SearchResult.data.items.size()),
                QLocale().toString(m_SearchResult.meta.totalCount)
            );
        ui->labelCounter->setText(counterText);

        foreach(const auto& item, m_SearchResult.data.items) {
            auto listItem = new QListWidgetItem(item.title);
            listItem->setSizeHint(QSize(ui->listVideos->width(), 130));
            ui->listVideos->addItem(listItem);
        }
    });

    m_HttpClient->sendRequest(HttpClient::Methods::kGet, req);
}

// 動画リストの行追加時
void MainWindow::onVideoListRowInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(last);

    auto dataItem = m_SearchResult.data.items[static_cast<size_t>(first)];

    QNetworkRequest req(dataItem.thumbnailUrl);
    auto client = new HttpClient(this);

    client->setReplyFinishedCallback([this, first](QNetworkReply* reply){
        if (reply->error() != QNetworkReply::NoError) {
            return;
        }

        auto body = reply->readAll();
        emit videoListItemIconDownloaded(first, std::move(body));
    });

    client->sendRequest(HttpClient::Methods::kGet, req);
}

// 動画リストの現在行変更時
void MainWindow::onVideoListCurrentRowChanged(int rowIndex)
{
    qDebug() << "current row index: " << rowIndex;
}


// 動画リスト行に使うアイコンのダウンロード完了時
void MainWindow::onVideoListItemIconDownloaded(int rowIndex, QByteArray data)
{
    auto listItem = ui->listVideos->item(rowIndex);
    if (!listItem) {
        return;
    }

    listItem->setIcon(convertToQIcon(data));
}
