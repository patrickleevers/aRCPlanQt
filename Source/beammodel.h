//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.
//  For the underlying model, see
//  http://www.sciencedirect.com/science/article/pii/S0013794412003530

//  Class which represents the problem of an infinitely long pressurised pipe
//  being 'unzipped' by a rapidly propagating axial crack, transforming it to
//  that of a dynamical Beam On Elastic Foundation emerging at constant speed
//  from an encastré support.
//  The pipe is loaded by:
//  (a) a pressure distribution, stationary with respect to the crack tip,
//      which decreases linearly from p1 at the tip to zero over an
//      axial distance L (the outflow length); and
//  (b) a uniform fictional pressure p_residual representing residual stress.
//
//  w(z) represents the crack opening displacement at a distance z from the tip.
//
//  To obtain the physical crack opening profile w(z) from tip to re-closure,
//  coordinates are transformed to
//      v* = v/v0, where v0 is a function of L, p1 and pipe properties, and
//      zeta = z/L,
//  and the solution is obtained using an external finite difference method.

//  Energy-based fracture mechanics uses properties of the COD profile to evaluate
//  the crack driving force.

#ifndef BeamModelH
#define BeamModelH

#include <vector>

#include "fdprofile.h"
#include "Backfill.h"
#include "watercontent.h"
#include "Creep.h"

class BeamModel
{
public:	
    double alpha[2];
    double baffle_leakage_area;
    double deltadstar;
    double m[2];
    short max_iterations;
    short print_opening_profile;
    double zetaclosure;
    double lambda_last;
    short iterations;
    short no_crack_opening;
    double gs1_ic, gs1_resid, gue2, gs2, gk2_bf, gk2,  g_total;
//  FIXME Bodge temp variables
    double gs1, gue, gsb, gkb, g0, gg0, gtotal;

    double adotovercl;
    double adotovercl_backfilled;
    double adotclfactor;
    double adotclfactor_backfilled;
    double dynamic_shear_modulus;
    double error;
    double error_last;
    double lambda;              //  Ratio of outflow length to mean diameter
    double lambda_factor;
    double p1bar;
    double p1p0r;
    double pipe_effective_density;
    double pipe_volume_availability;
    double residualpressure;
    double sdrminus1;
    double sdrminus2;
    double v00;
    double v0;

    double vStarRes;    //  FIXME

    double w_2;             //  Crack opening displacement at outflow point
    double w_max;           //  Absolute maximum crack opening displacement
    double wdash_2;         //  dw/dz at outflow point
    double wdash_max;       //  dw/dz at which any backfill is ejected
    double w2dash_2;        //  d2w/dz2 at outflow point
    double w_integral_12;   //  Total crack opening area in outflow length
    double w_residual;      //

    double z_outflow;

    double z_backfilled;
    double zeta_backfill_ejection;

    short node_at_closure;
    short lambda_is_converged;
    short closure_is_converged;

    vector<double> crack_displacement;
    vector<double> zeta;


//  Null constructor
    BeamModel();

//  Constructs dynamic beam-on-elastic-foundation model of a
//  specific pipe RCP problem
    BeamModel(const Parameters parameters);

//  Clears all values within beammodel
    void initialise();

//  Speed dependent properties and reset for outflow length
    void reset(const Parameters parameters,
               const Backfill backfill,
               const WaterContent watercontent,
               const Creep creep);

//  Calculate dimensionless foundation stiffness m[0] with backfill, [1] without
    void stiffness();

//  Calculate dimensionless crack speed alpha[0] with backfill, [1] without
    void crackSpeed(const Parameters parameters);

//  Implements boundary conditions on the beam model and controls its solution
//  by iteration, using a gas discharge model to determine the outflow length
    void SolveForCrackProfile(Parameters parameters,
                              Creep creep);

//  Finds final crack profile, and extracts converted physical parameters
//  needed to evaluate crack driving force
    void opening(Parameters parameters,
                 Creep creep);

//
    void crackDrivingForce(FDprofile final,
                           Parameters parameters,
                           Creep creep);
private:
    double radius;
};

#endif
