#include "mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{

    Q_INIT_RESOURCE(app);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    QSize size(QApplication::desktop()->availableGeometry().width() * 5 / 6 ,
               QApplication::desktop()->availableGeometry().height());
    w.resize(size);
    w.move ((QApplication::desktop()->width() - w.width())/2,
            (QApplication::desktop()->height() - w.height())/2);
    return a.exec();
}
