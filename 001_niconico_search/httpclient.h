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
        connect(m_Manager, &QNetworkAccessManager::finished, this, &HttpClient::onReplyFinished);
    }

    virtual ~HttpClient() {
        m_onReplyFinished = nullptr;
        delete m_Manager;
        m_Manager = nullptr;
    }

    void setReplyFinishedCallback(ReplyFinishedDelegate delegate) {
        m_onReplyFinished = delegate;
    }

    void sendRequest(Methods method, QNetworkRequest req, QByteArray *data = nullptr);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    ReplyFinishedDelegate m_onReplyFinished = nullptr;
    QNetworkAccessManager *m_Manager = nullptr;
};
