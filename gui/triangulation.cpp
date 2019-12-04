#include "triangulation.h"

void Triangulation::run()
{
    ear_clip::Polygon polygon;
    {
        auto ring = ring_;
        if (!ring) {
            return;
        }

        for (auto p: *ring) {
            polygon.push_back({p.x(), p.y()});
        }
    }

    auto newTriangles = std::make_shared<QVector<Triangle>>();
    for (auto t: ear_clip::triangulate(polygon)) {
        newTriangles->push_back({{t[0].x, t[0].y},
                                 {t[1].x, t[1].y},
                                 {t[2].x, t[2].y},
                                 {t[0].x, t[0].y}});
    }

    triangles = newTriangles;
}

void Triangulation::draw(QPainter& painter)
{
    painter.setPen(QPen(Qt::green, 1, Qt::SolidLine, Qt::RoundCap));
    auto ts = triangles;
    if (!ts)
        return;

    for (const auto& t: *ts) {
        painter.drawPolyline(t);
    }
}

void Triangulation::reset()
{
    triangles.reset();
    ring_.reset();
}

void Triangulation::setRing(Triangulation::Ring newPolygon)
{
    auto polygon = std::make_shared<Ring>();
    *polygon = std::move(newPolygon);
    ring_ = polygon;
}