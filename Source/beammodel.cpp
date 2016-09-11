//  aRCPLan
//  Copyright (c) [2016] [Fraser Edwards][Dr Patrick Leevers]
//  aRCPlan may be freely distributed under the MIT license.

#include <cmath>
#include <iostream>

#include "dialog.h"
#include "Constants.h"
#include "backfill.h"
#include "liquidcontent.h"
#include "BeamModel.h"
#include "Parameters.h"
#include "OutflowProcess.h"
#include "Decompression.h"
#include "FDprofile.h"
#include "Solution.h"
#include "File.h"


BeamModel::BeamModel()
//  Null constructor
{
    initialise();
}

BeamModel::BeamModel(const Parameters parameters)
//  Constructor
//  Calculates properties which don't depend on crack speed
{
    initialise();
    extern File file;

    //  Proportion of internal volume available for expansion
    pipe_volume_availability = 1.0 - 0.01 * parameters.liquid_inside_pipe;
    if (not parameters.fullscale)
         pipe_volume_availability -= 0.01 * parameters.solid_inside_pipe;

    //  Parameters for equivalent beam model
    dynamic_shear_modulus = parameters.dynamic_modulus
                                / 2.0 / (1.0 + parameters.poisson);
    sdrminus1 = parameters.sdr - 1.0;
    sdrminus2 = sdrminus1 - 1.0;
    radius = parameters.diameter * sdrminus1 / parameters.sdr / 2.0e3; //  (m)
    beam_width = 0.5 * parameters.diameter / Constants::kilo
                                * sdrminus2 / parameters.sdr;           //  (m)

    //  Effective internal pressure due to residual stress in pipe
    residualpressure = 2.0 / 3.0 * parameters.creep_modulus         // * 10^09
                        / pow(parameters.sdr, 3)
                        * (100.0 - parameters.diameter_creep_ratio) // * 10^-02
                        * 100.0;                                //  Hence bar

    //  Factors for modelling of S4 baffle leak
    baffle_leakage_area = parameters.diameter / Constants::kilo;        //  (m)
    baffle_leakage_area = pow(baffle_leakage_area, 2) * 0.01 * Constants::pi
                * sdrminus2 / parameters.sdr;
    lambda = parameters.lambda;
    file.collect(this,0);
}   //  end BeamModel()


void BeamModel::initialise()
// Clears all class variables
{
    adotclfactor = 0.0;
    adotclfactor_backfilled = 0.0;
    adotovercl = 0.0;
    baffle_leakage_area = 0.0;
    closure_is_converged = 0;
    dynamic_shear_modulus = 0.0;
    iterations = 0;
    l = 0;

    lambda = 0.0;
    lambda_is_converged = 0;
    lambda_factor = 0.0;
    lambda_last = 0.0;

    max_iterations = 0;
    node_at_closure = 0;
    p1bar = 0.0;
    p1p0r = 0.0;
    sdrminus1 = 0.0;
    sdrminus2 = 0.0;
    v0 = 0.0;
    v00 = 0.0;

    w_2 = 0.0;
    wdash_2 = 0.0;
    wdash_max = 0.0;
    w2dash_2 = 0.0;
    w_integral_12 = 0.0;
    w_max = 0.0;

    zeta_backfill_ejection = 0.0;
    zetaclosure = 0.0;

    m[0] = 0.0;
    m[1] = 0.0;
    alpha[0] = 0.0;
    alpha[1] = 0.0;
}   //  end initialise()


void BeamModel::reset(const Parameters parameters,
                      const Backfill backfill,
                      const LiquidContent liquidcontent,
                      const Creep creep)
