//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

#include "cmath"
#include "Backfill.h"

//Null constructor
Backfill::Backfill()
{
    effective_density=0.0;
}

//  Constructor taking parameters as arguments.
//  Computes an effective density ratio for backfill,
//  to be added to that of pipe
Backfill::Backfill(const Parameters parameters)
{
    effective_density = (2.0 * parameters.diameter + parameters.backfill_depth)
                / 4.0 / parameters.diameter / (1.0 - 1.0 / parameters.sdr)
                * parameters.backfill_density;
}   //  end constructor

