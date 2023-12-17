#ifndef EDITWINDOW_H
#define EDITWINDOW_H

#include "courses.h"
#include "editcourse.h"

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QApplication>
#include <QPushButton>
#include <QComboBox>
#include <QScrollArea>
#include <QMessageBox>
#include <QMainWindow>

class CourseLayout;
class EditCourse;

class EditWindow : public QScrollArea
{
public:
    EditWindow(const std::vector<Course>& courses, QMainWindow* parent);
    ~EditWindow() = default;

    void save();
    void exit();

private:
    QMainWindow* parent_;
    QVBoxLayout *layout;
};


class TypeLayout : public QHBoxLayout
{
public:
    TypeLayout(QWidget* window, const QString& name, const std::vector<AGHCourseData>& groups_data);
    ~TypeLayout() = default;

    void button_clicked();
    void update(QString day, QString group, QString time, QString other_info, QString length);

private:
    QWidget* window_;
    EditCourse* edit_course_;

    QLineEdit* type_name_;
    QComboBox* group_;
    QPushButton* edit_button_;

    std::vector<AGHCourseData>::iterator iterator_;
    std::vector<AGHCourseData> data_;
};


class CourseLayout : public QVBoxLayout
{
public:
    CourseLayout(QWidget* window, const Course& course);
    ~CourseLayout() = default;

private:
    QWidget* window_;

    QLineEdit* course_name_;
    std::vector<TypeLayout*> types_;
};


class EditCourse : public QWidget
{
public:
    EditCourse(QWidget* window, const AGHCourseData& data, TypeLayout* parent);
    ~EditCourse()= default;

    void save();
    void exit() {window_->setDisabled(false);
        delete this; }

protected:
    void closeEvent(QCloseEvent* event) override {exit(); }

private:
    QWidget* window_;
    TypeLayout* parent_;

    QVBoxLayout* layout_;

    QLineEdit* type_;
    QLineEdit* day_;
    QLineEdit* group_;
    QLineEdit* time_;
    QLineEdit* other_info_;
    QComboBox* length_;
};


#endif // EDITWINDOW_H
