#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "courses.h"
#include "editdialog.h"
#include "editwindow.h"
#include "variables.h"

#include <algorithm>
#include <vector>
#include <QMainWindow>
#include <QHttpPart>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QSplashScreen>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE


enum class PLAN{
    Semestralny,
    Tygodniowy
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void exit();
    void recognize();
    void networkData();
    void sort_courses(const std::vector<Box*>& boxes, int one_hour_height);

private slots:
    void on_pushButton_pressed();

private:
    Ui::MainWindow *ui;
    EditWindow *edit_dialog;
    QSplashScreen *splash;

    PLAN plan = PLAN::Semestralny;
    std::map<WeekDay, std::vector<Box*>> days;
    std::vector<Course> courses;

    QString fileName;
    QString text;
    QString language;

    QNetworkReply* reply;
    QNetworkAccessManager* manager;
    QHttpMultiPart* multipart;
};

bool compare_top(const QJsonValue& v1, const QJsonValue& v2);

bool compare_left(const QJsonValue& v1, const QJsonValue& v2);

bool box_compare_left(const Box* v1, const Box* v2);

inline void swap(QJsonValueRef v1, QJsonValueRef v2);

#endif // MAINWINDOW_H
