#pragma once

#include "polygon_builder.h"

#include <QWidget>
#include <functional>

class PaintArea : public QWidget {
 Q_OBJECT
 public:
  using PaintHandler = std::function<void(QPainter &, const QTransform &)>;
  using ClickHandler = std::function<void(QPointF &)>;
  using BBox = QRectF;

  explicit PaintArea(QWidget *p = nullptr);
  void addPaintLayer(PaintHandler handler);
  void setBBox(BBox bbox);

 signals:
  void newPoint(QPoint point);

 public slots:
  void resetView();

 protected:
  void paintEvent(QPaintEvent *event) final;
  void mouseReleaseEvent(QMouseEvent *event) override;

 private:
  std::vector<PaintHandler> paintHandlers;
  std::vector<ClickHandler> clickHandlers;
  BBox bBox;
};
