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

/**
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
    multipart->append(partParameter("apikey","helloworld")); // K82913451688957  helloworld

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
        //     qDebug() << text << left << min_top;
        // }
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

            if (left < days.begin()->first.get_left() && words.size()>1){
                words.erase(words.begin());
                line_obj = QJsonObject{
                    {"MaxHeihgt", line_obj["MaxHeight"].toInt()},
                    {"MinTop", line_obj["MinTop"].toInt()},
                    {"Words", words}
                };
                left = words.first()["Left"].toInt();
                qDebug() << "conversion";
            }

            if (left>520)
                continue;

            for (auto& day: days){

                if (left >= day.first.get_left() && left <= day.first.get_right()){
                    if (day.second.empty()){
                        day.second.push_back(new Box(line_obj, day.first.get_left(), day.first.get_right()));
                    }
                    else{
                        ADD_TEXT messange = ADD_TEXT::NEXT_COURSE;

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
                            int num_of_courses = day.second.size() + 1;
                            int width = (day.first.get_right() - day.first.get_left()) / num_of_courses;

                            day.second.push_back(new Box(line_obj, day.first.get_right()-width, day.first.get_right()));
                            std::sort(day.second.begin(), day.second.end(), box_compare_left);

                            for (std::size_t i = 0; i<day.second.size(); i++){
                                day.second[i]->set_lines(day.first.get_left()+width*i, day.first.get_left()+width*(i+1));
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

        for (auto& box: temp_boxes){
            if (box->get_left() < 520)
                qDebug() << box->get_text() << box->get_left() << box->get_right();
        }
    }
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
