//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

//  Class taking care of all file writing duties (Results, cracks and caseInputData.txt)
//  Also contains the log used to record values during run-time

#include <iostream>
#include <sys/stat.h>
using namespace std;

#include "File.h"

//  Null constructor
File::File()
{
   initialise();
}

//  Clears all variables within the file class
void File::initialise()
{
    adotc0 = 0.0;
    g0 = 0.0;
    diameter_res0 = 0.0;

//    dstarminus1 = 0.0;
//    dstarminus2 = 0.0;
    baffle_leakage_area = 0.0;
//    decompression = 0.0;

    zetaclosure = 0.0;

    max_iterations = 0;
    lambda_last = 0.0;
    closure_is_converged = 0;
    iterations = 0;
    no_crack_opening = 0;
    w_2 = 0.0;
    w_max = 0.0;
    wdash_2 = 0.0;
    w2dash_2 = 0.0;
    w_integral_12 = 0.0;
    zeta_backfill_ejection = 0.0;
    p1p0r = 0.0;
    dynamic_shear_modulus = 0.0;
    v0 = 0.0;
    lambda_factor = 0.0;
    p1bar = 0.0;
    vStarRes = 0.0;
    node_at_closure = 0;
    adotovercl = 0.0;
    adotclfactor = 0.0;
    adotclfactor_backfilled = 0.0;
//    lambdapow4 = 0;
//    sdrminus1 = 0.0;
//    sdrminus2 = 0.0;
    zeta_backfill_ejection = 0.0;
    lambda = 0.0;
    v00 = 0.0;
    error = 0.0;
    pipe_volume_availability = 0.0;
    wdash_max = 0.0;

    m[0] = 0.0;
    m[1] = 0.0;
    alpha[0] = 0.0;
    alpha[1] = 0.0;
//  OutflowProcess
    p_now = 0.0;
    pUnchoke = 0.0;
    pHalfp1 = 0.0;
    tStar = 0.0;
    unchoked = 0;
    xUnchoked_i = 0.0;
    tStarUnchoke = 0.0;
    integral_pressure_dt = 0.0;
    tStarOutflow = 0.0;
    tStarOutflow2 = 0.0;

    writeLogLine(1);

}

//Reduces directory path to where "aRCPlanQt.app" is found
void File::correct()
{

    found = directory.find("aRCPlanQt.app");

    if (found != string::npos)
    {
         directory.resize(found);
    }

}

//Checks if folders can be found for storing results files
int File::check()
{
    struct stat st;

    if(stat(directory.c_str(), &st) == 0)
    {

        if (st.st_mode & (S_IFDIR != 0))
        {
            subfolder = directory + "Results/";

            if(stat(subfolder.c_str(), &st) == 0)
            {
                if (st.st_mode & (S_IFDIR != 0))
                {
                    check_state = 0;
                }
            }
            else
            {
                mkdir(subfolder.c_str(), 0777);
                return check_state = 1;
            }

            subfolder = directory + "Profiles/";

            if(stat(subfolder.c_str(), &st) == 0)
            {
                if (st.st_mode & (S_IFDIR != 0))
                {
                    check_state = 0;
                }
            }
            else
            {
                mkdir(subfolder.c_str(), 0777);
                return check_state = 2;
            }

            subfolder = directory + "Log/";

            if(stat(subfolder.c_str(), &st) == 0)
            {
                if (st.st_mode & (S_IFDIR != 0))
                {
                    check_state = 0;
                }
            }
            else
            {
                mkdir(subfolder.c_str(), 0777);
                return check_state = 3;
            }
        }
    }
    else
    {
        return check_state = 4;
    }

    return check_state;

}

//Checks if file exists
int File::loadCheck(string name)
{

    struct stat st;

    if(stat((directory + name).c_str(), &st) == 0)
    {
        return 0;
    }

    return 1;
}

//Provides handler for writing caseInputData.txt file
int File::caseHandler(Parameters temp, string filename)
{
    if(filename.find(".txt",0)<100)
    {
        writeParTXT(temp, filename);
        return 0;
    }
    else
    {
        return 1;
    }
}

