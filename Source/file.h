//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

#ifndef _FILE_H
#define _FILE_H

#ifdef _WIN32
	#include <direct.h>
	#define GetCurrentDir _getcwd
#else
	#include <unistd.h>
	#define GetCurrentDir getcwd
#endif

#include <string>
#include <fstream>
using namespace std;

#include "Parameters.h"
#include "Solution.h"
#include "Creep.h"
#include "Backfill.h"
#include "BeamModel.h"
#include "OutflowProcess.h"

class File : public Creep,
        public Backfill,
        public BeamModel,
        public OutflowProcess
{
private:

    ofstream out;

    int i;
    int j;
    int check_state;
    string temp_dir;
    string subfolder;
    string file_name;
    unsigned long found;
    int log_number;

public:

    string directory;
    double adotc0;

    //  Null constructor
    File();

    //  Clears all values in file
    void initialise();

    //  Resizes directory, removing aRCPlan.app
    //  to find overall directory of program
    void correct();

    //  Check for existence of folder required for storing files
    int check();

    //  Checks if file exists
    int loadCheck(string name);

    //  Handles parameters, sending them to write function if file is found
    int caseHandler(Parameters temp, string filename);

    //  Writes parameters to csv file with appropriate formatting
    void writeParCSV(Parameters temp, string filename);

    //  Write parameters to txt file with appropriate formatting
    void writeParTXT(Parameters temp, string filename);

    //  Reads from global solution object and writes to a csv file
    void writeResults();

    //  Writes a single line to a csv file
    void writeLineCSV(string title, double value, ofstream &out);
    void writeLineCSV(string title, string value, ofstream &out);
    void writeLineCSV(string title, ofstream &out);

    //  Writes a single line to a txt file
    void writeLineTXT(string title, double value, ofstream &out, int format);
    void writeLineTXT(string title, string value, ofstream &out, int format);
    void writeLineTXT(string title, ofstream &out, int format);

    //  Handles log file, writing parameters and headers for remainder of logs
    void logPrepare(Parameters temp);

    //  Writes headers for log file
    void writeHeaders(string temp);

    //  Update log file before writing a line
    void collect(Creep creep);
    void collect(Backfill backfill);
    void collect(LiquidContent liquidcontent);
    void collect(BeamModel *beamModel, int newline);
    void collect(OutflowProcess *outflow, int newline);

    //  Writes a single line to the log file with current log values
    void writeLogLine(int newline);
};

#endif
