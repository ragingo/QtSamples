#pragma once

#include <functional>
#include <QString>
#include <QtNetwork>

class HttpClient : public QObject
{
Q_OBJECT

public:
    enum class Methods {
        kGet,
        kPost,
    };

    using ReplyFinishedDelegate = std::function<void (QNetworkReply*)>;

    HttpClient(QObject *parent) : QObject (parent) {
        m_Manager = new QNetworkAccessManager(this);
    }

    virtual ~HttpClient() {
        connect(m_Manager, &QNetworkAccessManager::finished, m_Manager, &QNetworkAccessManager::deleteLater);
    }

    void setReplyFinishedCallback(ReplyFinishedDelegate delegate) {
        m_onReplyFinished = delegate;
    }

    void sendRequest(Methods method, QNetworkRequest req, QByteArray *data = nullptr);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    ReplyFinishedDelegate m_onReplyFinished;
    QNetworkAccessManager *m_Manager = nullptr;
};
