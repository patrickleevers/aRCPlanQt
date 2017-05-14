//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model,
// see http://www.sciencedirect.com/science/article/pii/S0013794412003530

// Class providing function to iterate through independent variable

#include <iostream>
using namespace std;

#include "constants.h"
#include "simulation.h"
#include "guimain.h"
#include "ui_guimain.h"

//  Null constructor
Simulation::Simulation()
{
    i = 0;
    adotc0 = 0.0;
}

//  Taking parameters as arguments, peforms the calculation for a given range of
//  the independent variable eventually produced a solution
Solution Simulation::run(Parameters parameters)
{
//  Quick calculations with values read from the GUI
    parameters.h = parameters.diameter/parameters.sdr/Constants::kilo; // (m)

    parameters.crack_width = parameters.diameter / parameters.sdr
            - parameters.notch_depth;
                        // mm, giving kJ/m2 for G; not necessarily equal to h

    extern Solution solution;
    extern File file;

    file.logPrepare(parameters);

    //  Compute the effective density of a pipe wall with 'attached' backfill…
    Backfill backfill(parameters);
    file.collect(backfill);
    //  …and/or contained liquid
    LiquidContent liquidcontent(parameters);
    file.collect(liquidcontent);

    //	Preliminary calculations
    BeamModel beamModel(parameters);

    //  Clear the solution from any previous runs
    solution.clear();
    solution.displacement(parameters);

    //  Proceed to vary crack speed
    for (i = 0; i < parameters.range_number; i++)
    {

        qDebug() << "Test 1" << i;

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

        qDebug() << "Test 2" << i;

        //  Speed dependent properties
        beamModel.reset(parameters,
                        backfill,
                        liquidcontent,
                        creep);

        qDebug() << "Test 3" << i;

        // Determine crack opening displacement profile
        beamModel.SolveForCrackProfile(parameters,
                                       creep);

        qDebug() << "Test 4" << i;

        beamModel.crackDrivingForce(parameters,
                                    backfill,
                                    liquidcontent,
                                    creep);

        //  Pass values to solution for collection from there
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
                            beamModel.gue2,
                            beamModel.gs2,
                            beamModel.gk2,
                            beamModel.gs1_ic,
                            beamModel.gg0,
                            beamModel.g_total,
                            beamModel.w_integral_12,
                            beamModel.lambda_is_converged,
                            beamModel.closure_is_converged,
                            beamModel.iterations);

        //  Clear log values following end of simulation
        file.initialise();
    }
    return solution;
}
void collect(const double adotc0s,
             const double p0bars,
             const double tempdegcs,
             const double decompressions,
             const double alphas,
             const double ms,
             const double lambdas,
             const double deltadstars,
             const double gs1s,
             const double gues,
             const double gsbs,
             const double gkbs,
             const double g0s,
             const double gg0s,
             const double g_totals,
             const short w_integral_12,
             const short lambda_is_converged,
             const short closure_is_converged,
             const short iterations);
