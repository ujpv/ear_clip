#pragma once

#include "paint_area.h"
#include "polygon_builder.h"
#include "triangulation.h"

#include <QWidget>
#include <QPushButton>

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(PolygonBuilder* builder, Triangulation* triangulation, QWidget* p = nullptr);

public slots:
    void updateState(PolygonBuilder::State);

private:
    PaintArea* paintArea;
    QPushButton* resetPB;
    QPushButton* completePB;
    QPushButton* triangulatePB;
    QPushButton* zoomPB;
    PolygonBuilder* polygonBuilder;
};
