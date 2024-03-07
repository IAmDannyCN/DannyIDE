#include "dannyide.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DannyIDE w;
    w.show();
    return a.exec();
}
