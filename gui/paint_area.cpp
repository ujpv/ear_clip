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
    for (auto& h: paintHandlers)
        h(painter);
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

