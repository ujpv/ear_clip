#include "main_window.h"

#include "polygon_builder.h"
#include "triangulation.h"

#include <QApplication>
#include <QDebug>

#include <set>
#include <iostream>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    PolygonBuilder::Ring ring;
    std::set<std::string> keys(argv, argv + argc);
    if (keys.count("-i")) {
        std::cout << "Expected space separated polygon points sequence(x1 y1 x2 y2) and Ctrl+D: ";
        PolygonBuilder::Point point;
        while (std::cin >> point.rx() >> point.ry())
            ring.push_back(point);
    }

    if (ring.size() > 2 && ring.front() == ring.back())
        ring.pop_back();

    PolygonBuilder polygonBuilder(ring);
    Triangulation triangulation;

    MainWindow mainWindow(&polygonBuilder, &triangulation);

    mainWindow.setWindowState(Qt::WindowMaximized);
    mainWindow.show();

    return app.exec();
}