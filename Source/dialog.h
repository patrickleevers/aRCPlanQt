//     aRCPLan
//     Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//     aRCPlan may be freely distributed under the MIT license.
//     For the underlying model, see http://www.sciencedirect.com/science/article/pii/S0013794412003530

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
using namespace std;

namespace Ui {
class dialog;
}

class dialog : public QDialog
{
    Q_OBJECT

public:

    //Constructor
    explicit dialog(QWidget *parent = 0);

    //Destructor
    ~dialog();

    //Updates text on dialog to provide user with message
    void warning(string title);
    void warning(string title,
                 double value);
    void warning(string title,
                 double value,
                 string title1,
                 double value1);
    void warning(string title,
                 double value,
                 string title1,
                 double value1,
                 string title2,
                 double value2);
    void warning(string title,
                 double value,
                 string title1,
                 double value1,
                 string title2,
                 double value2,
                 string title3,
                 double value3);
    void warning(string title,
                 string title1,
                 double value1,
                 string title2,
                 double value2,
                 string title3,
                 double value3,
                 string title4,
                 double value4,
                 string title5,
                 double value5);

private:
    Ui::dialog *ui;
};

#endif // DIALOG_H
