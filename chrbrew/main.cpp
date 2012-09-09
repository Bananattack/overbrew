#include <QApplication>
#include <QTextEdit>

#include "mainwindow.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setOrganizationName("Overkill");
    app.setApplicationName(chrbrew::AppName);

    chrbrew::MainWindow win;
    win.show();
    return app.exec();
}
