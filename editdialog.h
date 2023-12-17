#ifndef EDITDIALOG_H
#define EDITDIALOG_H

#include "courses.h"

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QApplication>

namespace Ui {
class EditDialog;
}

class EditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditDialog(QWidget *parent = nullptr, const std::vector<Course>& courses = {});
    ~EditDialog();

private slots:
    void on_buttonBox_rejected();

private:
    Ui::EditDialog *ui;
};

#endif // EDITDIALOG_H
