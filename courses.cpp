#include "courses.h"
#include "qjsonarray.h"

void WeekDay::update(WeekDay& previous_day)
{
    left_ = previous_day.get_right();
    right_ = center_ + (center_ - left_);
}

void Course::add_group(const QString& group, const QString& teacher, const QString& room, const QString& time, const QString& week)
{
    CourseData* data = new CourseData;
    data->room = room;
    data->teacher = teacher;
    data->time = time;

    groups.insert({group, data});
}

Box::Box(const QJsonObject& text_obj, int left, int right)
{
    int height = text_obj["MaxHeight"].toInt();
    int min_top = text_obj["MinTop"].toInt();
    QJsonArray words = text_obj["Words"].toArray();
    int text_left = words.first()["Left"].toInt();
    int text_right = words.last()["Left"].toInt() + words.last()["Width"].toInt();
    QString text = "";
    for (auto word: words){
        text += word.toObject()["WordText"].toString() + " ";
    }

    left_ = left;
    right_ = right;
    text_ = text;
    vertical_top_ = min_top + height;
    vertical_expected_bottom_ = INT_MAX;
    horizontal_text_left_ = text_left;
    horizontal_text_right_ = text_right;
}

ADD_TEXT Box::add_text(const QJsonObject& text_obj)
{
    QString new_text = text_obj["LineText"].toString();
    int height = text_obj["MaxHeight"].toInt();
    int min_top = text_obj["MinTop"].toInt();
    QJsonArray words = text_obj["Words"].toArray();
    int left = words.first()["Left"].toInt();
    int right = words.last()["Left"].toInt() + words.last()["Width"].toInt();

    if (left_ > left || right_ < right){
        return ADD_TEXT::NEXT_COURSE;
    }

    ADD_TEXT return_value = ADD_TEXT::CORRECT;
    // int horizontal_center = left_ + (right_ - left_)/2;
    // int text_horizontal_center = left + (right - left)/2;

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
    text_ = text_ +  " " + new_text;

    return return_value;
}

void Box::update_bottom() {
    int center = (vertical_text_center_[0] + vertical_text_center_[1]) / 2;
    vertical_expected_bottom_ = center + (center - vertical_top_);
}

bool Box::is_under(int height) const {
    return (lines_ == 0 && height > vertical_expected_bottom_) || ( lines_ > 0 && height > vertical_bottom_ + text_height_*3);
}
