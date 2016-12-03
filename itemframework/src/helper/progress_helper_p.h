#ifndef PROGRESS_HELPER_P_H
#define PROGRESS_HELPER_P_H

#include "helper/progress_helper.h"

struct ProgressReporterPrivate
{
    explicit ProgressReporterPrivate(ProgressReporter::ReportingFunction f, int goal,
                                     int start = 0, bool enabled = true);
    int progress() const;

    ProgressReporter::ReportingFunction reporter_;
    int  goal_;
    int  current_;
    bool enabled_;
};

#endif // PROGRESS_HELPER_P_H
