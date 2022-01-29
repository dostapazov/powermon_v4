#include "mainwindow.h"
#include <QApplication>
#include <qscreen.h>
#include <qdesktopwidget.h>


int main(int argc, char *argv[])
{
    //QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
    MainWindow w;
#ifdef Q_OS_ANDROID
    w.showFullScreen();
#else
    w.showFullScreen();
    //w.showMaximized();
#endif

    return a.exec();
}
