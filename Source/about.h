//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

#ifndef ABOUT_H
#define ABOUT_H
#include <QObject>
#include <QDialog>

namespace Ui {
class about;
}

class about : public QDialog
{
    Q_OBJECT

public:

    //Constructor
    explicit about(QWidget *parent = 0);

    //Destructor
    ~about();

private:
    Ui::about *ui;
};

#endif // ABOUT_H
