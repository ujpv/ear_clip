#include "polygon_builder.h"

#include <QPolygon>

PolygonBuilder::PolygonBuilder(Ring ring)
    : shell(std::move(ring))
{
    if (ring.size() > 2) {
        shell.push_back(ring.front());
        state = {Stage::ShellCompleted, IsValid::Yes};
    }
}

void PolygonBuilder::addPoint(PolygonBuilder::Point p)
{
    if (state.first == Stage::ShellCompleted)
        return;

    auto stage = Stage::DrawingShell;
    shell.push_back(p);
    setState({stage, shell.size() > 2 ? IsValid::Yes : IsValid::No});

    emit changed();
}

void PolygonBuilder::reset()
{
    shell.clear();
    setState(INIT_STATE);
    emit changed();
}

void PolygonBuilder::draw(QPainter& painter)
{
    painter.setPen(QPen(Qt::black, 3, Qt::SolidLine, Qt::RoundCap));
    painter.drawPolyline(shell);
    painter.setPen(QPen(Qt::black, 9, Qt::SolidLine, Qt::RoundCap));
    painter.drawPoints(shell);
}

void PolygonBuilder::completeShell()
{
    if (state.first == Stage::ShellCompleted)
        return;

    setState({Stage::ShellCompleted, state.second});
    shell.push_back(shell.front());
    emit changed();
}

PolygonBuilder::Ring PolygonBuilder::getRing() const
{
    if (state.first != Stage::ShellCompleted)
        return {};

    auto result = shell;
    result.pop_back();
    return result;
}

void PolygonBuilder::setState(State newState)
{
    if (state != newState) {
        emit stateChanged(newState);
    }

    state = newState;
}

PolygonBuilder::State PolygonBuilder::getState() const
{
    return state;
}
