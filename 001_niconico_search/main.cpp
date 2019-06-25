#include "mainwindow.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QLocale::setDefault(QLocale(QLocale::Japanese, QLocale::Japan));
    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());

    MainWindow window;
    window.show();

    return app.exec();
}
