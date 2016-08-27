//     aRCPLan
//     Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//     aRCPlan may be freely distributed under the MIT license.
//     For the underlying model, see http://www.sciencedirect.com/science/article/pii/S0013794412003530

//     Class which reads values in from configFile and stores to pass val

#include <iostream>
using namespace std;

#include "Parameters.h"
#include "ConfigFile.h"

    string Parameters::pipeid_lib[5] = {"250mm_SDR11",
                                            "250mm_SDR17",
                                            "110mm_SDR11",
                                            "110mm_SDR17",
                                            "63mm_SDR11"};
    string Parameters::matid_lib[6] = {"Soft PE80",
                                            "Generic PE100",
                                            "Soft PE100",
                                            "Generic PE1",
                                            "Generic PE2",
                                            "Test"};
    double Parameters::diameter_lib[5] = {250.0, 250.0, 110.0, 110.0, 63.0};

    double Parameters::sdr_lib[5] = {11.0, 17.6, 11.0, 17.6, 11.0};

    double Parameters::density_lib[6] =
                    {938.0, 960.0, 938.0, 960.0, 950.0,0.0};
    double Parameters::edyn0degc_lib[6] =
                    {2.62, 3.17, 1.31, 1.585, 1.5,0.0};
    double Parameters::dedyndt_lib[6] =
                    {-0.037, -0.0427, -0.0185, -0.02135, -0.02,0.0};
    double Parameters::creepmodulus_lib[6] =
                    {0.3, 0.3, 0.3, 0.3, 0.3,0.0};
    double Parameters::poisson_lib[6] =
                    {0.38, 0.38, 0.38, 0.38, 0.38,0.0};
    double Parameters::from_lib[3] =
                    {0.0, 1.0, 1.0};
    double Parameters::to_lib[3] =
                    {1.0, 20.0, 50.0};

//  Null constructor
Parameters::Parameters()
{

    outflow_model_on = 0;
    lambda = 3.0;
    single_mode = 2;
    range_number = 0;
    elements_in_l = 20;
    adotc0 = 0.5;
    varname = 0;

    fullscale = 2;
    tempdegc = 0.0;
    p0bar = 5.0;
    is_backfilled = 2;
    backfill_depth = 100;
    backfill_density = 2200;
    solid_inside_pipe = 0.25;
    liquid_inside_pipe = 0.0;

    h=0.0;
    crack_width=0.0;

    geometryUpdate(0);
    materialUpdate(0);

    conditionToTemperature();

}
//  Updates the geometry for the pipe
void Parameters::geometryUpdate(int n)
{
    pipeid = Parameters::pipeid_lib[n];
    diameter = diameter_lib[n];
    sdr = sdr_lib[n];
    notch_depth = 0.0;
    diameter_creep_ratio = 1.0;

}

//  Update the material for the pipe
void Parameters::materialUpdate(int n)
{
    matid = matid[n];
    density = density_lib[n];
    edyn0degc = edyn0degc_lib[n];
    dedyndt = dedyndt_lib[n];
    creep_modulus = creepmodulus_lib[n];
    poisson = poisson_lib[n];
}

//  Constructor
Parameters::Parameters(ConfigFile config)
{
    config.readInto(outflow_model_on, "outflowModelOn");
    config.readInto(lambda, "lambda");
    config.readInto(single_mode, "Mode");
    config.readInto(range_number, "numberOfSpeedValues");
    config.readInto(elements_in_l, "elementsInL");
    config.readInto(adotc0, "aDotc0");

    config.readInto(pipeid, "pipeID");
    config.readInto(diameter, "diameter");
    config.readInto(sdr, "sdr");
    config.readInto(notch_depth, "notchDepth");
    config.readInto(diameter_creep_ratio, "diameterCreepRatio");

    config.readInto(matid, "matID" );
    config.readInto(density, "density" );
    config.readInto(edyn0degc, "eDyn0degC" );
    config.readInto(dedyndt, "dEdyndT" );
    config.readInto(creep_modulus, "creepModulus" );
    config.readInto(poisson, "poisson" );

    config.readInto(fullscale, "fullScale");
    config.readInto(tempdegc, "tempDegC");
    config.readInto(p0bar, "p0bar");
    config.readInto(is_backfilled, "isBackfilled");
    config.readInto(backfill_depth, "backfillDepth");
    config.readInto(backfill_density, "backfillDensity");
    config.readInto(solid_inside_pipe, "solidInsidePipe");
    config.readInto(liquid_inside_pipe, "liquidInsidePipe");
}

//  Creates the "=" operator for this class
Parameters& Parameters::operator=(const Parameters& rhs)
{
    outflow_model_on = rhs.outflow_model_on;
    lambda = rhs.lambda;
    single_mode = rhs.single_mode;
    range_number = rhs.range_number;
    elements_in_l = rhs.elements_in_l;
    adotc0 =  rhs.adotc0;
    from = rhs.from;
    to = rhs.to;
    varname = rhs.varname;
    verbose = rhs.verbose;

    pipeid = rhs.pipeid;
    diameter = rhs.diameter;
    sdr = rhs.sdr;
    notch_depth = rhs.notch_depth;
    diameter_creep_ratio = rhs.diameter_creep_ratio;
    h = rhs.h;
    crack_width = rhs.crack_width;

    matid = rhs.matid;
    density = rhs.density;
    edyn0degc = rhs.edyn0degc;
    dedyndt = rhs.dedyndt;
    dynamic_modulus = rhs.dynamic_modulus;
    creep_modulus = rhs.creep_modulus;
    poisson = rhs.poisson;

    methodid = rhs.methodid;
    fullscale = rhs.fullscale;
    tempdegc = rhs.tempdegc;
    p0bar = rhs.p0bar;
    is_backfilled = rhs.is_backfilled;
    backfill_depth = rhs.backfill_depth;
    backfill_density = rhs.backfill_density;
    solid_inside_pipe = rhs.solid_inside_pipe;
    liquid_inside_pipe = rhs.liquid_inside_pipe;

    return *this;
}

//  Modifies parameters to account for temperature
void Parameters::conditionToTemperature()
{
    dynamic_modulus = edyn0degc + tempdegc * dedyndt;
} 

