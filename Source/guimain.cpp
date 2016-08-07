//     aRCPLan
//     Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//     aRCPlan may be freely distributed under the MIT license.
//     For the underlying model, see http://www.sciencedirect.com/science/article/pii/S0013794412003530

//     Implementation of the guimain dialog box shown in guimain.ui

#include <QString>
#include <iostream>
#include <fstream>
#include <ios>

#include "guimain.h"
#include "ui_guimain.h"
#include "about.h"
#include "dialog.h"
#include "qcustomplot.h"
#include "File.h"
#include "Constants.h"

guimain::guimain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::guimain)
{
    ui->setupUi(this);

}

//Destructor
guimain::~guimain()
{
    delete ui;
}


//Sets initial values using stored values
//including checkboxes and dropdowns
void guimain::setnames(Parameters parameters, char dropdown)
{

//Set up palettes
QPalette *gray = new QPalette();
gray->setColor(QPalette::Base,Qt::gray);
gray->setColor(QPalette::Text,Qt::darkGray);

QPalette *white = new QPalette();
white->setColor(QPalette::Base,Qt::white);
white->setColor(QPalette::Text,Qt::black);

extern File file;

if(parameters.fullscale)
{
    ui->fs->setCheckState(Qt::Checked);
    ui->s4->setCheckState(Qt::Unchecked);
}
else
{
    ui->fs->setCheckState(Qt::Unchecked);
    ui->s4->setCheckState(Qt::Checked);
}

if(parameters.is_backfilled)
   {ui->backfill->setCheckState(Qt::Checked);}
else
   {ui->backfill->setCheckState(Qt::Unchecked);}

if(parameters.outflow_model_on)
   {ui->fixedlength->setCheckState(Qt::Checked);}
else
   {ui->fixedlength->setCheckState(Qt::Unchecked);}

ui -> materialname -> setEditable(true);
ui -> materialname -> lineEdit() -> setReadOnly(false);
ui -> materialname -> lineEdit() -> setAlignment(Qt::AlignRight);

ui -> pipename -> setEditable(true);
ui -> pipename -> lineEdit() -> setReadOnly(false);
ui -> pipename -> lineEdit() -> setAlignment(Qt::AlignRight);

ui -> parameter -> setEditable(true);
ui -> parameter -> lineEdit() -> setReadOnly(true);
ui -> parameter -> lineEdit() -> setAlignment(Qt::AlignRight);

//If dropdown is empty fill with names, alternatively change index
if(dropdown==0)
{
    ui -> materialname ->insertItems(0, QStringList() << "Soft PE80" << "Generic PE100" << "Soft PE100" << "Generic PE1" << "Generic PE2" << "Test");
    ui -> pipename ->insertItems(0, QStringList() << "250mm_SDR11" << "250mm_SDR17" << "110mm_SDR11" << "110mm_SDR17" << "63mm_SDR11");
    ui -> parameter ->insertItems(0, QStringList() << "Normalised Crack Speed" << "Initial Pressure" << "Test Temperature");
    ui -> varCombo ->insertItems(0,QStringList() << "");
    ui -> yCombo -> insertItems(0,QStringList() << "");

}
else
{
    ui -> materialname ->lineEdit()->setText(QString::fromStdString(parameters.matid));
    ui -> pipename ->lineEdit()->setText(QString::fromStdString(parameters.pipeid));
}

ui -> density -> setAlignment(Qt::AlignRight);
ui -> density->setText(QString::number(parameters.density));

ui -> dynamicmodulus -> setAlignment(Qt::AlignRight);
ui -> dynamicmodulus ->setText(QString::number(parameters.edyn0degc));

ui -> deltadynamicmodulus -> setAlignment(Qt::AlignRight);
ui -> deltadynamicmodulus ->setText(QString::number(parameters.dedyndt));

ui -> creepmodulus -> setAlignment(Qt::AlignRight);
ui -> creepmodulus -> setText(QString::number(parameters.creep_modulus));

ui -> dynpoissonratio -> setAlignment(Qt::AlignRight);
ui -> dynpoissonratio -> setText(QString::number(parameters.poisson));

ui -> from -> setAlignment(Qt::AlignRight);
ui -> from -> setText(QString::number(0));

ui -> to -> setAlignment(Qt::AlignRight);
ui -> to -> setText(QString::number(1));

ui->varTo ->setText("");
ui->fromVar->setText("");

ui -> noofpoints -> setAlignment(Qt::AlignRight);
ui -> noofpoints -> setText(QString::number(50));

ui -> outsidediameter -> setAlignment(Qt::AlignRight);
ui -> outsidediameter -> setText(QString::number(parameters.diameter));

ui -> sdr -> setAlignment(Qt::AlignRight);
ui -> sdr -> setText(QString::number(parameters.sdr));

ui -> groovedepth -> setAlignment(Qt::AlignRight);
ui -> groovedepth -> setText(QString::number(parameters.notch_depth));

ui -> relativediameter -> setAlignment(Qt::AlignRight);
ui -> relativediameter -> setText(QString::number(parameters.diameter_creep_ratio));

ui -> testtemperature -> setAlignment(Qt::AlignRight);
ui -> testtemperature -> setText(QString::number(parameters.tempdegc));

ui ->adotc0 -> setAlignment(Qt::AlignRight);
ui ->adotc0 -> setText(QString::number(parameters.adotc0));
ui -> adotc0->setReadOnly(true);
ui -> adotc0->setPalette(*gray);

ui -> initialpressure -> setAlignment(Qt::AlignRight);
ui -> initialpressure -> setText(QString::number(parameters.p0bar));


ui -> backfilldepth -> setAlignment(Qt::AlignRight);
ui -> backfilldepth -> setText(QString::number(parameters.backfill_depth));

ui -> backfilldensity -> setAlignment(Qt::AlignRight);
ui -> backfilldensity -> setText(QString::number(parameters.backfill_density));

ui -> solidfraction -> setAlignment(Qt::AlignRight);
ui -> solidfraction -> setText(QString::number(parameters.solid_inside_pipe));

ui -> waterfraction -> setAlignment(Qt::AlignRight);
ui -> waterfraction -> setText(QString::number(parameters.water_inside_pipe));

ui -> initiallength -> setAlignment(Qt::AlignRight);
ui -> initiallength -> setText(QString::number(parameters.lambda));

ui -> fdnumber -> setAlignment(Qt::AlignRight);
ui -> fdnumber -> setText(QString::number(parameters.elements_in_l));

}

