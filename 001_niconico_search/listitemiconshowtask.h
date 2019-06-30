#pragma once
#include <QtCore>
#include <QtWidgets>


class ListItemIconShowTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    ListItemIconShowTask(QListWidgetItem *item, QByteArray data)
        : m_Item(item), m_Data(data) {
        qDebug() << Q_FUNC_INFO;
        connect(this, &ListItemIconShowTask::iconCreated, this, &ListItemIconShowTask::onIconCreated, Qt::ConnectionType::DirectConnection);
    }

protected:
    void run() override {
        qDebug() << Q_FUNC_INFO;
        qDebug() << "tid: " << QThread::currentThreadId();
        auto icon = convertToQIcon(m_Data);
        emit iconCreated(icon);
    }

signals:
    void iconCreated(QIcon);

private slots:
    void onIconCreated(QIcon icon)
    {
        qDebug() << Q_FUNC_INFO;
        if (!m_Item) {
            qDebug() << "list item is null....";
            return;
        }
        qDebug() << "tid: " << QThread::currentThreadId();
        m_Item->setIcon(icon);
    }

private:
    QIcon convertToQIcon(QByteArray data) {
        QImage img;
        img.loadFromData(data);
        return QIcon(QPixmap::fromImage(img));
    }

    QListWidgetItem *m_Item = nullptr;
    QByteArray m_Data;
};
