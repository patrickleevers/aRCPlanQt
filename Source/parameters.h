//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

#ifndef _PARAMETERS_H
#define _PARAMETERS_H

#include <iostream>
#include <fstream>
using namespace std;

#include "ConfigFile.h"

class Parameters
{

public:

    static string matid_lib[6];
    static double density_lib[6];
    static double edyn0degc_lib[6];
    static double dedyndt_lib[6];
    static double creepmodulus_lib[6];
    static double poisson_lib[6];

    static string pipeid_lib[5];
    static double diameter_lib[5];
    static double sdr_lib[5];
    static double notchdepth_lib[5];
    static double diametercreepratio_lib[5];

    static double from_lib[3];
    static double to_lib[3];

    string methodid;
    short fullscale;
    double tempdegc;
    double p0bar;
    int is_backfilled;
    double backfill_depth;
    double backfill_density;
    double solid_inside_pipe;
    double liquid_inside_pipe;

    string matid;
    double density;
    double edyn0degc;
    double dedyndt;
    double dynamic_modulus;
    double creep_modulus;
    double poisson;
    double creep_modulus_ratio;

    string pipeid;
    double diameter;
    double sdr;
    double notch_depth;
    double diameter_creep_ratio;
    double h;
    double crack_width;

    short verbose;
    int outflow_model_on;
    double lambda;
    int single_mode;
    int range_number;
    int elements_in_l;
    double adotc0;
    double from;
    double to;
    double indvar;
    int varname;

    //  Null constructor
    Parameters();

    //  Constructor
    Parameters(ConfigFile config);

    //  '=' operator
    Parameters& operator=(const Parameters& rhs);

    //  Updates the geometry and material values respectively
    void geometryUpdate(int n);
    void materialUpdate(int n);

    //  Reads in the parameter values from the config file provided
    void collect(ConfigFile config);

    //  Modifies parameters for temperature changes
    void conditionToTemperature();

} ;

#endif