//Begins the calculation process, calls various functions which eventually provide
//a solution and the accompanying outputs
void guimain::on_Runbutton_clicked()
{

    extern File file;
    Parameters edited;

    //Update parameters from GUI
    edited = update();

    //Check if folders for results exist
    //All folders present returns 0
    exists = file.check();

    if(!exists)
    {
        Simulation simulation;
        Solution solution;

        //Run simulation
        solution = simulation.run(edited);

        //Fill comboboxes
        ui ->yCombo ->clear();
        ui->varCombo ->clear();
        ui -> yCombo -> insertItems(0, QStringList() << "" << "Decompression factor" << "Outflow length" << "Support factor" << "Speed factor" << "Non-dimensional crack driving force" << "Crack driving force");

        ui ->var ->setText(ui->parameter->currentText());

        switch (ui -> parameter -> currentIndex())
        {
            case 0:
            {
                ui -> varUnit->setText("");
                for (i= sizeof(solution.adotc0)+1; i > 0; i--)
                {
                    ui -> varCombo ->insertItems(0,QStringList() << QString::number(solution.adotc0[i]));
                }
                break;
            }
            case 1:
            {
                ui -> varUnit->setText("bar");
                for (i= sizeof(solution.p0bar)+1; i > 0; i--)
                {
                    ui -> varCombo ->insertItems(0,QStringList() << QString::number(solution.p0bar[i]));
                }
                break;
            }
            case 2:
            {
                 ui -> varUnit->setText("degrees Celsius");
                for (i= sizeof(solution.tempdegc)+1; i > 0; i--)
                {
                    ui -> varCombo ->insertItems(0,QStringList() << QString::number(solution.tempdegc[i]));
                }
                break;
            }
        }

        //Plots solution
        plotHandler(solution);

    }
    else
    {
        //Create dialog and warn users that folders can't be found
        dialog *e = new dialog;
        switch(exists)
        {
            case 1:
                e->warning("Results folder couldn't be found, new folder created");
                break;
            case 2:
                e->warning("Profile folder couldn't be found, new folder created");
                break;
            case 3:
                e->warning("Log folder couldn't be found, new folder created");
                break;
            case 4:
                e->warning("Please enter a valid directory");
                break;
        }
        e->show();
    }
}

