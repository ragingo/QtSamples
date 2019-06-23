#include "httpclient.h"

void HttpClient::sendRequest(Methods method, QNetworkRequest req, QByteArray *data)
{
    connect(m_Manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onReplyFinished(QNetworkReply*)));

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
    Q_ASSERT(reply);
    if (m_onReplyFinished) {
        m_onReplyFinished(reply);
    }
}
