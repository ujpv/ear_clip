#include "main_window.h"

#include "polygon_builder.h"
#include "triangulation.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    PolygonBuilder polygonBuilder;
    Triangulation triangulation;

    MainWindow mainWindow(&polygonBuilder, &triangulation);

    mainWindow.setWindowState(Qt::WindowMaximized);
    mainWindow.show();

    return app.exec();
}