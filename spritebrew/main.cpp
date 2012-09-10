#include <QApplication>
#include <QTextEdit>

#include "mainwindow.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setOrganizationName("Overkill");
    app.setApplicationName(spritebrew::AppName);

    spritebrew::MainWindow win;
    win.show();
    return app.exec();
}
