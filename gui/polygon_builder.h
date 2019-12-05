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

    enum class Stage {
        Empty,
        DrawingShell,
        ShellCompleted
    };
    enum class IsValid { Yes, No };
    using State = std::pair<Stage, IsValid>;

    explicit PolygonBuilder(Ring ring = {});
    static constexpr State INIT_STATE = {Stage::Empty, IsValid::No};

    State getState() const;

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