//  Set model parameters which depend on the independent variable
//  (speed, temperature, pressure)
//  and reset certain values at the beginning of each simulation
{
    extern File file;

//  Estimate initial positions of outflow, backfill ejection and reclosure
    lambda = parameters.lambda;
    zeta_backfill_ejection = 0.2;   // first guess
    zetaclosure = 2.0;
    node_at_closure = short(zetaclosure * parameters.elements_in_l);

//  For FS configuration, determine prior decompression
    file.adotc0 = parameters.adotc0;
    p1bar = parameters.p0bar;
    Decomp decomp;
//  Proportion of initial pressure which remains at crack tip
    decomp.p1p0r = 1.0;
    if (parameters.fullscale == 2)  //  (checkboxes are assigned 2 when ticked)
    {   // New p1, could replace with p1 from decomp object, need to trace
        decomp.p1p0(parameters.p0bar, Constants::gamma, parameters.adotc0);
        p1bar = parameters.p0bar * decomp.p1p0r;
    }

    p1p0r = decomp.p1p0r; //TODO: remove throughout, so only decomp.p1p0r used
    //  v00 becomes reference length v0 on multiplying by lambda^4:
    v00 = 4.0 / Constants::c1
            * sdrminus1 * sdrminus2 / parameters.sdr
            * p1bar                                         //  * 10^05
            / parameters.dynamic_modulus                    //  / 10^09
            * parameters.diameter                           //  * 10^-03
            / Constants::mega / 10.0;                       //	hence m

//  Effective dynamical density of wall, lumped with any liquid which it moves
    pipe_effective_density = parameters.density
                                      + liquidcontent.effective_density;

//  Parameters for equivalent beam model (speed dependent)
    adotovercl = parameters.adotc0 * Constants::vSonic
            / sqrt(parameters.dynamic_modulus * Constants::giga);
    adotovercl_backfilled = adotovercl
            * sqrt(pipe_effective_density + backfill.effective_density);
    adotovercl = adotovercl
            * sqrt(pipe_effective_density);
    adotclfactor = 1.0 + pow(adotovercl, 2);
    adotclfactor_backfilled = 1.0 + pow(adotovercl_backfilled, 2);
    lambda_factor = Constants::pi * Constants::c1
                        * 625.0                 // NB GPa / bar / 16 = 625
                        * parameters.dynamic_modulus / p1bar
                        * pipe_volume_availability
                        * sdrminus2 / sdrminus1 / sdrminus1
                        * parameters.adotc0;
    file.collect(this, 0);
}   //  end reset()


void BeamModel::stiffness()
{   //	Dimensionless beam foundation stiffness [0] with or [1] without backfill
    m[1] = 8.0 / 3.0 / Constants::pi / Constants::c1
            * pow(lambda, 4) / pow(sdrminus1, 2);
    m[0] = m[1] / adotclfactor_backfilled;
    m[1] /= adotclfactor;
}   //  end stiffness()


void BeamModel::crackSpeed(const Parameters parameters)
{   //	Dimensionless crack speed [0] with and [1] without backfill
    alpha[1] = lambda *
                sqrt(
                        (Constants::c3 * pow(adotovercl, 2)
                            + Constants::c4 / 2.0 / (1.0 + parameters.poisson)
                                    / pow(sdrminus1, 2)
                        ) / adotclfactor
                    );
    alpha[0] = lambda *
                sqrt(
                        (Constants::c3 * pow(adotovercl_backfilled, 2)
                            + Constants::c4 / 2.0 / (1.0 + parameters.poisson)
                                / pow(sdrminus1, 2)
                        ) / adotclfactor_backfilled
                    );
}   //  end crackSpeed()


void BeamModel::SolveForCrackProfile(const Parameters parameters,
                                     const Creep creep)
