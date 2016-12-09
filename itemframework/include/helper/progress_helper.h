#ifndef PROGRESS_HELPER_H
#define PROGRESS_HELPER_H

#include "appcore.h" // ITEMFRAMEWORK_EXPORT

#include <functional> // std::function
#include <QString>
#include <QScopedPointer>

class ITEMFRAMEWORK_EXPORT ProgressReporter
{
public:
    using ReportingFunction = std::function<void(int, QString const&)>;

    explicit ProgressReporter(ReportingFunction reporter, int goal, int start = 0, bool enabled = true);
    explicit ProgressReporter(ReportingFunction reporter, int goal, bool enabled);
    virtual ~ProgressReporter();
    int  current() const;
    int  goal()    const;
    void advance();
    void done();
    void report(QString const& message = "");

private:
    QScopedPointer<class ProgressReporterPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ProgressReporter)
};

#endif // PROGRESS_HELPER_H
