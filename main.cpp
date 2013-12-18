#include "mainwindow.h"
#include <QApplication>

#define myapp(x) x = 3

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();


    return a.exec();
}
