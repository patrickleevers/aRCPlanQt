//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

#ifndef _CREEP_H
#define _CREEP_H

#include "Parameters.h"
class Creep
{
	
private:

    double creep_modulus_ratio;

public:

    double diameter_res0;
    double residual_crack_closure;

    //  Null constructor
	Creep();

    //  Constructor
    //  Calculates natural diameter of pipe
    //  and crack closure due to residual strain contraction
	Creep(const Parameters parameters);

};

#endif
