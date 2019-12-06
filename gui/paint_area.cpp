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
        double scale = std::min(
            viewport.width() / bBox.width(),
            viewport.height() / bBox.height() );
        transform = QTransform()
            .scale(scale, scale)
            .translate(-bBox.left(), -bBox.top());
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

