#include "mainwindow.h"
#include "courses.h"

#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_pressed() {
    setDisabled(true);
    splash = new QSplashScreen();
    splash->showMessage("Loading...");
    splash->show();

    recognize();
}

void MainWindow::exit(){
    show();
    setDisabled(false);
}


/*
 * @brief Function that sets proper format of the multipart message
 * @param key parameter required to send to API
 * @param value
 * @return QHttpPart object to be added to multipart message
 */

QHttpPart partParameter(QString key, QString value) {
    QHttpPart part;
    part.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\""+ key +"\""));
    part.setBody(value.toLatin1());
    return part;
}

void MainWindow::recognize()
{
    multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    //set up http part message to send to api that contains image data
    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image.gif"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"*.gif\""));

    QString fileName = "C:/Users/wojte/Desktop/doPlanu/usos2.gif";
    QFile* file = new QFile(fileName);

    //debugging: make sure file was uploaded
    if(!file->open(QIODevice::ReadOnly)) {
        qDebug() << "# Could not upload/open file";
    }

    QByteArray fileContent(file->readAll());
    imagePart.setBody(fileContent);

    //append image data, api key, language, and overlay setting to multipart
    multipart->append(imagePart);
    multipart->append(partParameter("language", "pol"));

    //multipart->append(partParameter("isTable", "true"));
    multipart->append(partParameter("scale", "true"));
    multipart->append(partParameter("isOverlayRequired","true"));
    multipart->append(partParameter("apikey","helloworld")); //   helloworld

    QUrl api_url("https://api.ocr.space/parse/image");

    //create network request obj that contains the api url
    QNetworkRequest api_request(api_url);
    manager = new QNetworkAccessManager;
    reply = manager->post(api_request, multipart);

    QObject::connect(reply, SIGNAL(finished()), this, SLOT(networkData()));
    //debugging: make sure file was successfully opened
    qDebug() << file->size() << "bytes";

    imagePart.setBodyDevice(file);
    file->setParent(multipart);
    networkData();
}

