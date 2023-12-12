#ifndef COURSES_H
#define COURSES_H

#include <QJsonObject>
#include <QString>
#include <map>
#include <iostream>
#include <math.h>
#include <functional>
#include <algorithm>
#include <regex>

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

struct AGHCourseData
{
public:
    QString day;
    QString group;
    QString time;
    QString other_info;

};

class Course
{
public:
    Course(const QString& name, const QString& type, const AGHCourseData& data) : name_(name) {types_[type].push_back(data); };
    ~Course() = default;

    bool operator==(const QString& other);
    operator QString() const;

    void print() const;
    void add_group(const QString& type, const AGHCourseData& data);

private:
    QString name_;
    std::map<QString, std::vector<AGHCourseData>> types_;
};

class Box
{
public:
    Box(const QJsonObject& text_obj, int left, int right, QString day);
    ~Box() = default;

    ADD_TEXT add_text(const QJsonObject& text_obj);

    void set_lines(int left, int right) {left_ = left, right_ = right; }
    void set_divided(int div) {divide_width_ = div; }
    void update_bottom();

    bool is_under(int height) const;// {return height > vertical_expected_bottom_; }
    int get_divide() const {return divide_width_; }
    int get_right() const {return right_; }
    int get_left() const {return left_; }
    int get_horizontal_left() const {return horizontal_text_left_; }
    int get_horizontal_right() const {return horizontal_text_right_; }
    std::vector<QString> get_text() const {return text_; }
    QString get_day() const {return day_; }

private:
    std::vector<QString> text_;
    //QString text_;
    QString day_;
    int lines_ = 0;
    int divide_width_ = 1;
    int text_height_;

    int left_;
    int right_;

    int horizontal_text_left_;
    int horizontal_text_right_ = -1;

    int vertical_top_;
    int vertical_bottom_;
    int vertical_text_center_[2];
    int vertical_expected_bottom_;
};

void replace(std::string& str, const std::string& from, const std::string& to);



#endif // COURSES_H