//  Solves for beam model opening profile and fluid outflow length under the
//  specified boundary conditions, using a finite difference method and a fluid
//  outflow model.
//
//  The first loop evaluates the beam opening profile by successively adjusting
//  the outflow length (i.e. the length of pressure-loaded beam) according to
//  the outflow model. It is assumed that the beam's foundation stiffness
//  closes the crack at twice this length.
//
//  The second loop refines each profile solution by moving this closure point
//  towards the position at which the local bending moment is minimised.
{
    extern File file;
    extern Solution solution;
    max_iterations = 20;
    iterations = 0;
    double tolerance = 0.04;

//  GUI assigns checked checkbox = 2
    if ((parameters.outflow_model_on == 2) & (parameters.verbose == 2))
    {
        dialog *e = new dialog;
        e->warning("Starting outflowLength refinement with outflow length = ",
                   parameters.lambda);
        e->exec();
    }

//  Dimensionless foundation stiffness m
    stiffness();
//	Dimensionless crack speed alpha
    crackSpeed(parameters);

//    lambda_last = lambda;   //  Specified starting value
    do
    {
        lambda_is_converged = 0;
        closure_is_converged = 0;
        if (parameters.outflow_model_on==2)
            do  //  Refine outflow length using adiabatic discharge model:
            {
                iterations++;
                lambda_last = lambda;   //  using value about to be updated
                double dontNeedThis;
                //  Dimensionless foundation stiffness m
                stiffness();
                //	Dimensionless crack speed
                crackSpeed(parameters);
                file.collect(this, 0);

                beam_profile = FDprofile(alpha,
                                        m,
                                        zeta_backfill_ejection,
                                        residualpressure,
                                        parameters.elements_in_l,
                                        node_at_closure);
                closure_error = beam_profile.ClosureMoment();
                beam_profile.GetBackfillEjectPoint(zeta_backfill_ejection,
                                                  dontNeedThis);
                //  For subsequent refinement by moving closure point, if reqd:
                w_integral_12 = beam_profile.IntegralVStarDZeta_12();
                if ((w_integral_12 > 0.0)
                        and (parameters.outflow_model_on == 2))
                {
                    OutflowProcess outflow(p1bar);
                    lambda = pow(lambda_factor
                                * outflow.get_tStarOutflow()
                                 / w_integral_12, 0.2);
                    lambda = sqrt(lambda * lambda_last);
                    //	tStarOutflow: no. of characteristic times for discharge

                    if (parameters.verbose == 2)
                    {
                        dialog *e = new dialog;
                        e->warning("alpha = ", alpha[1],
                            "m = ", m[1],
                            "integral_w12 = ", w_integral_12,
                            "New outflowLength = ", lambda);
                        e->exec();
                    }
               }
                lambda_error = fabs(1.0 - lambda_last / lambda);
                lambda_is_converged = (lambda_error < tolerance);
            //  Continue to refine outflow length if required
            } while ((iterations < max_iterations) && !lambda_is_converged);
        //  Update m[] and alpha[] for this lambda:
        stiffness();
        crackSpeed(parameters);
        file.collect(this, 0);

//  For this new outflow length move the closure point, by whole elements,
//  towards the point at which the beam bending moment is minimised while
//  there is no crack surface contact between that point and the crack tip.
        short node_closure_move = 1;    //  Default
        double corrector;
        iterations = 0;
        error_code = 0;
        if (parameters.outflow_model_on == 2 && lambda_is_converged)
            do
            {
                iterations++;
                //  If COD is now <0 anywhere before official closure point,
                //  back up closure point until it isn't:
                short move_temp = 0;        //  for the record
                while (beam_profile.ContainsContactPoint() > -1)
                {
                    node_at_closure -= 1;
                    move_temp -= 1;     //  FIXME: use this?
                    beam_profile = FDprofile(alpha,
                                         m,
                                         zeta_backfill_ejection,
                                         residualpressure,
                                         parameters.elements_in_l,
                                         node_at_closure);
                    closure_error = beam_profile.ClosureMoment();
                }
                error_last = closure_error;

                //  Recalculate profile after doing planned closure-node move
                node_at_closure += node_closure_move;
                beam_profile = FDprofile(alpha,
                                     m,
                                     zeta_backfill_ejection,
                                     residualpressure,
                                     parameters.elements_in_l,
                                     node_at_closure);
                closure_error = beam_profile.ClosureMoment();
                //  Next corrector (in node intervals), using relaxn factor 1/4
                corrector = -0.25 * float(node_closure_move)
                                        / (1.0 - error_last / closure_error);
                //  If last two nodes tested straddle the correct one:
                if ((closure_error * error_last < 0.0)
                        && (abs(node_closure_move) == 1))
                {
                    closure_is_converged = 1;
                        if (fabs(closure_error) > fabs(error_last))
                            node_closure_move *= -1;    //  back to other one.
                }
                else    //  Use corrector to prepare next closure node move
                {
                    node_closure_move = 1;
                    if (corrector < 0.0)
                         node_closure_move = -1;
                    corrector = abs(corrector);
                    if (corrector > 1.5)
                        node_closure_move *= corrector;
                }
                //  Abort if move would take closure point somewhere silly
                if (node_closure_move + node_at_closure
                                        > 3 * parameters.elements_in_l)
                    error_code = 1;
                if (node_closure_move + node_at_closure
                                        < parameters.elements_in_l)
                    error_code = 2;
                 file.collect(this, 0);

                if (parameters.verbose==2)
                {
                    dialog *e = new dialog;
                    e->warning("At closure length iteration ",iterations,
                           " closure moment = ", closure_error);
                    e->exec();
                }
            }
        while (!closure_is_converged
                    and (iterations < max_iterations)
                    and error_code == 0);
        //  But movement of closure point will change outflow throat area,
        //  so continue refinement:
        w_integral_12 = beam_profile.IntegralVStarDZeta_12();
        if (w_integral_12 > 0.0)
        {
            OutflowProcess outflow(p1bar);
            lambda = pow(lambda_factor
                        * outflow.get_tStarOutflow() / w_integral_12, 0.2);}
    } while (fabs(1.0 - lambda_last / lambda) > tolerance
             and (parameters.outflow_model_on == 2)
             and error_code == 0);

    //  COD profile vStar(zeta) is now known, and can be output if needed
    if (parameters.verbose==2)
    {
        dialog *e = new dialog;
        e->warning("Final outflowLength convergence in ", iterations,
                       " iterations for outflowLength = ", lambda);
        e->exec();
    }

    //  Output the numerical solution, in mm versus diameters
    //  FIXME: to implement x axis, add x arrays to vptra, and replace 1.0 by
    //  lambda * parameters.sdr / sdrminus1
    beam_profile.ShowCODProfile(1.0, v0 * 1.0e3);
    beam_profile.GetBackfillEjectPoint(zeta_backfill_ejection,
                                       wdash_max);

    solution.collectProfile(beam_profile.vptra,
                            beam_profile.NodeAtClosure()+2);

    //  Flaring of pipe wall at decompression point:
    deltadstar = w_2 / Constants::pi / parameters.diameter * Constants::kilo
                    + creep.diameter_res0 / parameters.diameter - 1.0;

    if (iterations >= max_iterations)  cout << "UNCONVERGED";
}   //  end SolveForCrackProfile()


