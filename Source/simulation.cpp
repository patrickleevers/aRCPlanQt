//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model,
// see http://www.sciencedirect.com/science/article/pii/S0013794412003530

// Class providing function to iterate through independent variable

#include <iostream>
using namespace std;

#include "Constants.h"
#include "Simulation.h"
#include "guimain.h"
#include "ui_guimain.h"

//  Null constructor
Simulation::Simulation()
{
	i=0;
    adotc0=0.0;
}

//  Taking parameters as arguments, peforms the calculation for a given range of
//  the independent variable eventually produced a solution
Solution Simulation::run(Parameters parameters)
{
//  Quick calculations with values read from the GUI
    parameters.h = parameters.diameter/parameters.sdr/Constants::kilo; // (m)
//    parameters.hoverr = 2.0/ (parameters.sdr-1);
//    parameters.radius = parameters.h / parameters.hoverr;

    parameters.crack_width = parameters.diameter / parameters.sdr
            - parameters.notch_depth;
                        // mm, giving kJ/m2 for G; not necessarily equal to h

    extern Solution solution;
    extern File file;

    file.logPrepare(parameters);

    //  Compute the effective density of a pipe wall with 'attached' backfill…
    Backfill backfill(parameters);
    file.collect(backfill);
    //  …and/or contained water
    WaterContent watercontent(parameters);
    file.collect(watercontent);

    //	Preliminary calculations
    BeamModel beamModel(parameters);

    //  Clear the solution from any previous runs
    solution.clear();
    solution.displacement(parameters);

    //  Proceed to vary crack speed
    for (i = 0; i < parameters.range_number; i++)
    {
        //  Method for iterating independent variable
        switch(parameters.varname)
        {
            case 0:
                parameters.adotc0 = parameters.from
                        + ((i+1) * ((parameters.to - parameters.from)
                                    / parameters.range_number));
                break;
            case 1:
                parameters.p0bar = parameters.from
                        + ((i+1) * ((parameters.to - parameters.from)
                                    / parameters.range_number));
                break;
            case 2:
                parameters.tempdegc = parameters.from
                        + ((i+1) * ((parameters.to - parameters.from)
                                    / parameters.range_number));
                break;
            default:
                break;
        }

        //  Temperature dependent calculations
        parameters.dynamic_modulus = parameters.edyn0degc
                + parameters.tempdegc * parameters.dedyndt;

        //	Calculate natural diameter of pipe
        //  due to residual strain contraction in time scale of fracture
        Creep creep(parameters);
        file.collect(creep);

        //  Speed dependent properties
        beamModel.reset(parameters,
                        backfill,
                        watercontent,
                        creep);

        // Determine crack opening displacement profile
        beamModel.SolveForCrackProfile(parameters,
                                       creep);

        beamModel.crackDrivingForce(parameters,
                                    backfill,
                                    creep);

        //Pass values to solution to collect them
        //  TODO: Arguments passed individually
        //  to avoid circular header reference if arguments passed by object
        solution.collect(parameters.adotc0,
                            parameters.p0bar,
                            parameters.tempdegc,
                            beamModel.p1p0r,
                            beamModel.alpha[1],
                            beamModel.m[0],
                            beamModel.lambda,
                            beamModel.deltadstar,
                            beamModel.gs1,
                            beamModel.gue,
                            beamModel.gsb,
                            beamModel.gkb,
                            beamModel.g0,
                            beamModel.gg0,
                            beamModel.gtotal,
                            beamModel.no_crack_opening,
                            beamModel.lambda_is_converged,
                            beamModel.closure_is_converged,
                            beamModel.iterations);

        //Clear log values following end of simulation
        file.initialise();
    }
    return solution;
}
