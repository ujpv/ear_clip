#pragma once

#include "ear_clip.h"

#include <QThread>
#include <QPainter>

#include <memory>

class Triangulation : public QThread {
 Q_OBJECT
 public:
  using Point = QPointF;
  using Triangle = QVector<Point>;
  using Ring = QVector<Point>;
  void draw(QPainter &painter, const QTransform &transform);
  void setRing(Ring newPolygon);
  std::optional<std::string> getError() const;

 public slots:
  void reset();

 private:
  void run() final;

 private:
  std::shared_ptr<QVector<Triangle>> triangles;
  std::shared_ptr<Ring> ring_;
  std::optional<std::string> error;
};