#include "freefluidsmainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FreeFluidsMainWindow w;
    w.show();

    return a.exec();
}


