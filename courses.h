#ifndef COURSES_H
#define COURSES_H

#include <QJsonObject>
#include <QString>
#include <map>
#include <iostream>
#include <math.h>

enum class ADD_TEXT{
    CORRECT,
    NEXT_COURSE,
    ERROR,
};

class WeekDay
{
public:
    WeekDay(QString day, int left, int width) : day_(day), left_(left), center_(left+(width/2)) {}
    WeekDay(int left, int center) : day_("Poniedziałek"), left_(left), center_(center), right_(center+(center-left)) {}
    ~WeekDay() = default;

    void update(WeekDay& previous_day);
    //void update_monday(int left);

    bool operator<(const WeekDay& other) const {return left_ < other.get_left(); }
    void set_text(const QString& text) {day_ = text; }

    const QString get_day() const {return day_; }
    int get_left() const {return left_; }
    int get_right() const {return right_; }

private:
    QString day_;
    int left_;
    int right_;
    int center_;
};

struct CourseData
{
    QString teacher;
    QString room;
    QString time;
    //WeekDay& day;
};

class Course
{
public:
    Course(const QString& name) : name_(name) {};
    ~Course() = default;

    bool operator==(const QString& other) {return name_ == other; }

    void add_group(const QString& group, const QString& teacher, const QString& room, const QString& time, const QString& week);
    void set_length(int length) {length_ = length; }

private:
    QString name_;
    std::map<QString, CourseData*> groups;
    int length_;
};

class Box
{
public:
    Box(const QJsonObject& text_obj, int left, int right);
    ~Box() = default;

    ADD_TEXT add_text(const QJsonObject& text_obj);

    void set_lines(int left, int right) {left_ = left, right_ = right; }
    void update_bottom();

    bool is_under(int height) const {return height > vertical_expected_bottom_; }
    int get_right() const {return right_; }
    int get_left() const {return left_; }
    int get_horizontal_left() const {return horizontal_text_left_; }
    QString get_text() const {return text_; }

private:
    QString text_;
    int lines_ = 0;

    int left_;
    int right_;

    int horizontal_text_left_;

    int vertical_top_;
    int vertical_text_center_[2];
    int vertical_expected_bottom_;
};




#endif // COURSES_H