//Writes results file by accessing global object solution
void File::writeResults()
{
    extern File file;
    extern Solution solution;

    file_name = "Results.csv";

    out.open((file.directory + "Results/" + file_name).c_str(),
             std::fstream::in | std::fstream::out | std::fstream::trunc);

    out << "Normalised Crack Speed,"
           "Initial pressure, Temperature, Decomp. factor,Speed factor,Support factor,Outflow length,Flaring,Irwin Corten force,Crack driving force,Normalised total,"
            << "CrackOpening, Converged, Iterations \n";

    for(i=1; i<solution.soln+1;i++)
    {
        //Write solution line for a given independent variable value
        out << solution.adotc0[i] << ","
                << solution.p0bar[i] << ","
                << solution.tempdegc[i] << ","
                << solution.decompression[i] << ","
                << solution.alpha[i] << ","
                << solution.m[i] << ","
                << solution.lambda[i] << ", ,"
                << solution.g0[i] << ","
                << solution.gg0[i] << ","
                << solution.gtotal[i] << ","
                << solution.no_crack_opening[i] << ","
                << solution.lambda_is_converged[i] << ","
                << solution.iterations[i] << "\n";

    }

    out.close();

    file_name = "Crack Profiles.csv";

    out.open((file.directory + "Results/" +file_name).c_str(),
             std::fstream::in | std::fstream::out | std::fstream::trunc);

    out << "zeta" << ",";

    for(i = 1; i < solution.n; i++)
    {
        out << solution.z[i] << ",";
    }

    out << "\n";

    for(j = 1; j < solution.soln; j++)
    {
        for(i = 0; i < solution.n; i++)
        {
             out << solution.w[j][i] << ",";

        }
        out << "\n";
    }

    out.close();

}

//Writes parameters to txt file with appropriate formatting
void File::writeParTXT(Parameters temp, string file_name)
{

    out.open((directory + file_name).c_str());

    writeLineTXT("Input Data", out,1);

    writeLineTXT("Material Data", out, 1);

    writeLineTXT("matID = ", temp.matid, out, 0);
    writeLineTXT("density = ", temp.density, out, 0);
    writeLineTXT("eDyn0degC = ", temp.edyn0degc, out, 0);
    writeLineTXT("dEdyndT = ", temp.dedyndt, out, 0);
    writeLineTXT("creepModulus = ", temp.creep_modulus, out, 0);
    writeLineTXT("poisson = ", temp.poisson, out, 1);

    writeLineTXT("Pipe Data", out,1);

    writeLineTXT("pipeID = ", temp.pipeid, out, 0);
    writeLineTXT("diameter = ", temp.diameter, out, 0);
    writeLineTXT("sdr = ", temp.sdr, out, 0);
    writeLineTXT("notchDepth = ", temp.notch_depth, out, 0);
    writeLineTXT("diameterCreepRatio = ", temp.diameter_creep_ratio, out, 1);

    writeLineTXT("Test Setup Data", out, 1);

    writeLineTXT("fullScale = ", temp.fullscale, out, 0);
    writeLineTXT("tempDegC = ", temp.tempdegc, out, 0);
    writeLineTXT("p0bar = ", temp.p0bar, out, 0);
    writeLineTXT("isBackfilled = ", temp.is_backfilled, out, 0);
    writeLineTXT("backfillDepth = ", temp.backfill_depth, out, 0);
    writeLineTXT("backfillDensity = ", temp.backfill_density, out, 0);
    writeLineTXT("solidInsidePipe = ", temp.solid_inside_pipe, out, 0);
    writeLineTXT("waterInsidePipe = ", temp.water_inside_pipe, out, 1);

    writeLineTXT("Program Control Data", out, 1);

    writeLineTXT("outflowModelOn = ", temp.outflow_model_on, out, 0);
    writeLineTXT("lambda = ", temp.lambda, out, 0);
    writeLineTXT("numberOfSpeedValues = ", temp.range_number, out, 0);
    writeLineTXT("elementsInL = ", temp.elements_in_l, out, 0);
    writeLineTXT("aDotc0 = ", temp.adotc0, out, 0);

    out.close();

}