void MainWindow::networkData()
{
    qDebug() << "networkData";

    //test for network error
    QNetworkReply::NetworkError err = reply->error();
    if (err != QNetworkReply::NoError) {
        qDebug() << reply->errorString();
        return;
    }

    //store the network's response in a string
    QString response = (QString)reply->readAll();

    //network reply is stored in JSON format; get only the OCR'd text results
    QJsonDocument jsonDoc = QJsonDocument::fromJson(response.toUtf8());
    QJsonObject jsonObj = jsonDoc.object();
    QJsonArray jsonArr = jsonObj["ParsedResults"].toArray();

    foreach(const QJsonValue& value, jsonArr) {
        QJsonObject obj = value.toObject();
        QJsonObject overlay = obj["TextOverlay"].toObject();
        QJsonArray lines = overlay["Lines"].toArray();

        std::sort(lines.begin(), lines.end(), compare_top);
        std::sort(lines.begin(), lines.begin()+5, compare_left);

        std::vector<WeekDay> temp_days;
        std::vector<Box*> temp_boxes;

        // for (auto line: lines){
        //     QJsonObject line_obj = line.toObject();
        //     QString text = line_obj["LineText"].toString();
        //     int left = line_obj["Words"].toArray().first()["Left"].toInt();
        //     int min_top = line_obj["MinTop"].toInt();
        //     qDebug() << text << line_obj["Words"].toArray().size();// << left << min_top;
        // }
        // qDebug() << "_____________________________";
        // break;

        // add Monday
        QJsonObject line_obj = lines.begin()->toObject();
        QString text = line_obj["LineText"].toString();
        QJsonArray words = line_obj["Words"].toArray();
        int left = words.first()["Left"].toInt();
        int width = words.first()["Width"].toInt();

        QJsonObject hour_obj = (lines.begin()+5)->toObject();
        QJsonArray hour = hour_obj["Words"].toArray();
        int left_hour = hour.first()["Left"].toInt();
        int width_hour = hour.first()["Width"].toInt();

        temp_days.push_back(WeekDay(left_hour+width_hour+2, left+(width/2)));

        for (auto it = lines.begin()+1; it != lines.begin()+5; it++)
        {
            line_obj = it->toObject();
            words = line_obj["Words"].toArray();
            text = line_obj["LineText"].toString();
            left = words.first()["Left"].toInt();
            width = words.first()["Width"].toInt();

            temp_days.push_back(WeekDay(text, left, width));
            temp_days.back().update(temp_days.end()[-2]);
        }

        for (std::size_t i = 0; i<6; i++)
            lines.pop_front();

        foreach (const auto& t_day, temp_days)
            days[std::move(t_day)] = {};
        temp_days.clear();

        foreach (const auto& line, lines)
        {
            QJsonObject line_obj = line.toObject(); // keys() = ("LineText", "MaxHeight", "MinTop", "Words")

            int min_top = line_obj["MinTop"].toInt();
            QJsonArray words = line_obj["Words"].toArray();
            int left = words.first()["Left"].toInt();

            if ((left < days.begin()->first.get_left() && words.size()>1) || (words.first()["WordText"].toString() == "-")){

                words.erase(words.begin());
                line_obj = QJsonObject{
                    {"LineText", line_obj["LineText"].toString()},
                    {"MaxHeight", line_obj["MaxHeight"].toInt()},
                    {"MinTop", line_obj["MinTop"].toInt()},
                    {"Words", words}
                };
                left = words.first()["Left"].toInt();
            }

            if (words.last()["WordText"].toString() == "-"){

                words.erase(words.end());
                line_obj = QJsonObject{
                    {"LineText", line_obj["LineText"].toString()},
                    {"MaxHeight", line_obj["MaxHeight"].toInt()},
                    {"MinTop", line_obj["MinTop"].toInt()},
                    {"Words", words}
                };
                left = words.first()["Left"].toInt();
            }

            // if (left>2757 || left<2062)
            //     continue;

            for (auto& day: days){

                if (left >= day.first.get_left() && left <= day.first.get_right()){
                    if (day.second.empty()){
                        day.second.push_back(new Box(line_obj, day.first.get_left(), day.first.get_right(), day.first.get_day()));
                    }
                    else{
                        ADD_TEXT messange = ADD_TEXT::NEXT_COURSE;

                        // text = line_obj["LineText"].toString();
                        // if (text == "13:15, gr.3")
                        //     qDebug() << 1;

                        for (auto it = day.second.begin(); it != day.second.end(); it++){
                            if ((*it)->is_under(min_top)){
                                temp_boxes.push_back(std::move(*it));
                                it--;
                                day.second.erase(it+1);
                            }
                            else
                                if (messange != ADD_TEXT::CORRECT)
                                    messange = (*it)->add_text(line_obj);
                        }

                        if (messange == ADD_TEXT::NEXT_COURSE){
                            int curr_courses = 0;
                            if (day.second.size() > 0)
                                curr_courses =  day.second.front()->get_divide();

                            int num_of_courses = std::max(int(day.second.size() + 1), curr_courses);
                            int width = (day.first.get_right() - day.first.get_left()) / num_of_courses;

                            day.second.push_back(new Box(line_obj, day.first.get_right()-width, day.first.get_right(), day.first.get_day()));

                            bool is_correct = false;

                            while (!is_correct){
                                is_correct = true;
                                std::sort(day.second.begin(), day.second.end(), box_compare_left);

                                for (std::size_t i = 0; i<num_of_courses; i++){
                                    if (i<day.second.size() && day.second[i]->get_horizontal_left() < day.first.get_left()+width*(i+1)){

                                        if (day.second[i]->get_horizontal_left() >= day.first.get_left()+width*(i) &&
                                            (day.second[i]->get_horizontal_right() <= day.first.get_left()+width*(i+1)) ){

                                            day.second[i]->set_lines(day.first.get_left()+width*i, day.first.get_left()+width*(i+1));
                                            day.second[i]->set_divided(num_of_courses);
                                        }
                                        else{
                                            num_of_courses++;
                                            width = (day.first.get_right() - day.first.get_left()) / num_of_courses;
                                            is_correct = false;
                                            break;
                                        }
                                    }
                                }
                            }
                        }

                        if (messange == ADD_TEXT::ERROR){
                            qDebug() << "ERROR";
                        }
                    }
                    break;
                }
            }
        }

        for (auto& day: days){
            for (std::size_t idx = 0; idx < day.second.size(); idx++){
                temp_boxes.push_back(std::move(day.second[idx]));
                day.second.erase(day.second.begin()+idx);
                idx--;
            }
            day.second.clear();
        }

        // qDebug() << "Poniedziałek";
        //     for (auto& box: temp_boxes){
        //     if (box->get_left() < 520)
        //         qDebug() << box->get_text() << box->get_left() << box->get_right();
        // }
        // qDebug() << "Wtorek";
        // for (auto& box: temp_boxes){
        //     if (box->get_left() < 940 && box->get_left() >= 520)
        //         qDebug() << box->get_text() << box->get_left() << box->get_right();
        // }
        // qDebug() << "Środa";
        //     for (auto& box: temp_boxes){
        //     if (box->get_left() < 1358 && box->get_left() >= 940)
        //         qDebug() << box->get_text() << box->get_left() << box->get_right();
        // }
        // qDebug() << "Czwartek";
        // for (auto& box: temp_boxes){
        //     if (box->get_left() < 2060 && box->get_left() >= 1358)
        //         qDebug() << box->get_text() << box->get_left() << box->get_right();
        // }
        // qDebug() << "Piątek";
        //     for (auto& box: temp_boxes){
        //     if (box->get_left() < 2760 && box->get_left() >= 2060)
        //         qDebug() << box->get_text() << box->get_left() << box->get_right();
        // }

        sort_courses(temp_boxes);
    }
}

