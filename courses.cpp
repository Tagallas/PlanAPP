#include "courses.h"
#include "qjsonarray.h"

void WeekDay::update(WeekDay& previous_day)
{
    left_ = previous_day.get_right();
    right_ = center_ + (center_ - left_);
}

bool Course::operator==(const QString& qother)
{
    std::string name = name_.toStdString();
    std::string other = qother.toStdString();

    replace(name, " i ", "");
    replace(other, " i ", "");
    replace(name, " ", "");
    replace(other, " ", "");

    bool corr = (name == other);
    if (corr)
        if (name_.size() > other.size())
            name_ = qother;

    return corr;
}

Course::operator QString() const{

    QString os =  "Course: " + name_ + "\n";
    for (auto it = types_.begin(); it != types_.end(); ++it){
        os += "\t" + it->first + ": \n";
        for (const auto& course: it->second)
            os += "\t\t" + course.day + ", " + course.group + ", " + course.time + ", " + course.other_info + ", " + QString::number(course.length) + "\n";
    }

    return os;
}

void Course::print() const{
    qDebug() << "Course: " << name_;
    for (auto it = types_.begin(); it != types_.end(); ++it){
        qDebug() << "    " << it->first << ":";
        for (const auto& course: it->second)
            qDebug() << "        " << course.day << ", " << course.group << ", " << course.time << ", " << course.other_info + ", " + QString::number(course.length);
    }
}

void Course::add_group(const QString& type, const AGHCourseData& data)
{
    types_[type].push_back(data);
}

Box::Box(const QJsonObject& text_obj, int left, int right, QString day)
{
    int height = text_obj["MaxHeight"].toInt();
    int min_top = text_obj["MinTop"].toInt();
    QJsonArray words = text_obj["Words"].toArray();
    int text_left = words.first()["Left"].toInt();
    int text_right = words.last()["Left"].toInt() + words.last()["Width"].toInt();
    for (auto word: words){
        text_.push_back(word.toObject()["WordText"].toString());
    }

    day_ = day;
    left_ = left;
    right_ = right;
    vertical_top_ = min_top + height;
    vertical_expected_bottom_ = INT_MAX;
    horizontal_text_left_ = text_left;
    horizontal_text_right_ = text_right;
}

ADD_TEXT Box::add_text(const QJsonObject& text_obj)
{
    int height = text_obj["MaxHeight"].toInt();
    int min_top = text_obj["MinTop"].toInt();
    QJsonArray words = text_obj["Words"].toArray();
    int left = words.first()["Left"].toInt();
    int right = words.last()["Left"].toInt() + words.last()["Width"].toInt();

    if (left_ > left || right_ < right){
        return ADD_TEXT::NEXT_COURSE;
    }

    ADD_TEXT return_value = ADD_TEXT::CORRECT;

    if (lines_ > 0 && (left > horizontal_text_right_ || right < horizontal_text_left_)){
        return ADD_TEXT::NEXT_COURSE;
    }

    int text_vertical_center = min_top + (height/2);

    if (lines_ == 0){
        if (abs(vertical_top_ - (min_top + height)) < height)
            return ADD_TEXT::NEXT_COURSE;

        vertical_text_center_[0] = text_vertical_center;
        vertical_text_center_[1] = text_vertical_center;
    }
    else{
        vertical_text_center_[1] = text_vertical_center;
    }

    vertical_bottom_ = min_top;
    text_height_ = height;
    if (horizontal_text_right_ < right)
        horizontal_text_right_ = right;

    this->update_bottom();
    lines_ += 1;
    for (auto word: words)
        text_.push_back(word.toObject()["WordText"].toString());

    return return_value;
}

void Box::update_bottom() {
    int center = (vertical_text_center_[0] + vertical_text_center_[1]) / 2;
    vertical_expected_bottom_ = center + (center - vertical_top_);
}

bool Box::is_under(int height) const {
    return (lines_ == 0 && height > vertical_expected_bottom_) || ( lines_ > 0 && height > vertical_bottom_ + text_height_*3);
}

void replace(std::string& str, const std::string& from, const std::string& to){
    std::size_t found = str.find(from);
    while (found != std::string::npos) {
        str.replace(found, from.length(), to);
        found = str.find(from);
    }
}
