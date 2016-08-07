//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

#include "cmath"
#include "watercontent.h"

//Null constructor
WaterContent::WaterContent()
{
    effective_density=0.0;
}

//  Constructor taking parameters as arguments
//  Computes an effective density ratio for contained water,
//  to be added to that of pipe
WaterContent::WaterContent(const Parameters parameters)
{
    effective_density = 0.5 * parameters.water_inside_pipe
                                * (parameters.sdr - 2.0) * 1000;
}   //  end constructor

