#include "editdialog.h"
#include "ui_editdialog.h"

EditDialog::EditDialog(QWidget *parent, const std::vector<Course>& courses)
    : QDialog(parent)
    , ui(new Ui::EditDialog)
{
    ui->setupUi(this);

    QWidget window;
    QLabel *label = new QLabel(QApplication::translate("windowlayout", "Name:"));
    QLineEdit *lineEdit = new QLineEdit();

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(label);
    layout->addWidget(lineEdit);
    window.setLayout(layout);
    window.setWindowTitle(
        QApplication::translate("windowlayout", "Window layout"));
    window.show();
}

EditDialog::~EditDialog()
{
    delete ui;
}

void EditDialog::on_buttonBox_rejected()
{
    delete this;
}

