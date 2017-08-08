#include "fdnwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FDNwindow w;
    w.show();

    return a.exec();
}
