#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
//    QApplication::setSetuidAllowed(true);
    QApplication a(argc, argv);
    Widget w;
    QTranslator trans;
    trans.load("widget_ru.qm",".");
    a.installTranslator(&trans);
    w.show();
    return a.exec();
}