//Writes parameters to csv file with appropriate formatting
void File::writeParCSV(Parameters temp, string filename)
{

    out.open((directory + filename).c_str());

    writeLineCSV("Input Data", out);
    writeLineCSV("\n", out);

    writeLineCSV("Material Data", out);
    writeLineCSV("\n", out);

    writeLineCSV("matID", temp.matid, out);
    writeLineCSV("density", temp.density, out);
    writeLineCSV("eDyn0degC", temp.edyn0degc, out);
    writeLineCSV("dEdyndT", temp.dedyndt, out);
    writeLineCSV("creepModulus", temp.creep_modulus, out);
    writeLineCSV("poisson", temp.poisson, out);
    writeLineCSV("\n", out);

    writeLineCSV("Pipe Data", out);
    writeLineCSV("\n", out);

    writeLineCSV("pipeID", temp.pipeid, out);
    writeLineCSV("diameter", temp.diameter, out);
    writeLineCSV("sdr", temp.sdr, out);
    writeLineCSV("notchDepth", temp.notch_depth, out);
    writeLineCSV("diameterCreepRatio", temp.diameter_creep_ratio, out);

    writeLineCSV("\n", out);
    writeLineCSV("Test Setup Data", out);
    writeLineCSV("\n", out);

    writeLineCSV("fullScale", temp.fullscale, out);
    writeLineCSV("tempDegC", temp.tempdegc, out);
    writeLineCSV("p0bar", temp.p0bar, out);
    writeLineCSV("isBackfilled", temp.is_backfilled, out);
    writeLineCSV("backfillDepth", temp.backfill_depth, out);
    writeLineCSV("backfillDensity", temp.backfill_density, out);
    writeLineCSV("solidInsidePipe", temp.solid_inside_pipe, out);
    writeLineCSV("waterInsidePipe", temp.water_inside_pipe, out);

    writeLineCSV("\n", out);
    writeLineCSV("Program Control Data", out);
    writeLineCSV("\n", out);

    writeLineCSV("outflowModelOn", temp.outflow_model_on, out);
    writeLineCSV("lambda", temp.lambda, out);
    writeLineCSV("numberOfSpeedValues", temp.range_number, out);
    writeLineCSV("elementsInL", temp.elements_in_l, out);
    writeLineCSV("aDotc0", temp.adotc0, out);

    out.close();
}

//  The following functions write a single line to a txt file
//  taking various arguments for different requirements

void File::writeLineTXT(string title, double value, ofstream &out, int format)
{
    out << "\t" << title << value << endl;
    if(format == 1)
    {
        out << endl;
    }
}

void File::writeLineTXT(string title, string value, ofstream &out, int format)
{
    out << "\t" << title << value << endl;
    if(format == 1)
    {
        out << endl;
    }
}

void File::writeLineTXT(string title, ofstream &out, int format)
{
    out << title << endl;
    if(format == 1)
    {
        out << endl;
    }
}

//  The following functions write a single line to a csv file
//  taking various arguments for different requirements

void File::writeLineCSV(string title, double value, ofstream &out)
{
    out << title << "," << value << "\n";
}

void File::writeLineCSV(string title, string value, ofstream &out)
{
     out << title << "," << value << "\n";
}

void File::writeLineCSV(string title, ofstream &out)
{
    out << title << "," << "\n";
}

//  Writes the headers for a txt file
void File::writeHeaders(string temp)
{
    file_name = temp;
    out.open((directory + file_name).c_str(),
             std::fstream::in | std::fstream::out | std::fstream::app);

    writeLineCSV("\n", out);

    //Reset log number
    log_number = 0;

    //Set headers
    out << "Log number" << ","
        << "aDotc0" << ","
        << "Irwin Corten Force" << ","
        << "diameterRes0" << ","
        << "residualCrackClosure" << ","
        << "densityratio" << ","
//  OutflowProcess
        << "p_now" << ","
        << "pUnchoke" << ","
        << "pHalfp1" << ","
        << "tStar" << ","
        << "unchoked" << ","
        << "xUnchoked_i" << ","
        << "tStarUnchoke" << ","
        << "integral_pressure_dt" << ","
        << "tStarOutflow" << ","
        << "tStarOutflow2" << ","
//  BeamModel
        << "zetaclosure" << ","
        << "nodeAtClosure" << ","
        << "lambda" << ","
        << "p1bar" << ","
        << "v0" << ","
        << "vStarRes" << ","
        << "lambda_factor" << ","
        << "aDotCLfactor" << ","
        << "aDotcClfactor_backfilled" << ","
        << "maxIterations" << ","
        << "iterations" << ","
        << "m[0]" << "," << "m[1]" << ","
        << "alpha[0]" << "," << "alpha[1]" << ","
        << "error" << ","
        << "notConverged" << "\n";

    writeLineCSV("\n", out);
    out.close();
}


