#ifndef FILE_PROJECT_LOAD_DIALOG_H
#define FILE_PROJECT_LOAD_DIALOG_H

#include <QFileDialog>

class FileProjectLoadDialog : public QFileDialog
{
    Q_OBJECT

public:
    explicit FileProjectLoadDialog(const QString& directory, QWidget* parent = 0);
};

#endif // FILE_PROJECT_LOAD_DIALOG_H
