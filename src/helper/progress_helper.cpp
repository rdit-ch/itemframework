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
    : reporter_{reporter}, goal_{goal}, current_{start}, enabled_{enabled}
{}

int ProgressReporter::current() const
{
    Q_D(const ProgressReporter);

    return d->current_;
}

int ProgressReporter::goal() const
{
    Q_D(const ProgressReporter);

    return d->goal_;
}

void ProgressReporter::advance()
{
    Q_D(ProgressReporter);

    if (d->current_ >= d->goal_) {
        return;
    }

    d->current_++;
}

void ProgressReporter::done()
{
    Q_D(ProgressReporter);

    d->current_ = d->goal_;
}

void ProgressReporter::report(QString const& message)
{
    Q_D(ProgressReporter);

    if (!d->enabled_) {
        return;
    }

    d->reporter_(d->progress(), message);
}

int ProgressReporterPrivate::progress() const
{
    return current_ * 100 / goal_;
}
