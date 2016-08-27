//  aRCPlan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

//  Class modelling the effect on RCP along a pipeline
//  of liquid contained in the pipe.

//  The liquid is modelled as an inertial mass, attached to the wall and
//  moving radially with it as crack opening allows the pipe to flare.
//  The effect is expressed via an "effective" liquid density added to
//  the real density of the pipe wall.

#ifndef _LIQUIDCONTENT_H
#define _LIQUIDCONTENT_H

#include "Parameters.h"
#include "liquidcontent.h"
#include "Constants.h"

class LiquidContent
{
public:	
    //  Null Constructor
    LiquidContent();

    //  Constructor
    //  Calculates effective density
    LiquidContent(const Parameters parameters);
    //  Constructor taking parameters as arguments
    //  Using the 'gas sector' model, represents the liquid as an
    //  effective density ratio lumped at the wall, to be added to that of pipe.

    double density;
    double effective_density;
    double bulk_modulus;

private:
};
#endif
