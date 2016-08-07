//  aRCPlan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

//  Class representing the "guillotine" model for decompression of a pipe
//  containing a perfect gas at initial gauge pressure p0.
//  The crack extends axially at a constant fraction aDotc0 of sonic velocity,
//  and is assumed to discharge freely to atmosphere at its initiation point.
//  The gauge pressure p1 at the crack tip then depends only on p0 and aDotc0.

#include <cmath>

#include "Constants.h"
#include "Decompression.h"

Decomp::Decomp()
{   //  Null constructor
    p0  =  0.0;
    p1 = 0.0;
    p1p0r = 0;
    gamma_ = 0.0;
    aDotc0 = 0.0;
    argument = 0.0;
}   //  end null constructor

void Decomp::p1p0(double p0, double gamma_, double aDotc0)
{   //  Constructor
//  Simple handling to avoid errors
    if(aDotc0 < 0.0)
        p1p0r = 0.0;
    else
        if(aDotc0 > 1.0)
            p1p0r = 1.0;
        else
        {
            argument = 1.0 - (gamma_ - 1.0) / (gamma_ + 1.0) * (1.0 - aDotc0);
            p1 = (p0 + Constants::pAtm)
                        * pow(argument, 2.0 * gamma_ / (gamma_ - 1.0))
                 - Constants::pAtm;
            if (p1 < 0.0)
                p1 = 0.0;
        p1p0r = p1 / p0;
    }
}   //  end constructor