//Creates, displays and saves the crack and results plot
void guimain::plotHandler(Solution solution)
{
    //Plots graphs against different independent variable
    switch(ui->parameter ->currentIndex())
    {
        case 0:
        {
            plotResults(solution.adotc0, solution.decompression, "Decompression factor vs non-dimensional speed", "Non-dimensional speed", "Decompression factor",0);
            plotResults(solution.adotc0, solution.lambda, "Outflow length vs non-dimensional speed", "Non-dimensional speed", "Outflow length",0);
            plotResults(solution.adotc0, solution.m, "Support factor vs non-dimensional speed", "Non-dimensional speed", "Support factor",0);
            plotResults(solution.adotc0, solution.alpha, "Speed factor vs non-dimensional speed", "Non-dimensional speed", "Speed factor",0);
            plotResults(solution.adotc0, solution.gg0, "Non-dimensional crack driving force vs non-dimensional speed", "Non-dimensional speed", "Non-dimensional crack driving force",0);
            plotResults(solution.adotc0, solution.gtotal, "Crack driving force vs non-dimensional speed", "Non-dimensional speed", "Crack driving force",0);
            break;
        }
        case 1:
        {
            plotResults(solution.p0bar, solution.decompression, "Decompression factor vs initial pressure", "Initial pressure", "Decompression factor",0);
            plotResults(solution.p0bar, solution.lambda, "Outflow length vs initial pressure", "Initial pressure", "Outflow length",0);
            plotResults(solution.p0bar, solution.m, "Support factor vs initial pressure", "Initial pressure", "Support factor",0);
            plotResults(solution.p0bar, solution.alpha, "Speed factor vs initial pressure", "Initial pressure", "Speed factor",0);
            plotResults(solution.p0bar, solution.gg0, "Non-dimensional crack driving force vs initial pressure", "Initial pressure", "Non-dimensional crack driving force",0);
            plotResults(solution.p0bar, solution.gtotal, "Crack driving force vs initial pressure", "Initial pressure", "Crack driving force",0);
            break;
        }
        case 2:
        {
            plotResults(solution.tempdegc, solution.decompression, "Decompression factor vs temperature", "Temperature", "Decompression factor",0);
            plotResults(solution.tempdegc, solution.lambda, "Outflow length vs temperature", "Temperature", "Outflow length",0);
            plotResults(solution.tempdegc, solution.m, "Support factor vs temperature", "Temperature", "Support factor",0);
            plotResults(solution.tempdegc, solution.alpha, "Speed factor vs temperature", "Temperature", "Speed factor",0);
            plotResults(solution.tempdegc, solution.gg0, "Non-dimensional crack driving force vs temperature", "Temperature", "Non-dimensional crack driving force",0);
            plotResults(solution.tempdegc, solution.gtotal, "Crack driving force vs temperature", "Temperature", "Crack driving force",0);
            break;
        }
    }

    //Plots crack profile in colours according to whether the method converged or not
    if(solution.no_crack_opening[solution.k])
    {
        plotProfiles(solution.z, solution.w[solution.k], "Crack displacement profile", "Distance behind crack tip", "Crack opening displacement",0,1);
    }
    else
    {
        plotProfiles(solution.z, solution.w[solution.k], "Crack displacement profile", "Distance behind crack tip", "Crack opening displacement",0,0);
    }
}


