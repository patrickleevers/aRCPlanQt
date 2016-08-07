//     aRCPLan
//     Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//     aRCPlan may be freely distributed under the MIT license.
//     For the underlying model, see http://www.sciencedirect.com/science/article/pii/S0013794412003530

//     Implementation of the about dialog box shown in about.ui

#include <QString>

#include "dialog.h"
#include "ui_dialog.h"

dialog::dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dialog)
{
    ui->setupUi(this);
}

dialog::~dialog()
{
    delete ui;
}


//The following functions define the text presented on the dialog box depending on the arguments passed.
//Multiple arguments can be supplied to include values within the text


void dialog::warning(string title)
{
    ui-> warning -> setText(QString::fromStdString(title));
}

void dialog::warning(string title, double value)
{
    ui-> warning -> setText(QString::fromStdString(title) + QString::number(value));
}

void dialog::warning(string title, double value, string title1, double value1)
{
    ui-> warning -> setText(QString::fromStdString(title) + QString::number(value)+QString::fromStdString(title1) + QString::number(value1));
}

void dialog::warning(string title, double value, string title1, double value1,string title2, double value2)
{
    ui-> warning -> setText(QString::fromStdString(title) + QString::number(value)+QString::fromStdString(title1) + QString::number(value1) + QString::fromStdString(title2) + QString::number(value2));
}

void dialog::warning(string title, double value, string title1, double value1,string title2, double value2, string title3, double value3)
{
    ui-> warning -> setText(QString::fromStdString(title) + QString::number(value)+QString::fromStdString(title1) + QString::number(value1) + QString::fromStdString(title2) + QString::number(value2) + QString::fromStdString(title3) + QString::number(value3));
}

void dialog::warning(string title, string title1, double value1, string title2, double value2, string title3, double value3, string title4, double value4, string title5, double value5)
{
    ui-> warning -> setText(QString::fromStdString(title) + QString::fromStdString(title1) + QString::number(value1) + QString::fromStdString(title2) + QString::number(value2) + QString::fromStdString(title3) + QString::number(value3)  + QString::fromStdString(title4) + QString::number(value4) + QString::fromStdString(title5) + QString::number(value5));
}
