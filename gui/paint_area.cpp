#include "paint_area.h"

#include <QPainter>
#include <QMouseEvent>

void PaintArea::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPalette p(palette());
    p.setColor(QPalette::Background, Qt::white);
    setAutoFillBackground(true);
    setPalette(p);
    QPainter painter(this);

    QTransform transform;
    if (!bBox.isEmpty()) {
        auto viewport = painter.viewport();
        auto extendedBbox = bBox.marginsAdded(QMarginsF(
            bBox.width() * 0.025,
            bBox.height() * 0.025,
            bBox.width() * 0.025,
            bBox.height() * 0.025));
        double scale = std::min(
            viewport.width() / extendedBbox.width(),
            viewport.height() / extendedBbox.height() );
        transform = QTransform()
            .scale(scale, scale)
            .translate(-extendedBbox.left(), -extendedBbox.top());
    }

    for (auto& h: paintHandlers)
        h(painter, transform);
}

PaintArea::PaintArea(QWidget* p)
    : QWidget(p)
{}

void PaintArea::addPaintLayer(PaintArea::PaintHandler handler)
{
    paintHandlers.push_back(std::move(handler));
}

void PaintArea::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);
    emit newPoint({event->x(), event->y()});
}

void PaintArea::setBBox(PaintArea::BBox newBBox)
{
    bBox = newBBox;
}

void PaintArea::resetView()
{
    bBox = QRectF();
    update();
}

