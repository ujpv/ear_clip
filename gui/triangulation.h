#pragma once

#include "ear_clip.h"

#include <QThread>
#include <QPainter>

#include <memory>

class Triangulation: public QThread {
    Q_OBJECT
public:
    using Point = QPointF;
    using Triangle = QVector<Point>;
    using Ring = QVector<Point>;
    void draw(QPainter& painter);
    void setRing(Ring newPolygon);

public slots:
    void reset();

private:
    void run() final;

private:
    std::shared_ptr<QVector<Triangle>> triangles;
    std::shared_ptr<Ring> ring_;
};