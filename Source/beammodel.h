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

//  Energy-based fracture mechanics uses properties of the COD profile
//  to evaluate the crack driving force.

#ifndef BeamModelH
#define BeamModelH

#include <vector>

#include "fdprofile.h"
#include "Backfill.h"
#include "liquidcontent.h"
#include "Creep.h"

class BeamModel
{
public:	
    FDprofile beam_profile;

    double alpha[2];
    double baffle_leakage_area;
    double deltadstar;
    double m[2];
    short max_iterations;
    double zetaclosure;
    double lambda_last;
    short iterations;
    double gs1_ic;
    double gs1;
    double gs1_rsid;
    double gs1_liqd;
    double gue2;
    double gs2;
    double gk2_bf;
    double gk2;
    double g_total;
    double gg0;

    double adotovercl;
    double adotovercl_backfilled;
    double adotclfactor;
    double adotclfactor_backfilled;
    double dynamic_shear_modulus;
    double lambda_error;
    double closure_error;
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

    double vStarRes;            //  FIXME

    double w_2;                 //  Crack opening displacement at outflow point
    double w_max;               //  Absolute maximum crack opening displacement
    double wdash_2;             //  dw/dz at outflow point
    double wdash_max;           //  dw/dz at which any backfill is ejected
    double w2dash_2;            //  d2w/dz2 at outflow point
    double w_integral_12;       //  Crack opening area within outflow length
    double w_residual;          //

    double z_outflow;
    double z_backfilled;
    double zeta_backfill_ejection;

    int l;

    short node_at_closure;
    short lambda_is_converged;
    short closure_is_converged;

    vector<double> crack_displacement;
    vector<double> zeta;

//  Null constructor
    BeamModel();

//  Construct dynamic beam-on-elastic-foundation model of pipe system
    BeamModel(const Parameters parameters);

//  Clears all values within beammodel
    void initialise();

//  Speed dependent properties and reset for outflow length
    void reset(const Parameters parameters,
               const Backfill backfill,
               const LiquidContent liquidcontent,
               const Creep creep);

//  Calculate dimensionless foundation stiffness with backfill m[0], without [1]
    void stiffness();

//  Calculate dimensionless crack speed alpha[0] with backfill, [1] without
    void crackSpeed(const Parameters parameters);

//  Implement boundary conditions on the beam model and control its solution
//  by iteration, using a gas discharge model to determine the outflow length
    void SolveForCrackProfile(const Parameters parameters,
                              const Creep creep);

    void crackDrivingForce(Parameters parameters,
                           Backfill backfill,
                           LiquidContent liquidcontent,
                           Creep creep);
private:
    double radius;
};

#endif
