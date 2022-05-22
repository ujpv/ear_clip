#include "polygon_builder.h"

#include <QPolygon>

PolygonBuilder::PolygonBuilder(Ring ring)
    : shell(std::move(ring)) {
  if (shell.size() > 2) {
    shell.push_back(shell.front());
    state = {Stage::ShellCompleted, IsValid::Yes};
  }

  for (auto p : shell) {
    extendBBox(p);
  }
}

void PolygonBuilder::addPoint(PolygonBuilder::Point p) {
  if (state.first == Stage::ShellCompleted)
    return;

  auto stage = Stage::DrawingShell;
  shell.push_back(p);
  extendBBox(p);
  setState({stage, shell.size() > 2 ? IsValid::Yes : IsValid::No});

  emit changed();
}

void PolygonBuilder::reset() {
  shell.clear();
  setState(INIT_STATE);
  bBox = std::nullopt;

  emit changed();
}

void PolygonBuilder::draw(QPainter &painter, const QTransform &transform) {
  painter.setPen(QPen(Qt::black, 4, Qt::SolidLine, Qt::RoundCap));
  auto shellTransformed = transform.map(shell);
  painter.drawPolyline(shellTransformed);

  painter.setPen(QPen(Qt::black, 10, Qt::SolidLine, Qt::RoundCap));
  painter.drawPoints(shellTransformed);
}

void PolygonBuilder::completeShell() {
  if (state.first == Stage::ShellCompleted)
    return;

  setState({Stage::ShellCompleted, state.second});
  shell.push_back(shell.front());
  emit changed();
}

PolygonBuilder::Ring PolygonBuilder::getRing() const {
  if (state.first != Stage::ShellCompleted)
    return {};

  auto result = shell;
  result.pop_back();
  return result;
}

void PolygonBuilder::setState(State newState) {
  if (state != newState) {
    emit stateChanged(newState);
  }

  state = newState;
}

PolygonBuilder::State PolygonBuilder::getState() const {
  return state;
}

void PolygonBuilder::extendBBox(PolygonBuilder::Point p) {
  if (bBox) {
    bBox->setTopLeft({std::min(bBox->left(), p.x()),
                      std::min(bBox->top(), p.y())});

    bBox->setBottomRight({std::max(bBox->right(), p.x()),
                          std::max(bBox->bottom(), p.y())});
  } else {
    bBox = BBox(p, p);
  }
}

PolygonBuilder::BBox PolygonBuilder::getBBox() const {
  Q_ASSERT(bBox);
  return *bBox;
}
