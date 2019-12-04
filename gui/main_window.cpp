#include "main_window.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

MainWindow::MainWindow(PolygonBuilder* polygonBuilder, Triangulation* triangulation, QWidget* p)
    : QWidget(p)
    , polygonBuilder(polygonBuilder)
{
    auto mainLayout = new QVBoxLayout();

    auto buttonsLayout = new QHBoxLayout();
    resetPB = new QPushButton("Reset");
    buttonsLayout->addWidget(resetPB);
    completePB = new QPushButton("Complete");
    buttonsLayout->addWidget(completePB);
    triangulatePB = new QPushButton("Triangulate");
    buttonsLayout->addWidget(triangulatePB);

    mainLayout->addLayout(buttonsLayout);

    paintArea = new PaintArea();
    mainLayout->addWidget(paintArea);

    setLayout(mainLayout);
    setWindowTitle("Ear clipping demo");

    paintArea->setMinimumSize(512, 512);
    paintArea->addPaintLayer([polygonBuilder](QPainter& painter) {
        polygonBuilder->draw(painter);
    });

    paintArea->addPaintLayer([triangulation](QPainter& painter) {
        triangulation->draw(painter);
    });

    connect(resetPB, SIGNAL(clicked()), polygonBuilder, SLOT(reset()));
    connect(resetPB, SIGNAL(clicked()), triangulation, SLOT(reset()));

    connect(completePB, SIGNAL(clicked()), polygonBuilder, SLOT(completeShell()));
    connect(paintArea, &PaintArea::newPoint, polygonBuilder, &PolygonBuilder::addPoint);

    connect(triangulatePB, &QPushButton::clicked, triangulation, [=]() {
        triangulation->setRing(polygonBuilder->getRing());
        triangulation->start();
    });

    connect(polygonBuilder, SIGNAL(changed()), paintArea, SLOT(update()));
    connect(triangulation, SIGNAL(finished()), paintArea, SLOT(update()));
    connect(polygonBuilder, &PolygonBuilder::stateChanged, this, &MainWindow::updateState);

    updateState(PolygonBuilder::INIT_STATE);
}

void MainWindow::updateState(PolygonBuilder::State state)
{
    switch (state.first) {
        case PolygonBuilder::Stage::DrawingShell:
            resetPB->setEnabled(true);
            completePB->setEnabled(state.second == PolygonBuilder::IsValid::Yes);
            triangulatePB->setEnabled(false);
            break;
        case PolygonBuilder::Stage::ShellCompleted:
            resetPB->setEnabled(true);
            completePB->setEnabled(false);
            triangulatePB->setEnabled(true);
            break;
        default:
            throw std::runtime_error("Invalid state");
    }
}
