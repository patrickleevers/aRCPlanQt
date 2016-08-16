//  aRCPlan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

//  The effect on RCP of an incompressible backfill, surrounding the pipe as an
//  axisymmetrical sleeve, is represented only by the radially-moving mass
//  which it attaches to pipe wall.
//  The effect is expressed through an "effective" density added to the
//  real density of the pipe wall -- as long as the backfill remains attached.

#ifndef _BACKFILL_H
#define _BACKFILL_H

#include "Parameters.h"
#include "Backfill.h"

class Backfill
{
public:	
    //  Null Constructor
	Backfill();

    //  Constructor
    //  Calculates effective density
	Backfill(const Parameters parameters);

    double effective_density;   //  Added to pipe density if backfill attached
    double ke_factor;           //  Backfill KE / unit length / dwdt^2


private:
//    double density;
//    double sleeve_thickness;
};
#endif
