//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

#ifndef _SIMULATION_H
#define _SIMULATION_H

#include "configfile.h"
#include "parameters.h"
#include "file.h"
#include "creep.h"
#include "backfill.h"
#include "beammodel.h"
#include "qcustomplot.h"
#include "guimain.h"
#include "ui_guimain.h"

class Simulation
{

signals:

private:
	
    int i;
    double adotc0;
	
public:	
	
    //  Null constructor
    Simulation();

    //  Iteration function, parameters as arguments, returns solution
    Solution run(Parameters parameters);

};

#endif
