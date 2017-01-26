#include "helper/progress_helper.h"
#include "progress_helper_p.h"

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
    : d_ptr{new ProgressReporterPrivate{other.d_ptr->reporter_, other.d_ptr->goal_,
                                        other.d_ptr->current_, other.d_ptr->enabled_}}
{}

ProgressReporter::~ProgressReporter()
{}

ProgressReporterPrivate::ProgressReporterPrivate(ProgressReporter::ReportingFunction reporter,
                                                 int goal, int current, bool enabled)
    : reporter_{reporter}, goal_{goal}, current_{current}, enabled_{enabled}
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

void ProgressReporter::report(QString const& message) const
{
    Q_D(const ProgressReporter);

    if (!d->enabled_ || d->reporter_ == nullptr) {
        return;
    }

    d->reporter_(d->progress(), message);
}

void ProgressReporter::setEnabled(bool enabled)
{
    Q_D(ProgressReporter);

    d->enabled_ = enabled;
}

void ProgressReporter::reset(int goal, int current)
{
    Q_D(ProgressReporter);

    d->current_ = current;
    d->goal_    = goal;
}

int ProgressReporterPrivate::progress() const
{
    return current_ * 100 / goal_;
}

