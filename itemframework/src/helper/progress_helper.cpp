#include "helper/progress_helper.h"
#include "progress_helper_p.h"

ProgressReporter::ProgressReporter(ReportingFunction reporter, int goal, int start, bool enabled)
    : d_ptr{new ProgressReporterPrivate{reporter, goal, start, enabled}}
{}

ProgressReporter::ProgressReporter(ReportingFunction reporter, int goal, bool enabled)
    : ProgressReporter{reporter, goal, 0, enabled}
{}

ProgressReporter::~ProgressReporter()
{}

ProgressReporterPrivate::ProgressReporterPrivate(ProgressReporter::ReportingFunction reporter,
                                                 int goal, int start, bool enabled)
    : _reporter{reporter}, _goal{goal}, _current{start}, _enabled{enabled}
{}

int ProgressReporter::current() const
{
    Q_D(const ProgressReporter);

    return d->_current;
}

int ProgressReporter::goal() const
{
    Q_D(const ProgressReporter);

    return d->_goal;
}

void ProgressReporter::advance()
{
    Q_D(ProgressReporter);

    if (d->_current >= d->_goal) {
        return;
    }

    d->_current++;
}

void ProgressReporter::done()
{
    Q_D(ProgressReporter);

    d->_current = d->_goal;
}

void ProgressReporter::report(QString const& message)
{
    Q_D(ProgressReporter);

    if (!d->_enabled) {
        return;
    }

    d->_reporter(d->progress(), message);
}

int ProgressReporterPrivate::progress() const
{
    return _current * 100 / _goal;
}
