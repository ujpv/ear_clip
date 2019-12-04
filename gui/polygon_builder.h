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

    enum class Stage { DrawingShell, ShellCompleted };
    enum class IsValid { Yes, No };
    using State = std::pair<Stage, IsValid>;
    static constexpr State INIT_STATE = {Stage::DrawingShell, IsValid::No};

    void draw(QPainter& painter);

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
    State state = INIT_STATE;
    Ring shell;
};