//Plots the crack profiles using the qtcustomplot class
void guimain::plotProfiles(vector<double> x, vector<double> y, string title, string xtitle, string ytitle, char savestate, char valid)
{

    extern File file;

    path = file.directory + "Profiles/" + title;
    ui -> Crackplot -> addGraph();

    //Sets colour depending on method convergence
    if(valid==1)
    {
        ui -> Crackplot -> graph(0) ->setPen(QPen(Qt::green));
    }
    else
    {
        ui -> Crackplot -> graph(0) ->setPen(QPen(Qt::red));
    }

    ui -> Crackplot -> graph(0)->setLineStyle(QCPGraph::lsNone);
    ui -> Crackplot -> graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 4));

    //Converts std vectors to QVectors for use by QCPPainter
    QVector<double> Qx = QVector<double>::fromStdVector(x);

    QVector<double> Qy = QVector<double>::fromStdVector(y);

    ui -> Crackplot -> graph(0) -> setData(Qx,Qy);

    ui -> Crackplot -> xAxis->setLabel(QString::fromStdString(xtitle));
    ui -> Crackplot -> yAxis->setLabel(QString::fromStdString(ytitle));

    //Finds location of maximum value
    double z1 = *max_element(x.begin(), x.end());
    double z2 = *max_element(y.begin(), y.end());

    //Expands range to make plots useable
    ui -> Crackplot -> xAxis->setRange(0, z1+0.002);
    ui -> Crackplot -> yAxis->setRange(0, z2+0.002);
    ui -> Crackplot -> replot();

    //Saves plot
    if(!savestate)
    {
        ui -> Crackplot ->savePdf(QString::fromStdString(path)+".pdf",false,1000,1000,"Test","Test");
    }

}

//Plots the results graphs using the qtcustomplot class
void guimain::plotResults(vector<double> x, vector<double> y, string title, string xtitle, string ytitle, char savestate)
{
    extern File file;

    path = file.directory + "Results/" + title;
    ui -> Resultsplot -> addGraph();
    ui -> Resultsplot -> graph(0)->setLineStyle(QCPGraph::lsNone);
    ui -> Resultsplot -> graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));

    //Converts std vectors to QVectors for use by QCPPainter
    QVector<double> Qx = QVector<double>::fromStdVector(x);
    QVector<double> Qy = QVector<double>::fromStdVector(y);

    ui -> Resultsplot -> graph(0) -> setData(Qx,Qy);

    ui -> Resultsplot -> xAxis->setLabel(QString::fromStdString(xtitle));
    ui -> Resultsplot -> yAxis->setLabel(QString::fromStdString(ytitle));

    //Finds location of maximum value
    double z1 = *max_element(x.begin(), x.end());
    double z2 = *max_element(y.begin(), y.end());

    //Expands range to make plots useable
    ui -> Resultsplot -> xAxis->setRange(0, z1+0.2);
    ui -> Resultsplot -> yAxis->setRange(0, z2+0.2);
    ui -> Resultsplot -> replot();

    if(!savestate)
    {
        ui -> Resultsplot ->savePdf(QString::fromStdString(path) + ".pdf",false, 1000, 1000,"Test","Test");
    }
}

