#ifndef PROGRESS_REPORTER_H
#define PROGRESS_REPORTER_H

#include "appcore.h" // ITEMFRAMEWORK_EXPORT

#include <functional> // std::function
#include <QString>
#include <QScopedPointer>

struct ITEMFRAMEWORK_EXPORT ProgressReporter
{
    using ReportingFunction = std::function<void(int, QString const&)>;

    explicit ProgressReporter(ReportingFunction reporter = nullptr, bool enabled = true);
    explicit ProgressReporter(ReportingFunction reporter, int goal, int current = 0,
                              bool enabled = true);
    explicit ProgressReporter(ReportingFunction reporter, int goal, bool enabled);
    ProgressReporter(ProgressReporter const& other);
    virtual ~ProgressReporter();

    int current() const;
    int goal() const;
    void report(QString const& message = "") const;

    void advance();
    void done();
    void setEnabled(bool enabled);
    void reset(int goal, int current = 0);

private:
    QScopedPointer<class ProgressReporterPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ProgressReporter)
};

#endif // PROGRESS_REPORTER_H
