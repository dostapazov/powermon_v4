#include "mainwindow.h"
#include <QApplication>
#include <qscreen.h>
#include <qdesktopwidget.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile file(":/powermon.qss");
    if(!file.open(QIODevice::ReadOnly))
        qDebug() << "Cannot open stylesheet file powermon.qss";
    QString stylesheet = QString::fromUtf8(file.readAll());
    //qApp->setStyleSheet(stylesheet);

    MainWindow w;
#ifdef Q_OS_ANDROID
    w.showFullScreen();
#else
    #ifdef QT_DEBUG
        w.show();
    #else
        w.showMaximized();
    #endif
#endif

    return a.exec();
}
