#include "editwindow.h"

EditWindow::EditWindow(const std::vector<Course>& courses, QMainWindow* parent){
    parent_ = parent;

    layout = new QVBoxLayout();
    QWidget* layout_content = new QWidget;

    for (const auto& course: courses){
        CourseLayout* line = new CourseLayout(layout_content, course);
        layout->addLayout(line);
    }

    QPushButton* cancel = new QPushButton("CANCEL");
    QPushButton* ok = new QPushButton("NEXT");
    cancel->setMaximumWidth(100);
    ok->setMaximumWidth(100);

    QHBoxLayout *layout1 = new QHBoxLayout();
    layout1->addWidget(cancel);
    layout1->addWidget(ok);

    layout->addLayout(layout1);


    layout_content->setLayout(layout);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidgetResizable(true);
    setWidget(layout_content);

    resize(800, 500);
    setWindowTitle(
        QApplication::translate("editwindow", "Edit"));

    QObject::connect(cancel, &QPushButton::clicked, this, &EditWindow::exit);
    QObject::connect(ok, &QPushButton::clicked, this, &EditWindow::save);
}

void EditWindow::save() {
    qDebug() << "save data";
}

void EditWindow::exit() {
    parent_->show();
    delete this;
}


TypeLayout::TypeLayout(QWidget* window, const QString& name, const std::vector<AGHCourseData>& groups_data){
    window_ = window;
    data_ = groups_data;
    type_name_ = new QLineEdit (name);
    group_ = new QComboBox();
    edit_button_ = new QPushButton("Edit");

    group_->addItem("-");
    for (const auto& data: groups_data){
        group_->addItem(data.group);
    }

    type_name_->setMaximumWidth(250);
    group_->setMaximumWidth(50);
    edit_button_->setMaximumWidth(50);

    addWidget(type_name_);
    addWidget(group_);
    addWidget(edit_button_);

    QObject::connect(edit_button_, &QPushButton::clicked, this, &TypeLayout::button_clicked);
}


void TypeLayout::button_clicked() {
    for (auto it = data_.begin(); it != data_.end(); it++){
        if (it->group == group_->currentText()){
            edit_course_ = new EditCourse(window_, *it, this);
            edit_course_->show();
            window_->setDisabled(true);
            iterator_ = it;
            break;
        }
    }
    if (!edit_course_){
        qDebug() << "Select group";
        QMessageBox msgBox;
        msgBox.setText("Select Group");
        msgBox.exec();
    }
}

void TypeLayout::update(QString day, QString group, QString time, QString other_info, QString length){
    int idx = group_->findText(iterator_->group);
    group_->setItemText(idx, group);

    iterator_->day = day;
    iterator_->group = group;
    iterator_->time = time;
    iterator_->other_info = other_info;
    iterator_->length = length.toInt();
}


CourseLayout::CourseLayout(QWidget* window, const Course& course){
    window_ = window;

    course_name_ = new QLineEdit (course.get_name());
    course_name_->setMaximumWidth(300);

    addWidget(course_name_);

    for (const auto& type:course.get_types()){
        types_.push_back(new TypeLayout(window_, type.first, type.second));
        addLayout(types_.back());
    }
}


EditCourse::EditCourse(QWidget* window, const AGHCourseData& data, TypeLayout* parent) {
    window_ = window;
    parent_ = parent;

    day_ = new QLineEdit(data.day);
    group_ = new QLineEdit(data.group);
    time_ = new QLineEdit(data.time);
    other_info_ = new QLineEdit(data.other_info);
    length_ = new QComboBox();
    length_->addItem(QString::number(data.length));
    for (auto len: COURSE_LENGTHS){
        if (len != data.length)
            length_->addItem(QString::number(len));
    }

    QPushButton* cancel = new QPushButton("CANCEL");
    QPushButton* ok = new QPushButton("OK");
    cancel->setMaximumWidth(100);
    ok->setMaximumWidth(100);

    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->addWidget(cancel);
    button_layout->addWidget(ok);

    layout_ = new QVBoxLayout();

    layout_->addWidget(new QLabel(QString("day:")));
    layout_->addWidget(day_);
    layout_->addWidget(new QLabel(QString("group:")));
    layout_->addWidget(group_);
    layout_->addWidget(new QLabel(QString("time:")));
    layout_->addWidget(time_);
    layout_->addWidget(new QLabel(QString("other_info:")));
    layout_->addWidget(other_info_);
    layout_->addWidget(new QLabel(QString("length:")));
    layout_->addWidget(length_);

    layout_->addLayout(button_layout);

    setLayout(layout_);

    resize(600, 400);
    setWindowTitle(
        QApplication::translate("editwindow", "Edit"));

    QObject::connect(cancel, &QPushButton::clicked, this, &EditCourse::exit);
    QObject::connect(ok, &QPushButton::clicked, this, &EditCourse::save);
}


void EditCourse::save() {
    qDebug() << "save";
    parent_ -> update(day_->text(), group_->text(), time_->text(), other_info_->text(), length_->currentText());
    exit();
}



