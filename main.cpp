#include "win001.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Win001 w;
    w.show();
    return a.exec();
}
