#ifndef PROGRESS_REPORTER_P_H
#define PROGRESS_REPORTER_P_H

#include "helper/progress_reporter.h"

class ProgressReporterPrivate
{
public:
    explicit ProgressReporterPrivate(ProgressReporter::ReportingFunction f, int goal,
                                     int start = 0, bool enabled = true);
    int progress() const;

    ProgressReporter::ReportingFunction _reporter;
    int  _goal;
    int  _current;
    bool _enabled;
};

#endif // PROGRESS_REPORTER_P_H