void MainWindow::sort_courses(const std::vector<Box*>& boxes){
    for (const auto& box: boxes){
        if (plan == PLAN::Semestralny){
            std::vector<QString> text = box->get_text();
            QString day = box->get_day();

            std::map<QString, QString> data = {
                                                {"5-hour", ""},
                                                {"4-group", ""},
                                                {"1-name", ""},
                                                {"2-type", ""},
                                                {"3-other", ""}};
            auto it = data.end();
            it--;

            for (const auto& word: text){
                // if (word == "Gibiec")
                //     qDebug() << 1;
                // qDebug() << word;
                if (word.contains("(", Qt::CaseInsensitive)){
                    it = data.begin();
                    std::advance(it, 2);
                    it->second += word + " ";
                }
                else if (word.contains("-", Qt::CaseInsensitive) || word.contains("•", Qt::CaseInsensitive)){
                    it++;
                }
                else if ((word.contains("gr.", Qt::CaseInsensitive))|| (word.contains("gr,", Qt::CaseInsensitive)) || (data["4-group"].size() > 0 && data["4-group"].size() < 4)){
                    it = data.begin();
                    data["4-group"] += word;
                }
                else{
                    it->second += word + " ";
                }
            }

            if (data["2-type"] == ""){
                QStringList parts = data["1-name"].split(' ');
                data["2-type"] = parts.last();
                parts.pop_back();
                if (data["2-type"]==""){
                    data["2-type"] = parts.last();
                    parts.pop_back();
                }


                if (!data["2-type"].contains("wyk") && parts.capacity() > 2) {
                    data["2-type"] = parts.last() + ' ' + data["2-type"];
                    parts.pop_back();
                }
                data["1-name"] = parts.join(' ');
            }

            AGHCourseData data_struct;
            data_struct.day = day;
            data_struct.group = data["4-group"];
            data_struct.time = data["5-hour"];
            data_struct.other_info = data["3-other"];
            data["2-type"] = data["2-type"].toLower();
            if (data["2-type"].back() == ' ')
                data["2-type"].remove(data["2-type"].size()-1, 1);

            bool is_added = false;
            for (auto& course: courses){
                if (course == data["1-name"]){
                    course.add_group(data["2-type"], data_struct);
                    is_added = true;
                    break;
                }
            }
            if (!is_added)
                courses.push_back(Course(data["1-name"], data["2-type"], data_struct));
        }

        // if (plan == PLAN::Tygodniowy){
            // TODO
        // }
    }

    for (const auto& course: courses)
        course.print();

    delete splash;
    hide();
    edit_dialog = new EditDialog(this);
    edit_dialog->show();
    QObject::connect(edit_dialog, SIGNAL(rejected()), this, SLOT(exit()));
}


bool compare_top(const QJsonValue& v1, const QJsonValue& v2)
{
    QJsonObject jsonObject1 = v1.toObject();
    QJsonObject jsonObject2 = v2.toObject();

    int top1 = jsonObject1["MinTop"].toInt() + jsonObject1["MaxHeight"].toInt();
    int top2 = jsonObject2["MinTop"].toInt() + jsonObject2["MaxHeight"].toInt();

    return top1 < top2;
}

bool compare_left(const QJsonValue& v1, const QJsonValue& v2)
{
    QJsonObject jsonObject1 = v1.toObject();
    QJsonObject jsonObject2 = v2.toObject();

    QJsonArray words1 = jsonObject1["Words"].toArray();
    QJsonArray words2 = jsonObject2["Words"].toArray();

    return words1.first()["Left"].toInt() < words2.first()["Left"].toInt();
}

bool box_compare_left(const Box* v1, const Box* v2)
{
    return v1->get_horizontal_left() < v2->get_horizontal_left();
}

inline void swap(QJsonValueRef v1, QJsonValueRef v2)
{
    QJsonValue temp(v1);
    v1 = QJsonValue(v2);
    v2 = temp;
}