//Updates the values stored in main with the values in the GUI
Parameters guimain::update()
{

    Parameters temp;

    extern File file;
    temp.varname = ui ->parameter ->currentIndex();

    temp.from = ui -> from -> text().toDouble();
    temp.to = ui -> to -> text().toDouble();
    temp.range_number = ui -> noofpoints -> text().toDouble();

    temp.fullscale = ui -> fs -> checkState();
    temp.is_backfilled = ui -> backfill -> checkState();
    temp.outflow_model_on = ui -> fixedlength -> checkState();

    temp.matid = ui -> materialname ->currentText().toStdString();
    temp.density = ui -> density -> text().toDouble();    
    temp.edyn0degc = ui -> dynamicmodulus -> text().toDouble();
    temp.dedyndt = ui -> deltadynamicmodulus -> text().toDouble();
    temp.creep_modulus = ui -> creepmodulus -> text().toDouble();
    temp.poisson = ui -> dynpoissonratio -> text().toDouble();
    temp.edyn0degc = ui -> dynamicmodulus -> text().toDouble();
    temp.p0bar = ui ->initialpressure -> text().toDouble();

    temp.pipeid = ui -> pipename -> currentText().toStdString();
    temp.diameter = ui -> outsidediameter -> text().toDouble();
    temp.sdr = ui -> sdr -> text().toDouble();    
    temp.notch_depth = ui -> groovedepth -> text().toDouble();
    temp.diameter_creep_ratio = ui -> relativediameter -> text().toDouble();
    temp.tempdegc = ui -> testtemperature -> text().toDouble();
    temp.backfill_depth = ui -> backfilldepth -> text().toDouble();
    temp.backfill_density = ui -> backfilldensity -> text().toDouble();
    temp.solid_inside_pipe = ui -> solidfraction -> text().toDouble();
    temp.water_inside_pipe = ui -> waterfraction -> text().toDouble();

    temp.lambda = ui -> initiallength -> text().toDouble();    
    temp.elements_in_l = ui -> fdnumber -> text().toDouble();
    temp.verbose = ui ->verbose ->checkState();

    return temp;
}

//The following function implement the logic required for the checkboxes in the GUI

void guimain::on_fs_clicked()
{
    if(ui -> fs -> checkState())
    {
    ui -> s4 -> setCheckState(Qt::Unchecked);

    }
    else
    {
    ui -> s4 -> setCheckState(Qt::Checked);
    }
}

void guimain::on_s4_clicked()
{
    if(ui-> s4 -> checkState())
    {
    ui -> fs -> setCheckState(Qt::Unchecked);

    }
    else
    {
    ui -> fs -> setCheckState(Qt::Checked);
    }
}

//Saves the results shown in the table to a file as well as the crack profiles
void guimain::on_Save_clicked()
{
    extern File file;
    file.writeResults();
}

//The functions below change the associated values when the pipe or variable is changed
void guimain::on_materialname_currentIndexChanged(int index)
{

    ui -> density -> setText(QString::number(Parameters::density_lib[index]));
    ui -> dynamicmodulus -> setText(QString::number(Parameters::edyn0degc_lib[index]));
    ui -> deltadynamicmodulus -> setText(QString::number(Parameters::dedyndt_lib[index]));
    ui -> creepmodulus -> setText(QString::number(Parameters::creepmodulus_lib[index]));
    ui -> dynpoissonratio -> setText(QString::number(Parameters::poisson_lib[index]));

}

void guimain::on_pipename_currentIndexChanged(int index)
{

    ui -> outsidediameter -> setText(QString::number(Parameters::diameter_lib[index]));
    ui -> sdr -> setText(QString::number(Parameters::sdr_lib[index]));

}

//Loads "caseInputData.txt" storing these values in parameter
void guimain::on_action_Load_triggered()
{

    extern File file;

    //Uses output of loadCheck to switch cases, either loading the file or warning
    //the user it won't work
    switch(file.loadCheck("caseInputData.txt"))
    {
        case 0:
        {
            ConfigFile config(file.directory + "caseInputData.txt");
            Parameters temp(config);

            dialog *e = new dialog;
            e->warning("caseInputData.txt was loaded successfully");
            e->show();

            setnames(temp,1);
            break;
        }
        case 1:
        {
            dialog *e = new dialog;
            e->warning("caseInputData.txt could not be found in: \n \n" + file.directory);
            e->show();
            break;
        }

    }

}

