#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "httpclient.h"
#include <functional>
#include <QString>
#include <QtNetwork>
#include <QJsonDocument>

namespace {

    constexpr auto SearchApiUrl = QStringViewLiteral("https://api.search.nicovideo.jp/api/v2/video/contents/search");

    QUrlQuery buildSearchParams(QString keyword) {
        QUrlQuery params;
        params.addQueryItem("q", keyword);
        params.addQueryItem("targets", "title");
        params.addQueryItem("fields", "contentId,title,viewCounter");
        params.addQueryItem("_sort", "-viewCounter");
        params.addQueryItem("_offset", "0");
        params.addQueryItem("_limit", "10");
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
            return std::move(result);
        }

        auto data = root["data"];

        foreach(const auto& item, data.toArray()) {
            result.data.items.emplace_back(item["contentId"].toString(), item["title"].toString(), item["viewCounter"].toInt());
        }

        return std::move(result);
    }

    QString getIconUrl(QStringRef contentId) {
        Q_ASSERT(!contentId.isEmpty());
        Q_ASSERT(contentId.length() > 3);
        return "https://tn.smilevideo.jp/smile?i=" + contentId.right(contentId.length() - 2);
    }

    QIcon convertToQIcon(const QByteArray& data) {
        QImage img;
        img.loadFromData(data);
        return std::move(QIcon(QPixmap::fromImage(img)));
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->listVideos->model(), &QAbstractItemModel::rowsInserted, this, &MainWindow::onListRowsInserted);

    m_HttpClient = new HttpClient(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 検索ボタンクリック
void MainWindow::on_btnSearch_clicked()
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

        foreach(const auto& item, m_SearchResult.data.items) {
            auto listItem = new QListWidgetItem(item.title);
            ui->listVideos->model()->setProperty("contentId", item.contentId);
            ui->listVideos->addItem(listItem);
        }
    });

    m_HttpClient->sendRequest(HttpClient::Methods::kGet, req);
}

void MainWindow::onListRowsInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(last);

    auto dataItem = m_SearchResult.data.items[static_cast<size_t>(first)];
    auto listItem = ui->listVideos->item(first);
    auto iconUrl = getIconUrl(&dataItem.contentId);

    QNetworkRequest req(iconUrl);
    auto client = new HttpClient(this);
    client->setReplyFinishedCallback([listItem](QNetworkReply* reply){
        if (reply->error() != QNetworkReply::NoError) {
            return;
        }

        if (listItem) {
            auto icon = convertToQIcon(reply->readAll());
            listItem->setIcon(icon);
        }
    });
    client->sendRequest(HttpClient::Methods::kGet, req);
}
