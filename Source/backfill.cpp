//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

#include "cmath"
#include "Backfill.h"
#include "constants.h"

//  Null constructor
Backfill::Backfill()
{
    ke_factor = 0.0;
    effective_density = 0.0;
}

//  Constructor taking parameters as arguments.
//  Computes an effective density for backfill, to be added to that of pipe
Backfill::Backfill(const Parameters parameters)
{
    ke_factor = 0.0;    //  KE / unit
    effective_density = 0.0;
    if (parameters.is_backfilled == 2)
    {
        ke_factor = parameters.backfill_density / Constants::pi
            * log(1.0 + 2.0 * parameters.backfill_depth / parameters.diameter);
        effective_density = ke_factor / Constants::c2
                * pow(parameters.sdr, 2) / (parameters.sdr - 1.0);
        ke_factor *= pow(parameters.diameter, 2) / 4.0e6;
    }
}   //  end constructor