//Saves "caseInputData.txt" using the values in parameter
void guimain::on_action_Save_triggered()
{
    extern File file;

    Parameters temp;
    temp = update();
    dialog *e = new dialog;
    file_name = "caseInputData.txt";

    //Uses output of hander to switch cases, either saving the file or warning
    //the user it won't work
    if(!file.caseHandler(temp, file_name))
    {
        e->warning("caseInputData.txt saved successfully");

    }
    else
    {
        e->warning(file_name + " not saved successfully, enter a viable name");
    }
    e->show();
}

//Displays about dialog
void guimain::on_actionAbout_triggered()
{
    about *d = new about;
    d->show();
    d->setWindowTitle("About");
}

//Changes the range values depending on the independent variable selected
void guimain::on_parameter_currentIndexChanged(int index)
{
    ui -> from -> setText(QString::number(Parameters::from_lib[index]));
    ui -> to -> setText(QString::number(Parameters::to_lib[index]));
}

//Selects normalised crack velocity and displays graph
void guimain::on_varCombo_activated(int index)
{
    extern Solution solution;

    //Plots crack profiles with colours depending on method convergence
    if(index != 0)
    {
        plotProfiles(solution.z, solution.w[index+1], "Crack displacement profile", "Distance behind crack tip", "Crack opening displacement",0,solution.no_crack_opening[index+1]);
    }
    else
    {

    }
}

//Selects the x Axis variable and plots graph assuming y Axis variable has been selected
void guimain::on_yCombo_activated(int index)
{
    extern Solution solution;

    //Plots crack profiles with colours depending on method convergence
    if(index != 0)
    {
        //Plots graphs against different independent variable
        switch(ui -> parameter ->currentIndex())
        {
            case 0:
            {
                switch (index)
                {
                    case 1:{
                        plotResults(solution.adotc0, solution.decompression, "Decompression factor vs non-dimensional speed", "Non-dimensional speed", "Decompression factor",0);
                        break;
                          }
                    case 2:{
                        plotResults(solution.adotc0, solution.lambda, "Outflow length vs non-dimensional speed", "Non-dimensional speed", "Outflow length",0);
                        break;
                          }
                    case 3:{
                        plotResults(solution.adotc0, solution.m, "Support factor vs non-dimensional speed", "Non-dimensional speed", "Support factor",0);
                        break;
                          }
                    case 4:{
                        plotResults(solution.adotc0, solution.alpha, "Speed factor vs non-dimensional speed", "Non-dimensional speed", "Speed factor",0);
                        break;
                          }
                    case 5:{
                        plotResults(solution.adotc0, solution.gg0, "Non-dimensional crack driving force vs non-dimensional speed", "Non-dimensional speed", "Non-dimensional crack driving force",0);
                        break;
                          }
                    case 6:{
                        plotResults(solution.adotc0, solution.gtotal, "Crack driving force vs non-dimensional speed", "Non-dimensional speed", "Crack driving force",0);
                        break;
                          }
                }
                break;
            }
            case 1:
            {
                switch (index)
                {
                    case 1:{
                        plotResults(solution.p0bar, solution.decompression, "Decompression factor vs initial pressure", "Initial pressure", "Decompression factor",0);
                        break;
                          }
                    case 2:{
                        plotResults(solution.p0bar, solution.lambda, "Outflow length vs initial pressure", "Initial pressure", "Outflow length",0);
                        break;
                          }
                    case 3:{
                        plotResults(solution.p0bar, solution.m, "Support factor vs initial pressure", "Initial pressure", "Support factor",0);
                        break;
                          }
                    case 4:{
                        plotResults(solution.p0bar, solution.alpha, "Speed factor vs initial pressure", "Initial pressure", "Speed factor",0);
                        break;
                          }
                    case 5:{
                        plotResults(solution.p0bar, solution.gg0, "Non-dimensional crack driving force vs initial pressure", "Initial pressure", "Non-dimensional crack driving force",0);
                        break;
                          }
                    case 6:{
                        plotResults(solution.p0bar, solution.gtotal, "Crack driving force vs initial pressure", "Initial pressure", "Crack driving force",0);
                        break;
                          }
                }
                break;
            }
            case 2:
            {
                switch (index)
                {
                    case 1:{
                        plotResults(solution.tempdegc, solution.decompression, "Decompression factor vs temperature", "Temperature", "Decompression factor",0);
                        break;
                          }
                    case 2:{
                        plotResults(solution.tempdegc, solution.lambda, "Outflow length vs temperature", "Temperature", "Outflow length",0);
                        break;
                          }
                    case 3:{
                        plotResults(solution.tempdegc, solution.m, "Support factor vs temperature", "Temperature", "Support factor",0);
                        break;
                          }
                    case 4:{
                        plotResults(solution.tempdegc, solution.alpha, "Speed factor vs temperature", "Temperature", "Speed factor",0);
                        break;
                          }
                    case 5:{
                        plotResults(solution.tempdegc, solution.gg0, "Non-dimensional crack driving force vs temperature", "Temperature", "Non-dimensional crack driving force",0);
                        break;
                          }
                    case 6:{
                        plotResults(solution.tempdegc, solution.gtotal, "Crack driving force vs temperature", "Temperature", "Crack driving force",0);
                        break;
                          }
                }
                break;
            }
        }
    }
    else
    {

    }
}

