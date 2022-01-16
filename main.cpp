#include "mainwindow.h"
#include <QApplication>
#include <qscreen.h>
#include <qdesktopwidget.h>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setAttribute(Qt::AA_EnableHighDpiScaling);
    MainWindow w;
    w.adjustSize();
#ifdef Q_OS_ANDROID
    w.showFullScreen();
#else
        w.showMaximized();
#endif

    return a.exec();
}
