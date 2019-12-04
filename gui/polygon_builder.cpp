#include "polygon_builder.h"

#include <QPolygon>

void PolygonBuilder::addPoint(PolygonBuilder::Point p)
{
    if (state.first == Stage::ShellCompleted)
        return;

    shell.push_back(p);
    if (shell.size() == 3) {
        setState({state.first, IsValid::Yes});
    }
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
