#include "helper/progress_reporter.h"
#include "progress_reporter_p.h"

ProgressReporter::ProgressReporter(ReportingFunction reporter, bool enabled)
    : d_ptr{new ProgressReporterPrivate{reporter, 0, 0, enabled}}
{}

ProgressReporter::ProgressReporter(ReportingFunction reporter, int goal, int current, bool enabled)
    : d_ptr{new ProgressReporterPrivate{reporter, goal, current, enabled}}
{}

ProgressReporter::ProgressReporter(ReportingFunction reporter, int goal, bool enabled)
    : ProgressReporter{reporter, goal, 0, enabled}
{}

ProgressReporter::ProgressReporter(ProgressReporter const& other)
    : d_ptr{new ProgressReporterPrivate{other.d_ptr->_reporter, other.d_ptr->_goal,
                                        other.d_ptr->_current, other.d_ptr->_enabled}}
{}

ProgressReporter::~ProgressReporter()
{}

ProgressReporterPrivate::ProgressReporterPrivate(ProgressReporter::ReportingFunction reporter,
                                                 int goal, int current, bool enabled)
    : _reporter{reporter}, _goal{goal}, _current{current}, _enabled{enabled}
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

void ProgressReporter::report(QString const& message) const
{
    Q_D(const ProgressReporter);

    if (!d->_enabled || d->_reporter == nullptr) {
        return;
    }

    d->_reporter(d->progress(), message);
}

void ProgressReporter::setEnabled(bool enabled)
{
    Q_D(ProgressReporter);

    d->_enabled = enabled;
}

void ProgressReporter::reset(int goal, int current)
{
    Q_D(ProgressReporter);

    d->_current = current;
    d->_goal    = goal;
}

int ProgressReporterPrivate::progress() const
{
    return _current * 100 / _goal;
}