void BeamModel::crackDrivingForce(Parameters parameters,
                                  Backfill backfill,
                                  LiquidContent liquidcontent,
                                  Creep creep)
{
    extern File file;
    extern Solution solution;
    //  Extract from FD solution, and denormalise, key COD profile parameters.
    v0 = v00 * pow(lambda, 4);                      //	(m)
    z_outflow = lambda * parameters.diameter / Constants::kilo
                    * sdrminus2 / parameters.sdr;   //  (m)
    //  Plane 2 COD increased to virtual COD to represent residual strain
    w_2 = beam_profile.VStar_2() * v0
                + Constants::pi
                        * (parameters.diameter - creep.diameter_res0) / 1.0e3;
    w_max = beam_profile.VStarMax() * v0;
    wdash_2 = beam_profile.DVStarDZeta_2() * v0 / z_outflow;
    w2dash_2 = beam_profile.D2VStarDZeta2_2() * v0 / pow(z_outflow, 2);
    w_integral_12 = beam_profile.IntegralVStarDZeta_12() * v0 * z_outflow;

    beam_profile.GetBackfillEjectPoint(z_backfilled, wdash_max);
    z_backfilled *= z_outflow;
    wdash_max *= v0 / z_outflow;

    //  Maximum diametral flaring of pipe wall:
    deltadstar = w_max / Constants::pi / parameters.diameter * Constants::kilo
                + creep.diameter_res0 / parameters.diameter - 1.0;

    //  Compute signed contributions to strain energy release rate G.
    //  Irwin-Corten GIC, due to direct strain energy input @ crack tip plane 1:
    gs1_ic = Constants::pi / 8.0
                * pow(p1bar * sdrminus2, 2)         //  * 1e10
                / parameters.sdr * sdrminus1
                / parameters.dynamic_modulus        //  / 1e9
                * parameters.diameter               //  * 1e-3
                * 1.0e-5;                           //  hence kJ/m^2

    //	Strain energy input due to residual strain in pipe wall at plane 1:
    gs1_rsid = (parameters.diameter - creep.diameter_res0)
                                    / sdrminus1;    //  * 1e-3
    gs1_rsid = Constants::pi / 6.0
                * parameters.dynamic_modulus        //  * 1e9
                * pow(gs1_rsid, 2)                  //  * 1e-6
                / parameters.crack_width            //  / 1e-3
                * 1.0e3;                            //  hence kJ/m^2

    //	Strain energy input due to strain energy of liquid content at plane 1:
    gs1_liqd = p1bar * parameters.diameter * sdrminus2 / parameters.sdr;
                                                    //  * 1e2
    gs1_liqd = Constants::pi / 8.0
                * pow(gs1_liqd, 2)                  //  * 1e4
                / liquidcontent.bulk_modulus        //  / 1e9
                * parameters.liquid_inside_pipe     //  * 1e-2
                / parameters.crack_width            //  / 1e-3
                * 1.0e-7;                           //  hence kJ/m^2

    //  External work input to pipe wall flaps (= dUE/da) from planes 1-2:
    gue2 = beam_width
                * p1bar                             //  * 1e5
                * w_2
                / parameters.crack_width            //  / 1e-3
                * 1.0e5;                            //  hence kJ/m^2

    //  Strain energy output at plane 2 due to bending:
    //  (Pipe Model Manual p36, NB h/r = 2.0/ (parameters.sdr-1))
    gs2 = -(
                0.5 * parameters.dynamic_modulus *  //  * 1e9
                (
                    pow(w_2, 2)
                        / pow(sdrminus1, 3) / 3.0 / Constants::pi
                    + pow(w2dash_2, 2)
                        * Constants::c1 * pow(radius, 3)
                        * parameters.h
                )
                +   Constants::pi / 12.0
                        * dynamic_shear_modulus     //  * 1e9
                        * radius
                        * parameters.h
                        * pow(wdash_2 / sdrminus1, 2)
            ) / parameters.crack_width              //  / 1e-3
            * 1.0e9;                                //  hence kJ/m^2

    //  Kinetic energy output at plane 2 with pipe wall and any attached liquid:
    //  (Pipe Model Manual. pg36)
    gk2 = -0.5 * parameters.dynamic_modulus         //  * 1e9
                * pow(adotovercl, 2)
                * radius
                * parameters.h
                *   (
                        Constants::c2
                            * pow(wdash_2, 2)
                    +   Constants::c1
                            * pow(w2dash_2 * radius, 2)
                    )
                / parameters.crack_width            //  / 1e-3
                * 1.0e9;                            //  hence kJ/m^2

    //  Kinetic energy output as free backfill at ejection point:
    gk2_bf = wdash_max * parameters.adotc0 * Constants::vSonic;
                                                    //  = wdot_max
    gk2_bf = -backfill.ke_factor * pow(gk2_bf, 2)
                / parameters.crack_width            //  / 10^-03
                / Constants::mega;                  //  hence kJ/m^2

    gs1 = gs1_ic + gs1_liqd + gs1_rsid;
    g_total = gs1 + gs2 + gk2 + gk2_bf + gue2;
    if (g_total < 0.0)
        g_total = 0.0;
    if (not(closure_is_converged && lambda_is_converged))
        g_total = 0.0;

    gg0 = g_total / gs1_ic;
    file.collect(this, 0);

}   //  end crackDrivingForce