//Handler for the log file
void File::logPrepare(Parameters temp)
{
    initialise();
    file_name = "Log/Log.csv";
    writeParCSV(temp, file_name);
    writeHeaders(file_name);
}

//Writes single line using the current log values
void File::writeLogLine(int newline)
{
    out.open((directory + file_name).c_str(),
             std::fstream::in | std::fstream::out | std::fstream::app);

    //Reset count and create new line
    if(newline)
    {
    out << " \n";
    log_number = 0;
    }

    log_number++;
    out << log_number << ","
        << adotc0 << ","
        << g0 << ","
        << diameter_res0 << ","
        << residual_crack_closure << ","
//  OutflowProcess
        << p_now << ","
        << pUnchoke << ","
        << pHalfp1 << ","
        << tStar << ","
        << unchoked << ","
        << xUnchoked_i << ","
        << tStarUnchoke << ","
        << integral_pressure_dt << ","
        << tStarOutflow << ","
        << tStarOutflow2 << ","
//  BeamModel
        << zetaclosure << ","
        << node_at_closure << ","
        << lambda << ","
        << p1bar << ","
        << v0 << ","
        << vStarRes << ","
        << lambda_factor << ","
        << adotclfactor << ","
        << adotclfactor_backfilled << ","
        << max_iterations << ","
        << iterations << ","
        << m[0] << "," << m[1] << ","
        << alpha[0] << "," << alpha[1] << ","
        << error << ","
        << closure_is_converged << "\n";

    out.close();
}

//The following functions collect data from the argument object
//before writing a line using the function above

void File::collect(Creep creep)
{
    diameter_res0 = creep.diameter_res0;
    residual_crack_closure = creep.residual_crack_closure;
    writeLogLine(0);
}

void File::collect(Backfill backfill)
{
    effective_density =  backfill.effective_density;
    writeLogLine(0);
}

void File::collect(WaterContent watercontent)
{
    effective_density =  watercontent.effective_density;
    writeLogLine(0);
}

//Following functions take pointers to original objects and are built as such


void File::collect(OutflowProcess *outflow, int newline)
{
    p_now = outflow -> p_now;
    pUnchoke = outflow -> pUnchoke;
    pHalfp1 = outflow -> pHalfp1;
    tStar = outflow -> tStar;
    unchoked = outflow -> unchoked;
    xUnchoked_i = outflow -> xUnchoked_i;
    tStarUnchoke = outflow -> tStarUnchoke;
    integral_pressure_dt = outflow -> integral_pressure_dt;
    tStarOutflow =  outflow -> tStarOutflow;
    tStarOutflow2 =  outflow -> tStarOutflow2;

    writeLogLine(newline);
}   //  end collect(OutflowProcess *outflow, int newline)

void File::collect(BeamModel *beamModel, int newline)
{
    zetaclosure = beamModel -> zetaclosure;
    node_at_closure = beamModel -> node_at_closure;
    lambda = beamModel -> lambda;
    p1bar = beamModel -> p1bar;
    v0 = beamModel -> v0;
    vStarRes = beamModel -> vStarRes;
    adotclfactor = beamModel -> adotclfactor;
    adotclfactor_backfilled = beamModel -> adotclfactor_backfilled;
    max_iterations = beamModel -> max_iterations;
    iterations = beamModel -> iterations;
    m[0] = beamModel -> m[0];
    m[1] = beamModel -> m[1];
    alpha[0] = beamModel -> alpha[0];
    alpha[1] = beamModel -> alpha[1];
    error = beamModel -> error;
    closure_is_converged = beamModel -> closure_is_converged;
    lambda_factor = beamModel ->lambda_factor;

    writeLogLine(newline);
}   //  end collect(BeamModel *beamModel, int newline)

