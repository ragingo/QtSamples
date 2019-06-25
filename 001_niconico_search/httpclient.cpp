#include "httpclient.h"
#include <qdebug.h>

void HttpClient::sendRequest(Methods method, QNetworkRequest req, QByteArray *data)
{
    qDebug() << "[HttpClient] sendRequest: " << req.url().toString() << endl;

    req.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    req.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "dummy");

    switch (method) {
    case Methods::kGet:
        m_Manager->get(req);
        break;
    case Methods::kPost:
        m_Manager->post(req, data ? *data : QByteArray());
        break;
    }
}

void HttpClient::onReplyFinished(QNetworkReply* reply)
{
    qDebug() << "[HttpClient] onReplyFinished: " << reply << endl;

    if (reply && m_onReplyFinished) {
        m_onReplyFinished(reply);

        reply->deleteLater();
        reply = nullptr;

        m_onReplyFinished = nullptr;
    }
}
