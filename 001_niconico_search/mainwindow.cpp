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
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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
    req.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    req.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
    req.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "dummy");

    m_HttpClient->setReplyFinishedCallback([&](QNetworkReply *reply) {
        Q_ASSERT(reply);

        if(reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            return;
        }

        auto body = reply->readAll();
        auto json = QJsonDocument::fromJson(body);
        auto root = json.object();
        auto meta = root["meta"];
        auto metaObj = meta.toObject();

        if (metaObj["status"].toInt() != 200) {
            qDebug() << "invalid status";
            qDebug() << root;
            return;
        }

        int totalCount = metaObj["totalCount"].toInt();
        if (totalCount == 0) {
            qDebug() << "data is empty (.meta)";
            qDebug() << root;
            return;
        }

        auto data = root["data"];
        auto dataArray = data.toArray();

        if (dataArray.isEmpty()) {
            qDebug() << "data is empty (.data)";
            qDebug() << root;
            return;
        }

        ui->listVideos->clear();
        ui->listVideos->setIconSize(QSize(130, 100));

        foreach(auto item, dataArray) {
            if (!item.isObject()) {
                continue;
            }
            auto itemObj = item.toObject();
            auto contentId = itemObj["contentId"].toString();
            auto title = itemObj["title"].toString();
            auto listItem = new QListWidgetItem(title);
            auto iconUrl = "https://tn.smilevideo.jp/smile?i=" + contentId.right(contentId.length() - 2);

            auto client = new HttpClient(this);
            client->setReplyFinishedCallback([listItem](QNetworkReply* reply2){
                if (reply2->error() != QNetworkReply::NoError) {
                    return;
                }
                auto body2 = reply2->readAll();
                QImage img;
                img.loadFromData(body2);
                if (listItem) {
                    listItem->setIcon(QIcon(QPixmap::fromImage(img)));
                }
            });
            QNetworkRequest req2(iconUrl);
            req2.setSslConfiguration(QSslConfiguration::defaultConfiguration());
            req2.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "dummy");
            client->sendRequest(HttpClient::Methods::kGet, req2);

            ui->listVideos->addItem(listItem);
        }
    });

    m_HttpClient->sendRequest(HttpClient::Methods::kGet, req);
}