void guimain::on_parameter_activated(int index)
{
    QPalette *gray = new QPalette();
    gray->setColor(QPalette::Base,Qt::gray);
    gray->setColor(QPalette::Text,Qt::darkGray);

    QPalette *white = new QPalette();
    white->setColor(QPalette::Base,Qt::white);
    white->setColor(QPalette::Text,Qt::black);

    switch (index)
    {
        case 0:{
            ui->adotc0->setReadOnly(true);
            ui -> adotc0->setReadOnly(true);
            ui -> adotc0->setPalette(*gray);
            ui ->testtemperature->setReadOnly(false);
            ui -> testtemperature->setPalette(*white);
            ui ->initialpressure->setReadOnly(false);
            ui -> initialpressure->setPalette(*white);
            ui->varTo ->setText("");
            ui->fromVar->setText("");
            break;
              }
        case 1:{
            ui->initialpressure->setReadOnly(true);
            ui -> initialpressure->setReadOnly(true);
            ui -> initialpressure->setPalette(*gray);
            ui ->adotc0->setReadOnly(false);
            ui ->adotc0->setPalette(*white);
            ui ->testtemperature->setReadOnly(false);
            ui -> testtemperature->setPalette(*white);
            ui->varTo ->setText("bar");
            ui->fromVar->setText("bar");
            break;
              }
        case 2:{
            ui ->testtemperature->setReadOnly(true);
            ui -> testtemperature->setReadOnly(true);
            ui -> testtemperature->setPalette(*gray);
            ui ->adotc0->setReadOnly(false);
            ui ->adotc0->setPalette(*white);
            ui ->initialpressure->setReadOnly(false);
            ui -> initialpressure->setPalette(*white);
            ui->varTo ->setText("degrees Celsius");
            ui->fromVar->setText("degrees Celsuis");
            break;
              }
    }
}

void guimain::on_fixedlength_clicked(bool checked)
{
    QPalette *gray = new QPalette();
    gray->setColor(QPalette::Base,Qt::gray);
    gray->setColor(QPalette::Text,Qt::darkGray);

    QPalette *white = new QPalette();
    white->setColor(QPalette::Base,Qt::white);
    white->setColor(QPalette::Text,Qt::black);

    if (checked == 1){
        ui->initiallength -> setReadOnly(true);
        ui -> initiallength -> setPalette(*gray);
    }
    else
    {
        ui->initiallength -> setReadOnly(false);
        ui -> initiallength -> setPalette(*white);
    }
}
