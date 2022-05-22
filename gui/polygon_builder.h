#pragma once

#include <QObject>
#include <QPainter>
#include <QPoint>

#include <list>

class PolygonBuilder : public QObject {
 Q_OBJECT
 public:
  using Point = QPointF;
  using Ring = QVector<Point>;
  using BBox = QRectF;

  enum class Stage {
    Empty,
    DrawingShell,
    ShellCompleted
  };
  enum class IsValid { Yes, No };
  using State = std::pair<Stage, IsValid>;
  static constexpr State INIT_STATE = {Stage::Empty, IsValid::No};

  explicit PolygonBuilder(Ring ring = {});

  State getState() const;
  BBox getBBox() const;

  void draw(QPainter &painter, const QTransform &transform);

 public slots:
  void addPoint(Point point);
  void reset();
  void completeShell();
  Ring getRing() const;

 signals:
  void changed();
  void stateChanged(State);

 private:
  void setState(State state);
  void extendBBox(Point p);

  State state = INIT_STATE;
  Ring shell;
  std::optional<BBox> bBox;
};
