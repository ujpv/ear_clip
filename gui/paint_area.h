#pragma once

#include "polygon_builder.h"

#include <QWidget>
#include <functional>

class PaintArea : public QWidget
{
    Q_OBJECT
public:
    using PaintHandler = std::function<void(QPainter&)>;
    using ClickHandler = std::function<void(QPointF&)>;
    explicit PaintArea(QWidget* p = nullptr);
    void addPaintLayer(PaintHandler handler);

signals:
    void newPoint(QPoint point);

protected:
    void paintEvent(QPaintEvent *event) final;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    std::vector<PaintHandler> paintHandlers;
    std::vector<ClickHandler> clickHandlers;
};
