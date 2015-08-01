#include "mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QTextCodec>

int main(int argc, char *argv[])
{

    Q_INIT_RESOURCE(app);
    QApplication a(argc, argv);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    MainWindow w;
    w.showMaximized();
/*
    QSize size(QApplication::desktop()->availableGeometry().width(),
               QApplication::desktop()->availableGeometry().height());
    w.resize(size);
    w.move ((QApplication::desktop()->width() - w.width())/2,
            (QApplication::desktop()->height() - w.height())/2);
*/
    return a.exec();
}
